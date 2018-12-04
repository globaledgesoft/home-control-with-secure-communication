package com.globlaedge.cloud.blelibrary.blelibrary.Interface.Service;

import android.app.Activity;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCharacteristic;
import android.content.Context;
import android.widget.Toast;
import com.globlaedge.cloud.blelibrary.blelibrary.Interface.Util.BLEGattAttributes;
import com.globlaedge.cloud.blelibrary.blelibrary.Interface.Util.Logger;
import com.globlaedge.cloud.blelibrary.blelibrary.R;
import java.util.Arrays;
import java.util.UUID;


public class BleServiceCharecteristic {
    private static final String TAG = "BleServiceCharecteristic";
    private Context context;
    static BluetoothGatt mBluetoothGatt;
    private static BLEService mBLEService;
    private String status = "";


    private static BleServiceCharecteristic mBleServiceCharecteristic = null;

    public static BleServiceCharecteristic getInstance() {
        if(mBleServiceCharecteristic == null) {

            mBleServiceCharecteristic = new BleServiceCharecteristic();
        }
        if(BLEGattAttributes.getBLEService() != null) {
            mBLEService = BLEGattAttributes.getBLEService();
            if(mBLEService.getmBluetoothGatt() != null) {
                mBluetoothGatt = mBLEService.getmBluetoothGatt();
            }
        }
        return mBleServiceCharecteristic;
    }




    public void writeData(byte[] val,UUID SERVICE, UUID CHARECTERISTIC)
    {
        try {
            BluetoothGattCharacteristic characteristicSsid =  mBluetoothGatt.getService(SERVICE).getCharacteristic(CHARECTERISTIC);
            characteristicSsid.setValue(val);
            mBLEService.writeCharacteristic(characteristicSsid);
        }catch (Exception e)
        {
            if(context != null){
                Toast.makeText(context, R.string.toast_write_failed,Toast.LENGTH_SHORT).show();
            }

        }

    }

    public String readStatus(UUID SERVICE, UUID CHARECTERISTIC, Activity context) {

        try {
            if (mBluetoothGatt != null) {
                BluetoothGattCharacteristic characteristic = mBluetoothGatt.getService(SERVICE).getCharacteristic(CHARECTERISTIC);
                mBLEService.readCharacteristic(characteristic);
                byte[] value = characteristic.getValue();
                Logger.i(TAG,value+"");
                if(!(Arrays.toString(value).contains("null"))){
                    status = convert(value);
                }

                //newline
                Logger.i(TAG,status);
            }
        }catch (Exception e)
        {
            Toast.makeText(context,R.string.toast_read_failed,Toast.LENGTH_SHORT).show();
            e.printStackTrace();

        }
        return status;

    }
    String convert(byte[] data) {
        if (data == null) {
            return null;

        }
        StringBuilder sb = new StringBuilder(data.length);
        for (int i = 0; i <data.length ; ++i) {
            if (data[i] < 0) throw new IllegalArgumentException();
            Long decimal=Long.parseLong(data[i]+"",16);

            sb.append(decimal);
        }
        return sb.toString();
    }

    public void notifyData(UUID SERVICE, UUID CHARACTERISTIC) {
        try {
            if (mBluetoothGatt != null) {
                BluetoothGattCharacteristic characteristic = mBluetoothGatt.getService(SERVICE).getCharacteristic(CHARACTERISTIC);
                mBLEService.setCharacteristicNotification(characteristic, true);
                mBLEService.readCharacteristic(characteristic);
            }
        } catch (Exception e) {
            Logger.d(TAG, "Exception in notifyData() -> " + e);
        }
    }

    public void notifyReadData(UUID SERVICE, UUID CHARACTERISTIC) {
        try {
            if (mBluetoothGatt != null) {
                BluetoothGattCharacteristic characteristic = mBluetoothGatt.getService(SERVICE).getCharacteristic(CHARACTERISTIC);
               // mBLEService.setCharacteristicNotification(characteristic, true);
                mBLEService.readCharacteristic(characteristic);
            }
        } catch (Exception e) {
            Logger.d(TAG, "Exception in notifyData() -> " + e);
        }
    }
}
