package org.scourge.io;

import org.apache.commons.net.io.Util;
import org.scourge.Climate;

import java.io.*;
import java.util.logging.Logger;
import java.util.zip.GZIPInputStream;
import java.util.zip.ZipFile;
import java.util.zip.ZipInputStream;

/**
 * User: gabor
 * Date: Mar 31, 2010
 * Time: 10:11:37 AM
 */
public class MapIO {
    private RandomAccessFile mapFile;
    private int version, rows, cols;
    private long headerLength;
    private static Logger logger = Logger.getLogger(MapIO.class.toString());

    public MapIO() throws IOException {
        // first decompress the map file
        File file = new File("./data/maps/land.bin");
        if(!file.exists()) {
            logger.info("Inflating map file...");
            GZIPInputStream from = new GZIPInputStream(new FileInputStream("./data/maps/land.bin.gz"));
            FileOutputStream to = new FileOutputStream(file);
            Util.copyStream(from, to);
            from.close();
            to.close();
        }

        this.mapFile = new RandomAccessFile(file, "r");
        this.version = mapFile.readInt();
        this.rows = mapFile.readInt();
        this.cols = mapFile.readInt();
        this.headerLength = mapFile.getFilePointer();
        logger.info("Loading map: rows=" + rows + " cols=" + cols + " headerLength=" + headerLength);
    }

    public RegionPoint[][] readRegion(int x, int y, int w, int h) throws IOException {
        RegionPoint[][] points = new RegionPoint[w][h];
        for(int yy = y; yy < y + h; yy++) {
            for(int xx = x; xx < x + w; xx++) {
                mapFile.seek(headerLength + ((long)yy * cols * 4L) + ((long)xx * 4L));
                points[yy - y][xx - x] = new RegionPoint(mapFile.readInt());
            }
        }
        return points;
    }

    public class RegionPoint {
        private char c;
        private Climate climate;

        public RegionPoint(int value) {
            c = (char)(value & 0xff);
            climate = Climate.values()[(value & 0xff00) >> 8];
        }

        public char getC() {
            return c;
        }

        public void setC(char c) {
            this.c = c;
        }

        public void setClimate(Climate climate) {
            this.climate = climate;
        }

        public Climate getClimate() {
            return climate;
        }
    }

    public int getVersion() {
        return version;
    }

    public int getRows() {
        return rows;
    }

    public int getCols() {
        return cols;
    }
}
