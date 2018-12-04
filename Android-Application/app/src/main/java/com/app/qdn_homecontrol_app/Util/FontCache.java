package com.app.qdn_homecontrol_app.Util;

import android.content.Context;
import android.content.res.AssetManager;
import android.graphics.Typeface;

import java.util.HashMap;
import java.util.Locale;

public class FontCache {

    //To cache the font creating HashMap
    private static HashMap<String , Typeface> fontCache = new HashMap<>();

    public static Typeface getTypeFace(Context mContext, String mFont){
        Typeface typeface = fontCache.get(mFont);

        if (typeface == null){
            AssetManager am = mContext.getApplicationContext().getAssets();
            typeface = Typeface.createFromAsset(am , String.format(Locale.US, "fonts/%s", mFont));
            fontCache.put(mFont , typeface);
        }

        return typeface;
    }
}
