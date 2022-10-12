//fix original game bugs
//Fix: AI rocket melee attack

#include "_CActionAI.h"
#include "CActionAI.h"
#include "JNI_Fun.h"

bool CScene::NewCheckAttackAble(int StartAreaID, int TargetAreaID, int ArmyIndex,
                                       int ArmyAreaID){
    CArea *armyArea = g_Scene[ArmyAreaID];
    CArea *targetArea = g_Scene[TargetAreaID];
    CArea* startArea =  g_Scene[StartAreaID];
    CArmy* attackArmy = armyArea->GetArmy(ArmyIndex);
    CArmy* targetArmy = targetArea->GetArmy(0);

    //如果是潜艇，无法攻击陆地
    if (attackArmy->BasicAbilities->ID == ArmyDef::Submarine && targetArea->Sea == false){
        return false;
    }
    ArmyDef::ArmyType ArmyType = attackArmy->BasicAbilities->ID;
    //相等则说明不是寻路
    if (StartAreaID == ArmyAreaID){
        //远程攻击
        if (attackArmy->Is_remoteAttack()) {
            auto listArea = armyArea->Get_areas_inRange(ArmyIndex);
            int num = listArea->size();
            bool canAttack = false;
            for (int i = 0; i < num; ++i) {
                CArea* foundArea = g_Scene.GetArea((*listArea)[i]->ID);
                //必须处于迷雾外才能攻击
                if (targetArea == foundArea && g_Scene.Is_inWarFog(targetArea, armyArea->Country->Alliance) == false) {
                    canAttack = true;
                    break;
                }
            }
            delete listArea;
            if (canAttack == false)
                return false;
        }
            //圆半径
        else if (attackArmy->Is_radiudAttack()){
            //航空母舰
            if(ArmyType == ArmyDef::AircraftCarrier) {
                float d = g_Scene.GetTwoAreasDistance(StartAreaID, TargetAreaID);
                if (d > attackArmy->GetAirstrikeRadius()) {
                    return false;
                }
            }
                //火箭发射车
            else if (ArmyType == ArmyDef::IntercontinentalMissile ) {
                if (armyArea->Army[ArmyIndex]->Hp > 30) {
                    float d = g_Scene.GetTwoAreasDistance(StartAreaID, TargetAreaID);
                    if (d < 300.0 || d > 1500.0) {
                        return false;
                    }
                }
                else{
                    return false;
                }
            }
                //战斗机
            else if (attackArmy->Is_fightPlane() ) {
                float d = g_Scene.GetTwoAreasDistance(StartAreaID, TargetAreaID);
                if( d > attackArmy->GetAirstrikeRadius())
                    return false;

            }
                //轰炸机
            else if (attackArmy->Is_boomPlane()) {
                float d = g_Scene.GetTwoAreasDistance(StartAreaID, TargetAreaID);
                if(d > attackArmy->GetAirstrikeRadius() || targetArmy->Is_airForce() == false){
                    return false;
                }

            }
        }
        else{
            //普通近战返回是否接壤
            return g_Scene.CheckAdjacent(ArmyAreaID, TargetAreaID);
        }
    }
    //寻路状态下，任意地块均视为可攻击
    else{
        return false;
    }
    return true;
}

bool CActionAssist::aiCheckAttackable(int StartAreaID, int TargetAreaID, int ArmyIndex,
                                           int ArmyAreaID) {
 //旧
//        if (g_Scene.Is_ally(StartAreaID,TargetAreaID))
//            return false;
//        CArea *ArmyArea = g_Scene.GetArea(ArmyAreaID);
//        if (ArmyArea->ArmyCount <= ArmyIndex)
//            return false;
//        CArea *TargetArea = g_Scene.GetArea(TargetAreaID);
//        if (TargetArea->ArmyCount <= 0)
//            return false;
//        CArmy *Army = ArmyArea->GetArmy(ArmyIndex);
//        //Fix AI rocket melee attack
//        if (!Army->Active || Army->Movement <= 0){
//            return false;
//        }
//        if (Army->Is_remoteAttack() && g_Scene.CheckAdjacent(ArmyAreaID, TargetAreaID)) {
//            return false;
//        }
//        return Army->Movement > 0;

    ////////////////////////////////////////////////////
    CArea *armyArea = g_Scene[ArmyAreaID];
    CArea* startArea = g_Scene[StartAreaID];
    CArea *targetArea = g_Scene[TargetAreaID];
    CArmy* attackArmy = armyArea->GetArmy(ArmyIndex);
    if ( targetArea == nullptr || !startArea->Enable ||
        !targetArea->Enable || startArea->Country == nullptr || targetArea->Country == nullptr)
        return false;
    if (armyArea->Country == targetArea->Country)
        return false;
    if (g_Scene.Is_ally(armyArea->ID, targetArea->ID))
        return false;
    if (startArea->ArmyCount <= ArmyIndex)
        return false;
    if (targetArea->ArmyCount <= 0)
        return false;
    if (attackArmy->Movement < 1 || attackArmy->Active == false)
        return false;
    //如果是停战状态，返回false
    if (g_Scene.Is_armisticeStatus(armyArea->Country, targetArea->Country)){
            return false;
    }
    return g_Scene.NewCheckAttackAble(StartAreaID,TargetAreaID,ArmyIndex,ArmyAreaID);
}



