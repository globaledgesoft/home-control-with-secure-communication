package com.globlaedge.cloud.blelibrary.blelibrary.Interface.Java;

import android.app.Activity;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGattService;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.AsyncTask;
import android.os.Handler;
import android.os.IBinder;
import android.util.Log;
import android.widget.Toast;
import com.globlaedge.cloud.blelibrary.blelibrary.Interface.Interface.IServiceDiscovered;
import com.globlaedge.cloud.blelibrary.blelibrary.Interface.Service.BLEService;
import com.globlaedge.cloud.blelibrary.blelibrary.Interface.Util.BLEGattAttributes;
import com.globlaedge.cloud.blelibrary.blelibrary.Interface.Util.Constants;
import com.globlaedge.cloud.blelibrary.blelibrary.Interface.Util.Logger;
import com.globlaedge.cloud.blelibrary.blelibrary.Interface.Util.ProgressDialogClass;
import com.globlaedge.cloud.blelibrary.blelibrary.R;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;

import static com.globlaedge.cloud.blelibrary.blelibrary.Interface.Util.Constants.DISCOVER_SERVICE_DIALOG;


public class ConnectBle {

    private static final String TAG = "ConnectBle";
    public static List<BluetoothGattService> gattServices;
    private Activity activity;
    private List<BluetoothGattService> bleDeviceService;
    private IServiceDiscovered discoveredResult = null;
    private BLEService mBLEService;
    private ProgressDialogClass serviceDiscoverDialog;
    private BluetoothDevice mDevice;
    private String mDeviceAddress ,passkey;
    private IntentFilter intentFilter ;
    private boolean mConnected = false;
    private static ConnectBle instance = null;

    public ConnectBle(Activity mActivity){
        this.activity = mActivity;

    }
    static public ConnectBle getInstance(Activity activity){
        if(instance == null){
            instance = new ConnectBle(activity);
            return instance;
        }
        else return instance;
    }

    private final ServiceConnection mServiceConnection = new ServiceConnection() {

        @Override
        public void onServiceConnected(ComponentName componentName, IBinder service) {
            mBLEService = ((BLEService.LocalBinder) service).getService();
            BLEGattAttributes.setBLEService(mBLEService);

            if (!mBLEService.initialize()) {
                Logger.e(TAG, "Unable to initialize Bluetooth");
            }

            Logger.d(TAG,"ServiceConnection"+ mDeviceAddress);

            //  Automatically connects to the device upon successful start-up initialization.
            boolean isConnected = mBLEService.connect(mDeviceAddress);
            try {
                if (isConnected) {
                    new BleConnectionTask().execute();
                    Logger.d(TAG, "Connected");
                    Toast.makeText(activity, "Connected", Toast.LENGTH_SHORT).show();
                } else {
                    Logger.d(TAG, "Not Connected");
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        @Override
        public void onServiceDisconnected(ComponentName componentName) {
            Toast.makeText(activity, R.string.service_disconnected, Toast.LENGTH_SHORT).show();

        }
    };

    private final BroadcastReceiver mGattUpdateReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            final String action = intent.getAction();

            if (BLEService.ACTION_GATT_CONNECTED.equals(action)) {
                mConnected = false;
            } else if (BLEService.ACTION_GATT_DISCONNECTED.equals(action)) {
                mConnected = true;
                Logger.d(TAG, "availableGattServices" + gattServices);
                gattServices.clear();
                Toast.makeText(context, R.string.disconnected, Toast.LENGTH_LONG).show();

            }
             else if (BLEService.ACTION_GATT_SERVICES_DISCOVERED.equals(action)) {
                availableGattServices();

                Logger.d(TAG, "ACTONGATSERVICEDISCvRD" + gattServices);
                mConnected = true;


            }

        }
    };

    public void connect(final Activity context, BluetoothDevice mDevice, IServiceDiscovered discovered) {
        discoveredResult = discovered;
        this.activity = context;
        this.mDevice = mDevice;
        mDeviceAddress = mDevice.getAddress();
        this.serviceDiscoverDialog = new ProgressDialogClass(context);
        this.bleDeviceService = new ArrayList<>();
        gattServices = new ArrayList<>();
        Logger.d("ConnectBleConstructor", mDeviceAddress);

        bleConnect(mDevice);

    }

    private void bleConnect(final BluetoothDevice device) {

        if (mDevice.getBondState() == BluetoothDevice.BOND_BONDED) {
//            activity.unregisterReceiver(receiver);
            getService();
        }
        else {
            new Handler().postDelayed(new Runnable() {
                @Override
                public void run() {
                    if(!connectDevice(device)){
                        Toast.makeText(activity, R.string.toast_not_connected, Toast.LENGTH_SHORT).show();
                    }
                    serviceDiscoverDialog.cancelDialog();
                }
            },Constants.CONNECT_PERIOD);
            connectDevice(device);

        }
    }

    private void getService() {
        Intent gattServiceIntent = new Intent(activity, BLEService.class);
        activity.bindService(gattServiceIntent, mServiceConnection, activity.BIND_AUTO_CREATE);
        activity.registerReceiver(mGattUpdateReceiver, makeGattUpdateIntentFilter());
    }

    public  void unBindService(){
        activity.unbindService(mServiceConnection);

    }


    BroadcastReceiver receiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();

            if (BluetoothDevice.ACTION_PAIRING_REQUEST.equals(action)) {
                BluetoothDevice mBluetoothDevice = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
                if (mBluetoothDevice != null) {

                    try {
                        byte[] pin = (byte[]) BluetoothDevice.class.getMethod("convertPinToBytes", String.class).invoke(BluetoothDevice.class, passkey);
                        Method m = mBluetoothDevice.getClass().getMethod("setPin", byte[].class);
                        m.invoke(mBluetoothDevice, pin);
                    } catch (Exception e) {

                        e.printStackTrace();

                    }
                }
            }

            if (action.equals(BluetoothDevice.ACTION_BOND_STATE_CHANGED)) {
                int state = intent.getIntExtra(BluetoothDevice.EXTRA_BOND_STATE, -1);

                if (state < 0) {
                    Toast.makeText(context, "Please try again!", Toast.LENGTH_LONG).show();
                    //we should never get here
                } else if (state == BluetoothDevice.BOND_BONDING) {
                    //bonding process is still working
                    //essentially this means that the Confirmation Dialog is still visible

                    Logger.d(TAG, "Start Pairing...");


                }
                else if (state == BluetoothDevice.BOND_BONDED) {
                    Logger.d(TAG, "Paired");
//                    activity.unregisterReceiver(receiver);
                    getService();

                }
                else if (state == BluetoothDevice.BOND_NONE) {
                    Logger.d(TAG, "not Paired");

                }
            }
        }
    };

    private boolean connectDevice(BluetoothDevice device) {
        boolean isConnected = false;
        try {
            intentFilter = new IntentFilter();
            intentFilter.addAction(BluetoothDevice.ACTION_PAIRING_REQUEST);
            intentFilter.addAction(BluetoothDevice.ACTION_BOND_STATE_CHANGED);
            activity.registerReceiver(receiver, intentFilter);
            serviceDiscoverDialog.createDialog(Constants.PAIRING);
            isConnected = true;
            Method mBond = device.getClass().getMethod("createBond", (Class[]) null);
            mBond.invoke(device, (Object[]) null);

        } catch (Exception e) {
            e.printStackTrace();
            isConnected = false;
        }
        return isConnected;
    }

    public static void unpairDevice(BluetoothDevice device) {
        try {
            Method m = device.getClass()
                    .getMethod("removeBond", (Class[]) null);
            m.invoke(device, (Object[]) null);
        } catch (Exception e) {
            Log.e(TAG, e.getMessage());
        }
    }

    private static IntentFilter makeGattUpdateIntentFilter() {
        final IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(BLEService.ACTION_GATT_CONNECTED);
        intentFilter.addAction(BLEService.ACTION_GATT_DISCONNECTED);
        intentFilter.addAction(BLEService.ACTION_GATT_SERVICES_DISCOVERED);
        return intentFilter;
    }

    public void availableGattServices() {

        gattServices = mBLEService.getSupportedGattServices();
        bleDeviceService = gattServices;
        if(gattServices != null)
        {
            bleDeviceService = gattServices;
            discoveredResult.onServiceDiscovered(bleDeviceService);
        }
        else
        {
            Toast.makeText(activity,R.string.toast_service_not_discovered,Toast.LENGTH_SHORT).show();
        }


    }

    public void unRegisterRecievr(){
        activity.unregisterReceiver(mGattUpdateReceiver);
    }


    private class BleConnectionTask extends AsyncTask<Void, Void, Boolean> {

        @Override
        protected void onPreExecute() {
            super.onPreExecute();
            serviceDiscoverDialog.createDialog(DISCOVER_SERVICE_DIALOG);


        }

        protected Boolean doInBackground(Void... params) {
            if (mBLEService != null) {
                while (!mConnected) {
                    //Log.d("log", "waiting for services");
                }

                Logger.d(TAG, "found service");

            }
            return true;
        }

        protected void onPostExecute(Boolean hasActiveConnection) {
            serviceDiscoverDialog.cancelDialog();


        }
    }


}
