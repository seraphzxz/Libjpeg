package com.example.seraphzxz.libjpeg.utils;

import android.graphics.Bitmap;
import android.os.Environment;
import android.util.Log;

import java.io.File;

/**
 * Created by seraphzxz on 18-4-16.
 */

public class FileUtils {

    private static final String FILE_PATH_PREFIX = Environment.getExternalStorageDirectory() + File.separator;
    private static final String FOLDER_NAME = "NdkSample" + File.separator;
    public static final String FILE_PATH = FILE_PATH_PREFIX + FOLDER_NAME;


    public static boolean fileEncrypt() {
        String normalFilePath = FILE_PATH + "ic_launcher.png";
        String encryptFilePath = FILE_PATH + "cats_encrypt.png";
        try {
            return fileEncrypt(normalFilePath, encryptFilePath);
        } catch (Exception e) {
            e.printStackTrace();
        }
        return false;
    }
    public static boolean fileDecode() {
        String encryptFilePath = FILE_PATH + "cats_encrypt.png";
        String decodeFilePath = FILE_PATH + "cats_decode.png";
        try {
            return fileDecode(encryptFilePath, decodeFilePath);
        } catch (Exception e) {
            e.printStackTrace();
        }
        return false;
    }

    public static boolean fileSplit() {
        String splitFilePath = FILE_PATH + "img.jpg";
        String suffix = ".b";
        boolean result = false;
        try {
            result = fileSplit(splitFilePath, suffix, 4);
            Log.e("NDK_SAMPLE","Split " + result);
            return result;
        } catch (Exception e) {
            e.printStackTrace();
        }
        Log.e("NDK_SAMPLE","Split " + result);
        return false;
    }
    /**
     * 文件合并
     *
     * @return
     */
    public static boolean fileMerge() {
        String splitFilePath = FILE_PATH + "img.jpg";
        String splitSuffix = ".b";
        String mergeSuffix = ".jpeg";
        boolean result = false;
        try {
            result = fileMerge(splitFilePath, splitSuffix, mergeSuffix, 4);
            Log.e("NDK_SAMPLE","Merge " + result);
            return result;
        } catch (Exception e) {
            e.printStackTrace();
        }
        Log.e("NDK_SAMPLE","Merge " + result);
        return false;
    }


    private static native boolean fileEncrypt(String normalFilePath, String encryptFilePath);
    private static native boolean fileDecode(String encryptFilePath, String decodeFilePath);

    private static native boolean fileSplit(String splitFilePath, String suffix, int fileNum);

    private static native boolean fileMerge(String splitFilePath, String splitSuffix, String mergeSuffix, int fileNum);


    public static native boolean compressBitmap(Bitmap bitmap, int width, int height, String fileName, int quality);

    public static native String getMD5(String path);

}


