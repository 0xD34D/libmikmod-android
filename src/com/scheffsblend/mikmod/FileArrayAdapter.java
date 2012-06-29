/**
 * 
 */
package com.scheffsblend.mikmod;

import java.util.List;

import com.scheffsblend.mikmod.R;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.TextView;

/**
 * @author Clark Scheff
 * 
 */
public class FileArrayAdapter extends ArrayAdapter<Option> {

	private Context mContext;
	private int mId;
	private List<Option> mItems;

	public FileArrayAdapter(Context context, int textViewResourceId,
			List<Option> objects) {
		super(context, textViewResourceId, objects);
		mContext = context;
		mId = textViewResourceId;
		mItems = objects;
	}

	public Option getItem(int i) {
		return mItems.get(i);
	}

	@Override
	public View getView(int position, View convertView, ViewGroup parent) {
		View v = convertView;
		if (v == null) {
			LayoutInflater vi = (LayoutInflater) mContext
					.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
			v = vi.inflate(mId, null);
		}
		final Option o = mItems.get(position);
		if (o != null) {
			TextView t1 = (TextView) v.findViewById(R.id.TextView01);
			TextView t2 = (TextView) v.findViewById(R.id.TextView02);
			ImageView iv = (ImageView)v.findViewById(R.id.icon);

			if (t1 != null)
				t1.setText(o.getName());
			if (t2 != null)
				t2.setText(o.getData());
			
			if (iv != null) {
				if (o.isDir())
					iv.setImageDrawable(mContext.getResources().getDrawable(R.drawable.folder_icon));
				else
					iv.setImageDrawable(mContext.getResources().getDrawable(R.drawable.music_icon));
			}

		}
		return v;
	}
}
