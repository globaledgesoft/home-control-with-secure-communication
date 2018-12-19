package com.app.qdn_homecontrol_app.Util;

import android.Manifest;
import android.app.Activity;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Build;
import android.provider.Settings;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AlertDialog;
import android.text.TextUtils;
import android.widget.Toast;

import com.app.qdn_homecontrol_app.R;




public class RuntimePermission {

    private static RuntimePermission instance = null;
    private Activity activity;
    private static final String RUNTIME_PERMISSIONS =
            Manifest.permission.ACCESS_FINE_LOCATION;
    private boolean isLocation = true;

    public RuntimePermission(Activity activity) {
        this.activity = activity;
    }

    static public RuntimePermission getInstance(Activity activity) {
        if (instance == null) {
            instance = new RuntimePermission(activity);
            return instance;
        } else {
            return instance;
        }
    }


    public boolean hasAllPermissionsGranted() {
        isLocation = false;
        if(isPhoneLocationEnabled()) {
            if (ActivityCompat.checkSelfPermission(activity,RUNTIME_PERMISSIONS )
                    == PackageManager.PERMISSION_GRANTED)
            {
                isLocation = true;
            } else isLocation = false;
        } else isLocation = false;
        return isLocation;
    }



    public void openLocPermissionIntent()
    {

            AlertDialog.Builder dialog = new AlertDialog.Builder(activity);
            dialog.setMessage(R.string.gpsNetworkNotEnabled);
            dialog.setPositiveButton(R.string.openLocationSettings, new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface paramDialogInterface, int paramInt) {
                    // TODO Auto-generated method stub
                    Intent myIntent = new Intent(Settings.ACTION_LOCATION_SOURCE_SETTINGS);
                    activity.startActivity(myIntent);
                }
            });
            dialog.setNegativeButton(R.string.buttonCancel, new DialogInterface.OnClickListener() {

                @Override
                public void onClick(DialogInterface paramDialogInterface, int paramInt) {
                    isLocation = false;
                    Toast.makeText(activity, R.string.toast_permission_is_required, Toast.LENGTH_SHORT).show();

                    // TODO Auto-generated method stub

                }
            });
            if (activity != null && !activity.isFinishing()) {

                dialog.show();
            }

    }

    public void requestLocPermissions() {
        if(!isPhoneLocationEnabled())
            openLocPermissionIntent();
        if(isPhoneLocationEnabled())
            ActivityCompat.requestPermissions(activity, new String[]{Manifest.permission.ACCESS_FINE_LOCATION}, Constants.REQUEST_ID_MULTIPLE_PERMISSIONS);
    }


    public  boolean isPhoneLocationEnabled() {
        int locationMode = 0;
        String locationProviders;

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
            try {
                locationMode = Settings.Secure.getInt(activity.getContentResolver(), Settings.Secure.LOCATION_MODE);
            } catch (Settings.SettingNotFoundException e) {
                e.printStackTrace();
            }
            return locationMode != Settings.Secure.LOCATION_MODE_OFF;
        } else {
            locationProviders = Settings.Secure.getString(activity.getContentResolver(), Settings.Secure.LOCATION_MODE);
            return !TextUtils.isEmpty(locationProviders);
        }
    }



}