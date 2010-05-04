package org.scourge.ui;

/**
 * User: gabor
 * Date: May 3, 2010
 * Time: 9:08:33 PM
 */
public interface ProgressListener {
    public void progress(float percent);
    public void done();
}
