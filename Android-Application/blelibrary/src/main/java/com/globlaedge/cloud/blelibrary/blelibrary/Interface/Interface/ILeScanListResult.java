package com.globlaedge.cloud.blelibrary.blelibrary.Interface.Interface;

import android.bluetooth.BluetoothDevice;

import java.util.ArrayList;

/**
 * Interface for getting the device list
 */

public interface ILeScanListResult {

    void onScanComplete(ArrayList<BluetoothDevice> deviceList);

}
