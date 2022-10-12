//Auto:
//press end turn with battle animation settings on, and ai will take your turn for you
//battle animation is removed
//Skip:
//press end turn button while ai is playing to not watch
//changes in the ownerships of areas are still displayed
//Extra feature:
//mark neutral countries as defeated to exclude them from turn cycle and win check
//add player neutral country into defeat list
//TODO: Fix AI bug caused by controlling player country

#include <cmath>
#include <malloc.h>
#include <CGameManager.h>

#include "_CGameState.h"
#include "easytech.h"
#include "CStateManager/CGameState.h"
#include "CActionAI.h"
#include "CFightTextMgr.h"
#include "CGameSettings.h"
#include "JNI_Fun.h"
static bool SkipMode;
static bool SpectatorMode = false;
static bool AIAction;
static bool DisplayAreaChange;
static NewButton* g_newButton = new NewButton();

char str[16];   // 新增
char str1[16];   // 新增
char str2[16];   // 新增
char str3[16];   // 新增
char str4[16];   // 新增
def_easytech(_ZN10CGameState7OnEnterEv)
//每次进剧本时被调用
void CGameState::OnEnter() {
    easytech(_ZN10CGameState7OnEnterEv)(this);
    auto instance = GUIManager::Instance();
    g_newButton->Add_newButtons(instance);

    if (g_GameManager.IsNewGame)
        g_GameManager.SaveGame(
                (g_GameManager.GameMode == CGameManager::Conquest) ? "conquest6.sav" : "game6.sav");
}

//Initialize Auto and Skip flags
//Also: mark neutral countries as defeated to exclude them from turn cycle and win check
//Also: add player neutral country into defeat list
void CGameManager::InitBattle() {
    if (this->IsNewGame) {
        this->LoadBattle(this->BattleFileName);
        SpectatorMode = false;
        this->CurrentDialogueIndex = 0;
        this->CurrentTurnNumMinusOne = 0;
        this->RandomRewardMedal = 0;
        this->MovePlayerCountryToFront();
        CCountry *PlayerCountry = this->GetPlayerCountry();
        if (PlayerCountry != NULL)
            this->SetPlayercountryname(0, PlayerCountry->Name);
        size_t i;
        for (i = 0; i < this->ListCountry.size(); i++) {
            CCountry *country = this->ListCountry[i];
            if (this->GameMode == CGameManager::Conquest && PlayerCountry != NULL &&
                country == GetPlayerCountry())

                //取消盟友税率强制为1的设定  country->Alliance == PlayerCountry->Alliance 改为   country == GetPlayerCountry()
                country->TaxRate = 1.0;
            if (country->Alliance == CCountry::NeutralID) {
                if (country->AI)
                    country->Defeated = true;
                else
                    this->DefeatCountry.push_back(country);
            }
        }
        this->InitCameraPos();
    }
    else
        this->RealLoadGame(this->LoadFileName);
    this->Result = false;
    this->CampaignRewardMedal = 0;
    this->LocalHost = true;
    this->CircleProgress = 0.65f;
    this->IsReduce = false;
    AIAction = false;
    this->Set_skipMode(false);

    g_Scene.AllAreasEncirclement();

    CActionAI::Instance()->InitAI();
}

void CGameManager::TurnBegin() {
//    JNI_Fun::Show_TestResult_Dialog();
    CCountry *country = *this;
    if (country == NULL)
        return;

    country->TurnBegin();
    // TODO 没写完的新ai，暂时设置为 true
    if (country->IsNewActionOver == true){
        NewCountryAction actionCountry;
        actionCountry.Start_countryAction(country);
        country->IsNewActionOver = true;
    }
    if (g_GameManager.CurrentTurnNumMinusOne == 0){
        g_GameManager.IsPause = false;
    }
    if (g_GameManager.Is_spectetorMode()){
        AIAction = true;
    }
    else{
        AIAction = false;
    }
    CGameState *game = static_cast<CGameState *>(CStateManager::Instance()->GetStatePtr(Game));
    if (country->AI) {
        game->AIProgressGUI->Show();
    }
    else {
        //游戏开始
        if (g_GameManager.Is_spectetorMode())
            return;
        game->HideAIProgress();
        if (this->GameMode != CGameManager::Tutorial)
            game->PlayerCountryBegin();
        this->Set_skipMode(false);
        int AreaID = country->GetHighestValueArea();
        if (AreaID >= 0) {
            CountryAction action = {CountryAction::CameraMoveAction, NoCard, 0, AreaID, 0};
            country->Action(action);
        }
    }
}

//Auto is implemented here 时间单位 秒  time只可能0~0.05
void CGameManager::GameUpdate(float time) {
    if (IsReduce == false)
        CircleProgress += 0.35 * time;
    else
        CircleProgress -= 0.35 * time;
    if (CircleProgress >= 1.0f){
        IsReduce = true;
    }else if (CircleProgress <= 0.65f){
        CircleProgress = 0.65f;
        IsReduce = false;
    }
    sprintf(str, "%d", g_GameManager.CurrentTurnNumMinusOne+1);   // 新增
    g_newButton->Get_newButton(NewButton::oilText)->SetText(str);   // 新增
    if (g_GameManager.IsIntelligentAI == true){
        sprintf(str1, "HAI");   // 新增
    }else{
        sprintf(str1, "NAI");   // 新增
    }
    if (g_GameManager.GameDifficulty == 1){
        sprintf(str2, "EGD");
    }else if (g_GameManager.GameDifficulty == 2){
        sprintf(str2, "NGD");
    }else if (g_GameManager.GameDifficulty == 3){
        sprintf(str2, "HGD");
    }
    if (g_GameManager.Is_spectetorMode()){
        sprintf(str4, "T");
    }else{
        sprintf(str4, "F");
    }

    g_newButton->Get_newButton(NewButton::aiTypeText)->SetText(str1);   // 新增
    g_newButton->Get_newButton(NewButton::gameDifficultyText)->SetText(str2);   // 新增
    sprintf(str3, "1.0.0");   // 新增
    g_newButton->Get_newButton(NewButton::versionText)->SetText(str3);   // 新增
    g_newButton->Get_newButton(NewButton::spectatorModeText)->SetText(str4);   // 新增
    if (!this->LocalHost)
        return;
    if (this->Result)
        return;
    CCountry *country = *this;
    if (country == NULL)
        return;
    country->Update(time);
    if (!country->IsActionFinish())
        return;
    CCountry *DefeatedCountry = this->GetNewDefeatedCountry();
    CGameState *GameState = static_cast<CGameState *>(CStateManager::Instance()->GetStatePtr(Game));
    if (DefeatedCountry != NULL) {
        GameState->ShowDefeated(DefeatedCountry);
        return;
    }
    if (country->AI) {
//        NewCountryAction newCountryAction;
//        newCountryAction.Start_countryAction(country);
        AIAction = true;
    }

    if (!AIAction)
        return;
    this->Next();
}
//游戏每更新一次，执行一次新的最小行动指令
void CGameManager::Next() {
    CCountry *Country = *this;
    if (Country == NULL)
        return;

    //如果不是ai，直接返回
    if (!AIAction)
        return;
    //! set neutral to be defeated in InitBattle so that their turns are skipped
    if (Country->AreaIDList.empty() || Country->Defeated) {
        this->EndTurn();
        return;
    }

    CActionNode *ActionNode = CActionAI::Instance()->setCpuDriverbyId(this->CurrentCountryIndex,
                                                                      this->AlwaysZero);

    if (ActionNode == NULL) {
        this->EndTurn();
        return;
    }
    CountryAction countryAction = {CountryAction::NoAction,
                                   ActionNode->CardID,
                                   ActionNode->StartAreaID,
                                   ActionNode->TargetAreaID,
                                   ActionNode->ArmyIndex};
    switch (ActionNode->ActionType) {
        default:
            return;
        case CActionNode::UseCardNode:
            countryAction.ActionType = CountryAction::UseCardAction;
            countryAction.ArmyIndex = 0;
            break;
        case CActionNode::ArmyMoveNode:
            if (ActionNode->StartAreaID == ActionNode->TargetAreaID)
                countryAction.ActionType = CountryAction::ArmyInactiveAction;
            else
                countryAction.ActionType = CountryAction::ArmyMoveAction;
            break;
        case CActionNode::ArmyAttackNode:
            countryAction.ActionType = CountryAction::ArmyAttackAction;
            break;
        case CActionNode::ArmyMoveFrontNode:
            countryAction.ActionType = CountryAction::ArmyMoveFrontAction;
    }
    Country->Action(countryAction);
}

void CGameManager::Set_skipMode(bool isSkip) {
    SkipMode = isSkip;
}

bool CGameManager::Get_skipMode() {
    return SkipMode;
}

void CGameManager::Set_gameGivenResult(bool isWin) {
    this->Result = true;
    this->ResultWin = isWin;
}

void CGameManager::Set_spectatorMode() {
    if (SpectatorMode){
        SpectatorMode = false;
    }
    else{
        SpectatorMode = true;
    }
}

bool CGameManager::Is_spectetorMode() {
    return SpectatorMode;
}


//Move camera while ai is controlling player country
bool CCountry::IsLocalPlayer() {
    return !AIAction;
}

//Skip is implemented here
//Render game scene when the ownership of any area changes
void CGameState::Update(float time) {

    if( g_GameManager.IsPause == true){
        return;
    }
    if (this->IdleTimerEnabled) {
        this->IdleTimer -= time;
        if (this->IdleTimer <= 0.0) {
            this->IdleTimer = 0.0;
            this->IdleTimerEnabled = false;
        }
    }
    if (this->BuyingCard && g_GameManager.GetPlayerCountry()->IsActionFinish()) {
        if (this->BuyCardGUI->CanBuySelCard())
            this->BuyCardGUI->ResetCardTarget();
        else {
            this->BuyCardGUI->ReleaseTarget();
            this->GameButtonCardRemove->Hide();
            this->GameButtonCard->Show();
            this->SmallCardGUI->Hide();
        }
        this->BuyingCard = false;
    }
    do {
        if ((this->PauseBoxGUI->Flags & GUIElement::Shown)
            || (this->OptionsGUI && (this->OptionsGUI->Flags & GUIElement::Shown))
            || (this->SaveGUI && (this->SaveGUI->Flags & GUIElement::Shown))
            || (this->WarningGUI && (this->WarningGUI->Flags & GUIElement::Shown))
            || (this->DialogueGUI && (this->DialogueGUI->Flags & GUIElement::Shown)))
            break;
        if (g_GameManager.IsManipulate()) {
            float speed[2];
            this->CTouchInertia.Update(time);
            if (this->CTouchInertia.GetSpeed(speed[0], speed[1])) {
                if (g_Scene.Move((int) (-speed[0] * time), (int) (-speed[1] * time)))
                    this->CTouchInertia.Stop();
            } else if (this->ShouldCameraAutoFixPos) {
                g_Scene.CCamera.SetAutoFixPos(true);
                this->ShouldCameraAutoFixPos = false;
            }
        } else {
            if (g_GameManager.Get_skipMode())
                time = 1.0;
            else if (g_GameSettings.Speed > 2)
                time *= (g_GameSettings.Speed == 3) ? 1.5 : 2.0;
        }
        ecEffectManager::Instance()->Update(time);
        g_Scene.Update(time);
        g_FightTextMgr.Update(time);
        if ((this->DefeatedGUI && (this->DefeatedGUI->Flags & GUIElement::Shown)) ||
            !this->GameNotInterrupted || g_GameManager.Result)
            break;
        if (!g_Scene.IsBombing())
            g_GameManager.GameUpdate(time);
    } while (g_GameManager.Get_skipMode() && !DisplayAreaChange);
    DisplayAreaChange = false;
}
def_easytech(_ZN10CCSoundBox6PlaySEEPKc)
int CCSoundBox::PlaySE(char const* str) {
    if (g_GameManager.Get_skipMode())
        return -1;
    return easytech(_ZN10CCSoundBox6PlaySEEPKc)(this, str);
}
//Capture area ownership change
void CCountry::RemoveArea(int AreaID) {
    DisplayAreaChange = true;
    this->AreaIDList.remove(AreaID);
    if (g_Scene[AreaID]->Type == capital)
        this->CapitalIDList.remove(AreaID);
}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 检测一个地块是否已存在于国家地块列表中，如果未存在，添加进去。注意要把该地块的国家标记为新拥有者<p>
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * void <p>
     *=======================================================<p>
     * 【创建人】： easytech <p>
     * 【修改人】： limlimg <p>
     * 【创建时间】： ？？？<p>
     * 【修改时间】： ？？？
     */
void CCountry::AddArea(int AreaID) {
    DisplayAreaChange = true;
    if (!this->FindArea(AreaID)) {
        this->AreaIDList.push_back(AreaID);
        if (g_Scene[AreaID]->Type == capital)
            this->CapitalIDList.push_back(AreaID);
    }
}

//Press end turn button
//Also: in game save-load menu related input
def_easytech(_ZN10CGameState7OnEventERK5Event)

bool CGameState::OnEvent(const Event &event) {
    switch (event.type) {
        case Event::GUI : {
            switch (event.info.GUI.type) {
                case Event::info::GUI::Button : {
                    g_GameManager.SaveGame(g_GameManager.GameMode == CGameManager::Conquest?"conquest6.sav":"game6.sav");
                    //新按钮
                    if (event.info.GUI.ptr ==  g_newButton->Get_newButton(NewButton::resolutionButton)){
                        JNI_Fun::Show_resolutionForm();
                        return true;
                    }
                    if (event.info.GUI.ptr ==  g_newButton->Get_newButton(NewButton::spectatorModeButton)){
                       if(g_AllowSpectatorMode==false)
                           return true;
                        g_GameManager.Set_spectatorMode();
                        return true;
                    }
                    if (event.info.GUI.ptr == this->GameButtonRound) {

                        if (g_GameManager.IsManipulate()) {
                            if (this->BuyCardGUI->CardCanTarget) {
                                this->BuyCardGUI->ReleaseTarget();
                                this->GameButtonCardRemove->Hide();
                                this->GameButtonCard->Show();
                                this->SmallCardGUI->Hide();
                                this->BuyingCard = false;
                            }
                            this->GameButtonCard->Hide();
                            this->ResetTouchState();
                            this->UnselectArea();
                            g_Scene.CCamera.SetAutoFixPos(true);
                            this->ShouldCameraAutoFixPos = false;
                            g_Scene.NewRoundFlashing = false;
                            if (g_GameSettings.BattleAnimation) {
                                AIAction = true;
                                CActionAI::Instance()->InitAI();
                            } else {
                                g_GameManager.EndTurn();
                                CActionAI::Instance()->InitAI();
                                this->UpdateAIProgress();
                            }
                        } else {
                            if (AIAction)
                                g_GameManager.Set_skipMode(true);
                        }
                        return false;
                    }
                    GUIPauseBox *PauseBox = this->PauseBoxGUI;
                    if (event.info.GUI.ptr == PauseBox->ButtonRestart) {
                        return this->OpenLoadEvent(event);
                    }
                    if (this->SaveGUI != NULL && event.info.GUI.ptr == this->SaveGUI->ButtonOK)
                        return this->CloseLoadEvent(event);
                    if (this->SaveGUI != NULL && event.info.GUI.ptr == this->SaveGUI->ButtonBack)
                        return this->CancelLoadEvent(event);
                    break;
                }
                default:
                    break;
            }
            break;
        }
        default:
            break;
    }
    return easytech(_ZN10CGameState7OnEventERK5Event)(this, &event);
}

//Don't move camera under skip mode
void CCamera::Upate(float time) {
    if (!this->IsMoving) return;
    if (g_GameManager.Get_skipMode()) {
        this->Speed[0] = 0.0;
        this->Speed[1] = 0.0;
        this->IsMoving = false;
        this->AutoFixPos = false;
        return;
    }
    int i;
    for (i = 0; i < 2; i++) {
        if (this->Speed[i] == 0.0) continue;
        if (fabs(this->TargetPos[i] - this->CenterPos[i]) < fabs(this->Speed[i] * 30.0 * time)) {
            this->CenterPos[i] = this->TargetPos[i];
            this->Speed[i] = 0.0;
        } else {
            this->CenterPos[i] += this->Speed[i] * 30.0 * time;
        }
    }
    if (this->Speed[0] == 0.0 && this->Speed[1] == 0.0) {
        this->IsMoving = false;
        this->AutoFixPos = false;
    }
}

//Disables battle animation
void CGameState::StartBattale(int StartAreaID, int TargetAreaID, int a3, bool Animation) {
    CArea* startArea = g_Scene.GetArea(StartAreaID);
    CArea* targetArea = g_Scene.GetArea(TargetAreaID);

    if (startArea->ArmyPos[0] >= targetArea->ArmyPos[0]) {
        startArea->SetArmyDir(0, -1.0);
        targetArea->SetArmyDir(0, 1.0);
    }
    else {
        startArea->SetArmyDir(0, 1.0);
        targetArea->SetArmyDir(0, -1.0);
    }
    CFight fight;
    _ZN6CFightC1Ev(&fight);
    fight.FirstAttack(StartAreaID, TargetAreaID);
    fight.ApplyResult();
    if (fight.StartArmySecondAttack || fight.TargetArmySecondAttack) {
        fight.SecondAttack();
        fight.ApplyResult();
    }
    _ZN6CFightD1Ev(&fight);
}

__BEGIN_DECLS
void _ZN9GUIBattleC1Ev(GUIBattle *self) { _ZN10GUIElementC1Ev((struct GUIElement *) self); }
void _ZN9GUIBattleC2Ev(GUIBattle *self) __attribute__ ((alias("_ZN9GUIBattleC1Ev")));
void _ZN9GUIBattleD1Ev(GUIBattle *self) { _ZN10GUIElementD1Ev((struct GUIElement *) self); }
void _ZN9GUIBattleD2Ev(GUIBattle *self) __attribute__ ((alias("_ZN9GUIBattleD1Ev")));
void _ZN9GUIBattleD0Ev(GUIBattle *self) {
    _ZN9GUIBattleD1Ev(self);
    free(self);
}
void _ZN9GUIBattle4InitERK7GUIRect() {}
__END_DECLS

/*WIP: Fix Auto AI
int calcAreaArmyHp(CArea *Area, bool TopOnly) {
    if (Area == NULL)
        return -1;
    if (Area->ArmyCount <= 0)
        return 0;
    CArmy *army;
    if (TopOnly) {
        army = Area->GetArmy(0);
        return (army != NULL) ? army->Hp : -1;
    }
    int i, s = 0;
    for (i = 0; i < Area->ArmyCount; i++) {
        army = Area->GetArmy(i);
        if (army == NULL)
            return -1;
        s += army->Hp;
    }
    return s;
}*/

//Stop the game from saving commander data on game closing, which is suspected to cause data loss
//Now it will save whenever a change in medal number is detected

#include <ecLibrary.h>

void CCommander::Save() {}

def_easytech(_ZN10CCommander4SaveEv)
def_easytech(_Z12ecGameUpdatef)

void ecGameUpdate(float time) {
    static int medal = 50;
    if (medal != g_Commander.Medal) {
        easytech(_ZN10CCommander4SaveEv)(&g_Commander);
        medal = g_Commander.Medal;
    }
    easytech(_Z12ecGameUpdatef)(time);
}