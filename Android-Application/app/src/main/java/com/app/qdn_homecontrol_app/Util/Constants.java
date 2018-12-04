package com.app.qdn_homecontrol_app.Util;

import java.util.UUID;

public class Constants {

    public static final UUID SERVICE = UUID.fromString("91dcec60-6614-5486-784f-84829811f3a8");
    public static final UUID SMOKE_SENSOR_CHARACTERISTIC = UUID.fromString("abf9b671-e878-94ab-a84b-da9844897151");
    public static final UUID DESCRIPTOR_UUID = UUID.fromString("00002902-0000-1000-8000-00805f9b34fb");
    public static final UUID LOCK_UNLOCK_CHARECTERISTIC = UUID.fromString("aaf9b671-e878-94ab-a84b-da9844897151");
    public static final UUID BULB_SENSOR_CHARACTERISTIC = UUID.fromString("acf9b671-e878-94ab-a84b-da9844897151");
    public static final long SCAN_PERIOD = 5000;
    public static final String FONT = "qualcommnextregular.ttf";
    public static final int REQUEST_PERMISSIONS = 1;
    public static final String DEVICE_NAME = "device_name";
    public static final String MAC_ADDRESS = "mac_address";
    public static final int REQUEST_ID_MULTIPLE_PERMISSIONS = 25;
}
