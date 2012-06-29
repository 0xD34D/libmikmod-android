/**
 * 
 */
package com.scheffsblend.mikmod;

/**
 * @author lithium
 *
 */
public class Option implements Comparable<Option> {
	private String mName;
	private String mData;
	private String mPath;
	private boolean mIsDirectory;
	
	public Option(String name, String data, String path, boolean isDir) {
		mName = name;
		mData = data;
		mPath = path;
		mIsDirectory = isDir;
	}
	
	public String getName() {
		return mName;
	}
	
	public String getData() {
		return mData;
	}
	
	public String getPath() {
		return mPath;
	}
	
	public boolean isDir() {
		return mIsDirectory;
	}
	
	@Override
	public int compareTo(Option another) {
		if (this.mName != null)
			return this.mName.toLowerCase().compareTo(another.getName().toLowerCase());
		else
			throw new IllegalArgumentException();
	}
}
