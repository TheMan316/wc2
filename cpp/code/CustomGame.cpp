//Customize game rule

#define extends_CArea \
bool CanContainArmy(CArmy *army);

#include "_CActionAI.h"
#include "CustomizableData.h"
#include "CActionAI.h"
#include "CLi.h"
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * void <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： 2022 <p>
     * 【修改时间】： ？？？
     */
bool CScene::NewCheckMoveAble(int StartAreaID, int TargetAreaID, CArmy* attackArmy, int ArmyAreaID){
    CArea* targetArea = g_Scene[TargetAreaID];
    CArea* armyArea = g_Scene[ArmyAreaID];

    if (armyArea == nullptr || targetArea == nullptr || !armyArea->Enable || !targetArea->Enable){
        return false;
    }
    //如果是停战状态，返回false
    if (targetArea->Country != nullptr && g_Scene.Is_armisticeStatus(armyArea->Country,targetArea->Country)) {
        return false;
    }
    bool is_adjcentArea = g_Scene.CheckAdjacent(StartAreaID,TargetAreaID);
    //不相等说明是ai在寻路
    if (StartAreaID != ArmyAreaID){
        if (is_adjcentArea == false ){
            if (attackArmy->BasicAbilities->Movement < targetArea->Get_consumedMovement())
                return false;
        }
        //禁止火箭炮往有敌军在附近的地方走
        if (g_GameManager.IsIntelligentAI && attackArmy->Is_remoteAttack() && (attackArmy->Country->AI || g_GameManager.Is_spectetorMode())){
            int num = g_Scene.GetNumAdjacentAreas(TargetAreaID);
            for (int i = 0; i < num; ++i) {
                auto adjacentArea = g_Scene.GetAdjacentArea(TargetAreaID,i);
                if (g_Scene.Is_waring(adjacentArea->ID,ArmyAreaID)){
                    return false;
                }
            }
        }
    }
    //不在寻路
    else{
        if (is_adjcentArea == false)
            return false;
        if (targetArea->ArmyCount == 4)
            return false;
        if (attackArmy->Movement < targetArea->Get_consumedMovement())
            return false;
        //禁止ai的火箭炮和空军前往附近有敌军的地块
        if (g_GameManager.IsIntelligentAI && (armyArea->Country->AI || g_GameManager.Is_spectetorMode())){
            if (attackArmy->Is_remoteAttack() || attackArmy->Is_airForce()) {
                return false;
            }
        }
    }
    return true;
}



bool CScene::CheckMoveable(int StartAreaID, int TargetAreaID, int ArmyIndex) {
    //这个不应移动！
    if (!this->CheckAdjacent(StartAreaID, TargetAreaID))
        return false;
    return CActionAssist::Instance()->aiCheckMoveable(StartAreaID, TargetAreaID, ArmyIndex,
                                                      StartAreaID);

}

bool CArea::CanContainArmy(CArmy *army) {
    if (this->ArmyCount >= 4)
        return false;
    int i, LandCount = 0, NavyCount = 0;
    if (this->Sea){
        for (i = 0; i < this->ArmyCount; i++){
            if (this->GetArmy(i)->IsNavy())
                NavyCount++;
            else
                LandCount++;
        }
        if (NavyCount >= 3 && army->IsNavy())
            return false;
        return (army->Has_engineeringCorps() ||army->IsNavy() );
    }
        return !army->IsNavy();
}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 如果用于ai寻路，startAreaID和ArmyAreaID是不一样的.<p>
     * 调用这个函数，说明Start和Target必定相连。<p>
     * 只有当非寻路状态，startArea不等于ArmyArea时，我方满4移动力才不可通行。<p>
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * 返回结果 <p>
     *=======================================================<p>
     * 【创建人】： limlimg <p>
     * 【修改人】： 李德邻<p>
     * 【创建时间】： ？？？ <p>
     * 【修改时间】： 2022/10/10
     */
bool
CActionAssist::aiCheckMoveable(int StartAreaID, int TargetAreaID, int ArmyIndex, int ArmyAreaID) {
    //老 稳定
//    CArea *ArmyArea = g_Scene.GetArea(ArmyAreaID);
//    CArea *TargetArea = g_Scene.GetArea(TargetAreaID);
//    if (ArmyArea->Country != TargetArea->Country && TargetArea->ArmyCount != 0)
//        return false;
//    if (ArmyArea->ArmyCount <= ArmyIndex)
//        return false;
//    CArmy *Army = ArmyArea->GetArmy(ArmyIndex);
//    if (Army->Movement <= 0)
//        return false;
//    return TargetArea->CanContainArmy(Army);
    ////////////////////////////////  新

    CArea *ArmyArea = g_Scene[ArmyAreaID];
    CArea *TargetArea = g_Scene[TargetAreaID];
    if ( TargetArea->Country != nullptr && ArmyArea->Country != TargetArea->Country && TargetArea->ArmyCount != 0)
        return false;
    //是否可以移动到盟友地块
    if (ArmyArea->Country != TargetArea->Country && g_Scene.Is_ally(ArmyAreaID,TargetAreaID)){
        if (TargetArea->ArmyCount > 0){
            return false;
        }
        else if(TargetArea->Sea == false){
            return false;
        }
    }
    if (ArmyArea->ArmyCount <= ArmyIndex)
        return false;
    CArmy *Army = ArmyArea->GetArmy(ArmyIndex);
    if (Army->Movement <= 0)
        return false;
    if ( !g_Scene.NewCheckMoveAble(StartAreaID,TargetAreaID,Army,ArmyAreaID))
        return false;
    //有关海上移动
    return TargetArea->CanContainArmy(Army);
}

bool CScene::CheckAttackable(int StartAreaID, int TargetAreaID, int ArmyIndex) {

        return  CActionAssist::Instance()->aiCheckAttackable( StartAreaID, TargetAreaID, ArmyIndex, StartAreaID);
//    CArea *StartArea = g_Scene.GetArea(StartAreaID);
//    CArea* targetArea = g_Scene.GetArea(TargetAreaID);
//    CArmy* army = StartArea->GetArmy(ArmyIndex);
//    if (army->Is_remoteAttack()) {
//        auto listArea = StartArea->Get_areas_inRange(ArmyIndex);
//        int num = listArea->size();
//        bool canAttack = false;
//        for (int i = 0; i < num; ++i) {
//            CArea* foundArea = g_Scene.GetArea((*listArea)[i]->ID);
//            //必须处于迷雾外才能攻击
//            if (targetArea == foundArea && g_Scene.Is_inWarFog(targetArea,StartArea->Country->Alliance) == false) {
//                 canAttack = true;
//                break;
//            }
//        }
//        delete listArea;
//        if (canAttack == false)
//            return false;
//    }
//
//    else if (army->Is_radiudAttack()) {
//        float d = this->GetTwoAreasDistance(StartAreaID, TargetAreaID);
//        return (d > 0.0 && d < StartArea->Country->AirstrikeRadius() );
//    }
//    return this->CheckAdjacent(StartAreaID, TargetAreaID);


//    return this->CheckAdjacent(StartAreaID, TargetAreaID);
}
