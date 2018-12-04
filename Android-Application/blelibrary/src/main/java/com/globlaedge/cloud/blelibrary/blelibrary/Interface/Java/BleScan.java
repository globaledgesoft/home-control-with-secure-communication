package com.globlaedge.cloud.blelibrary.blelibrary.Interface.Java;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanFilter;
import android.bluetooth.le.ScanResult;
import android.bluetooth.le.ScanSettings;
import android.content.Context;
import android.os.Build;
import android.os.Handler;
import android.os.Message;
import android.widget.Toast;
import com.globlaedge.cloud.blelibrary.blelibrary.Interface.Util.Constants;
import com.globlaedge.cloud.blelibrary.blelibrary.Interface.Util.Logger;
import com.globlaedge.cloud.blelibrary.blelibrary.Interface.Interface.ILeScanListResult;
import com.globlaedge.cloud.blelibrary.blelibrary.Interface.Util.ProgressDialogClass;
import com.globlaedge.cloud.blelibrary.blelibrary.R;
import java.util.ArrayList;
import java.util.List;
import java.util.Timer;
import java.util.TimerTask;

import static com.globlaedge.cloud.blelibrary.blelibrary.Interface.Util.Constants.SCAN_PERIOD;


public class BleScan implements Handler.Callback {

    private BluetoothLeScanner mLEScanner;
    private ScanSettings settings;
    private List<ScanFilter> filters;
    private static final String TAG = "BleScan";
    private final Context context;
    private final BluetoothAdapter bluetoothAdapter;
    private ArrayList<BluetoothDevice> bleDeviceList;
    private ProgressDialogClass dialog;
    private Handler mHandler;
    private ILeScanListResult mResult = null;

    private BluetoothAdapter.LeScanCallback mLeScanCallback =
            new BluetoothAdapter.LeScanCallback() {
                @Override
                public void onLeScan(final BluetoothDevice device, int rssi,
                                     byte[] scanRecord) {
                    if (!bleDeviceList.contains(device)) {
                        bleDeviceList.add(device);
                        Logger.d(TAG, "Device : " + device);

                    }

                }
            };

    private ScanCallback mScanCallback = new ScanCallback() {
        @Override
        public void onScanResult(int callbackType, ScanResult result) {
            BluetoothDevice btDevice = null;
            if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.LOLLIPOP) {
                btDevice = result.getDevice();
            }
            if (!bleDeviceList.contains(btDevice)) {
                bleDeviceList.add(btDevice);
                Logger.d(TAG, "Device : " + btDevice);

            }
        }

        @Override
        public void onBatchScanResults(List<ScanResult> results) {
            for (ScanResult sr : results) {
                Logger.i(TAG, sr.toString());
            }
        }

        @Override
        public void onScanFailed(int errorCode) {
            Logger.e(TAG, "Error Code: " + errorCode);
        }
    };


    // check for context type, only accept activity context, throw an exception and add throws statement
    public BleScan(Activity context) throws IllegalArgumentException {
        this.bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        this.context = context;
        this.dialog = new ProgressDialogClass(context);
        this.bleDeviceList = new ArrayList<BluetoothDevice>();
        mHandler = new Handler(context.getMainLooper(), this);
        if (Build.VERSION.SDK_INT >= 21) {
            mLEScanner = bluetoothAdapter.getBluetoothLeScanner();
            settings = new ScanSettings.Builder()
                    .setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY)
                    .build();
            filters = new ArrayList<ScanFilter>();
        }
    }

    /*
     * Scan ble device method
     */
    public void scanLeDevice(final long scanPeriod, ILeScanListResult mScanResult) {

        if (scanPeriod > Constants.SCAN_PERIOD) {
            Toast.makeText(context, R.string.scan_period_long, Toast.LENGTH_SHORT).show();
            return;
        }

        mResult = mScanResult;
        bleDeviceList.clear();

        //A task that can be scheduled for one-time or repeated execution by a Timer
        TimerTask task = new TimerTask() {
            @Override
            public void run() {
                dialog.cancelDialog();
                if (Build.VERSION.SDK_INT < 21) {
                    if (bluetoothAdapter != null) {
                        bluetoothAdapter.stopLeScan(mLeScanCallback);
                    }
                } else {
                    if(mLEScanner!=null) {
                        mLEScanner.stopScan(mScanCallback);
                    }

                }
                Message message = new Message();
                message.what = Constants.GET_DEVICE_LIST_ID;
                mHandler.sendMessage(message);


            }
        };
        new Timer().schedule(task, Constants.SCAN_PERIOD);

        if (Build.VERSION.SDK_INT < 21) {
            if(bluetoothAdapter!= null){
                dialog.createDialog(Constants.SCAN_DEVICE_DIALOG);
                bluetoothAdapter.startLeScan(mLeScanCallback);
            }

        } else {
            if(mLEScanner!=null) {
                dialog.createDialog(Constants.SCAN_DEVICE_DIALOG);
                mLEScanner.startScan(filters, settings, mScanCallback);
            }
        }

    }

    /*
     * handling the message from handler
     */
    @Override
    public boolean handleMessage(Message message) {
        switch (message.what) {
            case Constants.GET_DEVICE_LIST_ID:// sending the getDeviceList to UI
                mResult.onScanComplete(bleDeviceList);
                break;


        }
        return true;
    }

}
