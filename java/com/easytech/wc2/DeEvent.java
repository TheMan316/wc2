package com.easytech.wc2;

import android.graphics.drawable.Drawable;
import android.widget.SeekBar;

//每个国家都有事件和决议两个java文件
public class DeEvent extends BaseEvent {

    //占领法国
    public static native void BtnEvent_0003_De(int eventNum,int eventType);

    public static void  展示_德国事件(int eventNum, int eventType,int money, int industry, int oil, String theCountryID, String theCountryName){
        初始化_事件成员变量(eventNum,eventType,money,industry,oil,theCountryID,theCountryName);
        //注意，如果有事件没有对应的case值，游戏界面会进入假死状态！
        switch (eventNum){
            case 1:
                展示_0001_事件测试_回合开始事件();
                break;
            case 3:

                break;
            default:
                break;
        }
    }
    //这些本国事件这能在本国范围内使用，所以设置为私有权限
    private static void 展示_0001_事件测试_回合开始事件(){
        testActivity 对象 = testActivity.得到唯一实例对象();
        对象.标题_于_单按钮窗口.setText("这是一个测试事件");
        对象.内容_于_单按钮窗口.setText("懂了吗?");
        对象.按钮_于_单按钮窗口.setText("我不比你懂?");
        Drawable image = 对象.getResources().getDrawable(R.drawable.icon_oil);//此处icon对应drawable文件夹下的icon.png
        对象.图片_于_单按钮窗口.setImageDrawable(image);//设置图片
        对象.按钮_于_单按钮窗口.setOnClickListener(v ->{
            对象.单按钮窗口.cancel();
            得到基类唯一实例对象().BtnEvent_addResource("de",100,1,1);
            展示_0002_事件测试1_回合开始事件();
        });
        对象.展示_单按钮窗口();
    }
    //像这种不依赖于外部条件而是根据按钮触发的,其实不用在外部把它生成进国家事件列表中
    private static void 展示_0002_事件测试1_回合开始事件(){
        testActivity 对象 =   testActivity.得到唯一实例对象();
        对象.标题_于_单按钮窗口.setText("呵呵,现在标题变了");
        对象.按钮_于_单按钮窗口.setOnClickListener(v ->{
            对象.单按钮窗口.cancel();
            得到基类唯一实例对象().BtnEvent_EventFinished(eventNum,eventType,theCountryID);
        });
        对象.展示_单按钮窗口();
    }
    private static void 展示_0003_法国请降_地块触发事件() {
        testActivity 对象 =  testActivity.得到唯一实例对象();
        对象.标题_于_双按钮窗口.setText("懦弱的法国请求投降!");
        对象.内容_于_双按钮窗口.setText("/t/t/t/t法国被吓尿了!我们要不要接收它们的投降?如若接收,我们将帮助他们组建新政府!");
        对象.按钮1_于_双按钮窗口.setText("我们将帮他们组建新政府");
        对象.按钮2_于_双按钮窗口.setText("我们要彻底击败他们!");
        Drawable image = 对象.getResources().getDrawable(R.drawable.icon_oil);//此处icon对应drawable文件夹下的icon.png
        对象.图片_于_双按钮窗口.setImageDrawable(image);//设置图片
        对象.按钮1_于_双按钮窗口.setOnClickListener(v -> {
            对象.双按钮窗口.cancel();
            BtnEvent_0003_De(eventNum,eventType);
            展示_0004_建立维希政府_地块触发事件();
            得到基类唯一实例对象().BtnEvent_EventFinished(eventNum,eventType,theCountryID);

        });
        对象.按钮2_于_双按钮窗口.setOnClickListener(v -> {
            对象.双按钮窗口.cancel();
            得到基类唯一实例对象().BtnEvent_ChangeArmysMorale(theCountryID,4,1);
            得到基类唯一实例对象().BtnEvent_EventFinished(eventNum,eventNum,theCountryID);
        });
    }

    private static void 展示_0004_建立维希政府_地块触发事件(){
        testActivity 对象 =  testActivity.得到唯一实例对象();
        对象.标题_于_双按钮三拖动条窗口.setText("维希政府成立!");
        对象.内容_于_双按钮三拖动条窗口.setText("维希政府目前非常脆弱,我们需要帮助他们");
        对象.按钮1_于_双按钮三拖动条窗口.setText("就这些吧!");
        对象.按钮2_于_双按钮三拖动条窗口.setText("一分都没有!!");
        // 设置拖动条当前进度
        对象.拖动条1_于_双选择三拖动条窗口.setProgress(100);
        对象.拖动条2_于_双选择三拖动条窗口.setProgress(100);
        对象.拖动条3_于_双选择三拖动条窗口.setProgress(100);
        // 设置拖动条进度上限
        对象.拖动条1_于_双选择三拖动条窗口.setMax(500);
        对象.拖动条2_于_双选择三拖动条窗口.setMax(500);
        对象.拖动条3_于_双选择三拖动条窗口.setMax(500);
        // 拖动条变更事件
        对象.拖动条1_于_双选择三拖动条窗口.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            // onProgressChanged: 拖动条的进度发生变化时调用
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                // 设置信息文本
                对象.拖动条1标题_于_双按钮三拖动条窗口.setText("当前提供该国新政府防卫的金钱为：" + 对象.拖动条1_于_双选择三拖动条窗口.getProgress() + "金钱");
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
            }
        });
        对象.拖动条2_于_双选择三拖动条窗口.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            // onProgressChanged: 拖动条的进度发生变化时调用
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                // 设置信息文本
                对象.拖动条2标题_于_双按钮三拖动条窗口.setText("当前提供该国新政府防卫的工业为：" + 对象.拖动条2_于_双选择三拖动条窗口.getProgress() + "工业");
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
            }
        });
        对象.拖动条3_于_双选择三拖动条窗口.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                对象.拖动条3标题_于_双按钮三拖动条窗口.setText("当前提供该国新政府防卫的石油为：" + 对象.拖动条3_于_双选择三拖动条窗口.getProgress() + "石油");
            }
            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
            }
        });
        对象.按钮1_于_双按钮三拖动条窗口.setOnClickListener(
                v -> {
                    对象.双按钮三拖动条窗口.cancel();
                    得到基类唯一实例对象().BtnEvent_addResource("fr",
                            对象.拖动条1_于_双选择三拖动条窗口.getProgress(),
                            对象.拖动条2_于_双选择三拖动条窗口.getProgress(),
                            对象.拖动条3_于_双选择三拖动条窗口.getProgress());
                    得到基类唯一实例对象().BtnEvent_addResource("de",
                            -对象.拖动条1_于_双选择三拖动条窗口.getProgress(),
                            -对象.拖动条2_于_双选择三拖动条窗口.getProgress(),
                            -对象.拖动条3_于_双选择三拖动条窗口.getProgress());
                    得到基类唯一实例对象().BtnEvent_EventFinished(eventNum,eventType,theCountryID);
                }
        );
        对象.按钮2_于_双按钮三拖动条窗口.setOnClickListener(
                v -> {
                    对象.双按钮三拖动条窗口.cancel();
                    得到基类唯一实例对象().BtnEvent_EventFinished(eventNum,eventType,theCountryID);
                }
        );
    }
}