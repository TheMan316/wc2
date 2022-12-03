package com.easytech.wc2;

import android.app.AlertDialog;
import android.view.Gravity;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.TextView;

public class BaseEvent {
    //由于继承，基类所有成员都有设置为公开权限
    private static BaseEvent 基类对象;
    public  static int countryMoney;
    public static int countryIndustry;
    public static int eventNum;
    public static int eventType;
    public  static int countryOil;
    public static String title;
    public static String content;
    public static String targetCountryID;
    public static String targetCountryName;
    public static String theCountryID;
    public static String theCountryName;
    public static AlertDialog dialog;
    // native关键字的函数是用来链接C++的  在基类专门放通用代码   0决议 1回合开始 2军队移动 3决议
    public native void BtnEvent_addResource (String theCountryID, int money,int industry,int oil);
    public native void BtnEvent_ChooseAI(int level);
    public native void BtnEvent_ChangeArmysMorale(String theCountryID,int turns,int moraleType);
    public native void BtnEvent_ChooseDifficulty(int level);
    //单纯标记事件为结束状态 用于结束弹窗的各个按钮
    public native void BtnEvent_EventFinished( int eventNum,int eventType,String theCountryID);
    public static BaseEvent 得到基类唯一实例对象(){
        if (BaseEvent.基类对象 == null){
            BaseEvent.基类对象 = new DeEvent();
            BaseEvent.countryMoney = 0;
            BaseEvent.countryIndustry = 0;
            BaseEvent.countryOil = 0;
            BaseEvent.title = "null";
            BaseEvent.content = "null";
            BaseEvent.targetCountryName = "null";
            BaseEvent.targetCountryID = "null";
            BaseEvent.theCountryID = "null";
            BaseEvent.theCountryName = "null";
        }
        return  BaseEvent.基类对象;
    }
    public static void 初始化_事件成员变量(int eventNum,int eventType,int money,int industry,int oil,String theCountryID,String theCountryName){
        BaseEvent.countryMoney = money;
        BaseEvent.countryIndustry = industry;
        BaseEvent.countryOil = oil;
        BaseEvent.theCountryID = theCountryID;
        BaseEvent.theCountryName = theCountryName;
        BaseEvent.eventNum = eventNum;
        BaseEvent.eventType = eventType;
    }

    public static void 旧展示_决议窗口() {
        testActivity 对象 = testActivity.得到唯一实例对象();
        ScrollView scrollView = new ScrollView(testActivity.得到唯一实例对象());
        scrollView.addView(对象.决议布局);
        对象.runOnUiThread(() -> 对象.决议窗口 = 对象.决议构造器.setCancelable(true).setView(scrollView).show());
    }
    public static void 旧重置_决议窗口() { ;
        testActivity.得到唯一实例对象().决议构造器 = new AlertDialog.Builder(testActivity.得到唯一实例对象());
        testActivity.得到唯一实例对象().决议布局 = new LinearLayout(testActivity.得到唯一实例对象());
        testActivity.得到唯一实例对象().决议布局.setOrientation(LinearLayout.VERTICAL);
    }
    public static void 生成_单按钮决议内容_小窗(String countryID,int eventNum,
                                     String title,String content,String result,String node,
                                     int startTurn,int endTurn,int money,int industry,int oil,
                                     String btn1Name){
        TextView 标题 =  new TextView(testActivity.得到唯一实例对象());
        标题.setGravity(Gravity.CENTER);
        标题.getPaint().setFakeBoldText(true);
        标题.setText(title);
        Button btn1 = new Button(testActivity.得到唯一实例对象());
        btn1.setText(btn1Name);
        TextView 内容 =  new TextView(testActivity.得到唯一实例对象());
        内容.setText("_____________________________________________\n[介绍:]\n"+content+"\n[效果:]\n"+result+"\n[解锁时间:]\n"+startTurn+"\n[失效时间:]\n"+endTurn+"\n[解锁所需金钱:]" +
                "\n"+money+"\n[解锁所需工业:]\n"+industry+"\n[解锁所需石油:]\n"+oil+"\n[注意事项:]\n"+node);
        testActivity.得到唯一实例对象().决议布局.addView(标题);
        testActivity.得到唯一实例对象().决议布局.addView(内容);
        testActivity.得到唯一实例对象().决议布局.addView(btn1);
        btn1.setOnClickListener(v -> {
            testActivity.得到唯一实例对象().TriggerResulution(countryID,eventNum,1 );
            testActivity.得到唯一实例对象().决议窗口.cancel();
        });
    }
    public static void 生成_双按钮决议内容_小窗(String countryID,int eventNum,
                                     String title,String content,String result,String node,
                                     int startTurn,int endTurn,int money,int industry,int oil,
                                     String btn1Name ,String btn2Name){
        TextView 标题 =  new TextView(testActivity.得到唯一实例对象());
        标题.setGravity(Gravity.CENTER);
        标题.getPaint().setFakeBoldText(true);
        标题.setText(title);
        Button btn1 = new Button(testActivity.得到唯一实例对象());
        btn1.setText(btn1Name);
        Button btn2 = new Button(testActivity.得到唯一实例对象());
        btn2.setText(btn2Name);
        TextView 内容 =  new TextView(testActivity.得到唯一实例对象());
        内容.setText("_____________________________________________\n[介绍:]\n"+content+"\n[效果:]\n"+result+"\n[解锁时间:]\n"+startTurn+"\n[失效时间:]\n"+endTurn+"\n[解锁所需金钱:]" +
                "\n"+money+"\n[解锁所需工业:]\n"+industry+"\n[解锁所需石油:]\n"+oil+"\n[注意事项:]\n"+node);
        testActivity.得到唯一实例对象().决议布局.addView(标题);
        testActivity.得到唯一实例对象().决议布局.addView(内容);
        testActivity.得到唯一实例对象().决议布局.addView(btn1);
        testActivity.得到唯一实例对象().决议布局.addView(btn2);

        btn1.setOnClickListener(v -> {
            testActivity.得到唯一实例对象().TriggerResulution(countryID,eventNum,1 );
            testActivity.得到唯一实例对象().决议窗口.cancel();
        });
        btn2.setOnClickListener(v -> {
            testActivity.得到唯一实例对象().TriggerResulution(countryID,eventNum,2 );
            testActivity.得到唯一实例对象().决议窗口.cancel();
        });

    }
    public static void 生成_三按钮决议内容_小窗(String countryID,int eventNum,
                                     String title,String content,String result,String node,
                                     int startTurn,int endTurn,int money,int industry,int oil,
                                     String btn1Name ,String btn2Name,String btn3Name){
        TextView 标题 =  new TextView(testActivity.得到唯一实例对象());
        标题.setGravity(Gravity.CENTER);
        标题.getPaint().setFakeBoldText(true);
        标题.setText(title);
        Button btn1 = new Button(testActivity.得到唯一实例对象());
        btn1.setText(btn1Name);
        Button btn2 = new Button(testActivity.得到唯一实例对象());
        btn2.setText(btn2Name);
        Button btn3 = new Button(testActivity.得到唯一实例对象());
        btn3.setText(btn3Name);
        TextView 内容 =  new TextView(testActivity.得到唯一实例对象());
        内容.setText("_____________________________________________\n[介绍:]\n"+content+"\n[效果:]\n"+result+"\n[解锁时间:]\n"+startTurn+"\n[失效时间:]\n"+endTurn+"\n[解锁所需金钱:]" +
                "\n"+money+"\n[解锁所需工业:]\n"+industry+"\n[解锁所需石油:]\n"+oil+"\n[注意事项:]\n"+node);
        testActivity.得到唯一实例对象().决议布局.addView(标题);
        testActivity.得到唯一实例对象().决议布局.addView(内容);
        testActivity.得到唯一实例对象().决议布局.addView(btn1);
        testActivity.得到唯一实例对象().决议布局.addView(btn2);
        testActivity.得到唯一实例对象().决议布局.addView(btn3);

        btn1.setOnClickListener(v -> {
            testActivity.得到唯一实例对象().TriggerResulution(countryID,eventNum, 1);
            testActivity.得到唯一实例对象().决议窗口.cancel();
        });
        btn2.setOnClickListener(v -> {
            testActivity.得到唯一实例对象().TriggerResulution(countryID,eventNum,2 );
            testActivity.得到唯一实例对象().决议窗口.cancel();
        });
        btn3.setOnClickListener(v -> {
            testActivity.得到唯一实例对象().TriggerResulution(countryID,eventNum,3 );
            testActivity.得到唯一实例对象().决议窗口.cancel();
        });
    }
}
