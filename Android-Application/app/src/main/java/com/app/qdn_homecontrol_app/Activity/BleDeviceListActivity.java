package com.app.qdn_homecontrol_app.Activity;

import android.annotation.SuppressLint;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothManager;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.support.constraint.ConstraintLayout;
import android.support.design.widget.Snackbar;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;
import com.app.qdn_homecontrol_app.Adapter.BleDeviceListAdapter;
import com.app.qdn_homecontrol_app.Model.BLEScanningModel;
import com.app.qdn_homecontrol_app.Presenter.BleScanningPresenter;
import com.app.qdn_homecontrol_app.R;
import com.app.qdn_homecontrol_app.Util.Constants;
import com.app.qdn_homecontrol_app.Util.QDNLogger;
import com.app.qdn_homecontrol_app.Util.RuntimePermission;
import com.app.qdn_homecontrol_app.View.BleScanningView;
import com.globlaedge.cloud.blelibrary.blelibrary.Interface.Service.BLEService;

import java.util.ArrayList;

/**
 * This Activity is responsible for displaying scanning device ,scan again and ref to
 * About screen
 */
public class BleDeviceListActivity extends AppCompatActivity
        implements BleScanningView, View.OnClickListener {

    private BluetoothAdapter mBluetoothAdapter;
    private static final int REQUEST_ENABLE_BT = 1;
    private RecyclerView recyclerView;
    private BleDeviceListAdapter mAdapter;
    private static final String TAG = "BleDeviceListActivity";
    private BluetoothManager bluetoothManager;
    private RecyclerView.LayoutManager mLayoutManager;
    private BleScanningPresenter mBleScanningPresenter;
    private ImageView aboutImage, scanImage;
    private RuntimePermission runtimePermission;
    private ConstraintLayout mdeviceListConstraintLayout;
    private ArrayList<BluetoothDevice> bleDeviceList;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_bledevice_list);
        init();

    }


    /**
     * Initialization
     */
    private void init() {
        mdeviceListConstraintLayout = (ConstraintLayout) findViewById(R.id.layoutConstraint);
        runtimePermission = RuntimePermission.getInstance(this);
        aboutImage = (ImageView) findViewById(R.id.aboutImageViewClick);
        scanImage = (ImageView) findViewById(R.id.scanBLEImageView);

        aboutImage.setOnClickListener(this);
        scanImage.setOnClickListener(this);
        bleDeviceList = new ArrayList<BluetoothDevice>();

        recyclerView = (RecyclerView) findViewById(R.id.recycler_view);
        recyclerView.setHasFixedSize(true);

        mLayoutManager = new LinearLayoutManager(getApplicationContext());
        recyclerView.setLayoutManager(mLayoutManager);
        bluetoothManager = (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
        mBluetoothAdapter = bluetoothManager.getAdapter();
        mBleScanningPresenter = new BLEScanningModel(BleDeviceListActivity.this, this);

    }


    @Override
    protected void onResume() {
        Log.i(TAG,"onResume");
        super.onResume();
        if(bleDeviceList.size()==0){
            scanBle();
        }


    }

    @Override
    protected void onPause() {
        Log.i("onPause","onPause");
        super.onPause();
    }


    /**
     * We will get the available BLE device and add to the adapter
     * @param bluetoothDevice ArrayList of BluetoothDevice
     */
    @SuppressLint("LongLogTag")
    @Override
    public void ScannedDevice(ArrayList<BluetoothDevice> bluetoothDevice) {
        mAdapter = new BleDeviceListAdapter(bluetoothDevice,this, mBluetoothAdapter);
        recyclerView.setAdapter(mAdapter);
        mAdapter.notifyDataSetChanged();
        bleDeviceList.addAll(bluetoothDevice);

        QDNLogger.i(TAG,"BluetoothDevice : " + bluetoothDevice);

    }

    /**
     * @param v view
     */
    @Override
    public void onClick(View v) {
        switch (v.getId()){
            case R.id.aboutImageViewClick:
                Intent mIntent = new Intent(this, SettingsScreenActivity.class);
                startActivity(mIntent);
                break;
            case R.id.scanBLEImageView:
                BLEService.bleDisconnection();
                if(mAdapter!= null){
                    mAdapter.clear();
                    mAdapter.notifyDataSetChanged();
                }
                scanBle();
                break;
            default:
                break;
        }
    }

    private void scanBle() {
        if (!runtimePermission.hasAllPermissionsGranted()) {
            runtimePermission.requestLocPermissions();

        } else {
            if (mBluetoothAdapter == null || !mBluetoothAdapter.isEnabled()) {
                Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
                startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
            }
            else if(mBluetoothAdapter != null && mBluetoothAdapter.isEnabled()){
                mBleScanningPresenter = new BLEScanningModel(BleDeviceListActivity.this, this);
                mBleScanningPresenter.scanLeDevice(mBluetoothAdapter);
            }

        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == Constants.REQUEST_ID_MULTIPLE_PERMISSIONS) {
            if (permissions.length > 0) {
                if ((ActivityCompat.checkSelfPermission(this, permissions[0]) != PackageManager.PERMISSION_GRANTED)) {
                    Snackbar.make(mdeviceListConstraintLayout, R.string.toast_permission_is_required, Snackbar.LENGTH_LONG)
                            .setAction("Action", null).show();
                } else if (runtimePermission.hasAllPermissionsGranted()) {

                    mBleScanningPresenter = new BLEScanningModel(BleDeviceListActivity.this, this);
                    mBleScanningPresenter.scanLeDevice(mBluetoothAdapter);
                } else runtimePermission.requestLocPermissions();

            }
        }
    }

}
