package com.easytech.wc2;

import android.app.AlertDialog;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.view.Gravity;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.SeekBar;
import android.widget.TextView;


import java.io.IOException;

public class testActivity extends ModActivity {
    private static testActivity activity;
    private LinearLayout terrainInfo;
    private ImageView terrainIcon;
    private TextView terrainName;
    private LinearLayout terrainPenaltyInfantryInfo;
    private TextView terrainPenaltyInfantryValue;
    private LinearLayout terrainPenaltyArtilleryInfo;
    private TextView terrainPenaltyArtilleryValue;
    private LinearLayout terrainPenaltyArmorInfo;
    private TextView terrainPenaltyArmorValue;
    private LinearLayout terrainPenaltyNavyInfo;
    private TextView terrainPenaltyNavyValue;
    private LinearLayout terrainPenaltyAirForceInfo;
    private TextView terrainPenaltyAirForceValue;
    private TextView terrainPenaltyMovementValue;

    //添加新的成员记得在onCreat中初始化

    public  Button 按钮_于_单按钮窗口;
    public  Button 按钮1_于_双按钮三拖动条窗口;
    public  Button 按钮2_于_双按钮三拖动条窗口;
    public  Button 按钮1_于_三按钮窗口;
    public  Button 按钮2_于_三按钮窗口;
    public  Button 按钮3_于_三按钮窗口;
    public  Button 按钮1_于_双按钮窗口;
    public  Button 按钮2_于_双按钮窗口;
    public  Button 按钮1_于_决议小窗;
    public  Button 按钮2_于_决议小窗;
    public  Button 按钮3_于_决议小窗;

    public  TextView 标题_于_单按钮窗口;
    public  TextView 内容_于_单按钮窗口;
    public  TextView 标题_于_双按钮三拖动条窗口;
    public  TextView 内容_于_双按钮三拖动条窗口;
    public TextView 拖动条1标题_于_双按钮三拖动条窗口;
    public TextView 拖动条2标题_于_双按钮三拖动条窗口;
    public TextView 拖动条3标题_于_双按钮三拖动条窗口;
    public  TextView 标题_于_三按钮窗口;
    public  TextView 内容_于_三按钮窗口;
    public  TextView 标题_于_双按钮窗口;
    public  TextView 内容_于_双按钮窗口;
    public  TextView 内容_于_决议小窗;
    public  TextView 标题_于_决议小窗;

    public  ImageView 图片_于_双选择三拖动条窗口;
    public  ImageView 图片_于_三按钮窗口;
    public  ImageView 图片_于_单按钮窗口;
    public  ImageView 图片_于_双按钮窗口;

    public SeekBar 拖动条1_于_双选择三拖动条窗口;
    public SeekBar 拖动条2_于_双选择三拖动条窗口;
    public SeekBar 拖动条3_于_双选择三拖动条窗口;

    public  AlertDialog.Builder 决议构造器;
    public  AlertDialog 决议窗口;
    public  LinearLayout 决议布局;
    //窗口模型（样式）
    public AlertDialog 单按钮窗口;
    public AlertDialog 双按钮窗口;
    public AlertDialog 三按钮窗口;
    public AlertDialog 双按钮三拖动条窗口;
    public native void TriggerResulution(String countryID, int eventNum, int btnNum);
    //初始化事件窗口 此函数名不可变更
    @Override
    public void onCreate(Bundle var1) {
        //注意！不要重复初始化！且确定所需控件全部初始化!否则会闪退!
        super.onCreate(var1);
        activity = this;
        按钮_于_单按钮窗口 = new Button(this);
        按钮1_于_三按钮窗口 = new Button(this);
        按钮2_于_三按钮窗口 = new Button(this);
        按钮3_于_三按钮窗口 = new Button(this);
        按钮1_于_双按钮三拖动条窗口 = new Button(this);
        按钮2_于_双按钮三拖动条窗口 = new Button(this);
        按钮1_于_双按钮窗口 = new Button(this);
        按钮2_于_双按钮窗口 = new Button(this);
        按钮1_于_决议小窗= new Button(this);
        按钮2_于_决议小窗 = new Button(this);
        按钮3_于_决议小窗= new Button(this);

        标题_于_决议小窗 = new TextView(this);
        内容_于_决议小窗 = new TextView(this);
        标题_于_单按钮窗口 = new TextView(this);
        内容_于_单按钮窗口 = new TextView(this);
        标题_于_双按钮窗口 = new TextView(this);
        内容_于_双按钮窗口 = new TextView(this);
        标题_于_双按钮三拖动条窗口 = new TextView(this);
        内容_于_双按钮三拖动条窗口 = new TextView(this);
        拖动条1标题_于_双按钮三拖动条窗口 = new TextView(this);
        拖动条2标题_于_双按钮三拖动条窗口 = new TextView(this);
        拖动条3标题_于_双按钮三拖动条窗口 = new TextView(this);
        标题_于_三按钮窗口 = new TextView(this);
        内容_于_三按钮窗口 = new TextView(this);

        图片_于_双选择三拖动条窗口 = new ImageView(this);
        图片_于_单按钮窗口 = new ImageView(this);
        图片_于_三按钮窗口 = new ImageView(this);
        图片_于_双按钮窗口 = new ImageView(this);

        拖动条1_于_双选择三拖动条窗口 = new SeekBar(this);
        拖动条2_于_双选择三拖动条窗口 = new SeekBar(this);
        拖动条3_于_双选择三拖动条窗口 = new SeekBar(this);

        初始化_单按钮窗口();
        初始化_双按钮窗口();
        初始化_三按钮窗口();
        初始化_双选择三进度条窗口();
        BaseEvent.得到基类唯一实例对象();
        初始化_地形信息_窗口();
    }
    public static testActivity 得到唯一实例对象(){
        return testActivity.activity;
    }

    //需要注意，同个按钮对象不能重复添加！
    public  void 初始化_单按钮窗口(){
        AlertDialog.Builder builder = new AlertDialog.Builder(testActivity.activity);
        // 创建一个布局
        LinearLayout layout = new LinearLayout(testActivity.activity);
        //线性排列
        layout.setOrientation(LinearLayout.VERTICAL);
        标题_于_单按钮窗口.getPaint().setFakeBoldText(true);        //加粗
        图片_于_单按钮窗口.setLayoutParams(new LinearLayout.LayoutParams(-1,-2));
        图片_于_单按钮窗口.setScaleType(ImageView.ScaleType.CENTER);

        layout.addView(图片_于_单按钮窗口);
        layout.addView(标题_于_单按钮窗口);
        layout.addView(内容_于_单按钮窗口);
        layout.addView(按钮_于_单按钮窗口);
        //添加布局
        builder.setView(layout);
        // 禁止取消对话框
        builder.setCancelable(false);
        //生成窗口
        单按钮窗口 = builder.create();
    }
    public  void 初始化_双按钮窗口(){
        AlertDialog.Builder builder = new AlertDialog.Builder(testActivity.activity);
        //滑轮窗口
        ScrollView sv = new ScrollView(this);
        // 创建一个布局
        LinearLayout layout = new LinearLayout(testActivity.activity);
        //线性排列
        layout.setOrientation(LinearLayout.VERTICAL);
        标题_于_双按钮窗口.getPaint().setFakeBoldText(true);        //加粗
        图片_于_双按钮窗口.setLayoutParams(new LinearLayout.LayoutParams(-1,-2));
        图片_于_双按钮窗口.setScaleType(ImageView.ScaleType.CENTER);
        标题_于_双按钮窗口.setGravity(Gravity.CENTER);
        标题_于_双按钮窗口.setTextColor(0xffff0000);
        layout.addView(图片_于_双按钮窗口);
        layout.addView(标题_于_双按钮窗口);
        layout.addView(内容_于_双按钮窗口);
        layout.addView(按钮1_于_双按钮窗口);
        layout.addView(按钮2_于_双按钮窗口);
        sv.addView(layout);
        //添加布局
        builder.setView(sv);
        // 禁止取消对话框
        builder.setCancelable(false);
        //生成窗口
        双按钮窗口 = builder.create();
    }
    public  void 初始化_三按钮窗口(){
        AlertDialog.Builder builder = new AlertDialog.Builder(testActivity.activity);
        // 创建一个布局
        LinearLayout layout = new LinearLayout(testActivity.activity);
        ScrollView sv = new ScrollView(testActivity.activity);
        //线性排列
        layout.setOrientation(LinearLayout.VERTICAL);
        标题_于_三按钮窗口.getPaint().setFakeBoldText(true);        //加粗
        图片_于_三按钮窗口.setLayoutParams(new LinearLayout.LayoutParams(-1,-2));
        图片_于_三按钮窗口.setScaleType(ImageView.ScaleType.CENTER);
        标题_于_三按钮窗口.setGravity(Gravity.CENTER);
        layout.addView(图片_于_三按钮窗口);
        layout.addView(标题_于_三按钮窗口);
        layout.addView(内容_于_三按钮窗口);
        layout.addView(按钮1_于_三按钮窗口);
        layout.addView(按钮2_于_三按钮窗口);
        layout.addView(按钮3_于_三按钮窗口);
        sv.addView(layout);
        //添加布局
        builder.setView(sv);
        // 禁止取消对话框
        builder.setCancelable(false);
        //生成
        三按钮窗口 = builder.create();
    }
    private void 初始化_双选择三进度条窗口() {
        AlertDialog.Builder builder = new AlertDialog.Builder(testActivity.activity);
        // 创建一个布局
        LinearLayout layout = new LinearLayout(testActivity.activity);
        //线性排列
        layout.setOrientation(LinearLayout.VERTICAL);
        图片_于_双选择三拖动条窗口.setLayoutParams(new LinearLayout.LayoutParams(-1,-2));
        图片_于_双选择三拖动条窗口.setScaleType(ImageView.ScaleType.CENTER);

        标题_于_双按钮三拖动条窗口.setGravity(Gravity.CENTER);
        内容_于_双按钮三拖动条窗口.setGravity(Gravity.LEFT);
        layout.addView(图片_于_双选择三拖动条窗口);
        layout.addView(标题_于_双按钮三拖动条窗口);
        layout.addView(内容_于_双按钮三拖动条窗口);
        layout.addView(按钮1_于_双按钮三拖动条窗口);
        layout.addView(按钮2_于_双按钮三拖动条窗口);
        layout.addView(拖动条1标题_于_双按钮三拖动条窗口);
        layout.addView(拖动条1_于_双选择三拖动条窗口);
        layout.addView(拖动条2标题_于_双按钮三拖动条窗口);
        layout.addView(拖动条2_于_双选择三拖动条窗口);
        layout.addView(拖动条3标题_于_双按钮三拖动条窗口);
        layout.addView(拖动条3_于_双选择三拖动条窗口);
        //添加布局
        builder.setView(layout);
        // 禁止取消对话框
        builder.setCancelable(false);
        // 创建对话框
        双按钮三拖动条窗口 = builder.create();
    }

    public void 展示_单按钮窗口(){
        runOnUiThread(()->{
            单按钮窗口.show(); }
        );
    }
    public void 展示_双按钮窗口(){
        runOnUiThread(()->{
            双按钮窗口.show(); }
        );
    }
    public void 展示_三按钮窗口(){
        runOnUiThread(()->{
            三按钮窗口.show(); }
        );
    }
    public void 展示_双按钮三拖动条窗口(){
        runOnUiThread(()->{
            双按钮三拖动条窗口.show(); }
        );
    }

    public static void 重置_决议窗口(){
        activity.按钮1_于_决议小窗= new Button(activity);
        activity.按钮2_于_决议小窗 = new Button(activity);
        activity.按钮3_于_决议小窗= new Button(activity);
        activity.标题_于_决议小窗 = new TextView(activity);
        activity.标题_于_决议小窗.setGravity(Gravity.CENTER);
        activity.标题_于_决议小窗.getPaint().setFakeBoldText(true);
        activity.内容_于_决议小窗 = new TextView(activity);
        activity.决议构造器 = new AlertDialog.Builder(activity);
        activity.决议布局 = new LinearLayout(activity);
        activity.决议布局.setOrientation(LinearLayout.VERTICAL);
    }

    public static void 展开_决议窗口(){

        ScrollView 滑动条窗口 = new ScrollView(activity);
        滑动条窗口.addView(得到唯一实例对象().决议布局);
        activity.决议窗口.setView(滑动条窗口);
        activity.决议构造器.setCancelable(true);
        activity.决议窗口 = activity.决议构造器.create();
        activity.runOnUiThread(() -> activity.决议窗口.show());
    }


//    @SuppressLint("SetTextI18n")
//    public static void 决议窗口_通用_宣战(int eventNum,String targetCountryID, int money, int industry, int oil, int startRound, int endRound, String title, String content, String effect, String notice, String btn) {
//        TextView title1 = new TextView(activity);
//        TextView content1 = new TextView(activity);
//        Button okBtn = new Button(activity);
//        title1.setGravity(Gravity.CENTER);
//        content1.setGravity(Gravity.LEFT);
//        title1.setText(title);
//        content1.setText("\t\t"+content+"\n需要的金钱："+money+"\n需要的工业："+industry+"\n需要的石油："+oil+"\n激活回合："+(startRound+1)+"\n过期回合："+(endRound+1)+"\n决议效果："+effect+"\n注意事项："+notice);
//        okBtn.setText(btn);
//        决议布局.addView(title1);
//        决议布局.addView(content1);
//        决议布局.addView(okBtn);
//        okBtn.setOnClickListener(v -> {
////            BtnEvent_general_AttackTheCountry_Resolution(eventNum,targetCountryID,money,industry,oil);
//            决议窗口.cancel();
//        });
//
////        countryBtn.setText((isAlliance ? "宣战" : "结盟") + " (国名:" + countryID + ",Money:" + money + ",Industry:" + industry + ")");
////        // 按钮点击事件
////        countryBtn.setOnClickListener(v -> {
////            playEffect(isAlliance ? "fire.wav" : "supply.wav");
////            DiplomaticOfCountry(countryID);
////            diplomaticDialog.cancel();
////        });
////        diplomaticLayout.addView(countryBtn);
//    }
//    //每次打开决议时该函数会执行一次，目的是更新决议窗口中的内容
//    public static void 清除_决议旧信息_窗口() {
//        得到唯一实例对象().决议构造器 = new AlertDialog.Builder(activity);
//        得到唯一实例对象().决议布局 = new LinearLayout(activity);
//        得到唯一实例对象().决议布局.setOrientation(LinearLayout.VERTICAL);
//    }



    private  void  初始化_地形信息_窗口() {
        terrainInfo = findViewById(R.id.ll_terrain_info);
        terrainIcon = findViewById(R.id.img_terrain_icon);
        terrainName = findViewById(R.id.tv_terrain_name);
        terrainPenaltyInfantryInfo = findViewById(R.id.ll_terrain_penalty_infantry);
        terrainPenaltyInfantryValue = findViewById(R.id.tv_terrain_penalty_infantry_value);
        terrainPenaltyArtilleryInfo = findViewById(R.id.ll_terrain_penalty_artillery);
        terrainPenaltyArtilleryValue = findViewById(R.id.tv_terrain_penalty_artillery_value);
        terrainPenaltyArmorInfo = findViewById(R.id.ll_terrain_penalty_armor);
        terrainPenaltyArmorValue = findViewById(R.id.tv_terrain_penalty_armor_value);
        terrainPenaltyNavyInfo = findViewById(R.id.ll_terrain_penalty_navy);
        terrainPenaltyNavyValue = findViewById(R.id.tv_terrain_penalty_navy_value);
        terrainPenaltyAirForceInfo = findViewById(R.id.ll_terrain_penalty_airForce);
        terrainPenaltyAirForceValue = findViewById(R.id.tv_terrain_penalty_airforce_value);
        terrainPenaltyMovementValue = findViewById(R.id.tv_terrain_penalty_movement_value);
        hideTerrainInfo();
    }

    public static void 展示_地形信息_窗口(String name, String type, int movementCost, int penalty_infantry, int penalty_artillery, int penalty_armor, int penalty_navy, int penalty_airForce) {
        activity.runOnUiThread(() -> {
            try {
                activity.terrainIcon.setImageBitmap(BitmapFactory.decodeStream(activity.getAssets().open("terrain_res/" + "terrain_" + type + ".png")));
            } catch (IOException e) {
                e.printStackTrace();
            }

            activity.terrainInfo.setVisibility(View.VISIBLE);

            activity.terrainName.setText(name);
            activity.terrainPenaltyMovementValue.setText(String.valueOf(movementCost));

            activity.terrainPenaltyInfantryValue.setText("" + penalty_infantry + "%");
            activity.terrainPenaltyInfantryInfo.setVisibility(penalty_infantry == 0 ? View.GONE : View.VISIBLE);

            activity.terrainPenaltyArtilleryValue.setText("" + penalty_artillery + "%");
            activity.terrainPenaltyArtilleryInfo.setVisibility(penalty_artillery == 0 ? View.GONE : View.VISIBLE);

            activity.terrainPenaltyArmorValue.setText("" + penalty_armor + "%");
            activity.terrainPenaltyArmorInfo.setVisibility(penalty_armor == 0 ? View.GONE : View.VISIBLE);

            activity.terrainPenaltyNavyValue.setText("" + penalty_navy + "%");
            activity.terrainPenaltyNavyInfo.setVisibility(penalty_navy == 0 ? View.GONE : View.VISIBLE);

            activity.terrainPenaltyAirForceValue.setText("" + penalty_airForce + "%");
            activity.terrainPenaltyAirForceInfo.setVisibility(penalty_airForce == 0 ? View.GONE : View.VISIBLE);
        });
    }

    public static void hideTerrainInfo() {
        activity.runOnUiThread(() -> activity.terrainInfo.setVisibility(View.GONE));
    }
}