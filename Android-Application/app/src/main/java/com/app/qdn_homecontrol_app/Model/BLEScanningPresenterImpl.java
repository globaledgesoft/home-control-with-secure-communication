package com.app.qdn_homecontrol_app.Model;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.Intent;

import com.app.qdn_homecontrol_app.Presenter.BleScanningPresenter;
import com.app.qdn_homecontrol_app.Util.Constants;
import com.app.qdn_homecontrol_app.Util.QDNLogger;
import com.app.qdn_homecontrol_app.View.BleScanningView;
import com.globlaedge.cloud.blelibrary.blelibrary.Interface.Interface.ILeScanListResult;
import com.globlaedge.cloud.blelibrary.blelibrary.Interface.Java.BleScan;
import java.util.ArrayList;

public class BLEScanningPresenterImpl implements BleScanningPresenter {

    private BleScanningView mBleScanningView;
    private static final int REQUEST_ENABLE_BT = 100;
    private Activity mActivity;
    private BleScan bleLibrary;
    private static final String TAG = "BLEScanningPresenterImpl";

    public BLEScanningPresenterImpl(BleScanningView mBleScanningView , Activity activity) {
        this.mBleScanningView = mBleScanningView;
        this.mActivity = activity;
        bleLibrary = new BleScan(mActivity);
    }

    @Override
    public void scanLeDevice(BluetoothAdapter mBluetoothAdapter) {
        if (mBluetoothAdapter == null || !mBluetoothAdapter.isEnabled()){
            Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            mActivity.startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
        }
        bleLibrary. scanLeDevice(Constants.SCAN_PERIOD, new ILeScanListResult() {
            @Override
            public void onScanComplete(ArrayList<BluetoothDevice> deviceList) {

                QDNLogger.i(TAG,"deviceList "+ deviceList);
                mBleScanningView.ScannedDevice(deviceList);
            }
        });

    }
}
