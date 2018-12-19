package com.app.qdn_homecontrol_app.Activity;

import android.content.Intent;
import android.os.Handler;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.widget.ImageView;
import com.app.qdn_homecontrol_app.R;
import com.app.qdn_homecontrol_app.Util.QDNLogger;

public class SplashScreenActivity extends AppCompatActivity {
    private ImageView imageView;
    private static final String TAG = "GE_SplashScreenActivity";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_splash_screen);
        imageView = findViewById(R.id.splashScreenImageView);

    }

    @Override
    protected void onResume() {
        super.onResume();
        new Handler().postDelayed(new Runnable() {
            @Override
            public void run() {
                QDNLogger.d(TAG, "Starting BleDeviceListActivity...");
                startActivity(new Intent(SplashScreenActivity.this, BleDeviceListActivity.class));
                finish();
            }
        },2000);

    }
}
