package com.app.qdn_homecontrol_app;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.IntentFilter;
import android.support.test.InstrumentationRegistry;
import android.support.test.annotation.UiThreadTest;
import android.support.test.rule.ActivityTestRule;
import android.support.test.runner.AndroidJUnit4;
import android.util.Log;

import com.app.qdn_homecontrol_app.Activity.BleDeviceListActivity;
import com.app.qdn_homecontrol_app.Activity.DeviceDetailsActivity;
import com.app.qdn_homecontrol_app.Model.BLEScanningModel;
import com.app.qdn_homecontrol_app.Presenter.BleScanningPresenter;
import com.app.qdn_homecontrol_app.Util.Constants;
import com.app.qdn_homecontrol_app.View.BleScanningView;
import com.globlaedge.cloud.blelibrary.blelibrary.Interface.Interface.IServiceDiscovered;
import com.globlaedge.cloud.blelibrary.blelibrary.Interface.Java.ConnectBle;
import com.globlaedge.cloud.blelibrary.blelibrary.Interface.Service.BLEService;
import com.globlaedge.cloud.blelibrary.blelibrary.Interface.Service.BleServiceCharecteristic;
import com.globlaedge.cloud.blelibrary.blelibrary.Interface.Util.Logger;

import org.junit.After;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;

import java.util.ArrayList;
import java.util.List;

import static android.support.test.espresso.Espresso.onView;
import static android.support.test.espresso.action.ViewActions.click;
import static android.support.test.espresso.assertion.ViewAssertions.matches;
import static android.support.test.espresso.matcher.ViewMatchers.withId;
import static android.support.test.espresso.matcher.ViewMatchers.withText;
import static junit.framework.TestCase.assertEquals;
import static junit.framework.TestCase.assertTrue;

@RunWith(AndroidJUnit4.class)

public class DeviceDetailsTests implements BleScanningView {
    private Context instruCtx;
    private BluetoothDevice device;
    private BleScanningPresenter mBleScanningPresenter;
    private BluetoothManager bluetoothManager;
    private BluetoothAdapter mBluetoothAdapter;
    private boolean isServiceDiscovered;
    private BleDeviceListActivity bleDeviceListActivity;
    private DeviceDetailsActivity deviceDetailsActivity;
    private String macAdress = "00:00:1D:12:34:56";
    private String macAdress_21 = "21:98:99:F0:FD:8C";
    private boolean isPositiveTest;
    private ArrayList<BluetoothDevice> mBluetoothDevices;
    private BroadcastReceiver notificationReceiver;
    private ConnectBle mConnectBle;

    @Rule
    public ActivityTestRule<BleDeviceListActivity> mActivityBleList = new ActivityTestRule<>(BleDeviceListActivity.class);
    public ActivityTestRule<DeviceDetailsActivity> mActivityDeviceDetails = new ActivityTestRule<>(DeviceDetailsActivity.class);



    @Before
    public void setUp() {
        instruCtx = InstrumentationRegistry.getTargetContext();
        bluetoothManager = (BluetoothManager) instruCtx.getSystemService(Context.BLUETOOTH_SERVICE);
        mBluetoothAdapter = bluetoothManager.getAdapter();
        bleDeviceListActivity = mActivityBleList.getActivity();
        mConnectBle = ConnectBle.getInstance(bleDeviceListActivity);
    }

    public void scanBle() {
        mBleScanningPresenter = new BLEScanningModel(DeviceDetailsTests.this,mActivityBleList.getActivity());
        mBleScanningPresenter.scanLeDevice(mBluetoothAdapter);
    }

    @Test
    @UiThreadTest
    public void Android_UT_lockServoMotor() throws  InterruptedException{
        isPositiveTest = true;
        scanBle();
        Thread.sleep(5000);
        new android.os.Handler().postDelayed(new Runnable() {
            @Override
            public void run() {
                Logger.d("devicedevice", device.getAddress() + "");
                mConnectBle.connect(bleDeviceListActivity, device, new IServiceDiscovered() {
                    @Override
                    public void onServiceDiscovered(List<BluetoothGattService> service) {

                        try {
                            String value = null;
                            Logger.d("onServiceDiscovered", service + "");
                            isServiceDiscovered = true;
                            byte[] obj = {1};
                            mActivityDeviceDetails.getActivity().write(obj, deviceDetailsActivity.getString(R.string.unlocking),Constants.LOCK_UNLOCK_CHARECTERISTIC);
                            Thread.sleep(3000);
                            value = BleServiceCharecteristic.getInstance().readStatus(Constants.SERVICE, Constants.LOCK_UNLOCK_CHARECTERISTIC,deviceDetailsActivity);
                            Thread.sleep(2000);
                            value = BleServiceCharecteristic.getInstance().readStatus(Constants.SERVICE, Constants.LOCK_UNLOCK_CHARECTERISTIC,deviceDetailsActivity);
                            assertEquals("1",value);
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                    }
                });
            }
        }, 2000);
        Thread.sleep(12000);

    }
    @Test
    @UiThreadTest
    public void Android_UT_unLockServoMotor() throws  InterruptedException{
        isPositiveTest = true;
        scanBle();
        Thread.sleep(5000);
        new android.os.Handler().postDelayed(new Runnable() {
            @Override
            public void run() {
                Logger.d("devicedevice", device.getAddress() + "");
                mConnectBle.connect(bleDeviceListActivity, device, new IServiceDiscovered() {
                    @Override
                    public void onServiceDiscovered(List<BluetoothGattService> service) {

                        try {
                            String value = null;
                            Logger.d("onServiceDiscovered", service + "");
                            isServiceDiscovered = true;
                            byte[] obj = {0};
                            mActivityDeviceDetails.getActivity().write(obj, deviceDetailsActivity.getString(R.string.unlocking),Constants.LOCK_UNLOCK_CHARECTERISTIC);
                            Thread.sleep(3000);
                            value = BleServiceCharecteristic.getInstance().readStatus(Constants.SERVICE, Constants.LOCK_UNLOCK_CHARECTERISTIC,deviceDetailsActivity);
                            Thread.sleep(2000);
                            value = BleServiceCharecteristic.getInstance().readStatus(Constants.SERVICE, Constants.LOCK_UNLOCK_CHARECTERISTIC,deviceDetailsActivity);
                            assertEquals("0",value);
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                    }
                });
            }
        }, 2000);
        Thread.sleep(10000);

    }

    @Test
    @UiThreadTest
    public void Android_UT_bulbOff() throws  InterruptedException{
        isPositiveTest = true;
        scanBle();
        Thread.sleep(5000);
        new android.os.Handler().postDelayed(new Runnable() {
            @Override
            public void run() {
                Logger.d("devicedevice", device.getAddress() + "");
                mConnectBle.connect(bleDeviceListActivity, device, new IServiceDiscovered() {
                    @Override
                    public void onServiceDiscovered(List<BluetoothGattService> service) {

                        try {
                            String value = null;
                            Logger.d("onServiceDiscovered", service + "");
                            isServiceDiscovered = true;
                            byte[] obj = {0};
                            mActivityDeviceDetails.getActivity().write(obj, deviceDetailsActivity.getString(R.string.unlocking),Constants.BULB_SENSOR_CHARACTERISTIC);
                            Thread.sleep(3000);
                            value = BleServiceCharecteristic.getInstance().readStatus(Constants.SERVICE, Constants.BULB_SENSOR_CHARACTERISTIC,deviceDetailsActivity);
                            Thread.sleep(2000);
                            value = BleServiceCharecteristic.getInstance().readStatus(Constants.SERVICE, Constants.BULB_SENSOR_CHARACTERISTIC,deviceDetailsActivity);
                            assertEquals("0",value);
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                    }
                });
            }
        }, 2000);
        Thread.sleep(10000);

    }
    @Test
    @UiThreadTest
    public void Android_UT_bulbOn() throws  InterruptedException{
        isPositiveTest = true;
        scanBle();
        Thread.sleep(5000);
        new android.os.Handler().postDelayed(new Runnable() {
            @Override
            public void run() {

                Logger.d("devicedevice", device.getAddress() + "");
                mConnectBle.connect(bleDeviceListActivity, device, new IServiceDiscovered() {
                    @Override
                    public void onServiceDiscovered(List<BluetoothGattService> service) {

                        try {
                            String value = null;
                            Logger.d("onServiceDiscovered", service + "");
                            isServiceDiscovered = true;
                            byte[] obj = {1};
                            mActivityDeviceDetails.getActivity().write(obj, deviceDetailsActivity.getString(R.string.unlocking),Constants.BULB_SENSOR_CHARACTERISTIC);
                            Thread.sleep(3000);
                            value = BleServiceCharecteristic.getInstance().readStatus(Constants.SERVICE, Constants.BULB_SENSOR_CHARACTERISTIC,deviceDetailsActivity);
                            Thread.sleep(2000);
                            value = BleServiceCharecteristic.getInstance().readStatus(Constants.SERVICE, Constants.BULB_SENSOR_CHARACTERISTIC,deviceDetailsActivity);

                            assertEquals("1",value);
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                    }
                });
            }
        }, 2000);
        Thread.sleep(10000);

    }

    @Test
    @UiThreadTest
    public void Android_UT_notify() throws  InterruptedException{

        registerReceiver();

        isPositiveTest = true;
        scanBle();
        Thread.sleep(5000);
        new android.os.Handler().postDelayed(new Runnable() {
            @Override
            public void run() {

                Logger.d("devicedevice", device.getAddress() + "");
                mConnectBle.connect(bleDeviceListActivity, device, new IServiceDiscovered() {
                    @Override
                    public void onServiceDiscovered(List<BluetoothGattService> service) {

                        try {
                            String value = null;
                            Logger.d("onServiceDiscovered", service + "");
                            isServiceDiscovered = true;
                            BleServiceCharecteristic.getInstance().notifyData(Constants.SERVICE, Constants.SMOKE_SENSOR_CHARACTERISTIC);
                            Thread.sleep(3000);
                            mActivityDeviceDetails.getActivity().notification();
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                    }
                });
            }
        }, 2000);
        Thread.sleep(10000);
    }

    private void registerReceiver() {
        notificationReceiver = mActivityDeviceDetails.getActivity().notification();
        final IntentFilter notificationIntentFilter = new IntentFilter();
        notificationIntentFilter.addAction(BLEService.ACTION_BULB_DATA_AVAILABLE);
        notificationIntentFilter.addAction(BLEService.ACTION_SMOKE_DATA_AVAILABLE);
        notificationIntentFilter.addAction(BLEService.ACTION_GATT_DISCONNECTED);
        mActivityDeviceDetails.getActivity().registerReceiver(notificationReceiver, notificationIntentFilter);
    }


    @Override
    public void ScannedDevice(ArrayList<BluetoothDevice> bluetoothDevice) {
        for (BluetoothDevice bleDevice : bluetoothDevice)
        {
            if(isPositiveTest) {
                if (bleDevice.getAddress().equals(macAdress_21))
                    device = bleDevice;
            }
            else
            if(bleDevice.getAddress().equals("00:97:99:F0:FD:8C"))
                device = bleDevice;
        }

        Log.d("devices.....",device+"");

    }

    @After
    public void tearDown()
    {
        BLEService.bleDisconnection();
    }
}
