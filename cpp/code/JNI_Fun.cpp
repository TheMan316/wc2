//
// Created by Administrator on 2021/9/11.
//

#include <easytech.h>
#include <CScene.h>
#include <cstdlib>
#include "JNI_Fun.h"
#include "com_easytech_wc2_TestActivity.h"
#include "event.h"


//主界面新窗口
void JNI_Fun::Show_resolutionForm() {
    JNIEnv* env = nullptr;
    if (g_vm != nullptr && (g_vm->GetEnv((void**) &env, JNI_VERSION_1_6) == JNI_OK)) {
        jclass activity = env->FindClass("com/easytech/wc2/BaseEvent");
        jmethodID clear = env->GetStaticMethodID(activity, "旧重置_决议窗口", "()V");
        jmethodID show = env->GetStaticMethodID(activity, "旧展示_决议窗口", "()V");


        env->CallStaticVoidMethod(activity, clear);
        Auto_creatResolutions_toTheForm(g_GameManager.GetCurCountry(), env, &activity);

        env->CallStaticVoidMethod(activity, show);
    }
}
void Auto_creatResolutions_toTheForm(CCountry* country, JNIEnv* env, jclass* activity) {

        for (int i = 0; i < country->List_resolution.size(); ++i) {
            auto resolution = country->List_resolution[i];
            if ( g_commonEvent.Can_triggere_resolution(resolution,country) == false)
                continue;
            if (resolution->btnCount == 1){
                jmethodID ff = env->GetStaticMethodID(*activity, "生成_单按钮决议内容_小窗", "(Ljava/lang/String;ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;IIIIILjava/lang/String;)V");
                env->CallStaticVoidMethod(*activity, ff, env->NewStringUTF(country->ID),
                                          resolution->eventNum,
                                          env->NewStringUTF(resolution->title),
                                          env->NewStringUTF(resolution->content),
                                          env->NewStringUTF(resolution->result),
                                          env->NewStringUTF(resolution->note),
                                          resolution->startRound,resolution->endRound,
                                          resolution->moneyForTriggerEvent, resolution->industryForTriggerEvent,
                                          resolution->oilForTriggerEvent,
                                          env->NewStringUTF(resolution->btn1));
            }
            else if(resolution->btnCount == 2){
                jmethodID  ff = env->GetStaticMethodID(*activity, "生成_双按钮决议内容_小窗", "(Ljava/lang/String;ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;IIIIILjava/lang/String;Ljava/lang/String;)V");
                env->CallStaticVoidMethod(*activity, ff, env->NewStringUTF(country->ID),
                                          resolution->eventNum,
                                          env->NewStringUTF(resolution->title),
                                          env->NewStringUTF(resolution->content),
                                          env->NewStringUTF(resolution->result),
                                          env->NewStringUTF(resolution->note),
                                          resolution->startRound,resolution->endRound,
                                          resolution->moneyForTriggerEvent, resolution->industryForTriggerEvent,
                                          resolution->oilForTriggerEvent,
                                          env->NewStringUTF(resolution->btn1),
                                          env->NewStringUTF(resolution->btn2));
            }
            else if(resolution->btnCount == 3){
                jmethodID   ff =  env->GetStaticMethodID(*activity, "生成_三按钮决议内容_小窗", "(Ljava/lang/String;ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;IIIIILjava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
                env->CallStaticVoidMethod(*activity, ff, env->NewStringUTF(country->ID),
                                          resolution->eventNum,
                                          env->NewStringUTF(resolution->title),
                                          env->NewStringUTF(resolution->content),
                                          env->NewStringUTF(resolution->result),
                                          env->NewStringUTF(resolution->note),
                                          resolution->startRound,resolution->endRound,
                                          resolution->moneyForTriggerEvent, resolution->industryForTriggerEvent,
                                          resolution->oilForTriggerEvent,
                                          env->NewStringUTF(resolution->btn1),
                                          env->NewStringUTF(resolution->btn2),
                                          env->NewStringUTF(resolution->btn3));
            }
        }

}
//主界面新窗口
extern "C" JNIEXPORT void JNICALL Java_com_easytech_wc2_testActivity_DiplomaticOfCountry(JNIEnv *Env, jclass, jstring CountryID) {
    CCountry *PlayerCountry = g_GameManager.GetPlayerCountry();
    CCountry *TargetCountry = g_GameManager.FindCountry(Env->GetStringUTFChars(CountryID, JNI_FALSE));
    if (PlayerCountry->Alliance == TargetCountry->Alliance) {
        TargetCountry->Alliance++;
    } else {
        TargetCountry->Alliance = PlayerCountry->Alliance;
    }
}

bool
CommonEvent::Can_triggere_armyMoveToEvent(EventBase* event, int targetAreaID, const char* targetCountryID, CCountry* theCountry) {
    if (event->isFinished == false &&
        g_GameManager.CurrentTurnNumMinusOne >= event->startRound &&
        g_GameManager.CurrentTurnNumMinusOne <= event->endRound &&
        theCountry->Money >= event->moneyForTriggerEvent&&
        theCountry->Industry >= event->industryForTriggerEvent&&
        theCountry->Oil >= event->oilForTriggerEvent){
        int size = event->map_area.size();
        if (size != 0){
            if (event->map_area.count(targetAreaID))
                return true;
        }
        if (g_GameManager.GetRand() %100 + 1 <= event->percentOfTrigger)
            return true;
    }
    return false;
}

bool
CommonEvent::Can_triggere_turnBeginEvent(EventBase* event, CCountry* theCountry) {
    //空的说明的公共事件
    if (theCountry == nullptr){
        if (event->isFinished == false &&
            g_GameManager.CurrentTurnNumMinusOne >= event->startRound &&
            g_GameManager.CurrentTurnNumMinusOne <= event->endRound )
            return true;
        return false;
    }
    if (event->isFinished == false &&
        g_GameManager.CurrentTurnNumMinusOne >= event->startRound &&
        g_GameManager.CurrentTurnNumMinusOne <= event->endRound &&
        theCountry->Money >= event->moneyForTriggerEvent&&
        theCountry->Industry >= event->industryForTriggerEvent&&
        theCountry->Oil >= event->oilForTriggerEvent){
        int size = event->map_area.size();
        if (size != 0){
            for(auto areaID : theCountry->AreaIDList){
                if (event->map_area.count(areaID))
                    return true;
            }
        }
        if (g_GameManager.GetRand() %100 + 1 <= event->percentOfTrigger)
            return true;
    }
    return false;
}

bool CommonEvent::Can_triggere_resolution(EventBase* event, CCountry* theCountry) {
    if (event->isFinished == false &&
        g_GameManager.CurrentTurnNumMinusOne >= event->startRound &&
        g_GameManager.CurrentTurnNumMinusOne <= event->endRound &&
        theCountry->Money >= event->moneyForTriggerEvent&&
        theCountry->Industry >= event->industryForTriggerEvent&&
        theCountry->Oil >= event->oilForTriggerEvent){
        int size = event->map_area.size();
        if (size != 0){
            for(auto areaID : theCountry->AreaIDList){
                if (event->map_area.count(areaID))
                    return true;
            }
        }
        if (g_GameManager.GetRand() %100 + 1 <= event->percentOfTrigger)
            return true;
    }
    return false;
}

////德国同意成立维希政府
//extern "C"
//JNIEXPORT void JNICALL
//Java_com_easytech_wc2_TestActivity_Event_1De1_1Choose1_1OccupyArea(JNIEnv *env, jobject thiz,
//                                                                   jstring beOccupyCountryID,
//                                                                   jstring OccupyCountryID, jint money,
//                                                                   jint industry,jint oil,jint percent,jstring newCountryID,jstring newCountryName) {
//    CCountry* beOccupyCountry = g_GameManager.FindCountry(env->GetStringUTFChars(beOccupyCountryID, JNI_FALSE));
//    CCountry* occupyCountry = g_GameManager.FindCountry(env->GetStringUTFChars(OccupyCountryID, JNI_FALSE));
//    Occupy_areas_ofTheCountry(beOccupyCountry,occupyCountry,percent);
//    const char* newCN = env->GetStringUTFChars(newCountryName,JNI_FALSE);
//    const char* newCI = env->GetStringUTFChars(newCountryID,JNI_FALSE);
//
//    event_CreatNewCountry(newCN,newCN,money,industry,oil,1,1,1.200,100,50,225,200, true,3,"common2",false);
//    occupyCountry->Money -= money;
//    occupyCountry->Industry -= industry;
//    occupyCountry->Oil -= oil;
//
//
//    CCountry* newCountry = g_GameManager.FindCountry(env->GetStringUTFChars(newCountryName,JNI_FALSE));
//    Add_areas_toTheCountry(newCountry,g_GameManager.EventTempListArea);
//    newCountry->Alliance = occupyCountry->Alliance;
//
//}

//德国解锁重坦克 火箭炮
extern "C"
JNIEXPORT void JNICALL
Java_com_easytech_wc2_testActivity_BtnEvent_1general_1UnlockEach_14(JNIEnv *env, jobject thiz,
                                                                    jstring country_id) {
    CCountry* theCountry = g_GameManager.FindCountry(env->GetStringUTFChars(country_id, JNI_FALSE));
    theCountry->Tech = 4;
}