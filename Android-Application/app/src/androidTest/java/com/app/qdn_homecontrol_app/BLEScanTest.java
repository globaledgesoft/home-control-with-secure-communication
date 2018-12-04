package com.app.qdn_homecontrol_app;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothManager;
import android.content.Context;
import android.support.test.InstrumentationRegistry;
import android.support.test.annotation.UiThreadTest;
import android.support.test.rule.ActivityTestRule;
import android.support.test.runner.AndroidJUnit4;

import com.app.qdn_homecontrol_app.Activity.BleDeviceListActivity;
import com.app.qdn_homecontrol_app.Model.BLEScanningModel;
import com.app.qdn_homecontrol_app.Presenter.BleScanningPresenter;
import com.app.qdn_homecontrol_app.View.BleScanningView;
import com.globlaedge.cloud.blelibrary.blelibrary.Interface.Util.Logger;

import android.os.Handler;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;

import java.util.ArrayList;

import static junit.framework.TestCase.assertTrue;

@RunWith(AndroidJUnit4.class)
public class BLEScanTest implements BleScanningView {

    private Context instruCtx;
    private BleScanningPresenter mBleScanningPresenter;
    private BluetoothManager bluetoothManager;
    private BluetoothAdapter mBluetoothAdapter;
    boolean isScanTrue;
    private ArrayList<BluetoothDevice> mBluetoothDevices;
    private int size = 0;

    @Rule
    public ActivityTestRule<BleDeviceListActivity> mActivityTestRule = new ActivityTestRule<>(BleDeviceListActivity.class);

    @Before
    public void setUp() {
        instruCtx = InstrumentationRegistry.getTargetContext();
        bluetoothManager = (BluetoothManager) instruCtx.getSystemService(Context.BLUETOOTH_SERVICE);
        mBluetoothAdapter = bluetoothManager.getAdapter();
        mBluetoothDevices = new ArrayList<BluetoothDevice>();
    }

    //For this test case to pass, Bluetooth, Location permission should be granted
    @Test
    @UiThreadTest
    public void Android_UT_ScanBle() throws InterruptedException {
        mBleScanningPresenter = new BLEScanningModel(BLEScanTest.this, mActivityTestRule.getActivity());
        mBleScanningPresenter.scanLeDevice(mBluetoothAdapter);
        Thread.sleep(6000);
        if(mBluetoothDevices != null)
            isScanTrue = true;
        assertTrue(isScanTrue);

    }

    @Test
    @UiThreadTest
    public void Android_UT_ScanBle_WithoutBoard() throws InterruptedException {
        mBleScanningPresenter = new BLEScanningModel(BLEScanTest.this, mActivityTestRule.getActivity());
        mBleScanningPresenter.scanLeDevice(mBluetoothAdapter);
        Thread.sleep(6000);
        new Handler().postDelayed(new Runnable() {
            @Override
            public void run() {
                if (mBluetoothDevices.size() == 0)
                    isScanTrue = true;
                assertTrue(isScanTrue);
            }
        }, 500);

    }

    @Test
    @UiThreadTest
    public void Android_UT_ScanBle_WithBoard() throws InterruptedException {
        mBleScanningPresenter = new BLEScanningModel(BLEScanTest.this, mActivityTestRule.getActivity());
        mBleScanningPresenter.scanLeDevice(mBluetoothAdapter);
        Thread.sleep(5000);
        new Handler().postDelayed(new Runnable() {
            @Override
            public void run() {
                if (mBluetoothDevices.size() > 0)
                    isScanTrue = true;
                assertTrue(isScanTrue);
            }
        }, 500);

    }



    @Override
    public void ScannedDevice(ArrayList<BluetoothDevice> bluetoothDevice) {
        mBluetoothDevices = bluetoothDevice;
        size = mBluetoothDevices.size();
        Logger.d("ppppppp",mBluetoothDevices.size()+"");
    }

}
