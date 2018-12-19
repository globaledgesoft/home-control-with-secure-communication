package com.globlaedge.cloud.blelibrary.blelibrary.Interface.Util;

import android.app.Activity;
import android.app.ProgressDialog;

public class ProgressDialogClass {

    public ProgressDialog dialog;
    private Activity mActivity;

    public ProgressDialogClass(Activity context) {
        this.mActivity = context;
        this.dialog = new ProgressDialog(mActivity);
    }

    public void createDialog(String dialogMsg) {
        dialog.setMessage(dialogMsg);
        dialog.setCancelable(false);
        if (mActivity != null && !mActivity.isFinishing()) {
            dialog.show();
        }

    }

    public void cancelDialog() {
        if (dialog != null) {
            dialog.dismiss();
        }
    }
}
