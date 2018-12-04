package com.app.qdn_homecontrol_app.Util;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Date;
import java.util.HashMap;

public class DbHelper extends SQLiteOpenHelper {
    //Database name
    private static final String DATABASE_NAME = "QDN_LIVE_DATA";

    //Database Version
    private static final int DATABASE_VERSION = 1;

    //Table Name
    private static final String LIVE_DATA_TABLE_NAME = "live_data_table";

    //primary key auto increment
    private static final String LIVE_DATA_UNIQUE_ID = "liveDataId";

    //Column Name of table
    private static final String LIVE_DATA_COL1 = "liveData";
    private static final String CURRENT_TIME_COL2 = "time";

    //constructor
    public DbHelper(Context context) {
        super(context, DATABASE_NAME, null, DATABASE_VERSION);
    }

    @Override
    public void onCreate(SQLiteDatabase db) {
        String dataTable = "create table " + LIVE_DATA_TABLE_NAME + "(" + LIVE_DATA_UNIQUE_ID + " PRIMARY KEY," + LIVE_DATA_COL1 + " INTEGER," + CURRENT_TIME_COL2 + " text" + ")";
        db.execSQL(dataTable);
    }

    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
        db.execSQL("DROP TABLE IF EXISTS " + LIVE_DATA_TABLE_NAME);
        onCreate(db);

    }

    /* adding the smoke sensor Live Data  */
    public void insertLiveData(int smokeVal, String currentTime) {
        SQLiteDatabase sqLiteDatabase = this.getWritableDatabase();

        ContentValues contentValues = new ContentValues();
        contentValues.put(LIVE_DATA_COL1, smokeVal);
        contentValues.put(CURRENT_TIME_COL2, currentTime);
        sqLiteDatabase.insert(LIVE_DATA_TABLE_NAME, null, contentValues);
        sqLiteDatabase.close();
        }

        //Get data from DB
    public HashMap<String,Integer> getDataFromDB()
    {
        HashMap<String,Integer> data = new HashMap<String,Integer>();

        String selectQuery = "SELECT  * FROM " + LIVE_DATA_TABLE_NAME;
        SQLiteDatabase db  = this.getReadableDatabase();
        Cursor cursor      = db.rawQuery(selectQuery, null);

        if (cursor.moveToFirst()) {
            do {

               int livedata = cursor.getInt(cursor.getColumnIndex(LIVE_DATA_COL1));
               String time = cursor.getString(cursor.getColumnIndex(CURRENT_TIME_COL2));
               data.put(time,livedata);
                // get the data into array, or class variable
            } while (cursor.moveToNext());
        }
        cursor.close();

        return data;
    }
}
