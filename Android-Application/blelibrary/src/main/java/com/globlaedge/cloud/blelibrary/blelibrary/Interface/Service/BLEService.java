package com.globlaedge.cloud.blelibrary.blelibrary.Interface.Service;

import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.content.Context;
import android.content.Intent;
import android.os.Binder;
import android.os.IBinder;
import android.util.Log;
import com.globlaedge.cloud.blelibrary.blelibrary.Interface.Util.Logger;
import java.io.UnsupportedEncodingException;
import java.util.List;
import java.util.UUID;

/**
 * Ble Service class for getting the ble service and read, write, notification charecteristics
 */

public class BLEService extends Service {

    public final static String ACTION_GATT_CONNECTED =
            "ACTION_GATT_CONNECTED";
    public final static String ACTION_GATT_DISCONNECTED =
            "ACTION_GATT_DISCONNECTED";
    public final static String ACTION_GATT_SERVICES_DISCOVERED =
            "ACTION_GATT_SERVICES_DISCOVERED";

    public final static String ACTION_SMOKE_DATA_AVAILABLE =
            "ACTION_SMOKE_DATA_AVAILABLE";
    public final static String ACTION_BULB_DATA_AVAILABLE =
            "ACTION_BULB_DATA_AVAILABLE";
    public final static String SMOKE_DATA =
            "SMOKE_DATA";
    public final static String BULB_DATA =
            "BULB_DATA";
    public final static String DISCONNECTION_STATUS =
            "DISCONNECTION_STATUS";
    private static final String TAG = "BLEServiceClass";
    private static final int STATE_DISCONNECTED = 0;
    private static final int STATE_CONNECTING = 1;
    private static final int STATE_CONNECTED = 2;
    private static BluetoothGatt mBluetoothGatt;
    private static BLEService mBleService = null;
    private final IBinder mBinder = new LocalBinder();
    private BluetoothManager mBluetoothManager;
    private BluetoothAdapter mBluetoothAdapter;
    private String mBluetoothDeviceAddress;
    private int mConnectionState = STATE_DISCONNECTED;
    private static final UUID CLIENT_CHARACTERISTIC_CONFIG_DESCRIPTOR_UUID = UUID.fromString("00002902-0000-1000-8000-00805f9b34fb");
    public static final UUID SMOKE_SENSOR_CHARACTERISTIC = UUID.fromString("abf9b671-e878-94ab-a84b-da9844897151");

    // Implements callback methods for GATT events that the app cares about.  For example,
    // connection change and services discovered.
    private final BluetoothGattCallback mGattCallback = new BluetoothGattCallback() {

        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            String intentAction;

            if (newState == BluetoothProfile.STATE_CONNECTED) {
                intentAction = ACTION_GATT_CONNECTED;
                mConnectionState = STATE_CONNECTED;
                broadcastUpdate(intentAction);
                Logger.i(TAG, "Connected to GATT server.");
                // Attempts to discover services after successful connection.
                Logger.i(TAG, "Attempting to start service discovery:" +
                        mBluetoothGatt.discoverServices());

            } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                intentAction = ACTION_GATT_DISCONNECTED;
                mConnectionState = STATE_DISCONNECTED;
                Logger.i(TAG, "Disconnected from GATT server.");
                broadcastUpdate(intentAction);
            } else if (newState == BluetoothProfile.STATE_CONNECTING) {
                Logger.i(TAG, "connecting from GATT server.");

            }
        }

        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            if (status == BluetoothGatt.GATT_SUCCESS) {
                broadcastUpdate(ACTION_GATT_SERVICES_DISCOVERED);
            } else {
                Logger.i(TAG, "onServicesDiscovered received: " + status);
            }
        }

        @Override
        public void onCharacteristicRead(BluetoothGatt gatt,
                                         BluetoothGattCharacteristic characteristic,
                                         int status) {
            if (status == BluetoothGatt.GATT_SUCCESS) {
//                broadcastUpdate(ACTION_DATA_AVAILABLE, characteristic);
            }
        }


        @Override
        public void onCharacteristicWrite(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
            byte[] bytes = characteristic.getValue();
            try {
//                broadcastUpdate(ACTION_DATA_AVAILABLE, characteristic);
                gatt.readCharacteristic(characteristic);
                Logger.d(TAG, new String(bytes, "UTF-8"));
            } catch (UnsupportedEncodingException e) {
                e.printStackTrace();
            } finally {
            }
        }

        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt,
                                            BluetoothGattCharacteristic characteristic) {
            String str = null;
            try {
                str = new String(characteristic.getValue(), "UTF-8");

            } catch (UnsupportedEncodingException e) {
                e.printStackTrace();
            }
            if (str != null)
                if(characteristic.getUuid().equals(SMOKE_SENSOR_CHARACTERISTIC)) {
                    broadcastUpdate(ACTION_SMOKE_DATA_AVAILABLE, characteristic);
                }else {
                broadcastUpdate(ACTION_BULB_DATA_AVAILABLE,characteristic);
                }
        }
    };

    /*
     * getting an instance of bleService
     */
    public static BLEService getInstance() {
        if (mBleService == null) {
            mBleService = new BLEService();
        }
        return mBleService;
    }

    public static BluetoothGatt getmBluetoothGatt() {
        return mBluetoothGatt;
    }

    public static void bleDisconnection() {
        BLEService.getInstance().disconnect();

    }

    public void close() {
        if (mBluetoothGatt == null) {
            return;
        }
        mBluetoothGatt.close();
        mBluetoothGatt = null;
    }


    /*
     * enabling the ble notification
     */
    public void setCharacteristicNotification(BluetoothGattCharacteristic characteristic,
                                              boolean enabled) {
        if (mBluetoothAdapter == null || mBluetoothGatt == null) {
            Log.w(TAG, "BluetoothAdapter not initialized");
            return;
        }
        mBluetoothGatt.setCharacteristicNotification(characteristic, enabled);
        BluetoothGattDescriptor descriptor = characteristic.getDescriptor(
                CLIENT_CHARACTERISTIC_CONFIG_DESCRIPTOR_UUID);
        descriptor.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
        mBluetoothGatt.writeDescriptor(descriptor);
    }


    public void readCharacteristic(BluetoothGattCharacteristic characteristic) {
        if (mBluetoothAdapter == null || mBluetoothGatt == null) {
            Log.i(TAG, "BluetoothAdapter not initialized");
            return;
        }
        mBluetoothGatt.readCharacteristic(characteristic);
        byte[] charValue = characteristic.getValue();
        if(charValue != null)
            Logger.i(TAG, "readCharacteristic BluetoothChar "+ charValue.toString());
        else {
            Logger.i(TAG, "readCharacteristic BluetoothChar not received");
        }
//        characteristic.getValue();
    }

    public void writeCharacteristic(final BluetoothGattCharacteristic characteristic) {
        if (mBluetoothAdapter == null || mBluetoothGatt == null) {
            return;
        }
        mBluetoothGatt.writeCharacteristic(characteristic);
    }

    private void broadcastUpdate(final String action) {
        final Intent intent = new Intent(action);
        intent.putExtra(DISCONNECTION_STATUS,"disconnection");
        sendBroadcast(intent);
    }


    private void broadcastUpdate(final String action,
                                 final BluetoothGattCharacteristic characteristic) {
        int smokeVal = 0;
        /*int byte_data = 0;
        final byte[] data = characteristic.getValue();*/
        int bulbData = 0;

        if(action.equals(ACTION_SMOKE_DATA_AVAILABLE)) {

            final Intent smokeIntent = new Intent(action);
            smokeVal = characteristic.getIntValue(BluetoothGattCharacteristic.FORMAT_UINT32
                    ,0);
            smokeIntent.putExtra(SMOKE_DATA, String.valueOf(smokeVal));
            sendBroadcast(smokeIntent);

            Logger.i(TAG, "Smoke Data is : " + smokeVal);

            /*if (data != null && data.length > 0) {
                for (int i = 0; i < data.length; i++) {
                    byte_data = data[i];
                    smokeVal = smokeVal | byte_data << (i * 8);
                }
                Logger.d("Smoke detector val", "byte :" + smokeVal);
                smokeIntent.putExtra(SMOKE_DATA, String.valueOf(smokeVal));
                sendBroadcast(smokeIntent);
            }*/
        }
        else if (action.equals(ACTION_BULB_DATA_AVAILABLE)) {
            final Intent bulbIntent = new Intent(action);
            bulbData = characteristic.getIntValue(BluetoothGattCharacteristic.FORMAT_UINT32,
                    0);
            Logger.d(TAG, "Bulb status is : " +bulbData);
            bulbIntent.putExtra(BULB_DATA, String.valueOf(bulbData));
            sendBroadcast(bulbIntent);
            }
        }

    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;
    }

    @Override
    public boolean onUnbind(Intent intent) {
        // After using a given device, you should make sure that BluetoothGatt.close() is called
        // such that resources are cleaned up properly.  In this particular example, close() is
        // invoked when the UI is disconnected from the Service.
        close();
        return super.onUnbind(intent);
    }

    public boolean initialize() {
        // For API level 18 and above, get a reference to BluetoothAdapter through
        // BluetoothManager.
        if (mBluetoothManager == null) {
            mBluetoothManager = (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
            if (mBluetoothManager == null) {
                Logger.i(TAG, "Unable to initialize BluetoothManager.");
                return false;
            }
        }

        mBluetoothAdapter = mBluetoothManager.getAdapter();
        if (mBluetoothAdapter == null) {
            Logger.i(TAG, "Unable to obtain a BluetoothAdapter.");
            return false;
        }

        return true;
    }

    public List<BluetoothGattService> getSupportedGattServices() {
        if (mBluetoothGatt == null) return null;

        return mBluetoothGatt.getServices();
    }

    public boolean connect(final String address) {
        if (mBluetoothAdapter == null || address == null) {
            Logger.i(TAG, "BluetoothAdapter not initialized or unspecified address.");
            return false;
        }

        // Previously connected device.  Try to reconnect.
        if (mBluetoothDeviceAddress != null && address.equals(mBluetoothDeviceAddress)
                && mBluetoothGatt != null) {
            Logger.i(TAG, "Trying to use an existing mBluetoothGatt for connection.");
            if (mBluetoothGatt.connect()) {
                mConnectionState = STATE_CONNECTING;
                return true;
            } else {
                return false;
            }
        }

        final BluetoothDevice device = mBluetoothAdapter.getRemoteDevice(address);
        if (device == null) {
            Logger.i(TAG, "Device not found.  Unable to connect.");
            return false;
        }
        // We want to directly connect to the device, so we are setting the autoConnect
        // parameter to false.
        mBluetoothGatt = device.connectGatt(this, false, mGattCallback);
        Logger.i(TAG, "Trying to create a new connection.");
        mBluetoothDeviceAddress = address;
        mConnectionState = STATE_CONNECTING;
        return true;
    }

    public void disconnect() {
        if (mBluetoothGatt == null) {
            return;
        }
        mBluetoothGatt.disconnect();
    }

    public class LocalBinder extends Binder {
        public BLEService getService() {
            return BLEService.this;
        }
    }

}
