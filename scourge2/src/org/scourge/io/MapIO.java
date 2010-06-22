package org.scourge.io;

import org.apache.commons.io.FileUtils;
import org.apache.commons.net.io.Util;
import org.scourge.Climate;
import org.scourge.model.Session;
import org.simpleframework.xml.Serializer;
import org.simpleframework.xml.core.Persister;

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
    public static final File GZIP_FILE = new File("./data/maps/land.bin.gz");

    public MapIO() throws IOException {
        // first decompress the map file
        File file = new File("./data/maps/land.bin");
        if(!file.exists()) {
            logger.info("Inflating map file...");
            GZIPInputStream from = new GZIPInputStream(new FileInputStream(GZIP_FILE));
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

    /**
     * Load the dynamic data for a region.
     * @param x the region's x coordinate. (region coordinates are in REGION_SIZE units)
     * @param y the region's y coordinate
     * @return the RegionData for this region
     * @throws Exception exception
     */
    public static RegionData loadRegionData(int x, int y) throws Exception {
        RegionData regionData;
        File file = getRegionDataFile(x, y);
        if(file.exists()) {
            Serializer serializer = new Persister();
            Reader reader = new BufferedReader(new FileReader(file));
            regionData = serializer.read(RegionData.class, reader);
            reader.close();
        } else {
            regionData = new RegionData(x, y);
        }
        return regionData;
    }

    /**
     * Save the dynamic data for a region.
     * @param x the region's x coordinate. (region coordinates are in REGION_SIZE units)
     * @param y the region's y coordinate
     * @param regionData the RegionData for this region
     * @throws Exception exception
     * @return true if the region has saved data, false otherwise
     */
    public static boolean saveRegionData(int x, int y, RegionData regionData) throws Exception {
        File file = getRegionDataFile(x, y);
        if(regionData.isEmpty()) {
            FileUtils.deleteQuietly(file);
            return false;
        } else {
            Serializer serializer = new Persister();
            Writer writer = new BufferedWriter(new FileWriter(file));
            serializer.write(regionData, writer);
            writer.close();
            return true;
        }
    }
    
    private static File getRegionDataFile(int x, int y) {
        return new File("./data/maps/region_" + x + "_" + y + ".xml");
    }

    public RegionPoint[][] readRegion(int x, int y, int w, int h) throws IOException {
        RegionPoint[][] points = new RegionPoint[w][h];
        byte[] buff = new byte[w * 4];
        for(int yy = y; yy < y + h; yy++) {
            mapFile.seek(headerLength + ((long)yy * cols * 4L) + ((long)x * 4L));
            mapFile.read(buff, 0, w * 4);
            for(int xx = 0; xx < w; xx++) {
                int n = (buff[xx * 4] << 24) + (buff[xx * 4 + 1] << 16) + (buff[xx * 4 + 2] << 8) + (buff[xx * 4 + 3]);
                points[yy - y][xx] = new RegionPoint(n);
            }
        }
        return points;
    }

    public int[][] readRaw() throws IOException {
        mapFile.seek(headerLength);
        int[][] values = new int[rows][cols];
        for(int yy = 0; yy < rows; yy++) {
            byte[] buff = new byte[cols * 4];
            mapFile.read(buff, 0, cols * 4);
            for(int xx = 0; xx < cols; xx++) {
                values[yy][xx] = (buff[xx * 4] << 24) + (buff[xx * 4 + 1] << 16) + (buff[xx * 4 + 2] << 8) + (buff[xx * 4 + 3]);
            }
        }
        return values;
    }

    public class RegionPoint {
        private char c;
        private Climate climate;
        private int level;

        public RegionPoint(int value) {
            c = (char)(value & 0xff);
            climate = Climate.values()[(value & 0xff00) >> 8];
            level = (value & 0xff0000) >> 16;
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

        public int getLevel() {
            return level;
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
