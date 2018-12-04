package com.app.qdn_homecontrol_app.View;

import android.bluetooth.BluetoothDevice;

import java.util.ArrayList;

public interface BleScanningView {
    void ScannedDevice(ArrayList<BluetoothDevice> bluetoothDevice);
}
