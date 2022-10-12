#include <GUI/GUIElement.h>
#include <easytech.h>
#include <CStateManager/CMenuState.h>
#include <jni.h>
#include <CStateManager/CGameState.h>
#include <tinyxml.h>
#include <cstring>
#include "JNI_Fun.h"
JavaVM *g_vm = NULL;

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved){
    JNIEnv* env = NULL;
    if (vm->GetEnv((void**) &env, JNI_VERSION_1_6) != JNI_OK)
        return JNI_ERR;
    g_vm = vm;
    return JNI_VERSION_1_6;
}

def_easytech(_ZN10CMenuState7OnEventERK5Event)
bool CMenuState::OnEvent(const Event &event){
    switch (event.type) {
        case Event::GUI : {
            switch (event.info.GUI.type) {
                case Event::info::GUI::Other : {
                    switch (event.info.GUI.info){
                        case OTHER_HOME:{
                            JNIEnv* env = NULL;
                            if (g_vm != NULL && (g_vm->GetEnv((void**) &env, JNI_VERSION_1_6) == JNI_OK)){
                                jclass cls_activity = env->FindClass("com/easytech/wc2/ModActivity");
                                jmethodID mtd_getInfoPopup = env->GetStaticMethodID(cls_activity,"getInfoPopup","()Lcom/easytech/wc2/InfoPopup;");
                                jobject obj_infoPopup = env->CallStaticObjectMethod(cls_activity,mtd_getInfoPopup);
                                jclass cls_infoPopup = env->FindClass("com/easytech/wc2/InfoPopup");
                                jmethodID mtd_showOnUiThread = env->GetMethodID(cls_infoPopup,"showOnUiThread","()V");
                                if (obj_infoPopup != NULL){
                                    env->CallVoidMethod(obj_infoPopup ,mtd_showOnUiThread);
                                    return false;
                                }
                            }

                        }
                    }
                }
            }
        }
    }
    return easytech(_ZN10CMenuState7OnEventERK5Event)(this,&event);
}

struct TerrainInfo {
    const char *Name;
    const char *Type;
    int MovementCost;
    int Penalty_Infantry;
    int Penalty_Artillery;
    int Penalty_Armor;
    int Penalty_Navy;
    int Penalty_AirForce;
};

std::map<AreaType, TerrainInfo> TerrainData = {{forest, {"森林", "forest", 2, -25, 0, -25, 0, -30}},
                                               {hilly, {"丘陵", "hilly", 3, -30, -30, -30, 0, -30}},
                                               {desert, {"沙漠", "desert", 2, 0, 30, 0, 0, 40}}};

void ShowTerrainInfo(TerrainInfo Info) {
    JNIEnv* env = nullptr;
    if (g_vm != nullptr && (g_vm->GetEnv((void**) &env, JNI_VERSION_1_6) == JNI_OK)) {
        jclass activity = env->FindClass("com/easytech/wc2/testActivity");
        jmethodID method = env->GetStaticMethodID(activity, "展示_地形信息_窗口", "(Ljava/lang/String;Ljava/lang/String;IIIIII)V");
        env->CallStaticVoidMethod(activity, method, env->NewStringUTF(Info.Name), env->NewStringUTF(Info.Type), Info.MovementCost, Info.Penalty_Infantry, Info.Penalty_Artillery, Info.Penalty_Armor, Info.Penalty_Navy, Info.Penalty_AirForce);

    }
}

void HideTerrainInfo() {
    JNIEnv* env = nullptr;
    if (g_vm != nullptr && (g_vm->GetEnv((void**) &env, JNI_VERSION_1_6) == JNI_OK)) {
        jclass activity = env->FindClass("com/easytech/wc2/testActivity");
        jmethodID method = env->GetStaticMethodID(activity, "hideTerrainInfo", "()V");
        env->CallStaticVoidMethod(activity, method);
    }
}

void CGameState::UnselectArea() {
    HideTerrainInfo();
    g_Scene.UnselectArea();
    this->TaxGUI->Hide();
    this->SelArmyGUI->Hide();
}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 选中地块时，右边单位框弹出。AI的地块被选中不会弹出。<p>
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * void <p>
     *=======================================================<p>
     * 【创建人】： limlimg <p>
     * 【修改人】： 李德邻 <p>
     * 【创建时间】： ？？？<p>
     * 【修改时间】： ？？？
     */
void CGameState::SelectArea(int AreaID) {
    if (TerrainData.count(g_Scene[AreaID]->Type)) {
        ShowTerrainInfo(TerrainData[g_Scene[AreaID]->Type]);
    }
    g_Scene.SelectArea(AreaID);
    this->TaxGUI->SetArea(AreaID);
    this->TaxGUI->Show();
    if (g_Scene[AreaID]->ArmyCount > 1) {
        this->SelArmyGUI->SetArea(AreaID);
        this->SelArmyGUI->UseCard = false;
        if (g_ShowArmysInfoOfEachArea){
            this->SelArmyGUI->Show();
        }
        else if(g_Scene[AreaID]->Country == g_GameManager.GetCurCountry()){
            this->SelArmyGUI->Show();
        }
    }
}


void CScene::LoadAreaTax(int MapID) {
    TiXmlDocument Xml{};
    char TaxFileName[16];
    ClearAreas();
    sprintf(TaxFileName, "areatax%d.xml", MapID);
    _ZN13TiXmlDocumentC2EPKc(&Xml, GetPath(TaxFileName, nullptr));
    if (Xml.LoadFile()) {
        TiXmlNode *AreaNode = Xml.FirstChild("areas");
        if (AreaNode != nullptr) {
            for (int i = 0; i < AreaCount; i++) {
                auto *Area = new CArea();
                Area->Init(i, {normal, 100, {AreasData[i].AreaPos[0], AreasData[i].AreaPos[1]}});
                AreaList.push_back(Area);
            }
            //循环整个areas节点
            for (TiXmlElement *AreasElement = AreaNode->FirstChildElement(); AreasElement != nullptr; AreasElement = AreasElement->NextSiblingElement()) {
                //创建AreaInfo并初始化
                AreaInfo areaInfo = {normal, 0,};
                //分别对应三个属性
                int AreaID, Tax ,Oil,CityGrowthRate,IndustryGrowthRate,OilGrowthRate;
                Oil = 0;
                CityGrowthRate = 0;
                IndustryGrowthRate= 0;
                OilGrowthRate = 0;
                const char *Type = AreasElement->Attribute("type");
                areaInfo.Type = Type != nullptr ?
                                !strcmp(Type, "capital") ? capital :
                                !strcmp(Type, "port") ? port :
                                !strcmp(Type, "large city") ? large_city :
                                !strcmp(Type, "normal city") ? normal_city :
                                !strcmp(Type, "forest") ? forest :
                                !strcmp(Type, "hilly") ? hilly :
                                !strcmp(Type, "desert") ? desert : normal
                                                : normal;
                if (AreasElement->QueryIntAttribute("id", &AreaID) != TIXML_SUCCESS)
                    AreaID = 0;
                if (AreasElement->QueryIntAttribute("tax", &areaInfo.Tax) != TIXML_SUCCESS)
                    areaInfo.Tax = Tax;
                if (AreasElement->QueryIntAttribute("oil", &AreaList[AreaID]->Oil) != TIXML_SUCCESS)
                    AreaList[AreaID]->Oil  = Oil;
                if (AreasElement->QueryIntAttribute("cityGrowthRate", &AreaList[AreaID]->CityGrowthRate) != TIXML_SUCCESS)
                    AreaList[AreaID]->CityGrowthRate  = CityGrowthRate;
                if (AreasElement->QueryIntAttribute("industryGrowthRate", & AreaList[AreaID]->IndustryGrowthRate ) != TIXML_SUCCESS)
                    AreaList[AreaID]->IndustryGrowthRate  = IndustryGrowthRate;
                if (AreasElement->QueryIntAttribute("oilGrowthRate", & AreaList[AreaID]->OilGrowthRate) != TIXML_SUCCESS)
                    AreaList[AreaID]->OilGrowthRate  = OilGrowthRate;
                areaInfo.ArmyPos[0] = AreasData[AreaID].ArmyPos[0];
                areaInfo.ArmyPos[1] = AreasData[AreaID].ArmyPos[1];
                areaInfo.ConstructionPos[0] = AreasData[AreaID].ConstructPos[0];
                areaInfo.ConstructionPos[1] = AreasData[AreaID].ConstructPos[1];
                areaInfo.InstalltionPos[0] = AreasData[AreaID].InstallationPos[0];
                areaInfo.InstalltionPos[1] = AreasData[AreaID].InstallationPos[1];
                AreaList[AreaID]->Init(AreaID, areaInfo);
                AreaList[AreaID]->Sea = AreasData[AreaID].Type == AreaData::SEA;

            }
        }
    }
    _ZN13TiXmlDocumentD1Ev(&Xml);

}