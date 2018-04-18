package com.example.seraphzxz.libjpeg;

import android.Manifest;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.os.Build;
import android.os.Environment;
import android.provider.MediaStore;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v4.content.FileProvider;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import com.bumptech.glide.Glide;
import com.example.seraphzxz.libjpeg.utils.FileUtils;

import java.io.File;
import java.util.List;

public class MainActivity extends AppCompatActivity {

    private Button encrypt, fileDecode,showE,showR,showD,split,merge;
    private Button compress, showCompress;
    private ImageView imageView;

    private String pathName = Environment.getExternalStorageDirectory().getAbsolutePath() + File.separator;

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        if (ContextCompat.checkSelfPermission(MainActivity.this, Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(MainActivity.this, new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE}, 1);
        }


        encrypt = findViewById(R.id.encode);
        encrypt.setOnClickListener(v -> FileUtils.fileEncrypt());
        fileDecode = findViewById(R.id.decode);
        fileDecode.setOnClickListener(v -> FileUtils.fileDecode());

        imageView = findViewById(R.id.photo);

        showE = findViewById(R.id.showE);
        showE.setOnClickListener(v->showE());
        showR = findViewById(R.id.showR);
        showR.setOnClickListener(v->showR());
        showD = findViewById(R.id.showD);
        showD.setOnClickListener(v->showD());
        split = findViewById(R.id.split);
        split.setOnClickListener(v->FileUtils.fileSplit());
        merge = findViewById(R.id.merge);
        merge.setOnClickListener(v->FileUtils.fileMerge());

        compress = findViewById(R.id.compress);
        compress.setOnClickListener(v->compress());

        showCompress = findViewById(R.id.showCompress);
        showCompress.setOnClickListener(v->showCompress());

    }

    private void showCompress() {
        Glide.with(this)
                .load(FileUtils.FILE_PATH+"imgCompress.jpg")
                .centerCrop()
                .into(imageView);

        Log.e("LIB_JPEG","Compress size:" + new File(FileUtils.FILE_PATH + "imgCompress.jpg").length());
    }


    private void compress() {

        new Thread(new Runnable() {
            @Override
            public void run() {
                Bitmap bitmap = BitmapFactory.decodeFile(FileUtils.FILE_PATH + "img.jpg");
                if (bitmap != null) {
                    FileUtils.compressBitmap(bitmap, bitmap.getWidth(), bitmap.getHeight(), FileUtils.FILE_PATH + "imgCompress.jpg", 20);
                } else {
                    Toast.makeText(MainActivity.this, "为空", Toast.LENGTH_SHORT).show();
                }
                Log.e("LIB_JPEG","Compress done!!!");
            }
        }).start();

    }

    private void showD() {
        Glide.with(this)
                .load(FileUtils.FILE_PATH+"img.jpeg")
                .centerCrop()
                .into(imageView);
    }

    private void showR() {
        Glide.with(this)
                .load(FileUtils.FILE_PATH+"img.jpg")
                .centerCrop()
                .into(imageView);
        Log.e("LIB_JPEG","Raw size:" + new File(FileUtils.FILE_PATH + "img.jpg").length());

    }

    private void showE() {
        Glide.with(this).load(FileUtils.FILE_PATH+"cats_encrypt.png").into(imageView);
    }


}
