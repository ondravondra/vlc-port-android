/*****************************************************************************
 * HistoryFragment.java
 *****************************************************************************
 * Copyright © 2012-2013 VLC authors and VideoLAN
 * Copyright © 2012-2013 Edward Wang
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/
package org.videolan.vlc.gui;

import org.videolan.libvlc.LibVLC;
import org.videolan.libvlc.LibVlcUtil;
import org.videolan.vlc.R;
import org.videolan.vlc.audio.AudioServiceController;
import org.videolan.vlc.interfaces.IRefreshable;

import android.os.Bundle;
import android.support.v4.app.ListFragment;
import android.support.v7.app.ActionBarActivity;
import android.util.Log;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.LayoutInflater;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView.AdapterContextMenuInfo;
import android.widget.ListView;

public class HistoryFragment extends ListFragment implements IRefreshable {
    public final static String TAG = "VLC/HistoryFragment";

    private HistoryAdapter mHistoryAdapter;

    /* All subclasses of Fragment must include a public empty constructor. */
    public HistoryFragment() { }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mHistoryAdapter = new HistoryAdapter(getActivity());
        Log.d(TAG, "HistoryFragment()");
    }

    private void focusHelper(boolean idIsEmpty) {
        View parent = View.inflate(getActivity(), R.layout.history_list,
            null);
        MainActivity main = (MainActivity)getActivity();
        main.setMenuFocusDown(idIsEmpty, android.R.id.list);
        main.setSearchAsFocusDown(idIsEmpty, parent,
            android.R.id.list);
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState)
    {
        ((ActionBarActivity) getActivity()).getSupportActionBar().setTitle(R.string.history);

        View v = inflater.inflate(R.layout.history_list, container, false);
        setListAdapter(mHistoryAdapter);
        final ListView listView = (ListView)v.findViewById(android.R.id.list);
        listView.setNextFocusUpId(R.id.ml_menu_search);
        listView.setNextFocusLeftId(android.R.id.list);
        listView.setNextFocusRightId(android.R.id.list);
        if (LibVlcUtil.isHoneycombOrLater())
            listView.setNextFocusForwardId(android.R.id.list);
        focusHelper(mHistoryAdapter.getCount() == 0);
        listView.requestFocus();
        registerForContextMenu(listView);
        return v;
    }

    @Override
    public void onCreateContextMenu(ContextMenu menu, View v, ContextMenuInfo menuInfo) {
        MenuInflater menuInflater = getActivity().getMenuInflater();
        menuInflater.inflate(R.menu.history_view, menu);
    }

    @Override
    public void onListItemClick(ListView l, View v, int p, long id) {
        playListIndex(p);
    }

    private void playListIndex(int position) {
        AudioServiceController audioController = AudioServiceController.getInstance();

        LibVLC.getExistingInstance().setMediaList();
        audioController.playIndex(position);
    }

    @Override
    public boolean onContextItemSelected(MenuItem item) {
        if(!getUserVisibleHint()) return super.onContextItemSelected(item);

        AdapterContextMenuInfo info = (AdapterContextMenuInfo) item.getMenuInfo();
        if(info == null) // info can be null
            return super.onContextItemSelected(item);
        int id = item.getItemId();

        if(id == R.id.history_view_play) {
            playListIndex(info.position);
            return true;
        } else if(id == R.id.history_view_delete) {
            LibVLC.getExistingInstance().getPrimaryMediaList().remove(info.position);
            mHistoryAdapter.refresh();
            return true;
        }
        return super.onContextItemSelected(item);
    }

    @Override
    public void refresh() {
        Log.d(TAG, "Refreshing view!");
        if( mHistoryAdapter != null ) {
            mHistoryAdapter.refresh();
            focusHelper(mHistoryAdapter.getCount() == 0);
        } else
            focusHelper(true);
    }
}
