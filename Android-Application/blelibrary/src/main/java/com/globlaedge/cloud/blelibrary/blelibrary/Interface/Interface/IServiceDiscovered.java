package com.globlaedge.cloud.blelibrary.blelibrary.Interface.Interface;

import android.bluetooth.BluetoothGattService;
import java.util.List;

/**
 * Interface for discovering the ble service
 */

public interface IServiceDiscovered {
    void onServiceDiscovered(List<BluetoothGattService> service);

}
