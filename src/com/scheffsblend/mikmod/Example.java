/**
 * 
 */
package com.scheffsblend.mikmod;

import java.io.IOException;

import com.scheffsblend.mikmod.R;

import android.app.Activity;
import android.content.Intent;
import android.content.res.AssetManager;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.TextView;
import android.widget.Toast;

/**
 * A simple example demonstrating the use of libmikmod in android!
 * @author Clark Scheff
 *
 */
public class Example extends Activity implements MikMod.EventListener {
	private static final String TAG = "MikMod"; 

    static final String MOD = "/sdcard/test.mod";
    static final String XM = "/sdcard/test.xm";

    static boolean isPlaying = false;
    static boolean isPaused = false;
    static boolean modLoaded = false;
    static boolean mikmodLoaded = false;
    
    static Button mPlayPauseButton;
    Button mStopButton;
    Button mChoose;
    Button mForward;
    Button mRewind;
    Button mRestart;
    TextView mInfo;
    TextView mPlayTime;
    static SeekBar mSongPosition;
    private AssetManager am;

	public static final Handler mHandler = new Handler();

	/** Called when the activity is first created. */
    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        setContentView(R.layout.main);
        
        am = getAssets();

       	MikMod.initialize();
       	MikMod.setListener(this);
        mikmodLoaded = true;
        
        mPlayPauseButton = (Button)findViewById(R.id.play_pause);
        mStopButton = (Button)findViewById(R.id.stop);
        mChoose = (Button)findViewById(R.id.choose);
        mForward = (Button)findViewById(R.id.forward);
        mRewind = (Button)findViewById(R.id.rewind);
        mRestart = (Button)findViewById(R.id.restart);
        mInfo = (TextView)findViewById(R.id.info);
        mPlayTime = (TextView)findViewById(R.id.playtime);
        mSongPosition = (SeekBar)findViewById(R.id.songPosition);
        mRewind.setText("<<");
        mRestart.setText("<<<");

        // initialize button click handlers
        ((Button)(findViewById(R.id.embedded))).setOnClickListener(new OnClickListener() {
            public void onClick(View view) {
    			if(MikMod.isPlaying()) {
            		MikMod.stop();
        			mPlayPauseButton.setText("||");
    			}
            	if(MikMod.loadModAsset(am, "demo.xm")) {
            		modLoaded = true;
            		String[] info = MikMod.getModInfo();
            		mInfo.setText("Title: " + info[0] + "\n" +
            			"Type: " + info[1] + "\n" +
            			"Comment: " + info[2] + "\n" +
            			"BPM: " + info[3] + "\n" +
            			"Tracks: " + info[4]);
            		mSongPosition.setMax(Integer.parseInt(info[4]));
            		mSongPosition.setProgress(0);
            		mSongPosition.setVisibility(View.VISIBLE);
            	} else {
            		Toast.makeText(view.getContext(), "Unable to load file!", Toast.LENGTH_SHORT).show();
            	}
            }
        });
        
        mPlayPauseButton.setOnClickListener(new OnClickListener() {
            public void onClick(View view) {
               	if (!MikMod.isModLoaded())
               		return;
            	if (!MikMod.isPaused() && !MikMod.isPlaying()) {
            		MikMod.play();
        			mPlayPauseButton.setText("||");
            	} else {
            		if (MikMod.isPaused()) {
            			MikMod.resume();
            			mPlayPauseButton.setText("||");
            		} else {
            			MikMod.pause();
            			mPlayPauseButton.setText(">");
            		}
            	}
            }
        });

        mStopButton.setOnClickListener(new OnClickListener() {
            public void onClick(View view) {
            	MikMod.stop();
    			mPlayPauseButton.setText(">");
            }
        });

        mForward.setOnClickListener(new OnClickListener() {
            public void onClick(View view) {
            	if(MikMod.isModLoaded())
            		MikMod.nextTrack();
            }
        });

        mRewind.setOnClickListener(new OnClickListener() {
            public void onClick(View view) {
            	if(MikMod.isModLoaded())
            		MikMod.previousTrack();
            }
        });

        mRestart.setOnClickListener(new OnClickListener() {
            public void onClick(View view) {
            	if(MikMod.isModLoaded()) {
            		MikMod.restartMod();
            		if(MikMod.isPaused())
            			MikMod.pause();
            	}
            }
        });
        
        mChoose.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
				Intent intent = new Intent(getApplicationContext(), FileChooser.class);
            	startActivityForResult(intent, 0);
			}
		});
        
        mSongPosition.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {
			@Override
			public void onStopTrackingTouch(SeekBar seekBar) {
			}
			
			@Override
			public void onStartTrackingTouch(SeekBar seekBar) {
			}
			
			@Override
			public void onProgressChanged(SeekBar seekBar, int progress,
					boolean fromUser) {
				if (fromUser) {
					if (MikMod.isModLoaded()) {
						MikMod.setTrack(progress);
						if(MikMod.isPaused())
							MikMod.pause();
					}
				}
			}
		});

    }

    @Override 
    public void onActivityResult(int requestCode, int resultCode, Intent data) { 
    	super.onActivityResult(requestCode, resultCode, data); 
    	if (requestCode == 0) {
    		if (resultCode == RESULT_OK) {
    			if (data == null) {
    				return;
    			}
    			
    			String file = data.getStringExtra("file");

    			if(MikMod.isPlaying()) {
            		MikMod.stop();
        			mPlayPauseButton.setText("||");
    			}
            	if(MikMod.loadModFile(file)) {
            		modLoaded = true;
            		String[] info = MikMod.getModInfo();
            		mInfo.setText("Title: " + info[0] + "\n" +
            			"Type: " + info[1] + "\n" +
            			"Comment: " + info[2] + "\n" +
            			"BPM: " + info[3] + "\n" +
            			"Tracks: " + info[4]);
            		mSongPosition.setMax(Integer.parseInt(info[4]));
            		mSongPosition.setProgress(0);
            		mSongPosition.setVisibility(View.VISIBLE);
            	} else {
            		Toast.makeText(this, "Unable to load file!", Toast.LENGTH_SHORT).show();
            	}
    		}
    	}
    }
    
    @Override 
    protected void onStart() {
    	super.onStart();
    }
    
    @Override
    protected void onRestart() {
    	super.onRestart();
    }
    
    /** Called when the activity is about to be destroyed. */
    @Override
    protected void onPause()
    {
   		MikMod.pause();
        super.onPause();
    }
    
    @Override
    protected void onResume() {
   		MikMod.resume();
    	super.onResume();
    }
    
    @Override
    protected void onStop() {
    	super.onStop();
    }

    /** Called when the activity is about to be destroyed. */
    @Override
    protected void onDestroy()
    {
    	MikMod.shutdownMikMod();
        super.onDestroy();
    }
/*    
    private void playMusic() {
    	MikMod.playMod();
		isPlaying = true;
		isPaused = false;
		mPlayPauseButton.setText(" || ");
		mSongPosition.setVisibility(View.VISIBLE);
    }
    
    private static void stopMusic() {
    	if (MikMod.isActive())
    		MikMod.stopMod();
    	mPlayPauseButton.setText(">");
    	mSongPosition.setVisibility(View.INVISIBLE);
    }
    
    private void pauseMusic() {
    	if (MikMod.isActive()) {
    		MikMod.pauseMod();
        	mPlayPauseButton.setText(">");
    	}
    	isPaused = true;
    }
    
    private void resumeMusic() {
    	if (MikMod.isActive()) {
    		MikMod.resumeMod();
        	mPlayPauseButton.setText("||");
    	}
    	isPaused = false;
    }
*/    
    /** Load jni .so on initialization */
    static {
         //System.loadLibrary("native-audio-jni");
         System.loadLibrary("mikmod");
    }

	@Override
	public void OnMusicEnd() {
		Log.i(TAG, "OnMusicEnd() called!");
		mHandler.post(new Runnable() {
			public void run() {
       			mPlayPauseButton.setText(">");
       			mSongPosition.setProgress(0);
			}
		});
	}

	@Override
	public void OnMusicUpdate(final float time, final int sngpos, final int numpos) {
		mHandler.post(new Runnable() {
			public void run() {
				mPlayTime.setText("Current: " + time + "\nPosition: " +
						sngpos + "/" + numpos);
				mSongPosition.setMax(numpos);
				if(mSongPosition.getProgress() != sngpos)
					mSongPosition.setProgress(sngpos);
			}
		});
	}

	@Override
	public void OnMusicLoaded(String name, String type, String comment,
			int bpm, int tracks) {
		// TODO Auto-generated method stub
		
	}
}
