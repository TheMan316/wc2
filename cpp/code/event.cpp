#include <stdlib.h>
#include <string>
#include "event.h"



// Created by KK on 2021/9/11.
//
CommonEvent g_commonEvent;

//占领目标国所有的地块
void CommonEvent::Occupy_areas_ofTheCountry(CCountry* targetCOuntry, CCountry* attackCountry, int occupyEachAreaPercent){
    auto it = targetCOuntry->AreaIDList.begin();
    while (it != targetCOuntry->AreaIDList.end()) {
        //这个地块的id等于it这个引用所指的值 it赋值后自增1
        int AreaID = *it++;
        if (GetRand() % 100+1 <= occupyEachAreaPercent) {
            g_Scene[AreaID]->ClearAllArmy();
            targetCOuntry->RemoveArea(AreaID);
            g_Scene[AreaID]->Country = attackCountry;
            g_Scene.AdjacentAreasEncirclement(AreaID);
            attackCountry->AddArea(AreaID);
        }
    }
}

//为某个国家分配领土
void CommonEvent::Add_areas_toTheCountry(CCountry* country, std::vector<int>& listArea){
    if (country == nullptr)
        return;
    if (listArea.size() == 0)
        return;
    auto i = listArea.begin();
    while (i != listArea.end()){
        int areaID = *i++;
        CArea* area = g_Scene.GetArea(areaID);
        //目标国家
        CCountry* targetCountry = g_Scene[areaID]->Country;
        //清除这个地块的所有军队
        area->ClearAllArmy();
        //暂存国 等于目标国家
        CCountry* tempCountry = targetCountry;
        //这个国家添加这个地块
        country->AddArea(areaID);
        //这个地块的下标国家等于这个国家
        area->Country = country;
        g_Scene.AdjacentAreasEncirclement(areaID);
        //如果暂存国不是空的
        if (tempCountry != nullptr){
            //暂存国移除这个地块
            tempCountry->RemoveArea(areaID);
        }
    }
}
//改变这个国家所有军队的士气
void CommonEvent::Change_armysMorale_ofTheCountry(CCountry* country, int Morale, int MoraleUpTurn){
    if (country == nullptr)
        return;
    auto it = country->AreaIDList.begin();
    while (it!= country->AreaIDList.end()){
        int AreaID = *it++;
        for (int i = 0; i < g_Scene.GetArea(AreaID)->ArmyCount; ++i) {
            g_Scene.GetArea(AreaID)->Army[i]->Morale = Morale;
            g_Scene.GetArea(AreaID)->Army[i]->MoraleUpTurn = MoraleUpTurn;
        }
    }
}
//设置两个国家进入交战状态
void CommonEvent::ToWar(const char* id1,const char* id2){
    g_GameManager.FindCountry(id1)->ListArmisticeStatus[id2] = 0;
    g_GameManager.FindCountry(id2)->ListArmisticeStatus[id1] = 0;
}

void CommonEvent::Add_eventTurnBegin_toSelf(EventTurnBegin* event) {
    this->list_event_TurnBegin.push_back(event);
}

std::vector<EventTurnBegin*>* CommonEvent::Get_events_turnBegin() {
    return &this->list_event_TurnBegin;
}

void CommonEvent::Add_resources_toTheCountry(CCountry* country, int money, int industry, int oil) {
    if (country != nullptr){
        country->Money += money;
        country->Industry += industry;
        country->Oil += oil;
    }
}

void
CommonEvent::Creat_newCountry(const char* ID, const char* name, int money, int industry, int oil, int defeatType, int alliance, double taxfactor, int r, int g, int b, int a, bool isAI, int tech, const char* commanderName, bool isCoastal, int oilConversionRate) {
    CCountry* country = new CCountry();
    country->Init(ID, name);
    country->Money = money;
    country->Industry = industry;
    country->Oil = oil;
    country->DefeatType = defeatType;
    country->Alliance = alliance;
    country->TaxRate =(float)taxfactor;
    country->R = r;
    country->G = g;
    country->B = b;
    country->A = a;
    country->AI = isAI;
    country->Tech = tech;
    country->SetCommander(commanderName);
    country->IsCoastal = isCoastal;
    country->OilConversionRate = oilConversionRate;
    country->SpecialMark = 0;
    for (int i = 0; i < g_GameManager.ListCountry.size(); ++i) {
        country->ListArmisticeStatus[g_GameManager.ListCountry[i]->ID] = 0;
        g_GameManager.ListCountry[i]->ListArmisticeStatus[country->ID] = 0;
    }
    //初始化卡牌回合
    int cardSize = NewCardDef.size();
    for (int j = 0; j < cardSize; ++j) {
        country->New_Card_CDRound_Each[j] = NewCardDef[j]->Round;
    }
    for (int j = 0; j < cardSize; ++j) {
        country->New_Card_CDRound_Current[j] = country->New_Card_CDRound_Each[j];
    }
    //下一个单位的番号
    country->MaxArmyDesignationsLand = 1;
    country->MaxArmyDesignationsSea = 1;
    country->MaxArmyDesignationsAir = 1;
    country->MaxArmyDesignationsTank = 1;
    country->PolicyType_military = 4;
    country->PolicyType_military = 4;
    g_GameManager.ListCountry.push_back(country);
}

void CommonEvent::Dispose() {
    if (list_event_TurnBegin.size()==0)
        return;
    for(auto event : list_event_TurnBegin){
        delete event;
    }
    list_event_TurnBegin.clear();
}

void Tirgger_moveArmyTo_de(CCountry* country, std::vector<EventMoveArmyTo*>& list_event_moveArmyTo, int targetAreaID, const char* targetCountryID){
    int size = list_event_moveArmyTo.size();
    for (int i = 0; i < size; ++i) {
        EventMoveArmyTo* event = list_event_moveArmyTo[i];
        if(g_commonEvent.Can_triggere_armyMoveToEvent(event, targetAreaID, targetCountryID, country)){
            JNIEnv* env = nullptr;
            //打开窗口时游戏暂停
            GamePause();
            //不是AI才打开窗口
            if (g_vm != nullptr && (g_vm->GetEnv((void**) &env, JNI_VERSION_1_6) == JNI_OK) && g_GameManager.GetCurCountry()->AI == false) {
                //下面传进去事件号是为了方便按钮事件找到相关事件号然后进行调用
                jclass javaClass = env->FindClass("com/easytech/wc2/DeEvent");
                jmethodID method = env->GetStaticMethodID(javaClass, "展示_德国事件",
                                                          "(IIIIILjava/lang/String;Ljava/lang/String;)V");

                env->CallStaticVoidMethod(javaClass, method,
                                          event->eventNum,
                                          event->eventType,
                                          event->moneyForTriggerEvent,
                                          event->industryForTriggerEvent,
                                          event->oilForTriggerEvent,
                                          env->NewStringUTF(event->theCountryID),
                                          env->NewStringUTF(event->theCountryName));
                event->isFinished = true;
            }
            return;
        }
    }
}
void Trigger_eventMoveArmyTo_theScript(CCountry* country, CArea* area){
    std::vector<EventMoveArmyTo*>* listEventOccupyArea = &country->List_event_moveArmyTo;
    if ((*listEventOccupyArea).size() == 0)
        return;
    if (strcmp(country->ID, "de") == 0) {
        Tirgger_moveArmyTo_de(country, *listEventOccupyArea, area->ID, country->ID);
    }
}
void Tirgger_eventTurnBegin_common(std::vector<EventTurnBegin*>& events) {
    int size = events.size();
    for (int i = 0; i < size; ++i) {
        EventTurnBegin* event = events[i];
        if(g_commonEvent.Can_triggere_turnBeginEvent(event, nullptr)){
            JNIEnv* env = nullptr;
            //不是AI才打开窗口
            if (g_vm != nullptr && (g_vm->GetEnv((void**) &env, JNI_VERSION_1_6) == JNI_OK) && g_GameManager.GetCurCountry()->AI == false) {
                //打开窗口时游戏暂停
                GamePause();
                //下面传进去事件号是为了方便按钮事件找到相关事件号然后进行调用
                jclass javaClass = env->FindClass("com/easytech/wc2/CommonEvent");
                jmethodID method = env->GetStaticMethodID(javaClass, "展示_公共事件",
                                                          "(IIIIILjava/lang/String;Ljava/lang/String;)V");

                env->CallStaticVoidMethod(javaClass, method,
                                          event->eventNum,
                                          event->eventType,
                                          event->moneyForTriggerEvent,
                                          event->industryForTriggerEvent,
                                          event->oilForTriggerEvent,
                                          env->NewStringUTF(event->theCountryID),
                                          env->NewStringUTF(event->theCountryName));
            }
            return;
        }
    }
}

void Tirgger_eventTurnBegin_de(CCountry* theCountry, std::vector<EventTurnBegin*>& events) {
    int size = events.size();
    for (int i = 0; i < size; ++i) {
        EventTurnBegin* event = events[i];
        if(g_commonEvent.Can_triggere_turnBeginEvent(event, theCountry)){
            JNIEnv* env = nullptr;
            //不是AI才打开窗口
            if (g_vm != nullptr && (g_vm->GetEnv((void**) &env, JNI_VERSION_1_6) == JNI_OK) && g_GameManager.GetCurCountry()->AI == false) {
                //打开窗口时游戏暂停
                GamePause();
                //下面传进去事件号是为了方便按钮事件找到相关事件号然后进行调用
                jclass javaClass = env->FindClass("com/easytech/wc2/DeEvent");
                jmethodID method = env->GetStaticMethodID(javaClass, "展示_德国事件",
                                                          "(IIIIILjava/lang/String;Ljava/lang/String;)V");

                env->CallStaticVoidMethod(javaClass, method,
                                          event->eventNum,
                                          event->eventType,
                                          event->moneyForTriggerEvent,
                                          event->industryForTriggerEvent,
                                          event->oilForTriggerEvent,
                                          env->NewStringUTF(event->theCountryID),
                                          env->NewStringUTF(event->theCountryName));
            }
            return;
        }
    }
}

void Trigger_eventTurnBegin_theScript(CCountry *country) {
    if (g_GameManager.GameMode != CGameManager::Conquest)
        return;
    Tirgger_eventTurnBegin_common(*g_commonEvent.Get_events_turnBegin());
    auto events = &country->List_event_turnBegin;
    int size = events->size();
    if (size ==0)
        return;
    if (strcmp(country->ID, "de") == 0) {
        Tirgger_eventTurnBegin_de(country, *events);

    }
}



void Init_events_theScript() {

    Init_events_common_conquer1();
    Init_events_de_conquer1();
    Init_resolutions_de_conquer1();

}

//征服剧本从0开始
void Init_resolutions_theScript(int sciptNum) {
    g_commonEvent.Dispose();

    EventTurnBegin *event_0001 = new EventTurnBegin();
    event_0001->Set_condition(0, 0);
    event_0001->Set_countryText(1, CommonEvent::turnBegin, "null", "null");
    g_commonEvent.Add_eventTurnBegin_toSelf(event_0001);
    switch (sciptNum) {
        case 0:
            Init_events_theScript();
            break;
    }
}



extern "C"
JNIEXPORT void JNICALL
Java_com_easytech_wc2_BaseEvent_BtnEvent_1addResource(JNIEnv* env, jobject thiz,jstring the_country_id ,jint money, jint industry, jint oil) {
    CCountry* theCountry = g_GameManager.FindCountry(env->GetStringUTFChars(the_country_id, JNI_FALSE));
    g_commonEvent.Add_resources_toTheCountry(theCountry, money, industry, oil);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_easytech_wc2_BaseEvent_BtnEvent_1ChooseAI(JNIEnv* env, jobject thiz, jint level) {
    if (level == 2){
        g_GameManager.IsIntelligentAI = true;
    }
    else{
        g_GameManager.IsIntelligentAI = false;
    }
    //游戏继续
    GameStart();
}
extern "C"
JNIEXPORT void JNICALL
Java_com_easytech_wc2_BaseEvent_BtnEvent_1ChooseDifficulty(JNIEnv* env, jobject thiz, jint level) {
    if (level == 1){
        for (int i = 0; i < g_GameManager.ListCountry.size(); ++i) {
            g_GameManager.ListCountry[i]->TaxRate = 1;
        }
        g_GameManager.GameDifficulty = 1;
    }else if (level == 3){
        CCountry* playerCountry = g_GameManager.GetPlayerCountry();
        for (int i = 0; i < g_GameManager.ListCountry.size(); ++i) {
            if (g_GameManager.ListCountry[i] != playerCountry){
                if (g_GameManager.ListCountry[i]->Alliance == playerCountry->Alliance){
                    g_GameManager.ListCountry[i]->TaxRate *= 1.4;
                }
                else {
                    g_GameManager.ListCountry[i]->TaxRate *= 2;

                }
            }
        }
        g_GameManager.GameDifficulty = 3;
    }else{
        g_GameManager.GameDifficulty = 2;
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_easytech_wc2_BaseEvent_BtnEvent_1EventFinished(JNIEnv* env, jobject thiz, jint event_num, jint event_type,jstring countryID) {
    g_commonEvent.Close_event(env->GetStringUTFChars(countryID, JNI_FALSE), event_num, event_type, 1);
    //游戏继续
    GameStart();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_easytech_wc2_BaseEvent_BtnEvent_1ChangeArmysMorale(JNIEnv* env, jobject thiz, jstring the_country_id, jint turns, jint morale_type) {
    auto theCountry = g_GameManager.FindCountry(env->GetStringUTFChars(the_country_id,JNI_FALSE));
    g_commonEvent.Change_armysMorale_ofTheCountry(theCountry, morale_type, turns);
}





extern "C"
JNIEXPORT void JNICALL
Java_com_easytech_wc2_DeEvent_BtnEvent_10003_1De(JNIEnv* env, jclass clazz, jint event_num, jint event_type) {
    auto fr = g_GameManager.FindCountry("fr");
    auto de = g_GameManager.FindCountry("de");
    if (fr != nullptr){

        g_commonEvent.Occupy_areas_ofTheCountry(fr, de, 70);
        g_commonEvent.Creat_newCountry("newFr","fr",0,0,0,0,de->Alliance,1,222,222,111,111,true,2,"common3",
                                       false,1);
        std::vector<int> list_area;
        list_area.push_back(981);
        list_area.push_back(986);
        list_area.push_back(987);
        auto newFr = g_GameManager.FindCountry("newFr");
        g_commonEvent.Add_areas_toTheCountry(newFr, list_area);
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_easytech_wc2_testActivity_TriggerResulution(JNIEnv* env, jobject thiz, jstring country_id, jint event_num, jint btn_num) {
    g_commonEvent.Close_event(env->GetStringUTFChars(country_id, JNI_FALSE), event_num, 3, btn_num);
}