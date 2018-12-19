package com.globlaedge.cloud.blelibrary.blelibrary.Interface.Util;

import com.globlaedge.cloud.blelibrary.blelibrary.Interface.Service.BLEService;

public class BLEGattAttributes {

    private static BLEService mBLEService;

    public static BLEService getBLEService() {
        return mBLEService;
    }

    public static void setBLEService(BLEService bleService) {
        mBLEService = bleService;
    }
}
