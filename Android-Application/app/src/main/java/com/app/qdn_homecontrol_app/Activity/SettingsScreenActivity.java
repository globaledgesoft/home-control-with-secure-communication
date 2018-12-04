package com.app.qdn_homecontrol_app.Activity;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.View;
import android.widget.ImageView;
import com.app.qdn_homecontrol_app.Adapter.SettingAdapter;
import com.app.qdn_homecontrol_app.R;
import java.util.ArrayList;

public class SettingsScreenActivity extends AppCompatActivity implements View.OnClickListener {

    private ImageView backButtonSetting;
    private RecyclerView settingsRecycler;
    private SettingAdapter settingAdapter;
    private ArrayList<String> settingListItem;
    private RecyclerView.LayoutManager mLayoutManager;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_settings_screen);

        init();
        backButtonSetting.setOnClickListener(this);
        settingsRecycler.setAdapter(settingAdapter);

    }

    /**
     * Initialization
     */
    private void init() {

        settingsRecycler = (RecyclerView) findViewById(R.id.settingRecyclerView);
        settingsRecycler.setHasFixedSize(true);

        mLayoutManager = new LinearLayoutManager(getApplicationContext());
        settingsRecycler.setLayoutManager(mLayoutManager);

        backButtonSetting = (ImageView) findViewById(R.id.backButtonSettings);
        settingsRecycler = (RecyclerView) findViewById(R.id.settingRecyclerView);
        settingListItem = new ArrayList<String>();

        //At this time this is hardcoded
        settingListItem.add(getString(R.string.about));
        settingAdapter = new SettingAdapter(settingListItem,this);
    }

    /**
     *
     * @param v
     */
    @Override
    public void onClick(View v) {
        switch (v.getId()){
            case R.id.backButtonSettings:
                onBackPressed();
                break;
            default:
                break;
        }
    }

    @Override
    public void onBackPressed() {
        super.onBackPressed();
    }
}
