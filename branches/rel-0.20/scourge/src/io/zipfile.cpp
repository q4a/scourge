/***************************************************************************
                          zipfile.h  -  description
                             -------------------
    begin                : Sat Jul 29, 2005
    copyright            : (C) 2005 by Gabor Torok
    email                : cctorok@yahoo.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "zipfile.h"

using namespace std;

//#define DEBUG_ZIP 1

#ifdef DEBUG_ZIP  
FILE *debugfp;
#endif

ZipFile::ZipFile( FILE *fp, int mode, int level ) : File( fp ) {
  this->mode = mode;
  if( mode == ZIP_WRITE ) {

#ifdef DEBUG_ZIP  
    debugfp = fopen( "write.tmp", "wb" );
#endif

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    int ret = deflateInit( &strm, level );
    if( ret != Z_OK ) {
      cerr << "Error in deflateInit: " << ret << endl;
      //return ret;
    }
  } else {

#ifdef DEBUG_ZIP  
    debugfp = fopen( "read.tmp", "wb" );
#endif

    /* allocate inflate state */
    tmpBuffOffset = CHUNK;
    ret = Z_OK;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    strm.avail_out = 0;
    int ret = inflateInit(&strm);
    if( ret != Z_OK ) {
      cerr << "Error in inflateInit: " << ret << endl;
      //return ret;
    }
  }
}

ZipFile::~ZipFile() {
#ifdef DEBUG_ZIP  
  fclose( debugfp );
  cerr << "ZipFile destructor." << endl;
#endif
  if( mode == ZIP_WRITE ) {
#ifdef DEBUG_ZIP  
    cerr << "\tcleaning up after write" << endl;
#endif
    // clean up
    writeZip( Z_FINISH );
    (void)deflateEnd( &strm );
  } else {
#ifdef DEBUG_ZIP  
    cerr << "\tcleaning up after read" << endl;
#endif
    /* clean up and return */
    (void)inflateEnd(&strm);
  }
}
  
int ZipFile::write( void *buff, size_t size, int count ) {
  //return File::write( buff, size, count );
  strm.avail_in = count * size;
  //flush = feof(source) ? Z_FINISH : Z_NO_FLUSH;
  strm.next_in = (Bytef*)buff;

#ifdef DEBUG_ZIP  
  fwrite( buff, size, count, debugfp );
#endif
  
  return writeZip( Z_NO_FLUSH );
}

int ZipFile::writeZip( int flush ) {
  /* run deflate() on input until output buffer not full, finish
  compression if all of source has been read in */
  unsigned have;
  do {
    strm.avail_out = CHUNK;
    strm.next_out = (Bytef*)zipBuff;
    int ret = deflate( &strm, flush );    /* no bad return value */
    if( ret == Z_STREAM_ERROR ) {
      cerr << "Error deflating: " << ret << endl;
      return -1;
    }

    if( flush == Z_FINISH ) {
#ifdef DEBUG_ZIP  
      cerr << "Z_FINISH:" << endl;
      if( ret == Z_OK ) cerr << "\tZ_OK" << endl;
      if( ret == Z_STREAM_END ) cerr << "\tZ_STREAM_END" << endl;
#endif      
    }

    have = CHUNK - strm.avail_out;
    if( File::write( zipBuff, 1, have ) != static_cast<int>(have) ) {
      (void)deflateEnd( &strm );
      return Z_ERRNO;
    }
  } while( strm.avail_out == 0 );
  if( strm.avail_in ) {
    cerr << "Error: not all input used!" << endl;
  }
  return static_cast<int>(have);
}


int ZipFile::read( void *buff, size_t size, int count ) {
  //cerr << "ZipFile::read, size=" << size << " count=" << count << endl;
  
  int bytesRead = 0;
  int bytesWanted = size * count;
  while( bytesWanted > 0 ) {

    // inflate more data as needed
    if( tmpBuffOffset >= static_cast<int>( CHUNK - strm.avail_out ) ) {
      tmpBuffOffset = 0;
      ret = inflateMore();
      if( !( ret == Z_STREAM_END || ret == Z_OK ) ) {
        cerr << "Error while inflating: " << ret << endl;
        return -1;
      }
    }

    // copy out data
    int bytesHave = ( CHUNK - strm.avail_out ) - tmpBuffOffset;
    int willCopy = ( bytesWanted <= bytesHave ? bytesWanted : bytesHave );
    memcpy( (char*)(buff) + bytesRead, tmpBuff + tmpBuffOffset, willCopy );
    bytesWanted -= willCopy;
    tmpBuffOffset += willCopy;
    bytesRead += willCopy;
  }

  return count;
}

int ZipFile::inflateMore() {

#ifdef DEBUG_ZIP  
  cerr << "inflating more: " << endl;
#endif
  
  // Inflate while there is space in the output buffer.
  int ret;
  strm.avail_out = CHUNK;
  strm.next_out = (Bytef*)tmpBuff;
  while( strm.avail_out != 0 ) {
    // need more input
    //if( ret == Z_BUF_ERROR ) {
    if( strm.avail_in == 0 ) {
      strm.avail_in = File::read( zipBuff, 1, CHUNK );
      if (strm.avail_in == 0) {
        cerr << "No data read." << endl;
        return Z_DATA_ERROR;
      }
      strm.next_in = (Bytef*)zipBuff;

#ifdef DEBUG_ZIP  
      cerr << "\tread " << strm.avail_in << " bytes from disk." << endl;
#endif
    }

    ret = inflate( &strm, Z_FULL_FLUSH );

#ifdef DEBUG_ZIP  
    fwrite( tmpBuff, 1, ( CHUNK - strm.avail_out ), debugfp );
    cerr << "ret=" << ret << endl;
    cerr << "\tinflated " << ( CHUNK - strm.avail_out ) << " bytes." << endl;
#endif

    if( ret == Z_STREAM_END ) return ret;

    //assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
    if( ret == Z_STREAM_ERROR ) {
      cerr << "Error inflating: " << ret << endl;
      return ret;
    }
    switch (ret) {
    case Z_NEED_DICT:
    ret = Z_DATA_ERROR;     /* and fall through */
    case Z_DATA_ERROR:
    case Z_MEM_ERROR:
    //(void)inflateEnd(&strm);
      cerr << "Error inflating! " << ret << endl;
      return ret;
    }
  // "have" is the length of the inflated data in tmpBuff
  //have = CHUNK - strm.avail_out;
  //assert(strm.avail_in == 0);     /* all input will be used */
  }
  
  return ret;
}

