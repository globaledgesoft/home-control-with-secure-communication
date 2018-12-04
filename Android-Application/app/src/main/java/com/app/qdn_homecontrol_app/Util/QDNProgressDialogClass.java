package com.app.qdn_homecontrol_app.Util;

import android.app.ProgressDialog;
import android.content.Context;

public class QDNProgressDialogClass {

    public ProgressDialog dialog;
    public QDNProgressDialogClass(Context context) {
        this.dialog = new ProgressDialog(context);
    }

    public void createDialog(String dialogMsg) {
        dialog.setMessage(dialogMsg);
        dialog.setCancelable(false);
        dialog.show();


    }

    public void cancelDialog() {
        if (dialog != null) {
            dialog.dismiss();
        }
    }
}
