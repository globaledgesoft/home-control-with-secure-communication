package com.app.qdn_homecontrol_app.Util;

import android.util.Log;


/**
 *
 * Generally, use the QDNLogger.v() QDNLogger.d() QDNLogger.i() QDNLogger.w() and QDNLogger.e()
 * methods.
 *
 * The order in terms of verbosity, from least to most is
 * ERROR, WARN, INFO, DEBUG, VERBOSE.  Verbose should never be compiled
 * into an application except during development.  Debug logs are compiled
 * in but stripped at runtime.  Error, warning and info logs are always kept.
 *
 *
 */

public class QDNLogger {

    private static final String MAIN_TAG = "QDNLogger";
    private static final boolean isLogEnable = true;

    private QDNLogger() {
        // only can access statically
    }


    /**
     * @param tag Used to identify the source of a log message.  It usually identifies
     *        the class or activity where the log call occurs.
     * @param msg The message you would like logged.
     */
    public static void  v(String tag, String msg) {
        if (isLogEnable){
            Log.v(MAIN_TAG + tag , msg);
        }
    }

    /**
     * @param tag Used to identify the source of a log message.  It usually identifies
     *        the class or activity where the log call occurs.
     * @param msg The message you would like logged.
     * @param tr An exception to log
     */
    public static void  v(String tag, String msg, Throwable tr) {
        if (isLogEnable){
            Log.v(MAIN_TAG + tag , msg , tr);
        }
    }

    /**
     * @param tag Used to identify the source of a log message.  It usually identifies
     *        the class or activity where the log call occurs.
     * @param msg The message you would like logged.
     */
    public static void  d(String tag, String msg) {
        if (isLogEnable){
            Log.d(MAIN_TAG + tag , msg);
        }
    }


    /**
     * @param tag Used to identify the source of a log message.  It usually identifies
     *        the class or activity where the log call occurs.
     * @param msg The message you would like logged.
     * @param tr An exception to log
     */
    public static void d(String tag, String msg, Throwable tr) {
        if (isLogEnable){
            Log.d(MAIN_TAG + tag , msg , tr);
        }
    }


    /**
     * @param tag Used to identify the source of a log message.  It usually identifies
     *        the class or activity where the log call occurs.
     * @param msg The message you would like logged.
     */
    public static void i(String tag, String msg) {
        if (isLogEnable){
            Log.i(MAIN_TAG + tag , msg);
        }
    }

    /**
     * @param tag Used to identify the source of a log message.  It usually identifies
     *        the class or activity where the log call occurs.
     * @param msg The message you would like logged.
     * @param tr An exception to log
     */
    public static void i(String tag, String msg, Throwable tr) {
        if (isLogEnable){
            Log.i(MAIN_TAG + tag , msg , tr);
        }
    }


    /**
     * @param tag Used to identify the source of a log message.  It usually identifies
     *        the class or activity where the log call occurs.
     * @param msg The message you would like logged.
     */
    public static void w(String tag, String msg) {
        if (isLogEnable){
            Log.w(MAIN_TAG + tag , msg);
        }
    }

    /**
     * @param tag Used to identify the source of a log message.  It usually identifies
     *        the class or activity where the log call occurs.
     * @param msg The message you would like logged.
     * @param tr An exception to log
     */
    public static void w(String tag, String msg, Throwable tr) {
        if (isLogEnable){
            Log.w(MAIN_TAG + tag , msg , tr);
        }
    }


    /**
     * @param tag Used to identify the source of a log message.  It usually identifies
     *        the class or activity where the log call occurs.
     * @param msg The message you would like logged.
     */
    public static void e(String tag, String msg) {
        if (isLogEnable){
            Log.e(MAIN_TAG + tag , msg);
        }
    }

    /**
     * @param tag Used to identify the source of a log message.  It usually identifies
     *        the class or activity where the log call occurs.
     * @param msg The message you would like logged.
     * @param tr An exception to log
     */
    public static void e(String tag, String msg, Throwable tr) {
        if (isLogEnable){
            Log.e(MAIN_TAG + tag , msg , tr);
        }
    }

}