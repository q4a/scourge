package org.scourge.util;

/**
 * User: gabor
 * Date: May 31, 2010
 * Time: 8:24:49 PM
 */
public class StringUtil {
    public static boolean isEmpty(String s) {
        return s == null || s.trim().length() == 0;
    }
}
