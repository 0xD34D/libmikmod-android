/**
 * 
 */
package com.scheffsblend.mikmod;

import java.io.File;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import com.scheffsblend.mikmod.R;

import android.app.ListActivity;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.os.Bundle;
import android.os.Environment;
import android.view.View;
import android.widget.ListView;
import android.widget.Button;
import android.view.View.OnClickListener;
/**
 * @author Clark Scheff
 *
 */
public class FileChooser extends ListActivity {
	private static final String PREFS = "MIKMOD_PREFS";
	private static final String[] EXTENSIONS = { ".669", ".amf", ".dsm",
												 ".far", ".gdm", ".imf",
												 ".it",  ".m15", ".med",
												 ".mod", ".mtm", ".okt",
												 ".s3m", ".stm", ".stx",
												 ".ult", ".uni", ".xm" };
	private File mCurrentDir;
	private FileArrayAdapter mAdapter;
	private SharedPreferences mPrefs;
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		mPrefs = this.getSharedPreferences(PREFS, 0);
		mCurrentDir = new File(mPrefs.getString("lastDir", 
				Environment.getExternalStorageDirectory().getAbsolutePath()));
		fill(mCurrentDir);
		
		this.setContentView(R.layout.file_chooser);

		Intent intent = getIntent();
        Bundle extras = intent.getExtras();
        
        ((Button)(findViewById(R.id.cancel))).setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
				finish();
			}
		});
	}
	
	@Override
	public void onBackPressed() {
		//super.onBackPressed();
		if (mCurrentDir.getAbsolutePath().equals(Environment.getExternalStorageDirectory().getAbsolutePath()))
			finish();
		else {
			fill( new File(mCurrentDir.getParent()) );
		}
	}
	
	private void fill(File f) {
        mCurrentDir = f;
		Editor editor = mPrefs.edit();
		editor.putString("lastDir", mCurrentDir.getAbsolutePath());
		editor.commit();
		File[]dirs = f.listFiles();
        this.setTitle("Current Dir: "+f.getName());
        List<Option>dir = new ArrayList<Option>();
        List<Option>fls = new ArrayList<Option>();
        try{
            for(File ff: dirs)
            {
               if(ff.isDirectory())
                   dir.add(new Option(ff.getName(),"Folder",ff.getAbsolutePath(), true));
               else
               {
            	   String name = ff.getName().toLowerCase();
            	   String extension = name.substring(name.lastIndexOf('.'), name.length());
                   for(int i = 0; i < EXTENSIONS.length; i++) {
                	   if (extension.equals(EXTENSIONS[i])) {
                    	   fls.add(new Option(ff.getName(),"File Size: "+ff.length(),ff.getAbsolutePath(), false));
                    	   break;
                	   }
                   }
               }
            }
        } catch(Exception e) {
            
        }
        Collections.sort(dir);
        Collections.sort(fls);
        dir.addAll(fls);
        if(!f.getName().equalsIgnoreCase("sdcard"))
            dir.add(0,new Option("..","Parent Directory",f.getParent(), true));
        
        mAdapter = new FileArrayAdapter(FileChooser.this, R.layout.file_view, dir);
        this.setListAdapter(mAdapter);
	}
	
	@Override
	protected void onListItemClick(ListView l, View v, int position, long id) {
		// TODO Auto-generated method stub
		super.onListItemClick(l, v, position, id);
		Option o = mAdapter.getItem(position);
		if(o.getData().equalsIgnoreCase("folder")||o.getData().equalsIgnoreCase("parent directory")){
				mCurrentDir = new File(o.getPath());
				fill(mCurrentDir);
		} else {
			onFileClick(o);
		}
	}
	
	private void onFileClick(Option o) {
        Bundle extras = new Bundle();
        extras.putString("file", o.getPath());
        
        setResult(RESULT_OK, new Intent()
        .putExtras(extras));
        finish();
	}
}
