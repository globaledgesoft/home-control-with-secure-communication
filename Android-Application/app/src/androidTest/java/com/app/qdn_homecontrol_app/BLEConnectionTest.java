package com.app.qdn_homecontrol_app;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGattService;
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
import com.globlaedge.cloud.blelibrary.blelibrary.Interface.Interface.IServiceDiscovered;
import com.globlaedge.cloud.blelibrary.blelibrary.Interface.Java.ConnectBle;
import com.globlaedge.cloud.blelibrary.blelibrary.Interface.Service.BLEService;
import com.globlaedge.cloud.blelibrary.blelibrary.Interface.Util.Logger;

import org.junit.After;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;

import java.util.ArrayList;
import java.util.List;
import static junit.framework.TestCase.assertFalse;
import static junit.framework.TestCase.assertTrue;

@RunWith(AndroidJUnit4.class)
public class BLEConnectionTest implements BleScanningView {
    private Context instruCtx;
    private BluetoothDevice device;
    private BleScanningPresenter mBleScanningPresenter;
    private BluetoothManager bluetoothManager;
    private BluetoothAdapter mBluetoothAdapter;
    private boolean isServiceDiscovered;
    private BleDeviceListActivity activity;
    private String macAdress = "00:00:1D:12:34:56";
    private boolean isPositiveTest;
    private ConnectBle mConnectBle;

    @Rule
    public ActivityTestRule<BleDeviceListActivity> mActivityTestRule = new ActivityTestRule<>(BleDeviceListActivity.class);


    @Before
    public void setUp() {
        instruCtx = InstrumentationRegistry.getTargetContext();
        bluetoothManager = (BluetoothManager) instruCtx.getSystemService(Context.BLUETOOTH_SERVICE);
        mBluetoothAdapter = bluetoothManager.getAdapter();
        activity = mActivityTestRule.getActivity();
        mConnectBle = ConnectBle.getInstance(activity);


    }

    public void scanBle() {
        mBleScanningPresenter = new BLEScanningModel(BLEConnectionTest.this, mActivityTestRule.getActivity());
        mBleScanningPresenter.scanLeDevice(mBluetoothAdapter);
    }

    @Test
    @UiThreadTest
    public void Android_UT_ConnectAndDiscoverBle_positive() throws InterruptedException {
        isPositiveTest = true;
        scanBle();
        Thread.sleep(5000);
        Logger.d("devicedevice", device + "");

        new android.os.Handler().postDelayed(new Runnable() {
            @Override
            public void run() {
                Logger.d("devicedevice", device + "");
                mConnectBle.connect(activity, device, new IServiceDiscovered() {
                    @Override
                    public void onServiceDiscovered(List<BluetoothGattService> service) {
                        Logger.d("onServiceDiscovered", service + "");
                        isServiceDiscovered = true;
                        assertTrue(isServiceDiscovered);
                    }
                });
            }
        }, 2000);
        Thread.sleep(10000);

    }

    @Test
    @UiThreadTest
    public void Android_UT_ConnectAndDiscoverBle_negative() throws InterruptedException {
        isPositiveTest = false;
        scanBle();
        Thread.sleep(5000);
        Logger.d("devicedevice", device + "");

        new android.os.Handler().postDelayed(new Runnable() {
            @Override
            public void run() {
                Logger.d("devicedevice", device + "");
                mConnectBle.connect(activity, device, new IServiceDiscovered() {
                    @Override
                    public void onServiceDiscovered(List<BluetoothGattService> service) {
                        Logger.d("onServiceDiscovered", service + "");
                        isServiceDiscovered = true;
                        assertTrue(isServiceDiscovered);
                    }
                });
            }
        }, 2000);
        Thread.sleep(10000);
        assertFalse(isServiceDiscovered);

    }


    @Override
    public void ScannedDevice(ArrayList<BluetoothDevice> bluetoothDevice) {
        for (BluetoothDevice bleDevice : bluetoothDevice)
        {
            if(isPositiveTest) {
                if (bleDevice.getAddress().equals(macAdress))
                    device = bleDevice;
            }
            else
            if(bleDevice.getAddress().equals("00:97:99:F0:FD:8C"))
                device = bleDevice;
        }

    }

    @After
    public void tearDown()
    {
        BLEService.bleDisconnection();
    }
}