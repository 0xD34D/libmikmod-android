/**
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 */
package com.scheffsblend.mikmod;

import android.content.res.AssetManager;
import android.util.Log;


/**
 * @author Clark Scheff
 *
 */
public class MikMod {
	private static final String TAG = "MikMod";
	private static final int STATE_UNINITIALIZED = 0;
	private static final int STATE_INITIALIZED = 1;
	private static final int STATE_MOD_LOADED = 2;
	private static final int STATE_MOD_STOPPED = 3;
	private static final int STATE_MOD_PLAYING = 4;
	private static final int STATE_MOD_PAUSED = 5;
	
	private static EventListener mListener = null;
	private static int mState = STATE_UNINITIALIZED;

	public static interface EventListener 
	{
		void OnMusicEnd();
		void OnMusicUpdate(float time, int sngpos, int numpos);
		void OnMusicLoaded(String name, String type, String comment,
				int bpm, int tracks);
	}

	public static void setListener (EventListener l) {
		mListener = l;
	}
	
    /**
     * Call this at startup to initialize the OpenSL ES sound system and MikMod
     * @return true if MikMod was successfully initialized
     */
	public static boolean initialize() {
		if (initMikMod())
			mState = STATE_INITIALIZED;
		
		return (mState == STATE_INITIALIZED);
	}
	
    /**
     * Call this at startup to initialize the OpenSL ES sound system and MikMod
     * @param l - Listener for MikMod event callbacks
     * @return true if MikMod was successfully initialized
     */
	public static boolean initialize(EventListener l) {
		if (initMikMod())
			mState = STATE_INITIALIZED;
		mListener = l;
		
		return (mState == STATE_INITIALIZED);
	}
	
	/**
	 * Load a mod from the file system
	 * @param filename - file to load including path
	 * @return true if successful, false if not
	 */
	public static boolean loadModFile(String filename) {
		if (mState == STATE_UNINITIALIZED) {
			Log.w(TAG, "MikMod not initialized.");
			return false;
		}
		
		if (!loadMod(filename)) {
			Log.e(TAG, "Unable to load module.");
			return false;
		}
		mState = STATE_MOD_LOADED;
		return true;
	}
	
    /**
     * Load a mod from the application's assets
     * @param am - a reference to the asset manager
     * @param filename - mod file to load from assets
     * @return true if successful, false if not
     */
	public static boolean loadModAsset(AssetManager am, String filename) {
		if (mState == STATE_UNINITIALIZED) {
			Log.w(TAG, "MikMod not initialized.");
			return false;
		}
		
		if (!loadModFromAsset(am, filename)) {
			Log.e(TAG, "Unable to load module.");
			return false;
		}
		mState = STATE_MOD_LOADED;
		return true;
	}
	
	/**
	 * Start playing currently loading mod
	 * @return true if a mod is loaded and can be played
	 */
	public static boolean play() {
		if (mState == STATE_UNINITIALIZED) {
			Log.w(TAG, "MikMod not initialized.");
			return false;
		} else if (mState < STATE_MOD_LOADED) {
			Log.w(TAG, "No module loaded.");
			return false;
		}
		
		if (mState == STATE_MOD_PAUSED)
			return resume();
		playMod();
		mState = STATE_MOD_PLAYING;
		return true;
		
	}
	
	/**
	 * Stop currently playing mod
	 * @return true if a mod is playing and can be stopped
	 */
	public static boolean stop() {
		if (mState == STATE_UNINITIALIZED) {
			Log.w(TAG, "MikMod not initialized.");
			return false;
		}
		if (isActive())
			stopMod();
		mState = STATE_MOD_STOPPED;
		return true;
	}
	
	/**
	 * Pause playback of currently playing mod
	 * @return true if mod is playing and has been paused
	 */
	public static boolean pause() {
		if (mState == STATE_UNINITIALIZED) {
			Log.w(TAG, "MikMod not initialized.");
			return false;
		}
		if (mState != STATE_MOD_PLAYING) {
			Log.w(TAG, "No mod is playing.");
			return false;
		}
		pauseMod();
		mState = STATE_MOD_PAUSED;
		return true;
	}
	
	/**
	 * Resume playback of currently paused mod
	 * @return true if playback is paused and can be resumed
	 */
	public static boolean resume() {
		if (mState == STATE_UNINITIALIZED) {
			Log.w(TAG, "MikMod not initialized.");
			return false;
		}
		if (mState != STATE_MOD_PAUSED) {
			Log.w(TAG, "No mod is paused.");
			return false;
		}
		resumeMod();
		mState = STATE_MOD_PLAYING;
		return true;
	}
	
	/**
	 * Call this to see if a mod is currently loaded
	 * @return true if a mod is currently loaded
	 */
	public static boolean isModLoaded() {
		return (mState >= STATE_MOD_LOADED);
	}
	
	/**
	 * Call this to see if a mod is currently playing
	 * @return true if mod if being played
	 */
	public static boolean isPlaying() {
		return (mState == STATE_MOD_PLAYING);
	}
	
	/**
	 * Call this to see if playback is currently paused
	 * @return true if playback is paused, false otherwise
	 */
	public static boolean isPaused() {
		return (mState == STATE_MOD_PAUSED);
	}
	
	/**
	 * Call this to see if playback is stopped
	 * @return true if playback is stopped, false otherwise
	 */
	public static boolean isStopped() {
		return (mState == STATE_MOD_STOPPED);
	}
	
	private static native boolean initMikMod();
    private static native boolean loadMod(String mod);
    private static native boolean loadModFromAsset(AssetManager am, String mod);
    public static native String[] getModInfo();
    private static native void playMod();
    private static native void pauseMod();
    private static native void resumeMod();
    private static native void stopMod();
    public static native void previousTrack();
    public static native void nextTrack();
    public static native void setTrack(int pos);
    public static native void restartMod();
    public static native void shutdownMikMod();
    private static native boolean isActive();
    
    private static void OnMusicEnd() {
    	if (mListener != null)
    		mListener.OnMusicEnd();
    	mState = STATE_MOD_STOPPED;
    }
    
    private static void OnMusicUpdate(float time, int sngpos, int numpos) {
    	if (mListener != null)
    		mListener.OnMusicUpdate(time, sngpos, numpos);
    }
    
    private static void OnMusicLoaded(String name, String type,
    		String comment, int bpm, int tracks) {
    	if (mListener != null)
    		mListener.OnMusicLoaded(name, type, comment, bpm, tracks);
    }
}
