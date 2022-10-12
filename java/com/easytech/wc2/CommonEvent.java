package com.easytech.wc2;

public class CommonEvent extends BaseEvent {
    public static void  展示_公共事件(int eventNum, int eventType,int money, int industry, int oil, String theCountryID, String theCountryName){
        初始化_事件成员变量(eventNum,eventType,money,industry,oil,theCountryID,theCountryName);
        switch (eventNum){
            case 1:
                展示_0001_C版介绍_回合开始事件();
                break;
            default:
                break;
        }
    }
    private static void 展示_0001_C版介绍_回合开始事件() {
        testActivity 对象 = testActivity.得到唯一实例对象();

        对象.标题_于_双按钮窗口.setText("《欢迎来到世界征服者2C版》");
        对象.内容_于_双按钮窗口.setText("世界征服者2C版介绍：\n\t\t\t世界征服者2C版是一个逆向世2原版" +
                "代码并以C++进行重写得到的版本。于2020年5月11日首次发布。奠定者昵称：’limlimg‘，其解析了世2大部分代码函数的功能" +
                "，同时也是世2超级加速的制作者。此后柒安、西蜀阿斗、李德邻、馅饼等人开始对原版诸多函数进行重写，并基于原版制作出一系列新功能" +
                "。而后C版框架也逐渐派生出柒安版、李德邻版、馅饼版三大框架，各框架各具特色。\n\t\t\t对世2代码的研究使得三人收获颇多，非常感谢苏州乐志制作" +
                "出如此优秀的游戏并允许我们在合法范围内对其进行逆向学习。如果对游戏开发感兴趣，欢迎大家参与到C版的学习研究中。游戏既可以成为中华文化传播的载体，" +
                "也可以成为文化输出的平台。望大家共同努力，为中国文化发展贡献出自己的力量。" +
                "\n\t\t\t\t本软件仅供学习交流，请勿用于违法用途！否则违法者自行承担法律责任！\n\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t ————————2022年9月30日  李德邻");
        对象.按钮1_于_双按钮窗口.setText("普通AI");
        对象.按钮2_于_双按钮窗口.setText("高级AI");
        对象.按钮1_于_双按钮窗口.setOnClickListener(v -> {
            得到基类唯一实例对象().BtnEvent_ChooseAI(1);

            对象.双按钮窗口.cancel();
            展示_0002_难度选择_回合开始事件();
        });
        对象.按钮2_于_双按钮窗口.setOnClickListener(v -> {
            得到基类唯一实例对象().BtnEvent_ChooseAI(2);

            对象.双按钮窗口.cancel();
            展示_0002_难度选择_回合开始事件();
        });
        对象.展示_双按钮窗口();
    }

    private static void 展示_0002_难度选择_回合开始事件() {
        testActivity 对象 = testActivity.得到唯一实例对象();
        对象.标题_于_三按钮窗口.setText("在选择难度之前，请你阅读：");
        对象.内容_于_三按钮窗口.setText("\t\t\t首先，AI的难度选择和游戏的难度选择是不一样的。前者只是单纯提高了AI的智力，后者则是给AI提供经济上的加成。" +
                "关于高级AI的智力提高有如下：\n\t\t\t" +
                "第一：" +
                "\n\t\t\tAI高价值单位在残血状态时，如果不是被包围,会尝试找到补给点自行回血并直至血量恢复到健康水平。\n\t\t\t" +
                "第二：" +
                "\n\t\t\tAI的远程单位不会接触敌军。如果敌军接触远程单位，该远程单位会选择一个安全的地块后撤。同时在寻路时会选择可进攻最近敌军" +
                "的有利地点。\n\t\t\t" +
                "第三：" +
                "\n\t\t\tAI不会在非危险区造兵，加强了AI集中兵力的效率。\n\t\t\t" +
                "第四：" +
                "\n\t\t\tAI在回合结束期间会把合适的低价值单位放在顶层。\n\t\t\t" +
                "第五：" +
                "\n\t\t\tAI会细化各军种的寻路，相互协作。\n\t\t\t" +
                "第六：" +
                "\n\t\t\t每回合开始AI会根据经济实力在受威胁的地块造兵。\n\n" +
                "关于游戏难度:" +
                "\n\t\t\t简单难度中，所有AI的基本税率加成统一为1倍(玩家税率始终强制为1);" +
                "标准难度则根据" +
                "模组作者的标准设定，不做任何变更（盟友税率不会像原版那样强制为1）；困难难度则所有开局的敌方AI基本税率*2，友军AI的基本税率*1.4。\n\n" +
                "【注意】：" +
                "\n\t\t\t初次进入游戏先在游戏设置中选择五速，如果需要快速过场双击两次回合结束键。");
        对象.按钮1_于_三按钮窗口.setText("简单难度");
        对象.按钮2_于_三按钮窗口.setText("标准难度");
        对象.按钮3_于_三按钮窗口.setText("困难难度");
        对象.按钮1_于_三按钮窗口.setOnClickListener(v -> {
            得到基类唯一实例对象().BtnEvent_ChooseDifficulty(1);
            得到基类唯一实例对象().BtnEvent_EventFinished(eventNum,eventType,"null");
            对象.三按钮窗口.cancel();
        });
        对象.按钮2_于_三按钮窗口.setOnClickListener(v -> {
            得到基类唯一实例对象().BtnEvent_ChooseDifficulty(2);
            得到基类唯一实例对象().BtnEvent_EventFinished(eventNum,eventType,"null");
            对象.三按钮窗口.cancel();

        });
        对象.按钮3_于_三按钮窗口.setOnClickListener(v -> {
            得到基类唯一实例对象().BtnEvent_ChooseDifficulty(3);
            得到基类唯一实例对象().BtnEvent_EventFinished(eventNum,eventType,"null");
            对象.三按钮窗口.cancel();
        });
        对象.展示_三按钮窗口();
    }
}