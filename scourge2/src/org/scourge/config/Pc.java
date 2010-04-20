package org.scourge.config;

/**
 * User: gabor
 * Date: Apr 19, 2010
 * Time: 8:48:58 PM
 */
public class Pc {
    public enum Sex { male, female }

    public static final String[][] PORTRAIT = new String[][] {
            // male portraits
            { "./data/portraits/m6.png",
              "./data/portraits/m7.png",
              "./data/portraits/m8.png",
              "./data/portraits/m9.png",
              "./data/portraits/m10.png",
              "./data/portraits/m11.png" },

            // female portraits
            { "./data/portraits/w1.png",
              "./data/portraits/w6.png",
              "./data/portraits/w12.png",
              "./data/portraits/w14.png",
              "./data/portraits/w15.png",
              "./data/portraits/w16.png",
              "./data/portraits/w17.png",
              "./data/portraits/girl.png" }
    };

    public static final String DEATH_PORTRAIT = "./data/portraits/death.png";
}
