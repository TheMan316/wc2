//For conquests, countries with ai="0" are available
//If no country is available in this way, then non-neutral countries are available
//The picture of country button will use its id. If the corresponding picture isn't found, its name is used

#include <tinyxml.h>
#include "CScene.h"
#include <queue>
#include <cstring>
#include <stdlib.h>
#include <CGameRes.h>
#include <easytech.h>
#include <vector>
#include <CActionAI.h>
#include "CGameSettings.h"
#include "CFightTextMgr.h"
#include "CLi.h"
#include "event.h"
#include "GUI/GameGUI.h"
#include "JNI_Fun.h"
#include <cstring>
#include <GUI/GUIElement.h>
#include <CGameManager.h>
#include <stack>
#include "CWarGas.h"

static bool s_NewArmyAttribute = false; //装甲硬度和穿甲值的开关
static bool s_Autoconstruction_byGrowthRate = true;
static bool s_OpenWarFog = true;
bool g_AllowSpectatorMode = false;
bool g_ShowArmysInfoOfEachArea = false;
#define NewArmyStartID 28
AreaSurroundingInfoManager g_areaSurroundingInfoManager;
NewAiAction g_newAiAction;
Algorithm g_algorithm;
//这里不用管，会自动添加  存放各个类别卡牌的id

static int s_ArrCardType[5][128] = {{0,  1,  2,  3,  4,  5},
                                    {6,  7,  8,  9},
                                    {10, 11, 12, 13},
                                    {14, 15, 16, 17, 18, 19, 20},
                                    {21, 22, 23, 24, 25, 26, 27}};
static const int s_Count_theMapAreas = 1949;


static bool sArr_foundArea[s_Count_theMapAreas]; //用来记录某某地块是否已经被遍历过  它有一个初始化函数

//新兵种新卡牌容器
static std::vector<CountryArmyDef *> s_ListCountryArmydef;  //在Add_newArmyDef_toList 中为其添加元素
std::vector<CardDef *> NewCardDef; //在LoadCardDef中初始化  有几张卡就有几个元素
static std::vector<int *> s_ListNewCardIndex; //按顺序存放每个类别各个卡牌的下标


static char TempStr[128];
static char TempStrCity[16];
static char TempStrIndustry[16];
static char TempStrOil[16];
ecText *OilTaxText;
ecText *CityGrowthRateText;
ecText *IndustryGrowthRateText;
ecText *OilGrowthRateText;
ecText *OilGoldText;
ecText *OilCollectText;

static float arrUnitDrawnOffset[]= {10.0f,13.0f,15.0f,17.0f,10.0f,13.0f,15.0f,17.0f,10.0f,13.0f,15.0f,17.0f,10.0f,13.0f,15.0f,17.0f}; //16
//
bool IsNewCard(CARD_ID ID) {
    return ID >= NewArmyStartID;
}

//
bool CArmy::IsNavy() {
    return ((this->BasicAbilities->ID >= 6&&this->BasicAbilities->ID <= 8) || this->BasicAbilities->ID == ArmyDef::Submarine );
}

/*
 * 添加受到伤害的单位到DestroyCount
 */
void CCountry::AddDestroy(int ArmyType) {
    if (ArmyType <= 9) {
        ++this->DestroyCount[ArmyType];
    }
}

const CommanderAbility &GetCommanderAbility(int Level) {
    static const CommanderAbility data[] = {{0, 0, 0,  0, 0,  0},
                                            {0, 0, 10, 0, 5,  4},
                                            {0, 0, 15, 0, 10, 4},
                                            {0, 0, 20, 0, 15, 4},
                                            {1, 0, 25, 0, 20, 4},
                                            {1, 0, 30, 0, 25, 4},
                                            {1, 0, 35, 1, 30, 3},
                                            {1, 1, 40, 2, 35, 3},
                                            {1, 1, 45, 3, 40, 3},
                                            {1, 1, 50, 4, 45, 3},
                                            {2, 1, 55, 5, 50, 3},
                                            {2, 1, 60, 6, 55, 2},
                                            {2, 1, 65, 7, 60, 2},
                                            {3, 1, 70, 8, 65, 2},
                                            {3, 2, 75, 9, 70, 2}};
    if (Level > 14) {
        return data[14];
    }
    else {
        return data[Level];
    }
}

const NewArmyAbility& Get_armyAbility(int Level) {
    static const NewArmyAbility data[] = {{0, 0, 0,0,0,0},
                                          {1, 0, 1,0,0,0},
                                          {1, 1, 1,1,5,0},
                                          {1, 1, 2,1,10,5},
                                          {1, 1, 2,2,20,10}};
    if (Level > 4) {
        return data[4];
    }
    else {
        return data[Level];
    }
}


//给g_GameManager 设置种子
void SetRandSeed(unsigned int seed) {
    g_GameManager.G_Seed = seed;
}

void SetArmySeed(CArmy *army) {
    army->Seed = GetRand();
}

/*
 * 得到随机数种子
 * 定义在CLI.h
 */
unsigned int GetRandSeed() {
    return g_GameManager.G_Seed;
}

/*
 * 定义在CStateManager/CGameState.h
 */
unsigned int GetRand() {
    g_GameManager.G_Seed = 1103515245 * g_GameManager.G_Seed + 12345;
    return g_GameManager.G_Seed >> 16 & 32767;
}

unsigned int CGameManager::GetRand() {
    g_GameManager.G_Seed = 1103515245 * g_GameManager.G_Seed + 12345;
    return g_GameManager.G_Seed >> 16 & 32767;
}

unsigned int GetRand(CArmy *army) {
    army->Seed = 1103515245 * army->Seed + 12345;
    return army->Seed >> 16 & 32767;
}

/*
 * 得到该等级的攻击加成
 * 定义在CLi.h
 */
int GetArmyAtkBuff(int level) {
    return Get_armyAbility(level).Assault;
}

/*
 * 得到该等级的防御加成
 * 定义在CLi.h
 */
int GetArmyDfsBuff(int level) {
    return Get_armyAbility(level).Shield;
}

/*
 * 得到军队对应等级的回血量
 * 定义在CLi.h
 */
int GetArmySupply(int level) {
    return Get_armyAbility(level).RecoverHp_eachTurn;

}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 判断两个国家是否是停战状态。请在传参前检测指针是否为空。<p>
     * 需要注意的是：盟友之间可能并非处于”和平条约“的束缚中。不可用于盟友与否的判断。<p>
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * 判断结果。 <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： ？？？<p>
     * 【修改时间】： ？？？
     */
bool  CScene::Is_armisticeStatus(CCountry *theCountry, CCountry *targetCountry) {
    //如果是停战状态，返回false
    if (theCountry->ListArmisticeStatus[targetCountry->ID] > 0 || targetCountry->ListArmisticeStatus[theCountry->ID] > 0) {
        return true;
    }
    return false;
}



/*
 * 根据勋章类型返回空袭范围  废弃
 */
float CCountry::AirstrikeRadius() {
    if (this->GetWarMedalLevel(AirForceMedal) < 1) {
        return 300.0;
    }
    else {
        return 400.0;
    }
}
float CArmy::GetAirstrikeRadius(){
    float d = 0.0f;
    ArmyDef* armyDef = this->BasicAbilities;
    d = 2400.0f;
    return d + (this->Country->GetWarMedalLevel(AirForceMedal)<2?400:(this->Country->GetWarMedalLevel(AirForceMedal)<3?650:900));
}
def_easytech(_ZN8CCountry7TurnEndEv);

void CCountry::TurnEnd() {
    for (int i = 0; i < g_GameManager.ListCountry.size(); ++i) {
        if (this->ListArmisticeStatus[g_GameManager.ListCountry[i]->ID] > 0) {
            this->ListArmisticeStatus[g_GameManager.ListCountry[i]->ID] -= 1;
        }
    }
    for (int i = 0; i < 128; ++i) {
        if (this->Arr_cardCD_inTheTurn[i] > 0) {
            this->Arr_cardCD_inTheTurn[i]--;
        }
    }

    easytech(_ZN8CCountry7TurnEndEv)(this);

}

//def_easytech(_ZN5CArea10RenderArmyEffiP5CArmy)
//
//void CArea::RenderArmy(float a, float b, int armyCount, CArmy *d) {
//  bool isNeedRender =  Is_inWarFog(this);
//    if (isNeedRender){
//    easytech(_ZN5CArea10RenderArmyEffiP5CArmy)(this, a, b, armyCount, d);
//    }
//}


/*
 * 判断这个地块能否建造建筑
 */
bool CArea::CanConstruct(int construction_willBeBuilt) {
    if (this->Sea) {
        return false;
    }
    if (this->ConstructionType == NoConstruction) {
        return true;
    }
    if (construction_willBeBuilt == city) {
        if (this->ConstructionLevel < 4 && this->ConstructionType == city) {
            return true;
        }
    }
    else if (construction_willBeBuilt == industry) {
        if (this->ConstructionLevel < 3 && this->ConstructionType == industry) {
            return true;
        }
    }
    else if (construction_willBeBuilt == airport) {
        if (this->ConstructionLevel <= 0) {
            return true;
        }
    }
    //以上条件都不成立，返回false
    return false;
}

/*
 * 设置军队的攻防加成buff
 * 定义在CLi.h
 */
void Set_armyBuff(CFight *fight, CArmy *army, CArea *area, bool isStartArmy) {
    int attack = 0;
    int defense = 0;
    int piercing = 0;
    //如果所在地块有工事或者有首都
    if (area->InstallationType != CArea::NoInstallation||area->Type == capital) {
        defense += 1;
    }
    //如果存在 工程卡
    if (army->Has_engineeringCorps()) {
        defense += 1;
    }
    //如果存在 轻炮卡
    if (army->HasCard(CArmy::LightArtillery)) {
        attack += 1;
        piercing+=1;
    }
    //如果是重炮
    if (army->HasCard(CArmy::HeavyArtillery)){
        piercing += 2;
        army->Country->Money -= 5;
        defense +=1;
        attack += 3;
    }
    if (army->HasCard(CArmy::AntitankGun)){
        piercing +=2;
    }
    if (army->HasCard(CArmy::TankDestroyer)){
        piercing +=3;
    }
    if (army->HasCard(CArmy::MoveArtillery)){
        piercing += 1;
        army->Country->Money -= 5;
        defense +=1;
        attack += 3;
    }
    if (army->Morale == 1) {
        attack -= 1;
    }
    else if (army->Morale == 2) {
        attack += 1;
    }
    int armyAtkBuff_fromLevel = GetArmyAtkBuff(army->Level);
    int armyDfsBuff_fromLevel = GetArmyDfsBuff(army->Level);
    //如果这个单位存在指挥官卡
    if (army->HasCard(CArmy::Commander)) {
        CommanderAbility commanderAbility = GetCommanderAbility(army->Country->GetCommanderLevel());
        int commanderAtkBuff = commanderAbility.Assault;
        int commanderDfsBuff = commanderAbility.Shield;
        //在指挥官和等级之中取提供的最高攻防加成
        if (armyAtkBuff_fromLevel > commanderAtkBuff) {
            attack += armyAtkBuff_fromLevel;
        }
        else {
            attack += commanderAtkBuff;
        }
        if (armyDfsBuff_fromLevel > commanderDfsBuff) {
            defense += armyDfsBuff_fromLevel;
        }
        else {
            defense += commanderDfsBuff;
        }

    }
    else {
        attack += armyAtkBuff_fromLevel;
        defense += armyDfsBuff_fromLevel;
    }
    if (isStartArmy == true) {
        fight->StartArmyDefence += defense;
        fight->StartArmyExtraAttack += attack;
        fight->StartArmyExtraPiercing += piercing;
    }
    else {
        fight->TargetArmyDefence += defense;
        fight->TargetArmyExtraAttack += attack;
        fight->TargetArmyExtraPiercing += piercing;
    }

}

/*
 * 添加单位
 */
void CArea::AddArmy(CArmy *army, bool bottom) {
    int num = this->ArmyCount;
    // 如果这个地块的军队数量小于等于3 则该地块可以添加军队
    if (num <= 3) {
        //如果这个被添加进来的新军队单位是在底部(false为顶部）
        if (bottom == true) {
            this->Army[num] = army;
        }
        else {
            //否则数组各个元素依次向后移动
            for (int i = num + 1; i > 0; i--) {
                this->Army[i] = this->Army[i - 1];
            }
            //顶部换成新添加的军队单位
            this->Army[0] = army;
        }
        //这个地块的军队数量+1
        this->ArmyCount = num + 1;
    }
}

void CArmy::TurnBegin() {
    this->Movement = this->BasicAbilities->Movement;
    this->Active = true;

    if (s_NewArmyAttribute && this->Is_collapsed()){
        this->Active = false;
    }
    auto theCountry = this->Country;
    SetPoisoning(GetPoisoningState() - 1);
}
/**
     *=======================================================<p>
     * 【功能描述】：回合结束时，单位的状态设置。军队的移动力会被强制为0.<p>
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * void <p>
     *=======================================================<p>
     * 【创建人】： ？？？ <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： ？？？<p>
     * 【修改时间】： ？？？
     */
void CArmy::TurnEnd() {
    this->Movement = 0;
    this->Active = false;
    if (this->MoraleUpTurn > 0) {
        this->MoraleUpTurn -= 1;
        if (this->MoraleUpTurn == 0) {
            this->Morale = 0;
        }
    }
    if(s_NewArmyAttribute){
        //恢复基础组织度
        this->Restore_oganization(this->BasicAbilities->RecoverOrganization);
        //根据等级恢复额外组织度
        auto data = Get_armyAbility(this->Level);
        this->Restore_oganization(data.RecoverOgranization_eachTurn);
    }
}

/*
 * 给该地块的某个军队添加血量
 */
void CArea::AddArmyStrength(int ArmyIndex, int Hp) {
    int num = this->ArmyCount;
    if (ArmyIndex < num) {
        if (this->Army[ArmyIndex]) {
            this->Army[ArmyIndex]->AddStrength(Hp);
        }
    }
}

void CArmy::Restore_oganization(int o){
    if(this->Organization >= this->MaxOrganization)
        return;
    if (this->Organization + o > this->MaxOrganization){
        this->Organization = this->MaxOrganization;
    }
    else{
        this->Organization += o;
    }
}

/*
 * 给指定的单位添加战术卡  CardID的值输入后分别对应 0->攻击战术 1->防御战术 2->运输船 3->指挥官
 */
void CArea::AddArmyCard(int ArmyIndex, int CardID) {
    if (ArmyIndex < this->ArmyCount) {
        if (this->Army[ArmyIndex]) {
            this->Army[ArmyIndex]->Cards |= 1 << CardID;
        }
    }
}

/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 检测该地块是否处于包围状态。<p>
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * 返回检测结果。<p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】：李德邻 <p>
     * 【创建时间】： ？？？<p>
     * 【修改时间】： ？？？
     */
bool CArea::CheckEncirclement() {
    if (this->Country == nullptr) {
        return false;
    }
    int adjacentNum = g_Scene.GetNumAdjacentAreas(this->ID);
    for (int i = 0; i < adjacentNum; ++i) {
        CArea *area = g_Scene.GetAdjacentArea(this->ID, i);
        if (area->Country == nullptr) {
            return false;
            break;
        }
        if (area->Type == capital||area->Type == large_city) {
            if (area->Country->Alliance == this->Country->Alliance||
                area->Country->Alliance == 4||area->ArmyCount == 0) {
                return false;
                break;
            }
        }
            //  恢复原版状态可以把上面的if去掉  这里加上       ||   area->ArmyCount == 0
        else if (area->Country->Alliance == this->Country->Alliance||
                 area->Country->Alliance == 4) {
            return false;
            break;
        }
    }
    return true;
}

/*
 * 在空白地建造建筑
 */
void CArea::Construct(int constructionType) {
    //如果这个地块有建筑，则这个建筑等级+1
    if (this->ConstructionType != NoConstruction) {
        this->ConstructionLevel += 1;
        return;
    }
    //默认情况下是所有建筑从空白起建都是1级
    this->ConstructionLevel = 1;
    if (constructionType == 1) {
        this->ConstructionType = city;
    }
    else if (constructionType == 2) {
        this->ConstructionType = industry;
    }
    else if (constructionType == 3) {
        this->ConstructionType = airport;
    }
    //但以下地块类型会特殊处理
    switch (this->Type) {
        case capital:
            if (this->ConstructionType == city) {
                this->ConstructionLevel = 4;
            }
            else if (this->ConstructionType == industry) {
                this->ConstructionLevel = 3;
            }
            break;
        case large_city:
            if (this->ConstructionType == city) {
                this->ConstructionLevel = 3;
            }
            else if (this->ConstructionType == industry) {
                this->ConstructionLevel = 2;
            }
            break;
        case normal_city:
            if (this->ConstructionType == city) {
                this->ConstructionLevel = 2;
            }
            else if (this->ConstructionType == industry) {
                this->ConstructionLevel = 1;
            }
            break;
        default:
            break;
    }
}

/*
 * 得到真实税收
 */
int CArea::GetRealTax() {
    return 5 * this->GetCityLevel() + this->Tax;
}

/*
 * 得到城市等级
 */
int CArea::GetCityLevel() {
    int cityLevel = 0;
    if (this->Type == capital) {
        cityLevel = 3;
    }
    else if (this->Type == large_city) {
        cityLevel = 2;
    }
    else if (this->Type == normal_city) {
        cityLevel = 1;
    }
    if (this->ConstructionType == city && cityLevel < this->ConstructionLevel) {
        return this->ConstructionLevel;
    }
    else {
        return cityLevel;
    }
}

/*
 * 得到工业等级
 */
int CArea::GetIndustryLevel() {
    int industryLevel = 0;
    if (this->Type == capital) {
        industryLevel = 2;
    }
    else if (this->Type == large_city) {
        industryLevel = 1;
    }
    if (this->ConstructionType == industry && industryLevel < this->ConstructionLevel) {
        return this->ConstructionLevel;
    }
    else {
        return industryLevel;
    }
}

int CArea::GetIndustry() {
    return 5 * this->GetIndustryLevel();
}

int CArea::Get_oil() {
    int oil = this->Oil;
    if (this->ConstructionType == oilwell) {
        oil += 30 * this->ConstructionLevel;
    }
    return oil;
}

int CArea::Get_cityGrowthRate() {
    return this->CityGrowthRate;
}

int CArea::Get_industryGrowthRate() {
    return this->IndustryGrowthRate;
}

int CArea::Get_oilGrowthRate() {
    return this->OilGrowthRate;
}
/*
 * 清除该地块的所有军队
 */
void CArea::ClearAllArmy() {
    int num = this->ArmyCount;
    for (int i = num; i > 0; i--) {
        CArmy *army = this->Army[i - 1];
        this->Army[i - 1] = nullptr;
        delete army;
    }
    this->ArmyCount = 0;
}

void CArea::ClearArmy(int count) {
    if (count > this->ArmyCount) {
        count = this->ArmyCount;
    }
    for (int i = 0; i < count; ++i) {
        CArmy *army = this->Army[0];
        this->RemoveArmy(this->Army[0]);
        delete army;
    }
}

/*
 * 清除该地块指定单位的战术卡  CardID的值输入后分别对应 0->攻击战术 1->防御战术 2->运输船 3->指挥官
 */
void CArea::DelArmyCard(int ArmyIndex, int CardID) {
    if (ArmyIndex < this->ArmyCount) {
        if (this->Army[ArmyIndex] != nullptr) {
            this->Army[ArmyIndex]->Cards &= ~(1 << CardID);
        }
    }
}

/*
 * 摧毁该地块所有的建筑
 */
void CArea::DestroyConstruction() {
    this->ConstructionLevel = 0;
    this->ConstructionType = NoConstruction;
}

void InitArrArmyDesignations(CCountry *country) {
    //初始化国家的番号列表
    for (int i = 0; i < 2048; ++i) {
        country->ListDesignationsLand.push_back(false);
    }
    for (int i = 0; i < 512; ++i) {
        country->ListDesignationsSea.push_back(false);
    }
    for (int i = 0; i < 512; ++i) {
        country->ListDesignationsAir.push_back(false);
    }
    for (int i = 0; i < 1024; ++i) {
        country->ListDesignationsTank.push_back(false);
    }
}

void SetArmyDesignationLand(CArmy *army, CCountry *country) {
    //如果超过最大限制
    if (country->MaxArmyDesignationsLand == 2048) {
        for (int i = 0; i < 2048; ++i) {
            country->ListDesignationsLand[i] = false;
        }
        country->MaxArmyDesignationsLand = 1;
    }
    //如果这个新番号未被使用过
    if (country->ListDesignationsLand[country->MaxArmyDesignationsLand] == false) {
        //设置番号
        army->Designation = country->MaxArmyDesignationsLand;
        //这个番号所在的列表位置设置为true
        country->ListDesignationsLand[country->MaxArmyDesignationsLand] = true;
        //下个单位的番号+1
        country->MaxArmyDesignationsLand++;
    }
    else {
        //新番号的数字+1
        country->MaxArmyDesignationsLand++;
        SetArmyDesignationLand(army, country);
        return;
    }
    return;
}

void SetArmyDesignationSea(CArmy *army, CCountry *country) {
    //如果超过最大限制
    if (country->MaxArmyDesignationsSea == 512) {
        for (int i = 0; i < 512; ++i) {
            country->ListDesignationsSea[i] = false;
        }
        country->MaxArmyDesignationsSea = 1;
    }
    //如果这个新番号未被使用过
    if (country->ListDesignationsSea[country->MaxArmyDesignationsSea] == false) {
        //设置番号
        army->Designation = country->MaxArmyDesignationsSea;
        //这个番号所在的列表位置设置为true
        country->ListDesignationsSea[country->MaxArmyDesignationsSea] = true;
        //下个单位的番号+1
        country->MaxArmyDesignationsSea++;
    }
    else {
        //新番号的数字+1
        country->MaxArmyDesignationsSea++;
        SetArmyDesignationSea(army, country);
        return;
    }
    return;
}

void SetArmyDesignationTank(CArmy *army, CCountry *country) {
    //如果超过最大限制
    if (country->MaxArmyDesignationsTank == 1024) {
        for (int i = 0; i < 1024; ++i) {
            country->ListDesignationsTank[i] = false;
        }
        country->MaxArmyDesignationsTank = 1;
    }
    //如果这个新番号未被使用过
    if (country->ListDesignationsTank[country->MaxArmyDesignationsTank] == false) {
        //设置番号
        army->Designation = country->MaxArmyDesignationsTank;
        //这个番号所在的列表位置设置为true
        country->ListDesignationsTank[country->MaxArmyDesignationsTank] = true;
        //下个单位的番号+1
        country->MaxArmyDesignationsTank++;
    }
    else {
        //新番号的数字+1
        country->MaxArmyDesignationsTank++;
        SetArmyDesignationTank(army, country);
    }
    return;
}

void SetArmyDesignationAir(CArmy *army, CCountry *country) {
    //如果超过最大限制
    if (country->MaxArmyDesignationsAir == 512) {
        for (int i = 0; i < 512; ++i) {
            country->ListDesignationsAir[i] = false;
        }
        country->MaxArmyDesignationsAir = 1;
    }
    //如果这个新番号未被使用过
    if (country->ListDesignationsAir[country->MaxArmyDesignationsAir] == false) {
        //设置番号
        army->Designation = country->MaxArmyDesignationsAir;
        //这个番号所在的列表位置设置为true
        country->ListDesignationsAir[country->MaxArmyDesignationsAir] = true;
        //下个单位的番号+1
        country->MaxArmyDesignationsAir++;
    }
    else {
        //新番号的数字+1
        country->MaxArmyDesignationsAir++;
        SetArmyDesignationAir(army, country);
        return;
    }
    return;
}

//创造军队的时候还需要配合SetArmyDesignation
void CArmy::Init(ArmyDef *armydef, CCountry *country) {
    this->BasicAbilities = armydef;
    this->Country = country;
    this->Hp = armydef->Hp;;
    this->MaxHp = armydef->Hp;
    this->Cards = 0;
    this->Level = 0;
    this->Movement = armydef->Movement;
    this->Exp = 0;
    this->Morale = 0;
    this->MoraleUpTurn = 0;
    this->Direction = 1.0;
    this->Active = 0;
    this->Seed = GetRand();
    this->Designation = 0;
    this->IsA = false;
    for (int i = 0; i < 3; ++i) {
        this->OldDesignation[i] = 0;
    }
    this->SetArmyOrganization(armydef);
    this->ResetMaxStrength(false);
}

void CArmy::SetArmyOrganization(ArmyDef *armydef) {
    this->Organization = armydef->MaxOrganization;
    this->MaxOrganization = armydef->MaxOrganization;
}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 给单位按照军种设置番号。其番号取决于所属国的所属军种编号的记录情况，该国该军种未赋予的最小番号会被优先赋值。<p>
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * void <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： ？？？<p>
     * 【修改时间】： ？？？
     */
void Set_armyDesignaion(CArmy *army, CCountry *country) {
    //如果需要设置新的军种，模仿下面的写法。

    //如果 是海军
    if (army->IsNavy()) {
        SetArmyDesignationSea(army, country);
    }
    else {
        //如果是空军
        if (army->Is_airForce()) {
            SetArmyDesignationAir(army, country);
        }
        else if (army->Is_tank()) {
            SetArmyDesignationTank(army, country);
        }
        else {
            SetArmyDesignationLand(army, country);
        }
    }
}
bool CArmy::Is_tank() {
    if (this->BasicAbilities->ID == ArmyDef::Tank||this->BasicAbilities->ID == ArmyDef::HeavyTank) {
        return true;
    }
    return false;
}
bool CArmy::Is_airForce() {
    if (Is_fightPlane() || Is_boomPlane())
        return true;
    return false;

}
bool CArmy::Is_fightPlane(){
    if (this->BasicAbilities->ID == ArmyDef::Fight_1914 )
        return true;
    return false;

}
bool CArmy::Is_boomPlane(){
    if (this->BasicAbilities->ID == ArmyDef::Boom_1914 )
        return true;
    return false;

}
//远距离火炮
bool CArmy::Is_remoteAttack(){
    if (this->BasicAbilities->ID == ArmyDef::Rocket) {
        return true;
    }
    return false;

}
//是否用圆半径的攻击方式
bool CArmy::Is_radiudAttack(){
    if (this->Is_boomPlane() || this->Is_fightPlane() || this->BasicAbilities->ID == ArmyDef::AircraftCarrier||this->BasicAbilities->ID == ArmyDef::IntercontinentalMissile)
        return true;
    return false;
}

int CArmy::Get_countOfCard(){
    int count = 0;
    for (int i = 0; i < 11; ++i) {
        if (((this->Cards >> i) & 1) != 0)
            count ++;
    }
    return count;
}

void CArmy::AddExp(int exp){
    if(this->Level <= 3){
        this->Exp += exp;
        NewArmyAbility armyAbility = Get_armyAbility(this->Level);
        if (!this->IsNavy()){
            if (this->Exp >= 700 && this->Level < 4){
                this->Upgrade();
            }
            else if(this->Exp >= 450 && this->Level < 3){
                this->Upgrade();
            }
            else if(this->Exp >= 250 && this->Level < 2){
                this->Upgrade();
            }
            else if(this->Exp >= 100 && this->Level < 1){
                this->Upgrade();
            }
        }
        else if(this->IsNavy()){
            if (this->Exp >= 1400 && this->Level < 4){
                this->Upgrade();
            }
            else if(this->Exp >= 700 && this->Level < 3){
                this->Upgrade();
            }
            else if(this->Exp >= 500 && this->Level < 2){
                this->Upgrade();
            }
            else if(this->Exp >= 200 && this->Level < 1){
                this->Upgrade();
            }
        }
    }
}
void CArmy::Upgrade(){
    if(this->Level <4){
        this->Level += 1;
        NewArmyAbility newArmyAbility = Get_armyAbility(this->Level);
        //回血量为最大血量的百分之20
        this->AddStrength(this->MaxHp / 5);

        auto data = Get_armyAbility(this->Level);
        this->MaxHp += this->MaxHp * data.Increase_percentageOfMaxHP / 100;
        this->MaxOrganization += data.Increase_percentageOfMaxOgranization;
    }
}

void CArmy::Lost_card(int percentNum){
    int r = GetRand(this) % 100 + 1;
    //20概率移除 其中一个战术卡
    if (r  <= percentNum){
        int cardCount = this->Get_countOfCard();
        if (cardCount >= 1){
            //随机到需要移除的的卡牌下标
            int r_cardIndex = GetRand(this) % cardCount ;
            //已找到的已装备的卡牌下标
            int foundIndex = -1;
            for (int i = 0; i < 10; ++i) {
                if (((this->Cards >> i) & 1) != 0){
                    //
                    foundIndex++;
                    if (foundIndex == r_cardIndex){
                        this->Cards &= (~(1 << i));
                    }
                }
            }

        }

    }
}
def_easytech(_ZN9CSoundRes10PlayCharSEE10SND_EFFECT);

/*
 * 给这个地块添加单位
 */
CArmy *CArea::DraftArmy(int ArmyType) {
    //此处为修改地块最大单位数量的一部分内容
    if (this->ArmyCount > 3) {
        return nullptr;
    }
    if (this->Country == nullptr) {
        return nullptr;
    }
    CObjectDef *instance = CObjectDef::Instance();

    ArmyDef *armyDef = instance->GetArmyDef(ArmyType, this->Country->Name);
    if (armyDef == nullptr) {
        return nullptr;
    }

    CArmy *army = new CArmy();
    army->Init(armyDef, this->Country);
    Set_armyDesignaion(army, this->Country);
    //此处修改新造单位的移动力是否为0
    army->Active = false;
    army->Movement = 0;
    this->ArmyDrafting = army;
    if (this->Country->AI)
        this->DraftingArmyOffset = 0.0;
    this->DraftingArmyOffset = -60.0;
    easytech(_ZN9CSoundRes10PlayCharSEE10SND_EFFECT)(&g_SoundRes, CSoundRes::aDraft_wav);
    return army;
}

/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 检查该地块是否处于包围。如果处于包围，设置该地块处于包围时，各单位的士气状态。<p>
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * void <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： 李德邻<p>
     * 【创建时间】： ？？？<p>
     * 【修改时间】： ？？？
     */
void CArea::Encirclement() {
    int num = this->ArmyCount;
    if (this->CheckEncirclement() == true) {
        //1是士气为负
        for (int i = 0; i < num; ++i) {
            this->Army[i]->SetMorale(1);
        }
    }
    else {
        for (int i = 0; i < num; ++i) {
            if (this->Army[i]->Morale == 1&&this->Army[i]->MoraleUpTurn < 1) {
                this->Army[i]->SetMorale(0);
            }
        }
    }
}

/*
 * 获得士气高涨状态
 */
void CArmy::Breakthrough() {
    //士气设置为高涨
    this->Morale = 2;
    //持续时间2回合
    this->MoraleUpTurn = 2;
}

/*
 * 得到这个地块军队中的指定单位
 */
CArmy *CArea::GetArmy(int ArmyIndex) {
    if (this->ArmyCount < 0||ArmyIndex > this->ArmyCount) {
        return nullptr;
    }
    return this->Army[ArmyIndex];
}

void SetCardText(CCountry *country) {
    if (g_GameManager.CurrentTurnNumMinusOne == 1&&
        g_GameManager.GameMode == CGameManager::Conquest) {

        if (g_GameManager.GetPlayerCountry() == country) {
            for (int i = 0; i < NewCardDef.size(); ++i) {
                string *s = (string *) "Infantry1_de Intro";
                NewCardDef[i]->Intro = s;
            }
        }
    }
}


def_easytech(_ZN8CCountry9TurnBeginEv);

void CCountry::TurnBegin() {

    //每次回合开始重新设置种子
    SetRandSeed(time(NULL));

    Trigger_eventTurnBegin_theScript(this);

    SetCardText(this);
    if (this->TechTurn > 0) {
        this->TechTurn--;
        if (this->TechTurn == 0) {
            this->Tech++;
        }
    }
    if (g_GameManager.CurrentTurnNumMinusOne > 0) {
        CollectTaxes();
        CollectIndustrys();
        Collect_oilMultiplied_toTheReserve();
    }
    for (auto &AreaID : AreaIDList)
        g_Scene.GetArea(AreaID)->TurnBegin();
    //建造单位防守和ai撤退
    if((this->AI || g_GameManager.Is_spectetorMode()) && g_GameManager.IsIntelligentAI){
        for (auto &AreaID : AreaIDList)
            g_Scene.GetArea(AreaID)->Check_retreatableArmy(6);
        g_newAiAction.Add_newArmy_toImportantTown(this);
    }

    this->IsNewActionOver = false;
}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 把这个地块顶部的军队移动到周围的地块里。<p>
     * 注意：该函数完成了军队指针的转移（且调用量removeArmy函数）。<p>
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * void <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： 2022 <p>
     * 【修改时间】： ？？？
     */
void CArea::Retreat_frontArmy_toAdjacentArea(CArea *adjacentArea) {
    if (adjacentArea == nullptr || adjacentArea->Enable == false)
        return;
    //如果这个地块的国家和目标地块的国家一样
    if (this->Country == adjacentArea->Country && adjacentArea->ArmyCount <= 3) {
        //则不会出现征服地块或者将领报怨的情况
        bool willOccupy = false;
        bool willComplain = false;
        CArmy *frontArmy = this->Army[0];
        //移除这个地块的顶部军队
        this->RemoveArmy(frontArmy);
        const char *commander = nullptr;
        //如果目标地块的国家不为空
        if (adjacentArea->Country != nullptr) {
            commander = adjacentArea->Country->GetCommanderName();
        }

        adjacentArea->SetMoveInArmy(this, frontArmy, willOccupy, willComplain, commander);
        if (this->ArmyCount == 0) {
            //重新设置该地块和其周围地块处于包围的状态（包含重新进行包围与否的检测）
            g_Scene.AdjacentAreasEncirclement(this->ID);
        }
        //撤退的单位放到最下面
        if (adjacentArea->ArmyCount > 1) {
            CArmy *topArmy = adjacentArea->Army[0];
            adjacentArea->Army[0] = adjacentArea->Army[adjacentArea->ArmyCount - 1];
            adjacentArea->Army[adjacentArea->ArmyCount - 1] = topArmy;
        }
    }
}

/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 移动顶层单位到目标地块。如果需要制定单位的位置，有一个对应的重载函数<p>
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * void <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： 2022 <p>
     * 【修改时间】： 2022/10/8
     */
void CArea::MoveArmyTo(CArea *targetArea) {
    if (g_Scene.Is_safedOpen(targetArea) == false || this->Country == nullptr)
        return;
    int a = 1;

    bool willOccupy = false;
    bool willComplain = false;
    //修行停战状态 ai强制进攻的行为
    if (targetArea->Country != nullptr && strcmp(this->Country->ID, targetArea->Country->ID) != 0 &&
        g_Scene.Is_armisticeStatus(this->Country, targetArea->Country)) {
        this->Army[0]->Movement = 0;
        this->Army[0]->Active = false;
        return;
    }
    //设置随机数种子
    srand(time(0));
    if (this->ArmyCount > 0) {
        CArmy* frontArmy = this->Army[0];
        //如果在顶部的军队的行动力大于0并且目标地块的单位小于4
        if (frontArmy->Movement > 0&&targetArea->ArmyCount <= 3) {
            auto targerCountry = targetArea->Country;
            if (targetArea->Country != this->Country){
                if (targetArea->Country!= nullptr){
                    //如果目标地块的国家和变个地块的国家都是ai 不会产生抱怨
                    if (targetArea->Country->AI && this->Country->AI) {
                        willComplain = false;

                    }
                        //否则如果是同盟关系，有50%的概率出现将领报怨对话
                    else if (this->Country->Alliance == targetArea->Country->Alliance) {
                        //求余，概率不得大于等于2
                        willComplain = rand() % 2;
                    }
                    //目标地块的国家移除目标地块
                    targerCountry->RemoveArea(targetArea->ID);
                }
                //添加地块
                this->Country->AddArea(targetArea->ID);
                targetArea->Country = this->Country;
                willOccupy = true;
                //只有发生军事占领才会触发事件
                Trigger_eventMoveArmyTo_theScript(this->Country, targetArea);
            }
            //如果目标地块国家不为空 且目标地块所属国因目标地块丢失而灭亡
            if (targetArea->Country != nullptr && targetArea->Country ->IsConquested()) {
                targetArea->Country->BeConquestedBy(this->Country);
            }
            //移除这个地块的顶部军队
            this->RemoveArmy(frontArmy);
            const char *commander = nullptr;
            //如果目标地块的国家不为空
            if (targetArea->Country != nullptr) {
                commander = targetArea->Country->GetCommanderName();
            }
            targetArea->SetMoveInArmy(this, frontArmy, willOccupy, willComplain, commander);
            if (targetArea->Sea) {
                //移动力清空
                frontArmy->Movement = 0;
            }
            else{
                if (this->Sea){
                    frontArmy->Movement = 0;
                }
                else {
                    frontArmy->Movement -= targetArea->Get_consumedMovement();
                }
            }
            if (this->ArmyCount == 0) {
                //重新设置该地块和其周围地块处于包围的状态（包含重新进行包围与否的检测）
                g_Scene.AdjacentAreasEncirclement(this->ID);
            }

        }

    }

}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 对原有函数MoveArmyTo进行重载。指定本地块一个单位移动到一个地块中。<p>
     * 1：如果目标地块未隶属于该军队之国家，则会占领目标地块，否则单纯移动。<p>
     * 2：如果目标地块是AI而该军队隶属于玩家，则会触发抱怨。<p>
     * 3：该单位以后过去后，如果出发地块单位数量下降为0，则该地块及其周围地块重新进行“包围”检测。<p>
     * 4：此函数在内部修正了AI强制进攻的行为。<p>
     * 5：单位过去后，移动力会相应减少，双方地块军队数量进行更新。<p>
     * 【参数说明】：<p>
     * actArmy：必须存在于本地块中。<p>
     * 【返回值说明】：<p>
     * void <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： 2022<p>
     * 【修改时间】： ？？？
     */
void CArea::MoveArmyTo(CArea *targetArea, CArmy* actArmy, bool bottom) {

    if (g_Scene.Is_safedOpen(targetArea) == false || this->Country == nullptr)
        return;
    //修正停战状态 ai强制进攻的行为
    if (targetArea->Country != nullptr && strcmp(this->Country->ID, targetArea->Country->ID) != 0 &&
        g_Scene.Is_armisticeStatus(this->Country, targetArea->Country)) {
        this->Army[0]->Movement = 0;
        this->Army[0]->Active = false;
        return;
    }
    //设置随机数种子
    srand(time(0));
    if (this->ArmyCount > 0) {
        //如果在顶部的军队的行动力大于0并且目标地块的单位小于4
        if (actArmy->Movement > 0 && targetArea->ArmyCount <= 3) {
            bool Willoccupy = false;
            //如果这个地块的国家和目标地块的国家一样
            if (this->Country == targetArea->Country) {
            }
            else {
                Willoccupy = true;
                //如果这个目标地块的存在国家
                if (targetArea->Country != nullptr) {
                    CCountry *theCountry = this->Country;
                    CCountry *targetCountry = targetArea->Country;
                    //目标地块的国家移除目标地块
                    Trigger_eventMoveArmyTo_theScript(theCountry, targetArea);
                    //目标地块的国家移除目标地块
                    targetCountry->RemoveArea(targetArea->ID);
                }
                CCountry *targetCountry = targetArea->Country;
                //移动过去的国家的地块列表中添加目标地块
                this->Country->AddArea(targetArea->ID);
                //更改目标地块的所属国信息
                targetArea->Country = this->Country;
                //如果目标地块国家不为空 且目标地块所属国因目标地块丢失而灭亡
                if (targetCountry != nullptr&&targetCountry->IsConquested()) {
                    targetCountry->BeConquestedBy(this->Country);
                }
            }
            //移除这个行动的单位
            this->RemoveArmy(actArmy);
            //不去掉该函数会造成指针异常
//            targetArea->SetMoveInArmy(this,actArmy,Willoccupy,false, nullptr);
            targetArea->AddArmy(actArmy, bottom);
            if (targetArea->Sea) {
                //移动力清空
                actArmy->Movement = 0;
            }
            else{
                if (this->Sea){
                    actArmy->Movement = 0;
                }
                else{
                    actArmy->Movement -= targetArea->Get_consumedMovement();
                }
            }
            if (this->ArmyCount == 0) {
                //重新设置该地块和其周围地块处于包围的状态（包含重新进行包围与否的检测）
                g_Scene.AdjacentAreasEncirclement(this->ID);
            }

        }
    }
    _ZN9CSoundRes10PlayCharSEE10SND_EFFECT(&g_SoundRes,
                                           targetArea->Sea ? CSoundRes::aShip_wav
                                                           : CSoundRes::aMove_wav);
}



//得到卡牌工业
int CCountry::GetCardIndustry(CardDef *card) {
    int industry = card->Industry;
    if (card->ID == ResearchCard) {
        return industry * this->Tech;
    }
    else {
        return industry;
    }
}

//得到卡牌价格
int CCountry::GetCardPrice(CardDef *card) {
    int money = card->Price;

     if (card->ID == CommanderCard) {
        return (money += 5 * this->GetCommanderLevel());
    }

    return money;
}

void CCountry::CollectTaxes() {
    int money = GetTaxes() + this->Money;
    if (money > 99999) {
        this->Money = 99999;
    }
    else {
        this->Money = money;
    }
}

void CCountry::CollectIndustrys() {
    int newIndustry = GetIndustrys();
    this->Industry += newIndustry;
    this->Oil += newIndustry / this->OilConversionRate;
    if (this->Industry > 99999) {
        this->Industry = 99999;
    }

}

void Init_conversionRate_ofOil_aboutAllCountry() {
    for (int i = 0; i < g_GameManager.ListCountry.size(); ++i) {
        g_GameManager.ListCountry[i]->OilConversionRate = 10;
    }
}

void CCountry::Collect_oilMultiplied_toTheReserve() {
    if (this->Oil > 99999) {
        this->Oil = 99999;
    }
    else {
        this->Oil += Get_oilMultiplied_fromAreas();
    }
}

int CCountry::Get_oilMultiplied_fromAreas() {
    auto it = this->AreaIDList.begin();
    int totalOils = 0;
    while (it != this->AreaIDList.end()) {
        int areaID = *it++;
        totalOils += g_Scene.GetArea(areaID)->Get_oil();

    }
    int warMedalLevel = this->GetWarMedalLevel(CommerceMedal);
    if (warMedalLevel > 0) {
        totalOils += totalOils * warMedalLevel / 10;// 1.1倍
    }
    return totalOils *= (int)this->TaxRate;
}

/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 设置选中地块时，会发生的情况（绘制箭头/可攻击目标箭头）。<p>
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * void <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： 李德邻<p>
     * 【创建时间】： ？？？<p>
     * 【修改时间】： 2022/7/4
     */
void CScene::SetSelAreaTargets(CArea *startArea) {

    int areas = g_Scene.GetNumAreas();
    Init_sArr_foundArea();

    sArr_foundArea[startArea->ID] = true;
    //下一圈需要被遍历的地块
    std::vector<int> list_nextLoop;
    if (startArea->ArmyCount <= 0||startArea->IsActive() == false||
        startArea->Country == nullptr||startArea->Country->AI) {
        return;
    }
    CArmy *army = startArea->GetArmy(0);
    auto theAreaSurroundingInfo = g_areaSurroundingInfoManager.Get_theAreaSurroundingInfo(startArea->ID);
    //得到它周围的圈层
    if (theAreaSurroundingInfo->Get_ringInfo(0) == nullptr)
        return;
    int size = theAreaSurroundingInfo->Get_ringInfo(0)->size();

    //循环它周围的圈层
    for (int i = 0; i < size; ++i) {
        CArea *targetArea = g_Scene.GetAdjacentArea(startArea->ID, i);
        //检查移动可能
        bool isCanMove = g_Scene.CheckMoveable(startArea->ID, targetArea->ID, 0);
        //如果可移动，绘制移动箭头
        if (isCanMove) {
            targetArea->TargetType = 3;
        }
        //如果可以攻击 绘制近战攻击箭头
        if (g_Scene.CheckAttackable(startArea->ID, targetArea->ID, 0)) {
            targetArea->TargetType = 4;
        }
        g_Scene.CreateArrow(startArea->ID, targetArea->ID);
    }

    bool isLimit = false;
    if (army->Is_remoteAttack() || army->Is_radiudAttack()) {
        auto listArea = startArea->Get_areas_inRange(0);
        int newsize = listArea->size();
        for (int i = 0; i < newsize; ++i) {
            //检查攻击可能
            if (g_Scene.CheckAttackable(startArea->ID, (*listArea)[i]->ID, 0)){
                g_Scene[(*listArea)[i]->ID]->TargetType = 2;
            }
        }
        delete listArea;
    }
    //TODO 此处用于显示ai的行动路线
//   int areaID =  CActionAssist::Instance()->searchNodeByID(startArea->ID,0);

}



int CCountry::GetTaxes() {
    auto it = this->AreaIDList.begin();
    int totalMoneys = 0;
    while (it != this->AreaIDList.end()) {
        int areaID = *it++;
        totalMoneys += g_Scene.GetArea(areaID)->GetRealTax();

    }
    int warMedalLevel = this->GetWarMedalLevel(CommerceMedal);
    if (warMedalLevel > 0) {
        totalMoneys += totalMoneys * warMedalLevel / 10;// 1.1倍
    }
    return totalMoneys *= (int)this->TaxRate;
}

int CCountry::GetIndustrys() {
    int result = this->Industry;
    auto it = this->AreaIDList.begin();
    int totalIndustrys = 0;
    while (it != this->AreaIDList.end()) {
        int areaID = *it++;
        totalIndustrys += g_Scene.GetArea(areaID)->GetIndustry();
    }
    int warMedalLevel = this->GetWarMedalLevel(CommerceMedal);
    if (warMedalLevel > 0) {
        totalIndustrys += totalIndustrys * warMedalLevel / 10;// 1.1倍
    }
    int a = 1;
    if (strcmp("de",this->ID) == 0){
         result = this->Industry;
        a++;
    }
    int result1 = totalIndustrys *= (int)this->TaxRate;
    return result1;
}



void CArea::TurnEnd() {
    this->Recover_armyState();
    if (g_GameManager.IsIntelligentAI &&( this->Country->AI || g_GameManager.Is_spectetorMode())) {
        this->Adjust_armyOrder();
    }
    if (s_Autoconstruction_byGrowthRate)
      Auto_construction(this);
}

void Auto_construction(CArea *area) {
//    if(area->Sea){
//        return;
//    }
    int r = g_GameManager.GetRand() % 100 + 1;  // 1 - 100
    if (area->ConstructionType == CArea::NoConstruction) {
        if (r <= area->CityGrowthRate) {
            area->Construct(1);
        }
        else if (r <= area->IndustryGrowthRate + area->CityGrowthRate) {
            area->Construct(2);
        }
        else if (r <= area->OilGrowthRate + area->CityGrowthRate + area->IndustryGrowthRate) {
            area->ConstructionType = CArea::oilwell;
        }
    }
    else {
        if (area->ConstructionType == CArea::city&&area->ConstructionLevel < 4) {
            if (r <= area->CityGrowthRate) {
                //如果大于2级，升级概率/2
                if (area->GetCityLevel() <= 2) {
                    area->Construct(1);
                }
                else {
                    int r1 = g_GameManager.GetRand() % 2 + 1;
                    if (r1 == 1) {
                        area->Construct(1);
                    }
                }
            }
        }
        else if (area->ConstructionType == CArea::industry&&area->ConstructionLevel < 3) {
            if (r <= area->CityGrowthRate) {
                //如果大于1级，升级概率/2
                if (area->GetCityLevel() <= 1) {
                    area->Construct(2);
                }
                else {
                    int r1 = g_GameManager.GetRand() % 2 + 1;
                    if (r1 == 1) {
                        area->Construct(2);
                    }
                }
            }
        }
        else if (area->ConstructionType == CArea::oilwell&&area->ConstructionLevel < 4) {
            if (r <= area->CityGrowthRate) {
                //如果大于2级，升级概率/2
                if (area->GetCityLevel() <= 2) {
                    area->Construct(2);
                }
                else {
                    int r1 = g_GameManager.GetRand() % 2 + 1;
                    if (r1 == 1) {
                        area->Construct(2);
                    }
                }
            }

        }
    }
}

def_easytech(_ZN6CScene6UpdateEf)

void CScene::Update(float time) {
    easytech(_ZN6CScene6UpdateEf)(this, time);
    for (CWarGas *Gas : g_GasList)
        if (Gas->IsActive)
            Gas->Update();
}

def_easytech(_ZN6CScene6RenderEv)



int CArmy::GetPoisoningState() {
    if (HasCard(CArmy::Poisoning_1)) {
        return 1;
    }
    else if (HasCard(CArmy::Poisoning_2)) {
        return 2;
    }
    else if (HasCard(CArmy::Poisoning_3)) {
        return 3;
    }
    return 0;
}

void CArmy::SetPoisoning(int state) {
    if (state >= 0&&state <= 3) {
        RemoveCard(CArmy::Poisoning_1);
        RemoveCard(CArmy::Poisoning_2);
        RemoveCard(CArmy::Poisoning_3);
        if (state > 0) {
            Hp = (Hp <= 0||Hp - Hp / 5 <= 5) ? 5 : Hp - Hp / 5;
            AddCard(static_cast<CArmy::Card>(CArmy::Poisoning_1 + state - 1));
        }
    }
}

void CArea::SetAllArmyPoisoning(int state) {
    for (int i = ArmyCount - 1; i >= 0; i--)
        Army[i]->SetPoisoning(state);
}

void CArea::RenderArmy(float x, float y, int ArmyCount, CArmy *Army) {
    bool isInWarFog =  g_Scene.Is_inWarFog(this, g_GameManager.GetPlayerCountry()->Alliance);
    if (isInWarFog){
        return;
    }
    unsigned int Color = 0xFFFFFFFF;
    if (Army->Movement > 0) {
        if (Army->Morale == 1) {
            Color = 0xFFFF4040;
        }
        else if (Army->Morale == 2) {
            Color = 0xFF4040FF;
        }
    }
    else if (Army->Morale == 1) {
        Color = 0xFFC04040;
    }
    else if (Army->Morale == 2) {
        Color = 0xFF4040C0;
    }
    else {
        Color = 0xFFC0C0C0;
    }
    if(g_GameManager.Get_skipMode() == false){
        for (int i = 0; i < ArmyCount; ++i) {
            if (this->GetArmy(i) != nullptr){
                if (this->GetArmy(i)->Movement > 0){
                    float offsetX = 58.0f*g_GameManager.CircleProgress ;
                    float offsetY = 80.0f*g_GameManager.CircleProgress ;
                    g_GameRes.Image_circle->RenderEx(x-offsetX,(y-20)-offsetY,0.0f,1.0f*g_GameManager.CircleProgress,1.0f*g_GameManager.CircleProgress);
                    break;
                }
            }
        }
    }
    if (Army->GetPoisoningState() > 0) {
        Color = 0xFF00EE00 - Army->GetPoisoningState() * 0x00003000;
    }
    g_GameRes.RenderArmy(Army->Country->Name, Army->Country->Alliance, ArmyCount, x, y,
                         Army->BasicAbilities->ID, Color, Sea, Army->Direction);

    g_GameRes.RenderArmyInfo(ArmyCount, x, y +
                                           (Sea&&(Army->Cards & (1 << CArmy::AntitankGun)) ? 4.0
                                                                                           : Army->IsNavy()
                                                                                             ? 8.0
                                                                                             : 0),
                             Army->Hp, Army->GetMaxStrength(), Army->Movement, Army->Cards,
                             Army->Level);
    if (Army != nullptr) {
        //绘制番号
        g_GameRes.RenderArmyDesignation(Army->Designation, x, y, ArmyCount);
        //绘制组织度
        g_GameRes.RenderArmyOrganization(x, y, Army->Organization, Army->BasicAbilities->Collapse,
                                         ArmyCount);
    }

    if (Army->Cards & (1 << CArmy::Commander)) {
        if (Army->Country->AI) {
            g_GameRes.RenderAICommanderMedal(ArmyCount, x, y, Army->Country->Name,
                                             Army->Country->Alliance);
        }
        else {
            g_GameRes.RenderCommanderMedal(ArmyCount, x, y, Army->Country->GetCommanderLevel());
        }
    }
}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 如果该地块和有敌军的敌人地块接壤，那么该地块的远程单位自动后撤。并且自动结束回合状态<p>
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * 如果有远程单位且撤退成功，返回true <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： 李德邻<p>
     * 【创建时间】： ？？？<p>
     * 【修改时间】： 2022/7/7
     */
bool CArea::Retreat_missileArmy(CArmy *needCheckArmy) {
    if (needCheckArmy->Is_remoteAttack() && needCheckArmy->Movement >= 0){
        if (needCheckArmy->Movement == 0){
            needCheckArmy->Movement = 1;
        }
        //如果周围有敌军
        if (this->Is_existMilitaryThreat( 1, 0,this->Country->Alliance) == false ) {
            int num = g_Scene.GetNumAdjacentAreas(this->ID);
            for (int j = 0; j < num; ++j) {
                CArea *targetArea = g_Scene.GetAdjacentArea(this->ID, j);
                if (targetArea->Enable == false || targetArea->Country == nullptr)
                    continue;
                //如果周围没有敌军 并且地块属性相符
                if (this->Is_existMilitaryThreat( 1, 0,this->Country->Alliance) == false
                    && targetArea->Country == this->Country && targetArea->ArmyCount < 4
                    && this->Sea == targetArea->Sea) {
                    this->MoveArmyTo(targetArea, needCheckArmy, true);
                    needCheckArmy->TurnEnd();
                    return true;
                }
            }
        }
    }
    return false;
}

/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 检测指定地块是否为指定单位的补给点。<p>
     * 1:如果是我方地块，且如果军队数量为4，要求必须有健康单位且周围有地块可进行转移。<p>
     * 2:如果是盟友地块且是海洋地块，单位数量必须为0。<p>
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * 检测结果 <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】：李德邻 <p>
     * 【创建时间】：？？？<p>
     * 【修改时间】： 2022/7/3
     */
bool CArea::Is_supplyPoint(CArmy* needSupplyArmy) {

    if (this->Enable== false || this->Country == nullptr )
        return false;
    CCountry* theCountry = needSupplyArmy->Country;
    //如果是海军，并且存在港口，且港口属于自己阵营 并且军队数量小于1
    if (needSupplyArmy->IsNavy()) {
        if (this->Is_supplyPort() && this->Country->Alliance == theCountry->Alliance) {
            //如果是盟友地块，军队数量必须为0
            if (this->Country != needSupplyArmy->Country ){
                if ( this->ArmyCount == 0){
                    return true;
                }
            }
                //如果是自己的地块，且军队数量大于等于4，则必须可移动里面的单位到其他地方。
            else{
                if(this->ArmyCount >= 4){
                    if (this->Check_moveBestStatusArmyToOtherArea(false) != -1)
                        return true;
                }else{
                    return true;
                }
            }
        }
    }
    else {
        if (this->Is_supplyCity() && this->ConstructionType != CArea::oilwell && this->Country == theCountry) {
            if(this->ArmyCount >= 4){
                if (this->Check_moveBestStatusArmyToOtherArea(false) != -1)
                    return true;
            }else{
                return true;
            }
        }
    }
    return false;
}

//这个地块周围是否有出口 只有周围地块有属于自己的地块才能撤退
bool IsFoundOut(CArea *targetArea) {

    int num = g_Scene.GetNumAdjacentAreas(targetArea->ID);
    for (int i = 0; i < num; ++i) {
        CArea *area = g_Scene.GetAdjacentArea(targetArea->ID, i);
        if (area != nullptr && area->Enable && area->Country != nullptr && area->Country == targetArea->Country) {
            return true;
        }
    }
    return false;
}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 检测地块的所有单位是否小于等于撤退的阈值，如果符合，则触发撤退。<p>撤退成功后，单位进入不活跃状态。<p>如果是安全地块的伤残单位，也会进入不活跃状态。<p>
     * 【参数说明】：<p>
     * thresholdDice：撤退阈值（单位的剩余骰子数）<p>
     * 【返回值说明】：<p>
     * void <p>
     *=======================================================<p>
     * 【创建人】：李德邻 <p>
     * 【修改人】： 李德邻<p>
     * 【创建时间】：？？？<p>
     * 【修改时间】：2022/10/5
     */
void CArea::Check_retreatableArmy(int ringCount) {
    if (this->Country == nullptr||this->ArmyCount == 0|| this->Enable == false) {
        return;
    }
    bool hasMilitaryThreat = this->Is_existMilitaryThreat(2, 1, this->Country->Alliance);
    CCountry *theCountry = this->Country;
    CAreaSurroundingInfo* c_theAreaSurroundingInfo = g_areaSurroundingInfoManager.Get_theAreaSurroundingInfo(this->ID);
    int a = 3;
    if (this->ID == 512){
        a++;
    }
    //循环这个地块的军队数  每次循环的时候都会重新动态获取 军队数量
    for (int armyIndex = 0; armyIndex < this->ArmyCount;) {
        //非伤兵，不活跃，下一个！
        CArmy *needSupplyArmy = this->Army[armyIndex];
        int size =  needSupplyArmy->GetNumDices();
        //TODO 组织度判定可以在这里加
        if(needSupplyArmy->Is_wounded() == false || needSupplyArmy->Active == false){
            armyIndex++;
            continue;
        }
        //如果没有军事威胁并且这个单位是伤残状态，并且位于补给点则进入不活跃状态，继续补给
        if (hasMilitaryThreat == false && this->Is_supplyPoint(needSupplyArmy)){
            needSupplyArmy->Active = false;
            armyIndex++;
            continue;
        }
        //如果这个地块很重要，并且守军少，那么就禁止撤退。
        if (this->ArmyCount <= 2 &&( this->Type == capital || this->Type == large_city)){
            break;
        }
        ArmyDef::ArmyType armyType = needSupplyArmy->BasicAbilities->ID;
        //如果 (是坦克 重坦克 战列舰，航母 其中一个 ，且等级大于等于1且补给时间小于3 ) 或  是指挥官  并且它在一段时间内没有“找不到补给点”
        if ((needSupplyArmy->Is_tank() || armyType == ArmyDef::Battleship || armyType == ArmyDef::AircraftCarrier || armyType == ArmyDef::Panzer) ||
            needSupplyArmy->HasCard(CArmy::Commander) ) {
            int maxRingCount = c_theAreaSurroundingInfo->Get_ringCount();
            //存放基本符合的地块
            std::vector<int> list_basicallyMeetAreaID;
            //存放最次符合的地块
            std::vector<int> list_barelyMeetAreaID;
            //是否撤退成功
            bool isRetreatSuccessful = false;
            //循环规定的所有圈层数
            for (int ringNumber = 0; ringNumber < ringCount; ++ringNumber) {
                // 安全检查，防止溢出
                if(ringNumber >= maxRingCount)
                    break;
                std::vector<CAreaOnRing*>* cList_areas_theRingInfo = (*c_theAreaSurroundingInfo).Get_ringInfo(ringNumber);
                if (cList_areas_theRingInfo == nullptr)
                    continue;
                int areas_theRing = (*cList_areas_theRingInfo).size();
                //循环圈中所有地块
                for (int j = 0; j < areas_theRing; ++j) {
                    CArea* foundArea = g_Scene.GetArea((*cList_areas_theRingInfo)[j]->ID);
                    if (g_Scene.Is_safedOpen(foundArea) == false)
                        continue;
                    //如果这个地块符合条件
                    if (foundArea->Is_supplyPoint(needSupplyArmy) &&
                        this->Get_consumedMovement() <= needSupplyArmy->BasicAbilities->Movement){
                        //如果这个地块周围3圈没有发现太大的敌情，则属于最佳地块 尝试直接撤退
                        if (foundArea->Is_existMilitaryThreat(3, 0, theCountry->Alliance) == false ){

                            std::stack<int>* stack_retreatLine = g_areaSurroundingInfoManager.Get_shortestPath(1, this->ID, foundArea->ID, armyIndex);
                            if (stack_retreatLine == nullptr)
                                continue;

                            //尝试直接撤退
                            g_newAiAction.Try_TheArmy_toTargetArea_alongOptimalRoute(this->ID, needSupplyArmy, *stack_retreatLine);

                            delete stack_retreatLine;
                            isRetreatSuccessful = true;
                            //TODO 这里的番号赋值用于测试
                            needSupplyArmy->Designation = 999;
                            break;

                        }
                            //如果周围2圈没有敌情，则属于标准地块，先标记
                        else if (foundArea->Is_existMilitaryThreat(2, 0, theCountry->Alliance) == false){
                            list_basicallyMeetAreaID.push_back(foundArea->ID);
                        }
                    }
                    //既然找已经撤退成功，那么就不用遍历了
                    if (isRetreatSuccessful)
                        break;
                }
                //既然找已经撤退成功，那么就不用遍历了
                if (isRetreatSuccessful)
                    break;
            }
            //可以直接循环下一个单位了
            if (isRetreatSuccessful){
                continue;
            }
            //如果执行到这里说明 没有找到最佳地块进行撤退，那就遍历及格地块
            int areas = list_basicallyMeetAreaID.size();
            //从最远处开始遍历
            for (int i = areas-1; i>= 0; --i) {

                CArea* foundArea = g_Scene.GetArea(list_basicallyMeetAreaID[i]);
                std::stack<int>* stack_retreatLine = g_areaSurroundingInfoManager.Get_shortestPath(1, this->ID, foundArea->ID, armyIndex);
                if (stack_retreatLine == nullptr)
                    continue;
                g_newAiAction.Try_TheArmy_toTargetArea_alongOptimalRoute(this->ID, needSupplyArmy, *stack_retreatLine);
                delete stack_retreatLine;
                isRetreatSuccessful = true;
                    //TODO 这里的番号赋值用于测试
                    needSupplyArmy->Designation = 777;
                    break;

            }
            //运行到此处，还没成功，本单位一定没找到撤退地块。说明周围都是危险的地区或者无法移动，可以直接退出本函数了。
            if(isRetreatSuccessful == false){
                //顺便把军队设置为不活跃状态
                needSupplyArmy->Active = false;
                return;

            }
        }
        else{
            armyIndex++;
        }
    }
}

def_easytech(_ZN9ecUniFont4InitEPKcb)
void ecUniFont::Init(const char *name,bool){
    if(name != nullptr && !(name[0] == 'n' && name[1] == 'u' && name[2] == 'm'))
        name = "MiSans-Regular.fnt";
    easytech(_ZN9ecUniFont4InitEPKcb)(this,name,true);
}
bool IsAroundEnemy(CArea *targetArea, int investigateNum, int allance) {
    int theAllance = allance;
    int num = g_Scene.GetNumAdjacentAreas(targetArea->ID);
    for (int i = 0; i < num; ++i) {
        CArea *adjoinArea = g_Scene.GetAdjacentArea(targetArea->ID, i);
        if (adjoinArea != nullptr && adjoinArea->Country != nullptr &&
            adjoinArea->Country->Alliance != theAllance && adjoinArea->ArmyCount > investigateNum) {
            return true;
        }
    }
    return false;
}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 把这个地块顶部处于溃败状态的军队移动到周围的地块里。<p>
     * 注意：如果撤退成功，该函数完成了军队指针的转移（且调用量removeArmy函数）。<p>
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * void <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： 2022 <p>
     * 【修改时间】： 2022/10/9
     */
bool Is_successful_colliapsedArmyRetreatSafeArea(CArea *theArea, CArmy *theArmy, int morale, int moraleUpTurn, int movement) {

    int count_adjacentAreas = g_Scene.GetNumAdjacentAreas(theArea->ID);
    if (!IsFoundOut(theArea))
        return false;
    int foundEnemys = 5;
    //一个安全的地块
    int standardAreaID = -1;
    int SafestAreaID = -1;
    //循环周围地块数量
    for (int i = 0; i < count_adjacentAreas; ++i) {
        CArea *adjacentArea_ofSelf_byFor = g_Scene.GetAdjacentArea(theArea->ID, i);
        //如果这个地块已开启 且是自己的领地
        if (adjacentArea_ofSelf_byFor != nullptr && adjacentArea_ofSelf_byFor->Enable && adjacentArea_ofSelf_byFor->Country == theArea->Country &&
            adjacentArea_ofSelf_byFor->Sea == theArea->Sea) {
            if (adjacentArea_ofSelf_byFor->ArmyCount < 4) {
                //周围敌军未大于1，属于可撤退地块
                if (adjacentArea_ofSelf_byFor->Is_existMilitaryThreat(1, 0, theArea->Country->Alliance) == false) {
                    //有我军的地块，属于安全地块。否则，属于标准地块
                    if (adjacentArea_ofSelf_byFor->ArmyCount >= 1) {
                        //百分之50直接撤退
                        int r = g_GameManager.GetRand() % 2;
                        if (r == 0) {
                            theArmy->Movement = movement;
                            theArea->Retreat_frontArmy_toAdjacentArea(adjacentArea_ofSelf_byFor);
                            theArmy->Morale = morale;
                            theArmy->MoraleUpTurn = moraleUpTurn;
                            return true;
                        }
                        //把这个地块标记为安全地块
                        SafestAreaID = adjacentArea_ofSelf_byFor->ID;
                    }
                    else{
                        standardAreaID = adjacentArea_ofSelf_byFor->ID;
                    }
                }
            }
        }
    }
    /*
     * 执行到这里，说明还没撤退。有两个原因：
     * 第一：虽然有可以撤退的地块，但是一直没有被随机进去。
     * 第二：没有可撤退地块
     */
    if (SafestAreaID != -1) {
        theArmy->Movement = movement;
        theArea->Retreat_frontArmy_toAdjacentArea(g_Scene[SafestAreaID]);
        theArmy->Morale = morale;
        theArmy->MoraleUpTurn = moraleUpTurn;
        return true;
    }
    if (standardAreaID != -1) {
        theArmy->Movement = movement;
        //执行到这里，说明未找到可直接溃败的安全地块，于是就溃败到可移动的地块中
        theArea->Retreat_frontArmy_toAdjacentArea(g_Scene[standardAreaID]);
        theArmy->Morale = morale;
        theArmy->MoraleUpTurn = moraleUpTurn;
        return true;
    }
    return false;
}



////递归
//void RecursiveAreas(std::vector<int> &list_nextLoop, CCountry *theCountry, CArmy *theArmy,
//                    CArea *theArea, bool isNavy, int &ergodicNum, int &f, int &num, bool &isFound,
//                    bool arrFoundAreas[]) {
//    num++;
//    if (num > ergodicNum) {
//        return;
//    }
//    std::vector<int> _list_nextLoop;
//    //根据需要被遍历的地块数量进行本次循环
//    for (int i = 0; i < list_nextLoop.size(); ++i) {
//        CArea *targetArea = g_Scene[list_nextLoop[i]];
//        //如果被遍历过
//        if (arrFoundAreas[targetArea->ID] == true) {
//            continue;
//        }
//        //如果被遍历到的这个地块符合条件并且周围没有发现敌情
//        if (Is_supplyPoint(targetArea, theCountry, isNavy) == true &&
//            IsAroundEnemy(targetArea, 1, theCountry->Alliance) == false) {
//            TransferArmyTo(theArea, targetArea, theArmy, 1, 3, 5);
//            isFound = true;
//            if (f != 0) {
//                f--;
//            }
//            return;
//        }
//        arrFoundAreas[targetArea->ID] = true;
//        int num = g_Scene.GetNumAdjacentAreas(targetArea->ID);
//        //循环第x圈地块之一的周围地块
//        for (int j = 0; j < num; ++j) {
//            CArea *adjoinArea = g_Scene.GetAdjacentArea(targetArea->ID, j);
//            //把这个地块添加到下一圈的地块中
//            _list_nextLoop.push_back(adjoinArea->ID);
//
//        }
//        if (isFound == true) {
//            return;
//        }
//    }
//    if (isFound == true) {
//        return;
//    }
//    RecursiveAreas(_list_nextLoop, theCountry, theArmy, theArea, isNavy, ergodicNum, num, f,
//                   isFound, arrFoundAreas);
//    return;
//}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 得到指定单位在本地块的军队数组下标。<p>
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * 如果该单位不存在于本地块，返回-1.否则，返回对应下标。 <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： 李德邻 <p>
     * 【创建时间】： ？？？<p>
     * 【修改时间】： ？？？
     */
int CArea::GetArmyIdx(CArmy *army) {
    int armyCount = this->ArmyCount;
    for (int i = 0; i < armyCount; ++i) {
        if (army == this->Army[i]) {
            return i;
        }
    }
    return -1;
}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 移除该地块的指定单位。如果该单位不在本地块，不会移除。<p>
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * void <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： 李德邻 <p>
     * 【创建时间】： ？？？<p>
     * 【修改时间】： ？？？
     */
void CArea::RemoveArmy(CArmy *army) {
    int item = GetArmyIdx(army);
    //如果这个军队在这个地块存在
    if (item >= 0) {
        //后面的单位往前移动
        int max = this->ArmyCount - 1;
        for (int i = item; i < this->ArmyCount - 1; ++i) {
            this->Army[i] = this->Army[i + 1];
        }
        this->Army[max] = nullptr;
        this->ArmyCount -= 1;

    }
}

/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 根据各个单位的价值调整ai的排兵。价值越低，优先在上面。<p>
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * void <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： ？？？ <p>
     * 【修改时间】： 2022/10/22
     */
void  CArea::Adjust_armyOrder() {
    if (this->ArmyCount < 2)
        return;
    std::vector<int> list_armyValue;
    int tempValue ;
    CArmy* tempArmy;
    for (int i = 0; i < this->ArmyCount; ++i) {
        list_armyValue.push_back(this->GetArmy(i)->Get_armyValue());
    }
    //选择排序
    for (int i = 0; i < list_armyValue.size()-1; ++i) {
        int index_minValue = i;
        //找到这个下标后面 最大值的下标
        for (int j = i+1; j < list_armyValue.size(); ++j) {
            if (list_armyValue[j] < list_armyValue[index_minValue])
                index_minValue = j;
        }
        //如果后面的数中，最大的是自己则不用交换
        if (index_minValue != i){
            tempValue = list_armyValue[index_minValue];
            tempArmy = this->Army[index_minValue];
            list_armyValue[index_minValue] = list_armyValue[i];
            this->Army[index_minValue] = this->Army[i];
            list_armyValue[i] = tempValue;
            this->Army[i] = tempArmy;
        }
    }
}

//设置ai使用卡牌的逻辑
void Set_AI_DraftLogic(CArea *area, int money, int industry, int r, int ID, int level, int cards) {

    srand(time(NULL));
    int random = rand() % 101 + 1;
    CCountry *country = area->Country;
    if (country->Money > money&&country->Industry > industry) {
        if (random + r >= 100) {
            CArmy *Army = new CArmy();
            ArmyDef *armyDef = CObjectDef::Instance()->GetArmyDef(ID, country->Name);
            Army->Init(armyDef, country);
            SetArmyDesignationSea(Army, country);
            Army->Cards = cards;
            Army->Level = level;
            Army->Movement = 0;
            Army->Active = false;
            area->AddArmy(Army, true);
            country->Money -= NewCardDef[0]->Price;
            country->Industry -= NewCardDef[0]->Industry;
        }
    }
}

void AI_Uses_NewCard(CArea *theArea) {
    if (theArea != nullptr&&theArea->Country != nullptr&&theArea->Enable != false&&
        theArea->Sea == true) {
        CCountry *theCountry = theArea->Country;
        if (theArea->Type == port&&theCountry->AI == true) {
            if (strcmp(theCountry->ID, "gb") == 0) {
                Set_AI_DraftLogic(theArea, 400, 200, 8, 28, 0, 1);
            }
            if (strcmp(theCountry->ID, "de") == 0) {
                Set_AI_DraftLogic(theArea, 400, 200, 13, 28, 1, 1);
            }
        }
    }
}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 指定这个地块一定范围内是否有超过指定范围的敌军数量，如果有则设置为非安全，否则为安全地块。<p>
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * void <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： 李德邻 <p>
     * 【创建时间】： ？？？<p>
     * 【修改时间】： 2022/7/4
     */
void CArea::Set_SafeOfArea(int ringCount, int enemyCount) {
    if (this->Country == nullptr||this->Enable == false) {
        return;
    }
    int allance = this->Country->Alliance;
    //如果不是海洋国家
    if (this->Country->IsCoastal == false) {
        if (this->Type != capital) {
            //循环指定的圈层数
            auto theAreaSurroundingInfo = g_areaSurroundingInfoManager.Get_theAreaSurroundingInfo(this->ID);
            for (int i = 0; i < ringCount; ++i) {
                //获得当前圈层的地块数量
                std::vector<CAreaOnRing*>* theRingInfo = theAreaSurroundingInfo->Get_ringInfo(i);
                if (theRingInfo == nullptr)
                    break;
                int areas = theRingInfo->size();
                for (int j = 0; j < areas; ++j) {
                    int id = (*theRingInfo)[j]->ID;
                    if (Is_inSpecifiedRangeOfAreas(id)){
                        CArea* foundArea = g_Scene[id];
                        if (foundArea != nullptr &&  foundArea->Enable && foundArea->Country != nullptr &&
                            IsAroundEnemy(foundArea, enemyCount, allance)){
                            this->IsSafe = false;
                            return;
                        }
                    }
                }
            }
        }
            this->IsSafe = true;

    }
    else{
        this->IsSafe = false;
    }

}

def_easytech(_ZN5CArea9TurnBeginEv);

/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 回合开始时，进行地块各项数据的更新。<p>
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * void <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】：李德邻 <p>
     * 【创建时间】： ？？？<p>
     * 【修改时间】： ？？？
     */
void CArea::TurnBegin() {
//    AI_Uses_NewCard(this);
    for (int i = 0; i < this->ArmyCount; ++i) {
        CArmy* foundArmy = this->Army[i];
        foundArmy->TurnBegin();
        if (this->Sea) {
            if (foundArmy->Movement > 2) {
                foundArmy->Movement = 2;
            }
        }
    }
    if (g_GameManager.IsIntelligentAI  && (this->Country->AI || g_GameManager.Is_spectetorMode())){
        this->Set_SafeOfArea(7, 0);
        //触发的骰子数阈值（小于等于）
        int thresholdDice = 4;
        //遍历的圈数
        int ringCount = 4;

    }
//    easytech(_ZN5CArea9TurnBeginEv)(this);
}

/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 重新检测周围地块是否位于包围状态，并进行各地块单位的状态更新。<p>
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * void <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： 李德邻<p>
     * 【创建时间】： ？？？<p>
     * 【修改时间】： ？？？
     */
void CScene::AdjacentAreasEncirclement(int AreaID) {
    CArea *area = this->GetArea(AreaID);
    area->Encirclement();
    int Num = this->GetNumAdjacentAreas(AreaID);
    //循环和这个地块相连的所有地块
    for (int i = 0; i < Num; ++i) {
        CArea *temArea = this->GetAdjacentArea(AreaID, i);
        temArea->Encirclement();
    }
}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 判断两个地块是否是同盟国家关系。<p>
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * 注意：如果返回false，有可能地块是空值或者国家是空值。 <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： 2022/7/31 <p>
     * 【修改时间】： ？？？
     */
bool CScene::Is_ally(int startAreaID, int targetAreaID) {
    CArea* startArea = this->GetArea(startAreaID);
    CArea* targetArea = this->GetArea(targetAreaID);
    if(Is_safedOpen(targetArea)==false || Is_safedOpen(startArea) == false)
        return false;
    if (startArea->Country == nullptr || targetArea->Country == nullptr)
        return false;
    return startArea->Country->Alliance == targetArea->Country->Alliance;
}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 判断两个地块是否是非盟友关系。该状态包括敌对/和平<p>
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * 注意：如果返回false，有可能地块是空值或者国家是空值。或者地块未开启 <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： 2022/7/31 <p>
     * 【修改时间】： ？？？
     */
bool CScene::Is_nonAlly(int startAreaID, int targetAreaID) {
    CArea* startArea = this->GetArea(startAreaID);
    CArea* targetArea = this->GetArea(targetAreaID);
    if(startArea == nullptr || targetArea == nullptr || startArea->Enable == false || targetArea->Enable == false)
        return false;
    if (startArea->Country == nullptr || targetArea->Country == nullptr)
        return false;
    return startArea->Country->Alliance != targetArea->Country->Alliance;
}


/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 初始化 InitListAreaSurroundingInfo
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * void <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： 李德邻<p>
     * 【创建时间】： ？？？<p>
     * 【修改时间】： 2022/7/25
     */
void AreaSurroundingInfoManager::InitListAreaSurroundingInfo() {

    ecFile *file_areaSurroundingInfo = new ecFile();
    int size = s_Count_theMapAreas;
    //读取二进制文件  则写 rb
    file_areaSurroundingInfo->Open(GetPath("areaSurroundingInfo.bin", nullptr), "rb");
    int areaTotal;
    file_areaSurroundingInfo->Read(&areaTotal, sizeof(int));
    for (int i = 0; i < size; ++i) {
        CAreaSurroundingInfo* areaSurroundingInfo = new CAreaSurroundingInfo();
        //这个地块所有圈层的地块总和
        int total_areaOnRIng;
        file_areaSurroundingInfo->Read(&total_areaOnRIng, sizeof(int));
        int numOfRings;
        for (int j = 0; j < total_areaOnRIng; ++j) {
            int areaOnRing_ID,ringNum;
            file_areaSurroundingInfo->Read(&areaOnRing_ID, sizeof(int));
            file_areaSurroundingInfo->Read(&ringNum, sizeof(int));
            areaSurroundingInfo->Set_ringNum_areaOnRing(areaOnRing_ID, ringNum);
        }
        for (int j = 0; j < total_areaOnRIng; ++j) {
            int areaOnRing_ID,indexOnRing;
            file_areaSurroundingInfo->Read(&areaOnRing_ID, sizeof(int));
            file_areaSurroundingInfo->Read(&indexOnRing, sizeof(int));
            areaSurroundingInfo->Set_index_ofAreaOnRingAtRing(areaOnRing_ID, indexOnRing);
        }
        file_areaSurroundingInfo->Read(&numOfRings, sizeof(int));
        //读取圈层数量
        for (int j = 0; j < numOfRings ; ++j) {
            //这个圈层的所有 areaOnRing
            std::vector<CAreaOnRing*>* list_areaOnRing_theRing = new std::vector<CAreaOnRing*>() ;
            int areaOnRings_theRing  ;
            file_areaSurroundingInfo->Read(&areaOnRings_theRing, sizeof(int));
            //循环该圈层地块数量
            for (int k = 0; k < areaOnRings_theRing; ++k) {
                int areaID ;
                //链接上一圈层的地块数
                int  areas_linkLastRing ;
                file_areaSurroundingInfo->Read(&areaID, sizeof(int));
                file_areaSurroundingInfo->Read(&areas_linkLastRing, sizeof(int));
                CAreaOnRing* areaOnRing  = new CAreaOnRing();
                for (int l = 0; l < areas_linkLastRing; ++l) {
                    //链接上一圈层的地块id
                    int areaID_linkLastRing;
                    file_areaSurroundingInfo->Read(&areaID_linkLastRing, sizeof(int));
                    (areaOnRing->List_areaID_linkLastRing).push_back(areaID_linkLastRing);
                }
                int areas_linkNextRing;
                file_areaSurroundingInfo->Read(&areas_linkNextRing, sizeof(int));
                for (int l = 0; l < areas_linkNextRing; ++l) {
                    int areaID_linkNextRing;
                    file_areaSurroundingInfo->Read(&areaID_linkNextRing, sizeof(int));
                    (areaOnRing->List_areaID_linkNextRing).push_back(areaID_linkNextRing);
                }
                if (Is_inSpecifiedRangeOfAreas(areaID)){
                    areaOnRing->ID = areaID;
                    (list_areaOnRing_theRing)->push_back(areaOnRing);
                }
            }
            //把这个圈层地块记录 推进这个地块的areaSurroundingInfo
            (*areaSurroundingInfo).Add_ringInfo(list_areaOnRing_theRing);
        }
        this->ListAreaSurroundingInfo->push_back(areaSurroundingInfo);
    }
    file_areaSurroundingInfo->Close();
    delete file_areaSurroundingInfo;
    ecFile* file_areaJointlyLinkInfo = new ecFile();
    file_areaJointlyLinkInfo->Open(GetPath("areaJointlyLinkInfo.bin", nullptr), "rb");
    file_areaJointlyLinkInfo->Read(&areaTotal, sizeof(int));
    for (int i = 0; i < size; ++i) {
        int areaID, count_linkWithAroundAreas;
        file_areaJointlyLinkInfo->Read(&areaID,sizeof(int));
        file_areaJointlyLinkInfo->Read(&count_linkWithAroundAreas,sizeof(int));
        CJointlyLinkWithAroundAreasInfo* jointlyLinkWithAroundAreaInfo = new CJointlyLinkWithAroundAreasInfo(areaID);
        std::vector<std::vector<int>>* list_areas_jointlyLink = new std::vector<std::vector<int>>();
        jointlyLinkWithAroundAreaInfo->Init_ListAreasJointlyLink(list_areas_jointlyLink);
        //循环该地块所连接的周围地块
        for (int j = 0; j < count_linkWithAroundAreas; ++j) {
            int count_jointlyLink_withAroundArea;
            file_areaJointlyLinkInfo->Read(&count_jointlyLink_withAroundArea,sizeof(int));
            list_areas_jointlyLink->push_back(std::vector<int>());
            //循环与这个周围地块共同接壤的地块
            for (int k = 0; k < count_jointlyLink_withAroundArea; ++k) {
                int area_jointlyLink;
                file_areaJointlyLinkInfo->Read(&area_jointlyLink,sizeof(int));
                (*list_areas_jointlyLink)[j].push_back(area_jointlyLink);
            }
        }
        this->ListAreaJointlyLinkInfo->push_back(jointlyLinkWithAroundAreaInfo);
    }
    auto s = this->ListAreaJointlyLinkInfo;

    file_areaJointlyLinkInfo->Close();
    delete file_areaJointlyLinkInfo;
}
//请确保参数一定不会越界
CAreaSurroundingInfo* AreaSurroundingInfoManager::Get_theAreaSurroundingInfo(int theAreaID) {
    return (*ListAreaSurroundingInfo)[theAreaID];
}

void AreaSurroundingInfoManager::Init() {
    this->ListAreaSurroundingInfo = new std::vector<CAreaSurroundingInfo*>();
    this->ListAreaJointlyLinkInfo = new std::vector<CJointlyLinkWithAroundAreasInfo*>();
}


/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 得到起始点到目的地的最佳路线。<p>
     * 【参数说明】：<p>
     * targetAreaID：请确定该参数一定起始地块的周围地块信息的范围内。<p>
     * pathType：<p>
     * 1表示谨慎行军路线。只走无人区。<p>
     * 2表示就近行军路线。无视敌军数量，优先走最近路线。<p>
     * 3表示激进行军路线。敌军越少，越优先走.<p>
     * 4旧aI专用。只要可移动可攻击的，都在路线中。<p>
     * 各个路线： 如果是海洋地块有概率造船入水，否则绕开.<p>
     * 【返回值说明】：<p>
     * 从起点到目标点的一条最佳路径（返回的路径不包括起点）。目的地在stack底部，出发地在stack顶部。<p>
     * 如果去不了，返回null<p>
     * 注意：记得用完释放。 <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】：2022/7/25 <p>
     * 【修改时间】： 2022/10/10
     */
std::stack<int>* AreaSurroundingInfoManager::Get_shortestPath(int pathType, int startAreaID, int targetAreaID, int armyIndex) {
    auto area = g_Scene.GetArea(targetAreaID);
    if(area->Enable == false)
        return nullptr;
    g_algorithm.Dijkstra(pathType, startAreaID, targetAreaID, armyIndex, 15);
    std::stack<int>* stack_shortestPath = new std::stack<int>();
    int areaID = targetAreaID;
    std::vector<int> list;
    //开始组成最短路径
    while (areaID != startAreaID ){
        list.push_back(areaID);
        stack_shortestPath->push(areaID);
        areaID = g_algorithm.Arr_lastAreaID_inShortestPath[areaID];
        if (areaID == -1)
            break;
    }
    if (areaID == -1){
        delete stack_shortestPath;
        return nullptr;
    }

//    int ilast1 = g_algorithm.Arr_lastAreaID_inShortestPath[1865];
//    int ilast2 = g_algorithm.Arr_lastAreaID_inShortestPath[1868];
    //如果大于等于500，肯定是不能过去的
    areaID = (*stack_shortestPath).top();
    if(g_algorithm.Arr_distance_ofStartAreaToTargetArea[areaID] >= 500){
        delete stack_shortestPath;
        return nullptr;
    }


    return stack_shortestPath;
}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 得到两个相连地块，出发点到目标点所需的移动力消耗。<p>
     * 【参数说明】：<p>
     * targetAreaID：请确定这个值一定和startArea相连。<p>
     * 【返回值说明】：<p>
     * 如果无法通过（包括最大行动力依然不可过去），返回100.否则，单位移动过去所需的移动力。 <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： 2022/7/25<p>
     * 【修改时间】： 2022/7/28
     */
int
AreaSurroundingInfoManager::Get_consume_betweenTwoAreas(int startAreaID, int targetAreaID, CArmy* theArmy, CArea* armyArea) {
    CArea* area1 = g_Scene.GetArea(startAreaID);
    CArea* area2 = g_Scene.GetArea(targetAreaID);
    if (area1 == nullptr || area2 == nullptr || area1->Enable == false ||area2->Enable == false)
        return 999;
    int consume = area2->Get_consumedMovement(theArmy);
    return consume <= theArmy->BasicAbilities->Movement?consume:999;
}
int Algorithm::Arr_distance_ofStartAreaToTargetArea[s_Count_theMapAreas]{};
int Algorithm::Arr_lastAreaID_inShortestPath[s_Count_theMapAreas]{};
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 获取到所有点的最短路径.迪杰斯特拉算法的一部分。<p>
     * 【参数说明】：<p>
     * pathType：<p>
     * 1表示谨慎行军路线。<p>
     * 2表示就近行军路线。<p>
     * 3表示激进攻击路线。<p>
     * 4表示旧AI进攻路线。<p>
     * 【返回值说明】：<p>
     * void <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： 李德邻<p>
     * 【创建时间】：  2022/7/25 <p>
     * 【修改时间】：  2022/7/28
     */
void
Algorithm::Set_lastID_inShortestPath(int targetAreaID, CArmy* theArmy, int pathType, CArea* armyArea) {
    int armyIndex = armyArea->GetArmyIdx(theArmy);
    CArea* targetArea = g_Scene.GetArea(targetAreaID);
    int count_withLinkAroundAreas = g_Scene.GetNumAdjacentAreas(targetAreaID);
    //循环目标地块的周围地块
    for (int i = 0; i < count_withLinkAroundAreas; ++i) {
        int aroundAreaID_withLinkTargetArea = g_Scene.GetAdjacentAreaID(targetAreaID, i);
        int distance = g_areaSurroundingInfoManager.Get_consume_betweenTwoAreas(targetAreaID, aroundAreaID_withLinkTargetArea, theArmy, armyArea);
        //根据路线类型，地块距离需要附加”额外数值“。
        int  extraDistance = g_areaSurroundingInfoManager.Get_extraDistance_basisOfPathType(theArmy->Country, aroundAreaID_withLinkTargetArea, pathType, armyArea->ID,armyIndex , targetAreaID);
        //加上到上一个地块的id
        int NewDistance = distance + extraDistance + Arr_distance_ofStartAreaToTargetArea[targetAreaID];
        //如果小于原有距离，更新
        if (  NewDistance < Arr_distance_ofStartAreaToTargetArea[aroundAreaID_withLinkTargetArea] ){
            Arr_distance_ofStartAreaToTargetArea[aroundAreaID_withLinkTargetArea] =  NewDistance;
            Arr_lastAreaID_inShortestPath[aroundAreaID_withLinkTargetArea] = targetAreaID;
        }
    }
    sArr_foundArea[targetAreaID] = true;
}
//请确保参数一定不会越界 得指定地块与各个周围地块所共同接壤地块的信息。
CJointlyLinkWithAroundAreasInfo* AreaSurroundingInfoManager::Get_theAreaJointlyLinkWithAroundAreasInfo(int theAreaID) {
    return (*this->ListAreaJointlyLinkInfo)[theAreaID];
}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 根据行动路线的设定，对各个地块的权重“距离”进行增减设定。<p>
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * 额外权重 <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： 2022/7/28<p>
     * 【修改时间】： 2022/10/10
     */
int
AreaSurroundingInfoManager::Get_extraDistance_basisOfPathType(CCountry* theCountry, int targetAreaID, int pathType, int armyAreaID, int armyIndex, int startAreaID) {
    CArea* targetArea = g_Scene.GetArea(targetAreaID);
    //TODO ai权重在此设定
    auto army = g_Scene.GetArea(armyAreaID)->GetArmy(armyIndex);
    /*
     * 谨慎行军路线，只走本国路线，且优先无敌情区。
     */
    if (pathType == 1){
        //只走本国和中立地块
        if (targetArea != nullptr && targetArea->Country != theCountry){
                return 999;
        }
        int result = 0;
        if (CActionAssist::Instance()->aiCheckMoveable(startAreaID, targetAreaID, armyIndex, armyAreaID) == false &&
            CActionAssist::Instance()->NewAiCheckAttackAble(startAreaID, targetAreaID, armyIndex, armyAreaID) == false ){
            result = 999;
            //要下水，当然要准备好船
            if(targetArea->Sea && army->IsNavy() == false && army->HasCard(CArmy::EngineeringCorps) == false  ){
                auto ecrCard = CObjectDef::Instance()->GetCardDef(CARD_ID::ECRCard);
                if (theCountry->Money > ecrCard->Price / 2 ||theCountry->Industry > ecrCard->Industry / 2) {
                    int r = g_GameManager.GetRand() % 100 + 1;
                    if (r <= 70)
                        result = 0;
                }
            }
        }

        int count_linkWithAroundAreas = g_Scene.GetNumAdjacentAreas(targetAreaID);
        //地块周围地块有敌军，并且这个地块本身也没有守军
        for (int i = 0; i < count_linkWithAroundAreas; ++i) {
            CArea* aroundArea_byFor = g_Scene.GetAdjacentArea(targetAreaID,i);
            if (aroundArea_byFor->Country!= nullptr && aroundArea_byFor->Country != theCountry && aroundArea_byFor->ArmyCount > 0 && targetArea->ArmyCount <1 ){
                result = 2;
            }
        }
        return result;
    }
        /*
         * 就近行军路线，无论敌情，优先走最近距离
         */
    else if (pathType == 2) {
        int result = 0;
        if (CActionAssist::Instance()->aiCheckMoveable(startAreaID, targetAreaID, armyIndex, armyAreaID) == false &&
            CActionAssist::Instance()->NewAiCheckAttackAble(startAreaID, targetAreaID, armyIndex, armyAreaID) == false ){
            result = 999;
            //要下水，当然要准备好船
            if(targetArea->Sea && army->IsNavy() == false && army->HasCard(CArmy::EngineeringCorps) == false  ){
                auto ecrCard = CObjectDef::Instance()->GetCardDef(CARD_ID::ECRCard);
                if (theCountry->Money > ecrCard->Price / 2 ||theCountry->Industry> ecrCard->Industry / 2) {
                    int r = g_GameManager.GetRand() % 100 + 1;
                    if (r <= 70)
                        result = 0;
                }
            }
        }
        return result;
    }
        /*
         * 激进行军路线，敌军越少，越优先走
         */
    else if (pathType == 3){
        int result = 0;
        if (CActionAssist::Instance()->aiCheckMoveable(startAreaID, targetAreaID, armyIndex, armyAreaID) == false &&
            CActionAssist::Instance()->NewAiCheckAttackAble(startAreaID, targetAreaID, armyIndex, armyAreaID) == false ){
            result = 999;
            //要下水，当然要准备好船
            if(targetArea->Sea && army->IsNavy() == false && army->HasCard(CArmy::EngineeringCorps) == false  ){
                auto ecrCard = CObjectDef::Instance()->GetCardDef(CARD_ID::ECRCard);
                if (theCountry->Money > ecrCard->Price / 2 ||theCountry->Industry > ecrCard->Industry / 2) {
                    int r = g_GameManager.GetRand() % 100 + 1;
                    if (r <= 70)
                        result = 0;
                }
            }
        }
        if(targetArea->ArmyCount > 0&& targetArea->Country != theCountry){
            result  +=( targetArea->ArmyCount);
        }
        return result;
    }
    return 0;
}


/*
 * 空袭
 */
void CFight::AirStrikesAttack(CCountry *startCountry, int TargetAreaID, int type) {
    this->StartArmyReceiveDamage=0;
    //被进攻的地块
    this->TargetAreaID = TargetAreaID;
    //发动进攻的地块
    this->StartAreaID = -1;
    //空袭的类型
    int airStrikesCardType = 11;
    CArea *targetArea = g_Scene.GetArea(TargetAreaID);
    CObjectDef *obj = CObjectDef::Instance();
    int maxAttack = 0;
    int minAttack = 0;

    //如果是机枪空袭
    if (type == 1) {
        airStrikesCardType = 11;
        obj = CObjectDef::Instance();
    }
    else {
        //如果是轰炸
        if (type == 2) {
            airStrikesCardType = 12;
        }
        else {
            minAttack = 0;
            maxAttack = 0;
        }
    }
    if (type == 1||type == 2) {
        //获取对应空袭类型的攻击数值
        ArmyDef *armyDef = obj->GetArmyDef(airStrikesCardType, startCountry->Name);
        minAttack = armyDef->MinAttack;
        maxAttack = armyDef->MaxAttack;
    }
    //如果发动进攻的国家指挥官所用的空袭章等级 大于0，攻击数值+1
    if (startCountry->GetWarMedalLevel(AirForceMedal) > 0) {
        minAttack++;
        maxAttack++;
    }
    int airStrikesAttack = GetRand() % (maxAttack - minAttack + 1);
    this->AirStrikeAttack = airStrikesAttack + minAttack;
    //如果是地块是首都类型
    if (targetArea->InstallationType == CArea::radar||
        targetArea->InstallationType == CArea::antiaircraft) {
        this->TargetArmyReceiveDamage = this->AirStrikeAttack * 3;
    }
    else {
        this->TargetArmyReceiveDamage = this->AirStrikeAttack * 5;
    }
    if (type == 3) {
        this->TargetArmyReceiveDamage = 0;
    }
    //地形
    if (targetArea->Type == forest) {
        TargetArmyReceiveDamage *= 0.75;
    }
    else if (targetArea->Type == hilly) {
        TargetArmyReceiveDamage *= 0.75;
    }
    else if (targetArea->Type == desert) {
        TargetArmyReceiveDamage *= 1.40;
    }
    this->AirStrikeType = type;
    this->SecondAttackIndex = 0;
    this->StartArmySecondAttack = false;
    this->TargetArmySecondAttack = false;
    //被攻击方能否反击
    this->CanCounter = false;
    this->AirStrikeCountry = startCountry;
}

/*
 * 默认空袭
 */
void CFight::AirStrikesAttack(int StartAreaID, int TargetAreaID) {
    CArea *startArea = g_Scene.GetArea(StartAreaID);
    CArea *targetAre = g_Scene.GetArea(TargetAreaID);
    AirStrikesAttack(startArea->Country, TargetAreaID, 1);
    this->StartAreaID = StartAreaID;
    this->AirStrikeType = 4;
}

/*
 * 首次攻击  然后转到NormalAttack函数中
 */
void CFight::FirstAttack(int StartAreaID, int TargetAreaID) {
    this->SecondAttackIndex = 0;
    this->NormalAttack(StartAreaID, TargetAreaID);
    this->StartArmyReceiveDamageFirst = this->StartArmyReceiveDamage;
    this->TargetArmyReceiveDamageFirst = this->TargetArmyReceiveDamage;
}

int CArmy::GetMaxStrength() {
    return this->MaxHp;
}

/*
 * 得到战斗的骰子
 */
int CArmy::GetNumDices() {
    int maxHp = this->GetMaxStrength() ;
    int currentHp = (this->Hp) ;
    int x = currentHp * 100 / maxHp;
    if (x <= 5) {
        return 1;
    }
    if (x < 15) {
        return 2;
    }

    if (x < 25) {
        return 3;
    }
    if (x < 50) {
        return 4;
    }

    return 5;

}

void GameStart() {
    g_GameManager.IsPause = false;
}

void GamePause() {
    g_GameManager.IsPause = true;
}




void Init_armisticeStatus(int scriptNum) {
    switch (scriptNum) {
        case 0:
            Init_armisticeStatus_conquest1();
            break;
        default:
            break;
    }

}

void Init_armisticeStatus_conquest1() {
    //双循环   每个国家要和除自己之外的所有国家建立和平关系
    for (int i = 0; i < g_GameManager.ListCountry.size(); ++i) {
        for (int j = 0; j < g_GameManager.ListCountry.size(); ++j) {
            //如果是自己
            if ((strcmp(g_GameManager.ListCountry[i]->ID, g_GameManager.ListCountry[j]->ID) == 0)) {
                continue;
            }
            //如果是苏德两国
            if (((strcmp(g_GameManager.ListCountry[j]->ID, "ru") == 0) &&
                 (strcmp(g_GameManager.ListCountry[i]->ID, "de") == 0)) ||
                ((strcmp(g_GameManager.ListCountry[j]->ID, "de") == 0) &&
                 (strcmp(g_GameManager.ListCountry[i]->ID, "ru") == 0))) {
                g_GameManager.ListCountry[i]->ListArmisticeStatus[g_GameManager.ListCountry[j]->ID] = 0;
                continue;
            }
            //否则
            g_GameManager.ListCountry[i]->ListArmisticeStatus[g_GameManager.ListCountry[j]->ID] = 0;
        }
    }
}

/*
 * 得到该单位失去血量后的骰子数
 */
int CArmy::GetNumDicesIfLostStrength(int damage) {

    if (damage > this->Hp) {
        return 0;
    }
    int maxHp = this->GetMaxStrength();
    int currentHp = (this->Hp - damage) * 100;
    int x = currentHp / maxHp;
    if (x <= 5) {
        return 1;
    }
    else if (x < 15) {
        return 2;
    }
    else if (x < 25) {
        return 3;
    }
    else if (x < 50) {
        return 4;
    }
    else {
        return 5;
    }
}

int Get_attackValue_formStartArmyAttackTargetArmy(CFight *fight, CArmy *startArmy, CArmy *targetArmy, bool isStart) {
    int minAttack = startArmy->BasicAbilities->MinAttack;
    int maxAttack = startArmy->BasicAbilities->MaxAttack;
    int minPiercing = startArmy->BasicAbilities->MinPiercing;
    int maxPiercing = startArmy->BasicAbilities->MaxPiercing;
    if (isStart){
        minAttack += fight->StartArmyExtraAttack;
        maxAttack += fight->StartArmyExtraAttack;
        minPiercing += fight->StartArmyExtraPiercing;
        maxPiercing += fight->StartArmyExtraPiercing;
    }else{
        minAttack += fight->TargetArmyExtraAttack;
        maxAttack += fight->TargetArmyExtraAttack;
        minPiercing += fight->TargetArmyExtraPiercing;
        maxPiercing += fight->TargetArmyExtraPiercing;
    }
    int attackValue_total = 0;
    int attackValue_byDice = 0;
    int dices = startArmy->GetNumDices();
    //初始化开始军队的骰子
    for (int i = 0; i < dices; ++i) {
        int targetArmor = targetArmy->BasicAbilities->Armor;
        attackValue_byDice = GetRand(startArmy) % (maxAttack - minAttack + 1);
        attackValue_byDice += minAttack;
        //如果敌军的装甲度大于0,获取装甲攻击
        if (s_NewArmyAttribute && targetArmor > 0) {
            int piercing = GetRand(startArmy) % (maxPiercing - minPiercing + 1);
            piercing += minPiercing;
            //如果未能击穿对方的装甲 攻击值下降50%
            if (piercing <= targetArmor) {
                attackValue_byDice /= 2;
            }
        }
        if (isStart) {
            fight->StartArmyDices.push_back(attackValue_byDice);
        }
        else {
            fight->TargetArmyDices.push_back(attackValue_byDice);
        }
        attackValue_total += attackValue_byDice;
    }
    return attackValue_total;
}


//普通战斗
void CFight::NormalAttack(int StartAreaID, int TargetAreaID) {
    CArea *startArea = g_Scene.GetArea(StartAreaID);
    CArea *targetArea = g_Scene.GetArea(TargetAreaID);
    this->StartAreaID = StartAreaID;
    this->TargetAreaID = TargetAreaID;
    CArmy *startArmy = startArea->GetArmy(0);
    CArmy *targetArmy = targetArea->GetArmy(0);
    ArmyDef::ArmyType startArmy_ID = startArmy->BasicAbilities->ID;
    ArmyDef::ArmyType targetArmy_ID = targetArmy->BasicAbilities->ID;
    //如果是停战状态
    if (g_Scene.Is_armisticeStatus(startArea->Country, targetArea->Country)) {
        this->StartArmyReceiveDamage = 0;
        this->TargetArmyReceiveDamage = 0;
        startArmy->Movement = 0;
        targetArmy->Movement = 0;
        startArmy->Active = false;
        targetArmy->Active = false;
        //空袭类型设置为0，表示这是普通攻击
        this->AirStrikeType = 0;
        //二次进攻的重设为0
        this->StartArmySecondAttack = false;
        this->TargetArmySecondAttack = false;
        return;
    }
    SetCounter(this, startArmy_ID, targetArmy_ID, startArmy, targetArmy, targetArea);
    //设置他们的骰子数
    int dices_startArmy = startArmy->GetNumDices();
    int dices_targetArmy = targetArmy->GetNumDices();

    this->StartArmyDefence = 0;
    this->StartArmyExtraAttack = 0;
    this->TargetArmyDefence = 0;
    this->TargetArmyExtraAttack = 0;
    this->StartArmyExtraPiercing = 0;
    this->TargetArmyExtraPiercing = 0;
    // 下面为攻击双方设置攻防点数 包括根据战术卡和上述的Extra值。
    Set_armyBuff(this, startArmy, startArea, true);
    Set_armyBuff(this, targetArmy, targetArea, false);
    //打出的伤害（还未减掉防御力）
    int attackValue_fromStartArmy = Get_attackValue_formStartArmyAttackTargetArmy(this, startArmy, targetArmy, true);
    int attackValue_fromTargetArmy = Get_attackValue_formStartArmyAttackTargetArmy(this, targetArmy, startArmy, false);
    //减掉防御力
    attackValue_fromStartArmy -= this->TargetArmyDefence * dices_startArmy;
    attackValue_fromTargetArmy -= this->StartArmyDefence * dices_targetArmy;
    //设置伤害加伤（根据兵种克制关系）
    Increase_damage_accordingToRestraintRelationship(startArmy, targetArmy, attackValue_fromTargetArmy, attackValue_fromStartArmy);
    //设置地形效果
    Set_effect_byAreaType(targetArea, targetArmy->BasicAbilities->ID, attackValue_fromTargetArmy);
    //伤害为0，修正为1
    if (attackValue_fromStartArmy <= 0) {
        attackValue_fromStartArmy = 1;
    }
    if (attackValue_fromTargetArmy <= 0) {
        attackValue_fromTargetArmy = 1;
    }
    if(s_NewArmyAttribute){
        //计算闪避
        if (Is_dodged(startArmy, targetArmy)) {
            attackValue_fromStartArmy = 0;
        }
        if (Is_dodged(targetArmy, startArmy)) {
            attackValue_fromTargetArmy = 0;
        }
        int blow_ofStartArmy = Get_blow(startArmy, dices_startArmy);
        int blow_ofTargetArmy = Get_blow(targetArmy, dices_targetArmy);
        //如果打出的伤害小于5，对方不会受到组织度伤害
        if (attackValue_fromStartArmy < 5) {
            blow_ofStartArmy = 0;
        }
        if (attackValue_fromTargetArmy < 5) {
            blow_ofTargetArmy = 0;
        }
        //如果其中一方处于溃败状态 ，双方打出的伤害下降50%
        if (startArmy->Is_collapsed()|| targetArmy->Is_collapsed()) {
            attackValue_fromStartArmy /= 2;
            attackValue_fromTargetArmy /= 2;
            blow_ofTargetArmy /=2;
            blow_ofStartArmy/=2;
        }
        //如果完全没有能力击穿对方的装甲硬度，造成的组织度损失减少50%
        if (Can_breakTargetArmor(startArmy,targetArmy) == false) {
            blow_ofStartArmy /= 2;
        }
        if (Can_breakTargetArmor(targetArmy,startArmy) == false) {
            blow_ofTargetArmy /= 2;
        }
        //如果装甲硬度相差大于等于2倍。则高装甲度单位对低装甲度单位的进攻造成额外的1.4倍组织度伤害。
        if (startArmy->BasicAbilities->Armor >= targetArmy->BasicAbilities->Armor * 2) {
            blow_ofStartArmy *= 1.4;
        }
        //组织度结算
        startArmy->Organization -= blow_ofTargetArmy;
        targetArmy->Organization -= blow_ofStartArmy;
        if (startArmy->Organization < 0) {
            startArmy->Organization = 0;
        }
        if (targetArmy->Organization < 0) {
            targetArmy->Organization = 0;
        }
    }
    //伤害结算
    this->StartArmyReceiveDamage = attackValue_fromTargetArmy;
    this->TargetArmyReceiveDamage = attackValue_fromStartArmy;
    if (this->CanCounter) {
        this->StartArmyReceiveDamage = attackValue_fromTargetArmy;
    }
    else {
        this->StartArmyReceiveDamage = 0;
    }
    //如果第二次攻击发生了(要塞炮）
    if (this->SecondAttackIndex == 1) {
        //如果是进攻方打出的
        if (this->StartArmySecondAttack) {
            this->StartArmyReceiveDamage = 0;
        }
        else if (this->TargetArmySecondAttack) {
            this->TargetArmyReceiveDamage = this->StartArmySecondAttack;
        }
    }
    //空袭类型设置为0，表示这是普通攻击
    this->AirStrikeType = 0;
    //二次进攻的重设为0
    this->StartArmySecondAttack = false;
    this->TargetArmySecondAttack = false;
    //进攻方剩余的骰子数
    int dices = startArmy->GetNumDicesIfLostStrength(this->StartArmyReceiveDamage);
    //战斗后进攻方失去的骰子数
    this->StartArmyNumDicesLost = dices_startArmy - dices;
    dices = targetArmy->GetNumDicesIfLostStrength(this->TargetArmyReceiveDamage);
    this->TargetArmyNumDicesLost = dices_targetArmy - dices;

}

int Get_blow(CArmy *army, int dicesNum) {
    int minBlow = army->BasicAbilities->MinBlow;
    int maxBlow = army->BasicAbilities->MaxBlow;
    int total = 0;
    int r = 0;
    for (int i = 0; i < dicesNum; ++i) {
        r = GetRand(army) % (maxBlow - minBlow + 1);
        r += minBlow;
        total += r;
    }
    return total;
}

bool Is_dodged(CArmy *startArmy, CArmy *targetArmy) {
    //计算闪避
    if (targetArmy->BasicAbilities->Miss > 0) {
        int miss = targetArmy->BasicAbilities->Miss;
        int r = GetRand(startArmy) % 100 + 1;
        if (r <= miss) {
            return true;
        }
    }
    return false;
}
//设置反击
void SetCounter(CFight *fight, ArmyDef::ArmyType &startArmy_ID, ArmyDef::ArmyType &targetArmy_ID,
                CArmy *startArmy, CArmy *targetArmy, CArea *targetArea) {
    fight->CanCounter = true;
    //如果发动进攻的是大炮
    if (startArmy->Is_artillery()) {
        if (targetArmy->Is_tank()){
            if (targetArmy_ID == ArmyDef::HeavyTank_1918){
                fight->CanCounter = true;
            }
        }
        else if (targetArmy->Is_artillery()){
            fight->CanCounter = true;
        }
        else if ( targetArmy->IsNavy()) {
            fight->CanCounter = true;
        }
            //如果被进攻的地块工事有：要塞炮
        else if (targetArea->InstallationType == CArea::fort) {
            fight->CanCounter = true;
        }
        else{
            fight->CanCounter = false;
        }
    }
        //如果进攻者是潜艇
    else if (startArmy_ID == ArmyDef::Submarine) {
        fight->CanCounter = false;
        ecEffect *ecEffect1 = ecEffectManager::Instance()->AddEffect("effect_nuclearbomb.xml",
                                                                     true);
        easytech(_ZN9CSoundRes10PlayCharSEE10SND_EFFECT)(&g_SoundRes, g_SoundRes.aExp_wav);
    }
    else if (startArmy_ID == ArmyDef::IntercontinentalMissile) {
        fight->CanCounter = false;
        //每发射一次，消耗30滴血模拟弹药
        startArmy->Hp -= 30;
        //毒气
        std::vector<CArea *> listAreas;
        for (int i = 0; i < 3; ++i) {
            CArea *area = g_Scene.GetAdjacentArea(targetArea->ID, i);
            listAreas.push_back(area);
        }
        auto *pGas = new CWarGas("effect_gas.xml", targetArea, listAreas, 3000000);
        g_GasList.push_back(pGas);
        //下面是原有的导弹轰炸效果设计
//        if (targetArea->ArmyCount > 1){
//            //顶层之下士气下降
//            for (int i = 1; i < targetArea->ArmyCount; i++) {
//                targetArea->Army[i]->Morale =1;
//                targetArea->Army[i]->MoraleUpTurn = 4;
//            }
//            targetArea->ReduceConstructionLevel();
//            targetArea->InstallationType = CArea::NoInstallation;
//            }
        ecEffect *ecEffect1 = ecEffectManager::Instance()->AddEffect("effect_nuclearbomb.xml",
                                                                     true);
        CCSoundBox::GetInstance()->PlaySE("boom.wav");
    }
    else if(startArmy->Is_remoteAttack()){
        if (targetArmy->Is_remoteAttack()){
            //概率反击
            if (GetRand(startArmy)%100  < 50)
                fight->CanCounter = true;
        }
        fight->CanCounter = false;
    }
    else if(startArmy->Is_airForce()){
        if (startArmy->Is_fightPlane() && targetArmy->Is_fightPlane())
            fight->CanCounter = true;
        fight->CanCounter = false;
    }
        //进攻方是近战情况下
    else{
        if(targetArmy->Is_remoteAttack())
            fight->CanCounter = false;
    }

}

void
Set_effect_byAreaType(CArea *targetArea, ArmyDef::ArmyType &targetArmy_ID, int &targetAttackTotal) {

    if (targetArea->Type == forest) {
        if (targetArmy_ID == ArmyDef::InfantryD1905||targetArmy_ID == ArmyDef::Panzer||
            targetArmy_ID == ArmyDef::Tank) {
            targetAttackTotal *= 0.75;
        }
    }
    else if (targetArea->Type == hilly) {
        if (targetArmy_ID == ArmyDef::InfantryD1905||targetArmy_ID == ArmyDef::Panzer||
            targetArmy_ID == ArmyDef::Tank||targetArmy_ID == ArmyDef::Artillery||
            targetArmy_ID == ArmyDef::Rocket) {
            targetAttackTotal *= 0.70;
        }
    }
    else if (targetArea->Type == desert) {
        if (targetArmy_ID == ArmyDef::Artillery||targetArmy_ID == ArmyDef::Rocket) {
            targetAttackTotal *= 1.30;
        }
    }
}

bool Can_breakTargetArmor(CArmy* startArmy,CArmy* targetArmy){
    return (startArmy->BasicAbilities->MaxPiercing > targetArmy->BasicAbilities->Armor);
}
void Increase_damage_accordingToRestraintRelationship(CArmy *startArmy, CArmy *targetArmy, int &targetAttackTotal,
                                                      int &startAttackTotal) {
    //如果进攻方是装甲车
    if (startArmy->BasicAbilities->ID == ArmyDef::Panzer) {
        //如果防御方是步兵
        if (targetArmy->BasicAbilities->ID == ArmyDef::InfantryD1905) {
            //进攻方打出的伤害*1.4
            startAttackTotal *= 1.4;
        }
    }
    if (targetArmy->BasicAbilities->ID == ArmyDef::Panzer) {
        if (startArmy->BasicAbilities->ID == ArmyDef::InfantryD1905) {
            targetAttackTotal *= 1.4;
        }
    }
}

// 战斗结果
void CFight::ApplyResult() {

    if (this->StartAreaID < 0) {
        this->StartAreaID = 0;
    }
    if (this->TargetAreaID < 0) {
        return;
    }
    CArea* targetArea = g_Scene.GetArea(this->TargetAreaID);
    CArea* startArea = g_Scene.GetArea(this->StartAreaID);
    if (targetArea == nullptr || startArea == nullptr) {
        return;
    }
    int type = this->AirStrikeType;

    //如果空袭类型是0，则战斗为普通的陆地战斗
    if (this->AirStrikeType == 0) {
        CArmy* startArmy = startArea->GetArmy(0);
        CArmy* targetArmy = targetArea->GetArmy(0);
        //百分之10的概率 运输团被打掉
        if (startArmy->HasCard(CArmy::TransportationTroops)) {
            int r = GetRand(startArmy) % 10;
            if (r == 3)
                startArmy->RemoveCard(CArmy::TransportationTroops);
        }
        if (targetArmy->HasCard(CArmy::TransportationTroops)) {
            int r = GetRand(targetArmy) % 10;
            if (r == 3)
                targetArmy->RemoveCard(CArmy::TransportationTroops);
        }
        ArmyDef::ArmyType startArmy_ID = startArmy->BasicAbilities->ID;
        ArmyDef::ArmyType targetArmy_ID = targetArmy->BasicAbilities->ID;
        //判断发动攻击的军队是否死亡
        bool isLost_StartArmy = startArea->LostArmyStrength(0, this->StartArmyReceiveDamage);
        //判断防御的军队是否死亡
        bool isLost_TargetArmy = targetArea->LostArmyStrength(0, this->TargetArmyReceiveDamage);
        //得到它们的小骰子，如果低于等于3 则有20%的概率被打掉战术卡
        int startDices = startArmy->GetNumDices();
        int targetDices = targetArmy->GetNumDices();
        //如果发动攻击的军队死亡
        if (isLost_StartArmy) {
            //如果发动进攻的地块的军队变成0了
            if (startArea->ArmyCount == 0) {
                //重置其地块周围的包围情况
                g_Scene.AdjacentAreasEncirclement(StartAreaID);
            }
            targetArea->Country->AddDestroy(startArmy_ID);
        }
            //进攻方未死亡
        else {
            if (startDices <= 3) {
                startArmy->Lost_card(10);
            }
            //进攻方增加经验（经验为目标军队受到的实际伤害）
            startArmy->AddExp(this->TargetArmyReceiveDamage);
            //如果防御方未死亡
            if (isLost_TargetArmy == false) {
                if (targetDices <= 3) {
                    targetArmy->Lost_card(20);
                }
                //增加经验
                targetArmy->AddExp(this->StartArmyReceiveDamage);
                // 组织度到达阈值则溃败
                if (s_NewArmyAttribute && targetArmy->Is_collapsed()) {
                    //判断撤退单位是否撤退成功。如果没成功，直接歼灭之
                   if(Is_successful_colliapsedArmyRetreatSafeArea(targetArea, targetArmy, 1, 3, 0) == false){
                       targetArea->RemoveArmy(targetArmy);
                       delete targetArmy;

                       isLost_TargetArmy = true;
                   }
                }
            }
            //扣除一点移动力。
            startArmy->Movement -= 1;
            //如果目标地块是战壕
            if (targetArea->InstallationType == CArea::entrenchment) {
                //如果进攻的军队不是坦克或者重坦克，同时也不是海军
                if (startArmy_ID != ArmyDef::Tank && startArmy_ID != ArmyDef::HeavyTank &&
                    !startArmy->IsNavy()) {
                    startArmy->Movement = 0;
                }
            }
        }
        // 防御方死亡，
        if(isLost_TargetArmy){
            if (startArmy->Is_tank()){
                startArmy->Movement += 1;
            }
        }
        if (targetArea->ArmyCount == 0) {
            int r = GetRand() % 100;
            //50%概率打掉工事
            if (r <= 49) {
                targetArea->InstallationType = CArea::NoInstallation;
            }
            //开始的军队还活着才能占领
            if (startArmy != nullptr) {
                //如果进攻方不是 大炮或者不是火箭炮，同时双方的地块类型一致
                if ((startArmy->Is_airForce() ==false && startArmy->Is_artillery() == false && startArmy->Is_remoteAttack() == false) &&
                    targetArea->Sea == startArea->Sea) {
                    if (startArmy->Is_tank())
                        startArmy->Movement -= 1;
                    startArea->OccupyArea(targetArea);
                    targetArea->ReduceConstructionLevel();
                }
                r = GetRand() % 100;
                //30%的概率进攻方出现士气上涨
                if (r <= 29) {
                    startArmy->Breakthrough();
                }
            }
        }
        //如果是战列舰，目标地块有30%的概率让目标建筑降级
        if (startArmy_ID == ArmyDef::Battleship) {
            int r = GetRand() % 100;
            if (r <= 29) {
                targetArea->ReduceConstructionLevel();
            }
        }

        //如果攻击方是海军，目标地块的守军能打出双倍伤害
        if (this->SecondAttackIndex == 0 && !isLost_TargetArmy && !isLost_StartArmy && targetArea->InstallationType ==CArea::fort ) {
            if (startArmy->IsNavy()) {
                this->TargetArmySecondAttack = true;
            }
        }
    }
    else if (this->AirStrikeType == 1) {

        ArmyDef* armyDef = targetArea->Army[0]->BasicAbilities;

        if (targetArea->LostArmyStrength(0, this->TargetArmyReceiveDamage) == true) {
            if (targetArea->ArmyCount == 0) {
                g_Scene.AdjacentAreasEncirclement(TargetAreaID);
            }
            this->AirStrikeCountry->AddDestroy(armyDef->ID);
        }
        else {
            //百分之10的概率 运输团被打掉
            if (targetArea->Army[0]->HasCard(CArmy::TransportationTroops)) {
                int r = GetRand(targetArea->Army[0]) % 10;
                if (r == 3)
                    targetArea->Army[0]->RemoveCard(CArmy::TransportationTroops);
            }
        }

    }
        //轰炸和核弹
    else if (this->AirStrikeType == 2 || this->AirStrikeType == 3) {
        //轰炸
        if (this->AirStrikeType == 2) {
            for (int i = targetArea->ArmyCount - 1; i >= 0; i--) {
                int airDamage = this->TargetArmyReceiveDamage;
                //轰炸减伤：每层减20%
                int reduceDamage = airDamage * i / -5;
                ArmyDef* armyDef = targetArea->Army[i]->BasicAbilities;
                bool isDead = targetArea->LostArmyStrength(i, airDamage + reduceDamage);
                if (isDead == true) {
                    this->AirStrikeCountry->AddDestroy(armyDef->ID);
                }
                else {
                    //百分之10的概率 运输团被打掉
                    if (targetArea->Army[i]->HasCard(CArmy::TransportationTroops)) {
                        int r = GetRand(targetArea->Army[i]) % 10;
                        if (r == 3)
                            targetArea->Army[i]->RemoveCard(CArmy::TransportationTroops);
                    }
                }
            }
            if (targetArea->ArmyCount == 0) {
                g_Scene.AdjacentAreasEncirclement(this->TargetAreaID);
            }
            //防空机枪有防降级的作用
            if (targetArea->InstallationType != CArea::antiaircraft) {
                targetArea->ReduceConstructionLevel();
            }
        }
            //核弹
        else if (this->AirStrikeType == 3) {
            //消灭所有的军队
            for (int i = 0; i < targetArea->ArmyCount; i++) {
                ArmyDef* armyDef = targetArea->Army[i]->BasicAbilities;
                this->AirStrikeCountry->AddDestroy(armyDef->ID);
            }
            targetArea->DestroyConstruction();
            targetArea->ClearAllArmy();
            targetArea->InstallationType = CArea::NoInstallation;
            g_Scene.AdjacentAreasEncirclement(targetArea->ID);
        }
    }
        //航母
    else if (this->AirStrikeType == 4) {
        CArmy* starArmy = startArea->Army[0];
        starArmy->AddExp(this->TargetArmyReceiveDamage);
        --starArmy->Movement;
        ArmyDef* armyDef = targetArea->Army[0]->BasicAbilities;
        bool isDead = targetArea->LostArmyStrength(0, this->TargetArmyReceiveDamage);
        if (isDead == true) {
            this->AirStrikeCountry->AddDestroy(armyDef->ID);
            g_Scene.AdjacentAreasEncirclement(TargetAreaID);
        }
    }
    if (startArea->Country != nullptr) {
        if (startArea->Country->AI == false) {
            CActionAI* ActionAI = CActionAI::Instance();
            //获得勋章
            bool isGetMedal = ActionAI->getMedal(this->TargetArmyReceiveDamage, targetArea);
            if (isGetMedal) {
                if (startArea->Country->IsLocalPlayer()) {
                    ++g_Commander.Medal;
                    ++g_GameManager.RandomRewardMedal;
                }
                g_Scene.GainMedal(startArea->ArmyPos[0], startArea->ArmyPos[1]);
                easytech(_ZN9CSoundRes10PlayCharSEE10SND_EFFECT)(&g_SoundRes,
                                                                 g_SoundRes.aLvup_wav);
            }
        }
    }

    DrawImage_FightResult(this);

}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 检测游戏升级。<p>
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * void <p>
     *=======================================================<p>
     * 【创建人】： 馅饼 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： 2022 <p>
     * 【修改时间】： ？？？
     */
bool CGameManager::CheckAndSetResult() {
    if (this->CurrentTurnNumMinusOne >= this->VictoryTurn){
        this->Set_gameGivenResult(false);
        return true;
    }
//    这个似乎看上去并没有用，判断超时的函数在CGameManager::EndTurn

    CCountry *player_country = this->GetPlayerCountry();
    if (player_country->IsConquested()){
        this->Set_gameGivenResult(false);
        return true;
    }
    for (auto country_here : this->ListCountry){
        if (!country_here->IsConquested()){
            if (country_here->Alliance != CCountry::NeutralID &&
                country_here->Alliance != player_country->Alliance){
                return false;
            }
        }
    }
    this->Set_gameGivenResult(true);
    return true;
}
void CGameRes::RenderConstruction(int Type, int Level, float x, float y) {
    switch (Type) {
        case CArea::city: {
            if (Level > 0)
                Image_buildmark_city[Level - 1]->Render(x, y);
        }
            break;
        case CArea::industry: {
            if (Level > 0)
                Image_buildmark_factory[Level - 1]->Render(x, y);
        }
            break;
        case CArea::airport: {
            Image_buildmark_airport->Render(x, y);
        }
            break;
        case CArea::oilwell: {
            Image_buildmark_oilwell->Render(x, y);
        }
    }
}

struct CGameRes g_GameRes{};

ecImage::ecImage() {}

//设置各种场景下的兵模
ecImage *CGameRes::GetArmyImage(const char *CountryName, int ArmyType, bool Sea) {
    ecImage *ArmyImage;
    if (Sea&&(ArmyType < ArmyDef::Destroyer||ArmyType > ArmyDef::AircraftCarrier)&&
        ArmyType != ArmyDef::Submarine) {
        ArmyType = ArmyDef::Carrier;
        ArmyImage = map_countryArmyImages["ne"][ArmyType];
    }
    else {
        //如果是新兵模
        if (ArmyType >= 28) {
            if (ArmyType == 32){
                ArmyImage = map_countryArmyImages[CountryName][1];
            }
            else{
                ArmyImage = map_countryArmyImages[CountryName][ArmyType - 13];
            }
            if (ArmyImage == nullptr)
                ArmyImage = map_countryArmyImages["ne"][ArmyType - 13];
        }
        else {
            ArmyImage = map_countryArmyImages[CountryName][ArmyType];
            if (ArmyImage == nullptr)
                ArmyImage = map_countryArmyImages["ne"][ArmyType];
        }
    }

    return ArmyImage;
}

ecImage *ecTextureRes::GetImageByFormat(const char *Format, ...) {
    static char imgFileName[128];
    ecImage *img = nullptr;
    va_list ap;
    va_start(ap, Format);
    vsprintf(imgFileName, Format, ap);
    va_end(ap);
    ecImageAttr *imgAttr = GetImage(imgFileName);
    if (imgAttr != nullptr)
        img = new ecImage(imgAttr);
    return img;
}

int CCommander::GetNumPlayedBatttles(int CampaignSeriesID) {
    return sizeof(CampaignStar[0]) / sizeof(int);
}

/*
 * 这个地块的军队占领对方地块 扣除移动力需在这个函数外部扣除
 * 调用量 RemoveArmy 和 SetMoveInArmy 函数
 */
void CArea::OccupyArea(CArea *targetArea) {
    if (targetArea == nullptr)
        return;
    CCountry *targetCountry = targetArea->Country;
    CCountry *country = this->Country;

    //占领地块的军队

    bool isOccupy = false;
    if (this->ArmyCount > 0) {
        CArmy *theArmy = this->Army[0];
        if (theArmy->BasicAbilities->ID == 29) {
            return;
        }
        if (this->Country == targetCountry) {
            isOccupy = false;
        }
        else {
            if (targetCountry != nullptr) {
                targetArea->Country->RemoveArea(targetArea->ID);
                this->Country->AddArea(targetArea->ID);
                targetArea->Country = this->Country;
            }
            if (targetCountry != nullptr&&targetCountry->IsConquested() == true) {
                targetCountry->BeConquestedBy(this->Country);
            }
            isOccupy = true;
        }
        this->RemoveArmy(theArmy);
        targetArea->SetMoveInArmy(this, theArmy, isOccupy, false, nullptr);
        if (this->ArmyCount == 0) {
            g_Scene.AdjacentAreasEncirclement(this->ID);
        }

    }
}


/*
 *减少建筑等级
 */
void CArea::ReduceConstructionLevel() {
    if (this->ConstructionType != NoConstruction) {
        this->ConstructionLevel -= 1;
        if (this->ConstructionLevel == 0) {
            this->ConstructionType = NoConstruction;
            return;
        }
        switch (this->Type) {
            case capital:
                //直接清空
                if (this->ConstructionType == city &&this->ConstructionLevel == 3) {
                    this->ConstructionLevel = 0;
                    this->ConstructionType = NoConstruction;
                }
                else  if (this->ConstructionType == industry &&this->ConstructionLevel == 2){
                    this->ConstructionLevel = 0;
                    this->ConstructionType = NoConstruction;
                }
            case large_city:
                if (this->ConstructionType == city&&this->ConstructionLevel == 2) {
                    this->ConstructionLevel = 0;
                    this->ConstructionType = NoConstruction;
                }
                else if (this->ConstructionType == industry &&this->ConstructionLevel == 1){
                    this->ConstructionLevel = 0;
                    this->ConstructionType = NoConstruction;
                }
            case normal_city:
                if (this->ConstructionType == city&&this->ConstructionLevel == 1) {
                    this->ConstructionLevel = 0;
                    this->ConstructionType = NoConstruction;
                }
        }
    }
}


def_easytech(_ZN8ecEffect6FireAtEfff);

//绘制战斗结果的图像和播放声音
void DrawImage_FightResult(CFight *fight) {
    CArea *startArea = g_Scene.GetArea(fight->StartAreaID);
    CArea *targetArea = g_Scene.GetArea(fight->TargetAreaID);
    //如果有两次交火（要塞炮）
    if (fight->SecondAttackIndex == 1) {

        if (startArea != nullptr&&fight->StartArmyReceiveDamageFirst > 0) {
            ecEffect *ecEffect1 = ecEffectManager::Instance()->AddEffect("effect_exp.xml", true);
            ecEffect1->FireAt(startArea->ArmyPos[0], startArea->ArmyPos[1], 0.0);
            char str[12];
            //格式化
            sprintf(str, "-%d", fight->StartArmyReceiveDamageFirst);
            g_FightTextMgr.AddText(startArea->ArmyPos[0], startArea->ArmyPos[1] - 50.0, str,
                                   0xFF0000FF);
        }
        if (fight->TargetArmyReceiveDamageFirst > 0) {
            ecEffect *ecEffect1 = ecEffectManager::Instance()->AddEffect("effect_exp.xml", true);
            easytech_ZN8ecEffect6FireAtEfff(ecEffect1, targetArea->ArmyPos[0],
                                            targetArea->ArmyPos[1], 0.0);

            ecEffect1->FireAt(targetArea->ArmyPos[0], targetArea->ArmyPos[1], 0.0);

            char str[12];
            //格式化
            sprintf(str, "-%d", fight->TargetArmyReceiveDamageFirst);
            g_FightTextMgr.AddText(targetArea->ArmyPos[0], targetArea->ArmyPos[1] - 30.0, str,
                                   0xFF0000FF);
        }
    }
    if (fight->TargetArmySecondAttack == false&&fight->StartArmySecondAttack == false) {
        if (startArea != nullptr&&fight->StartArmyReceiveDamage > 0) {
            ecEffect *ecEffect1 = ecEffectManager::Instance()->AddEffect("effect_exp.xml", true);

            ecEffect1->FireAt(startArea->ArmyPos[0], startArea->ArmyPos[1], 0.0);

            char str[12];
            //格式化
            sprintf(str, "-%d", fight->StartArmyReceiveDamage);
            g_FightTextMgr.AddText(startArea->ArmyPos[0], startArea->ArmyPos[1] - 30.0, str,
                                   0xFF0000FF);
        }
        if (fight->TargetArmyReceiveDamage > 0) {
            ecEffect *ecEffect1 = ecEffectManager::Instance()->AddEffect("effect_exp.xml", true);

            ecEffect1->FireAt(targetArea->ArmyPos[0], targetArea->ArmyPos[1], 0.0);

            char str[12];
            //格式化
            sprintf(str, "-%d", fight->TargetArmyReceiveDamage);
            g_FightTextMgr.AddText(targetArea->ArmyPos[0], targetArea->ArmyPos[1] - 30.0, str,
                                   0xFF0000FF);
        }
        if (fight->AirStrikeType == 3) {
            ecEffect *ecEffect1 = ecEffectManager::Instance()->AddEffect("effect_nuclearbomb.xml",
                                                                         true);
            ecEffect1->FireAt(targetArea->ArmyPos[0], targetArea->ArmyPos[1], 0.0);


        }

        easytech(_ZN9CSoundRes10PlayCharSEE10SND_EFFECT)(&g_SoundRes, g_SoundRes.aExp_wav);
    }

}


/*
 * 得到勋章的概率
 */
bool CActionAI::getMedal(int damage, CArea *TargetArea) {
    int r = GetRand() % 100;
    enum AreaType areaType = TargetArea->Type;
    if (TargetArea != nullptr) {
        switch (areaType) {
            case capital:
                r += 12;
                break;
            case large_city:
                r += 8;
                break;
            case normal_city:
                r += 4;
                break;
        }
    }
    if (damage <= 24) {
        if (r >= 95) {
            return true;
        }
    }
    else if (damage > 24&&damage <= 29) {
        if (r >= 91) {
            return true;
        }
    }
    else if (damage > 29&&damage <= 34) {
        if (r >= 87) {
            return true;
        }
    }
    else if (damage > 34) {
        if (r >= 82) {
            return true;
        }
    }
    return false;
}

/*
 * 返回这个失去血量的军队的血量是否为0 同时该军队的当前血量为减去伤害后的血量
 */
bool CArmy::LostStrength(int damage) {
    if (this->Hp - damage < 0) {
        this->Hp = 0;
    }
    else {
        this->Hp = this->Hp - damage;
    }
    return this->Hp <= 0;
}

/*
 * 军队失去血量 函数包含了军队丢失血量（LostStrength）和死亡后的一系列处理。包括delete对象<p>
 * 如果军队死亡，返回true，否则false
 */
bool CArea::LostArmyStrength(int ArmyIndex, int damage) {
    //如果这个下标小于该地块的军队数量 并且 这个下标对应的军队不为空 并且 这个军队死亡
    if (ArmyIndex < this->ArmyCount&&this->Army[ArmyIndex] != nullptr&&
        this->Army[ArmyIndex]->LostStrength(damage) == true) {
        CArmy *theArmy = this->Army[ArmyIndex];
        //如果这个军队是指挥官
        if (theArmy->HasCard(CArmy::Commander)) {
            //指挥官死亡
            this->Country->CommanderDie();
        }
        //移除这个地块的军队
        this->RemoveArmy(theArmy);
        delete theArmy;
        if (this->Sea == true&&this->ArmyCount == 0) {
            if (this->Country != nullptr) {
                bool isConquested = this->Country->IsConquested();
                if (isConquested == true) {
                    this->Country->BeConquestedBy(0);
                }
            }
        }
    }
    else {
        return false;
    }
    return true;
}


void LoadMusic() {

    CCSoundBox::GetInstance()->LoadSE("boom.wav");
}

//读取事件
//void LoadEventXml(int x){
//    //创建一个xml
//    TiXmlDocument xml;
//    //尝试读取eventFileName
//    switch (x) {
//        case 0:
//            _ZN13TiXmlDocumentC2EPKc(&xml, GetPath("conquest1Event.xml", nullptr));
//            break;
//    }
//    //如果读取xml成功
//    if (xml.LoadFile()){
//        TiXmlElement *xRoot = static_cast<TiXmlElement *>(xml.FirstChild("Root"));
//        //如果读取Root节点成功
//        if (xRoot != nullptr) {
//            //循环xRoot下的所有节点   xList
//            for (TiXmlElement *xList = xRoot->FirstChildElement();
//                 xList != NULL; xList = xList->NextSiblingElement()){
//                const char *name = xList->Attribute("name");
//                //如果是events节点
//                if (strcmp(name,"events")==0){
//                    //循环events节点下的所有国家   xCountry
//                    for (TiXmlElement *xCountry = xList->FirstChildElement();
//                         xCountry != NULL; xCountry = xCountry->NextSiblingElement()){
//                        const char* countryName = xCountry->Attribute("name");
//                        if (countryName != nullptr){
//                            CCountry* country = g_GameManager.FindCountry(countryName);
//                            if ( country!= nullptr){
//                                //循环 每个国家下的事件（listEvent,turnEvents）  xCountryEvents
//                                for (TiXmlElement *xCountryEvents = xCountry->FirstChildElement();
//                                     xCountryEvents != NULL; xCountryEvents = xCountryEvents->NextSiblingElement()){
//                                    const char* theEventType = xCountryEvents->Attribute("choosedBtnNum");
//                                    int money,industry,oil,targetAreaID,armies,areas,startRound,endRound,eachTurnPr,choosedBtnNum;
//                                   const char* targetCountryName[16];
//                                    const char* theCountryName[16];
//                                    const char* tile[50];
//                                    const char* news[1024];
//                                    //如果是地块事件
//                                    if (strcmp(theEventType,"areaEvents")==0){
//                                        //循环所有的地块事件
//                                        for (TiXmlElement *xAreaEvents = xCountryEvents->FirstChildElement();
//                                             xAreaEvents != NULL; xAreaEvents = xAreaEvents->NextSiblingElement()){
//
//                                            //创建事件对象
//                                            EventOccupyArea* eventOccupyArea = new EventOccupyArea();
//                                            const char* titleName = xAreaEvents->Attribute("titleName");
//                                            //给标题名赋值
//                                                strcpy(eventOccupyArea->title , titleName);
//                                                //循环内容和事件其它内容   xNewsAndDetail
//                                                for (TiXmlElement *xNewsAndDetail = xAreaEvents->FirstChildElement();
//                                                     xNewsAndDetail != NULL; xNewsAndDetail = xNewsAndDetail->NextSiblingElement()) {
//                                                    const char *name = xNewsAndDetail->Attribute("name");
//                                                    //如果 是news
//                                                    if (strcmp(name,"news") == 0){
//                                                        //获取事件内容
//                                                       strcpy(eventOccupyArea->news,xNewsAndDetail->Attribute("news"));
//                                                    }
//                                                    else if(strcmp(name,"detail") == 0){
//                                                        if (xNewsAndDetail->QueryIntAttribute("targetAreaID",&targetAreaID) != TIXML_SUCCESS){
//                                                            targetAreaID = 0;
//                                                        }
//
//                                                        strcpy(eventOccupyArea->targetCountryID, xNewsAndDetail->Attribute("targetCountryID"));
//                                                        if (xNewsAndDetail->QueryIntAttribute("money",&money) != TIXML_SUCCESS){
//                                                            money = 0;
//                                                        }
//                                                        strcpy(eventOccupyArea->theCountryID, xNewsAndDetail->Attribute("theCountryID"));
//
//                                                        if (xNewsAndDetail->QueryIntAttribute("industry",&industry) != TIXML_SUCCESS){
//                                                            industry = 0;
//                                                        }
//                                                        if (xNewsAndDetail->QueryIntAttribute("oil",&oil) != TIXML_SUCCESS){
//                                                            oil = 0;
//                                                        }
//                                                        if (xNewsAndDetail->QueryIntAttribute("startRound",&startRound) != TIXML_SUCCESS){
//                                                            startRound = 0;
//                                                        }
//                                                        if (xNewsAndDetail->QueryIntAttribute("endRound",&endRound) != TIXML_SUCCESS){
//                                                            endRound = 0;
//                                                        }
//                                                        if (xNewsAndDetail->QueryIntAttribute("armies",&armies) != TIXML_SUCCESS){
//                                                            armies = 0;
//                                                        }
//                                                        if (xNewsAndDetail->QueryIntAttribute("areas",&areas) != TIXML_SUCCESS){
//                                                            areas = 0;
//                                                        }
//
//
//
//                                                        if (xNewsAndDetail->QueryIntAttribute("eachTurnPr",&eachTurnPr) != TIXML_SUCCESS){
//                                                            eachTurnPr = 0;
//                                                        }
//                                                        if (xNewsAndDetail->QueryIntAttribute("choosedBtnNum",&choosedBtnNum) != TIXML_SUCCESS){
//                                                            choosedBtnNum = 0;
//                                                        }
//
//                                                        eventOccupyArea->money = money;
//                                                        eventOccupyArea->industry = industry;
//                                                        eventOccupyArea->oil = oil;
//                                                        eventOccupyArea->startRound = startRound;
//                                                        eventOccupyArea->endRound = endRound;
//                                                        eventOccupyArea->areas = areas;
//                                                        eventOccupyArea->armies = armies;
//                                                        eventOccupyArea->isTrue = false;
//                                                        eventOccupyArea->targetAreaID = targetAreaID;
//                                                        eventOccupyArea->eachTurnPr = eachTurnPr;
//                                                        //如果是事件类型为0
//                                                        if (eventOccupyArea->choosedBtnNum == 0) {
//                                                            // xListAreas
//                                                            for (TiXmlElement *xListAreas = xNewsAndDetail->FirstChildElement();
//                                                                 xListAreas !=NULL; xListAreas = xListAreas->NextSiblingElement()) {
//                                                                const char *newCountryID;
//                                                                const char *newCountryName;
//
//                                                                int moneyForTriggerEvent, industryForTriggerEvent, oilForTriggerEvent;
//                                                                newCountryID = xListAreas->Attribute("newCountryID");
//                                                                newCountryName = xNewsAndDetail->FirstChildElement()->Attribute("newCountryName");
//
//                                                                if (xListAreas->QueryIntAttribute("moneyForTriggerEvent", &moneyForTriggerEvent) !=TIXML_SUCCESS) {
//                                                                    moneyForTriggerEvent = 0;
//                                                                }
//                                                                if (xListAreas->QueryIntAttribute("industryForTriggerEvent",&industryForTriggerEvent) !=TIXML_SUCCESS) {
//                                                                    industryForTriggerEvent = 0;
//                                                                }
//                                                                if (xListAreas->QueryIntAttribute("oilForTriggerEvent", &oilForTriggerEvent) != TIXML_SUCCESS) {
//                                                                    oilForTriggerEvent = 0;
//                                                                }
//                                                                strcpy(eventOccupyArea->newCountryName,
//                                                                       newCountryName);
//                                                                strcpy(eventOccupyArea->newCountryID,
//                                                                       newCountryID);
//                                                                eventOccupyArea->moneyForTriggerEvent = moneyForTriggerEvent;
//                                                                eventOccupyArea->industryForTriggerEvent = industryForTriggerEvent;
//                                                                eventOccupyArea->oilForTriggerEvent = oilForTriggerEvent;
//                                                                //循环被吞并的地块 xOccupyAreas
//                                                                for (TiXmlElement *xOccupyAreas = xListAreas->FirstChildElement();
//                                                                     xOccupyAreas !=
//                                                                     NULL; xOccupyAreas = xOccupyAreas->NextSiblingElement()) {
//                                                                    int id, armyType;
//                                                                    if (xOccupyAreas->QueryIntAttribute(
//                                                                            "id", &id) !=
//                                                                        TIXML_SUCCESS) {
//                                                                        id = 0;
//                                                                    }
//                                                                    if (xOccupyAreas->QueryIntAttribute(
//                                                                            "armyType",
//                                                                            &armyType) !=
//                                                                        TIXML_SUCCESS) {
//                                                                        armyType = -1;
//                                                                    }
//                                                                    eventOccupyArea->listAreas.push_back(
//                                                                            id);
//                                                                    eventOccupyArea->listArmyTypes.push_back(
//                                                                            armyType);
//                                                                }
//                                                            }
//                                                        }
//                                                }
//
//                                            }
//                                            country->List_event_moveArmyTo.push_back(*eventOccupyArea);  //暂时移除
//                                        }
//                                    }
//                                    else if(strcmp(theEventType,"turnEvent")==0){
//                                        //循环所有的地块事件
//                                        for (TiXmlElement *xAreaEvents = xCountryEvents->FirstChildElement();
//                                             xAreaEvents != NULL; xAreaEvents = xAreaEvents->NextSiblingElement()){
//
//                                            //创建事件对象
//                                            EventTurnBegin* eventTurnBegin = new EventTurnBegin();
//                                            const char* titleName = xAreaEvents->Attribute("titleName");
//                                            //给标题名赋值
//                                            strcpy(eventTurnBegin->title , titleName);
//                                            //循环内容和事件其它内容   xNewsAndDetail
//                                            for (TiXmlElement *xNewsAndDetail = xAreaEvents->FirstChildElement();
//                                                 xNewsAndDetail != NULL; xNewsAndDetail = xNewsAndDetail->NextSiblingElement()) {
//                                                const char *name = xNewsAndDetail->Attribute("name");
//                                                //如果 是news
//                                                if (strcmp(name,"news") == 0){
//                                                    //获取事件内容
//                                                    strcpy(eventTurnBegin->news,xNewsAndDetail->Attribute("news"));
//                                                }
//                                                else if(strcmp(name,"detail")){
//                                                    if (xNewsAndDetail->QueryIntAttribute("money",&money) != TIXML_SUCCESS){
//                                                        money = 0;
//                                                    }
//                                                    if (xNewsAndDetail->QueryIntAttribute("industry",&industry) != TIXML_SUCCESS){
//                                                        industry = 0;
//                                                    }
//                                                    if (xNewsAndDetail->QueryIntAttribute("oil",&oil) != TIXML_SUCCESS){
//                                                        oil = 0;
//                                                    }
//                                                    if (xNewsAndDetail->QueryIntAttribute("areas",&areas) != TIXML_SUCCESS){
//                                                        areas = 0;
//                                                    }
//                                                    if (xNewsAndDetail->QueryIntAttribute("armies",&armies) != TIXML_SUCCESS){
//                                                        armies = 0;
//                                                    }
//                                                    if (xNewsAndDetail->QueryIntAttribute("targetAreaID",&targetAreaID) != TIXML_SUCCESS){
//                                                        targetAreaID = 0;
//                                                    }
//                                                    if (xNewsAndDetail->QueryIntAttribute("startRound",&startRound) != TIXML_SUCCESS){
//                                                        startRound = 0;
//                                                    }
//                                                    if (xNewsAndDetail->QueryIntAttribute("endRound",&endRound) != TIXML_SUCCESS){
//                                                        endRound = 0;
//                                                    }
//                                                    if (xNewsAndDetail->QueryIntAttribute("eachTurnPr",&eachTurnPr) != TIXML_SUCCESS){
//                                                        eachTurnPr = 0;
//                                                    }
//                                                    if (xNewsAndDetail->QueryIntAttribute("choosedBtnNum",&choosedBtnNum) != TIXML_SUCCESS){
//                                                        choosedBtnNum = 0;
//                                                    }
//                                                    *theCountryName = xNewsAndDetail->Attribute("theCountryID");
//                                                    *targetCountryName =  xNewsAndDetail->Attribute("targetCountryID");
//                                                    eventTurnBegin->money = money;
//                                                    eventTurnBegin->industry = industry;
//                                                    eventTurnBegin->oil = oil;
//                                                    eventTurnBegin->startRound = startRound;
//                                                    eventTurnBegin->endRound = endRound;
//                                                    eventTurnBegin->areas = areas;
//                                                    eventTurnBegin->armies = armies;
//                                                    eventTurnBegin->isTrue = false;
//                                                    eventTurnBegin->targetAreaID = targetAreaID;
//                                                    eventTurnBegin->eachTurnPr = eachTurnPr;
//                                                    strcpy(eventTurnBegin->targetCountryName,*targetCountryName);
//                                                    strcpy(eventTurnBegin->theCountryName,*theCountryName);
//                                                    //如果还能往下找
//                                                    if (xNewsAndDetail->FirstChildElement()!=nullptr){
//                                                        const char* newCountryID;
//                                                        const char* newCountryName;
//
//                                                        int moneyForTriggerEvent,industryForTriggerEvent,oilForTriggerEvent;
//                                                        newCountryID = xNewsAndDetail->FirstChildElement()->Attribute("newCountryID");
//                                                        newCountryName = xNewsAndDetail->FirstChildElement()->Attribute("newCountryName");
//
//                                                        if (xNewsAndDetail->FirstChildElement()->QueryIntAttribute("moneyForTriggerEvent",&moneyForTriggerEvent) != TIXML_SUCCESS){
//                                                            moneyForTriggerEvent = 0;
//                                                        }
//                                                        if (xNewsAndDetail->FirstChildElement()->QueryIntAttribute("industryForTriggerEvent",&industryForTriggerEvent) != TIXML_SUCCESS){
//                                                            industryForTriggerEvent = 0;
//                                                        }
//                                                        if (xNewsAndDetail->FirstChildElement()->QueryIntAttribute("oilForTriggerEvent",&oilForTriggerEvent) != TIXML_SUCCESS){
//                                                            oilForTriggerEvent = 0;
//                                                        }
//                                                        strcpy(eventTurnBegin->newCountryName,newCountryName);
//                                                        strcpy(eventTurnBegin->newCountryID,newCountryID);
//                                                        eventTurnBegin->moneyForTriggerEvent = moneyForTriggerEvent;
//                                                        eventTurnBegin->industryForTriggerEvent = industryForTriggerEvent;
//                                                        eventTurnBegin->oilForTriggerEvent = oilForTriggerEvent;
//                                                        //循环被吞并的地块 xOccupyAreas
//                                                        for (TiXmlElement *xOccupyAreas = xNewsAndDetail->FirstChildElement();
//                                                             xOccupyAreas != NULL; xOccupyAreas = xOccupyAreas->NextSiblingElement()){
//                                                            int id,armyType;
//                                                            if (xOccupyAreas->QueryIntAttribute("id",&id) != TIXML_SUCCESS){
//                                                                id = 0;
//                                                            }
//                                                            if (xOccupyAreas->QueryIntAttribute("armyType",&armyType) != TIXML_SUCCESS){
//                                                                armyType = -1;
//                                                            }
//                                                            eventTurnBegin->listAreas.push_back(id);
//                                                            eventTurnBegin->listArmyTypes.push_back(armyType);
//                                                        }
//                                                    }
//                                                }
//
//                                            }
//                                        }
//                                    }
//                                }
//                            }
//                        }
//                    }
//                }
//            }
//        }
//
//    }
//
//}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 进入剧本的时候读取相关资源。<p>
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * void <p>
     *=======================================================<p>
     * 【创建人】： 柒安 <p>
     * 【修改人】： 李德邻 <p>
     * 【创建时间】： ？？？<p>
     * 【修改时间】： ？？？
     */
void CGameManager::LoadBattle(const char *FileName) {
    ClearBattle();

    //创建一个xml
    TiXmlDocument Xml;
    //读取一个xml
    _ZN13TiXmlDocumentC2EPKc(&Xml, GetPath(FileName, nullptr));
    //如果xml读取成功
    if (Xml.LoadFile()) {
        //读取battle节点
        TiXmlElement *BattleElement = static_cast<TiXmlElement *>(Xml.FirstChild("battle"));
        //如果battle节点不为空
        if (BattleElement != nullptr) {
            //如果battle的属性map未找到
            if (BattleElement->QueryIntAttribute("map", &MapID) != TIXML_SUCCESS) {
                //mapID设置为1
                MapID = 1;
            }
            memset(AreasEnable, 0, sizeof(AreasEnable));
            const char *Enable = BattleElement->Attribute("areasenable");
            //如果获取了 areasenable节点
            if (Enable != nullptr) {
                strcpy(AreasEnable, Enable);
            }
            //初始化  g_Scene
            g_Scene.Init(AreasEnable, MapID);
            //循环所有的节点
            for (TiXmlElement *ListElement = BattleElement->FirstChildElement();
                 ListElement != NULL; ListElement = ListElement->NextSiblingElement()) {
                const char *Name = ListElement->Attribute("name");
                if (!strcmp(Name, "country")) {
                    for (TiXmlElement *CountryElement = ListElement->FirstChildElement();
                         CountryElement !=
                         NULL; CountryElement = CountryElement->NextSiblingElement()) {
                        CCountry *country = new CCountry();
                        const char *AllianceStr, *DefeatedStr;
                        int AI, Money, Industry, Oil, TechLevel, Defeated, Alliance, R, G, B, A;   // 新增石油

                        double TaxFactor;
                        country->Init(CountryElement->Attribute("id"),
                                      CountryElement->Attribute("name"));
                        AllianceStr = CountryElement->Attribute("alliance");
                        Alliance = atoi(AllianceStr);
//                         Alliance = (AllianceStr != NULL && AllianceStr[0] != 'n')? AllianceStr[0] - 'a' + 1 : 4;
                        DefeatedStr = CountryElement->Attribute("defeated");
                        Defeated = DefeatedStr != nullptr ? !strcmp(DefeatedStr, "army") : 0;
                        if (CountryElement->QueryDoubleAttribute("taxfactor", &TaxFactor) !=
                            TIXML_SUCCESS)
                            TaxFactor = 1.0;
                        if (CountryElement->QueryIntAttribute("ai", &AI) != TIXML_SUCCESS)
                            AI = 1;
                        if (CountryElement->QueryIntAttribute("money", &Money) != TIXML_SUCCESS)
                            Money = 0;
                        if (CountryElement->QueryIntAttribute("oil", &Oil) != TIXML_SUCCESS)   // 新增
                            Oil = 0;   // 新增
                        if (CountryElement->QueryIntAttribute("industry", &Industry) !=
                            TIXML_SUCCESS)
                            Industry = 0;
                        if (CountryElement->QueryIntAttribute("techlevel", &TechLevel) !=
                            TIXML_SUCCESS)
                            TechLevel = 1;
                        if (CountryElement->QueryIntAttribute("a", &A) != TIXML_SUCCESS)
                            A = 255;
                        if (CountryElement->QueryIntAttribute("r", &R) != TIXML_SUCCESS)
                            R = 255;
                        if (CountryElement->QueryIntAttribute("g", &G) != TIXML_SUCCESS)
                            G = 255;
                        if (CountryElement->QueryIntAttribute("b", &B) != TIXML_SUCCESS)
                            B = 255;

                        country->Alliance = Alliance;
                        country->DefeatType = Defeated;
                        country->TaxRate = (float) TaxFactor;
                        country->AI = AI;
                        country->Money = Money;
                        country->Industry = Industry;
                        country->Oil = Oil;   // 新增
                        country->Tech = TechLevel;
                        country->A = A;
                        country->R = R;
                        country->G = G;
                        country->B = B;
                        if(strcmp( country->ID,"am")==0)
                            country->IsCoastal = true;
                        else
                            country->IsCoastal = false;
                        InitArrArmyDesignations(country);
                        country->MaxArmyDesignationsLand = 1;
                        country->MaxArmyDesignationsSea = 1;
                        country->MaxArmyDesignationsAir = 1;
                        country->MaxArmyDesignationsTank = 1;
                        if (GameMode == Conquest)
                            country->AI = strcmp(country->ID, ConquestPlayerCountryID);
                        country->SetCommander(CountryElement->Attribute("commander"));
                        ListCountry.push_back(country);
                        if (country->Alliance != 4)
                            DefeatCountry.push_back(country);
                    }
                    CurrentCountryIndex = 0;
                }
                else if (!strcmp(Name, "area")) {
                    for (TiXmlElement *AreaElement = ListElement->FirstChildElement();
                         AreaElement != NULL; AreaElement = AreaElement->NextSiblingElement()) {
                        CArea *Area = nullptr;
                        CCountry *country = nullptr;
                        enum CArea::ConstructionType ConstructionType;
                        enum CArea::InstallationType InstallationType;
                        int AreaID, ConstructionLevel;
                        const char *ConstructionStr, *InstallationStr, *CountryStr;
                        if (AreaElement->QueryIntAttribute("id", &AreaID) != TIXML_SUCCESS)
                            AreaID = 0;
                        Area = g_Scene.GetArea(AreaID);
                        if (Area->Enable) {
                            CountryStr = AreaElement->Attribute("country");
                            if (CountryStr != nullptr)
                                country = FindCountry(CountryStr);
                            g_Scene.SetAreaCountry(AreaID, country);
                            if (country != nullptr)
                                country->AddArea(AreaID);
                            ConstructionStr = AreaElement->Attribute("construction");
                            ConstructionType = ConstructionStr != nullptr ?
                                    (!strcmp(ConstructionStr, "city")? CArea::city :
                                    !strcmp( ConstructionStr, "industry") ?CArea::industry:
                                    !strcmp(ConstructionStr, "airport")?CArea::airport:
                                    !strcmp(ConstructionStr, "oilwell")? CArea::oilwell:
                                    CArea::NoConstruction)
                                    : CArea::NoConstruction;
                            if (AreaElement->QueryIntAttribute("level", &ConstructionLevel) !=
                                TIXML_SUCCESS)
                                ConstructionLevel = 0;
                            if (ConstructionLevel <= 0) {
                                ConstructionType = CArea::NoConstruction;
                                ConstructionLevel = 0;
                            }
                            if (!Area->Sea)
                                Area->SetConstruction(ConstructionType, ConstructionLevel);
                            InstallationStr = AreaElement->Attribute("installation");
                            InstallationType =
                                    InstallationStr != nullptr ? (!strcmp(InstallationStr, "fort")
                                                                  ? CArea::fort : !strcmp(
                                                    InstallationStr, "entrenchment")
                                                                                  ? CArea::entrenchment
                                                                                  : !strcmp(
                                                            InstallationStr, "antiaircraft")
                                                                                    ? CArea::antiaircraft
                                                                                    : !strcmp(
                                                                    InstallationStr, "radar")
                                                                                      ? CArea::radar
                                                                                      : CArea::NoInstallation)
                                                               : CArea::NoInstallation;
                            Area->InstallationType = InstallationType;
                            for (TiXmlElement *ArmyElement = AreaElement->FirstChildElement();
                                 ArmyElement !=
                                 NULL; ArmyElement = ArmyElement->NextSiblingElement()) {
                                ArmyDef::ArmyType armyName;
                                int ArmyLevel, ArmyCards, Designation;
                                const char *ArmyTypeStr;
                                ArmyTypeStr = ArmyElement->Attribute("type");
                                armyName =
                                        ArmyTypeStr != nullptr ? (!strcmp(ArmyTypeStr, "infantry")
                                                                  ? ArmyDef::InfantryD1905
                                                                  : !strcmp(ArmyTypeStr, "panzer")
                                                                    ? ArmyDef::Panzer : !strcmp(
                                                                ArmyTypeStr, "artillery")
                                                                                        ? ArmyDef::Artillery
                                                                                        : !strcmp(
                                                                        ArmyTypeStr, "rocket")
                                                                                          ? ArmyDef::Rocket
                                                                                          : !strcmp(
                                                                                ArmyTypeStr, "tank")
                                                                                            ? ArmyDef::Tank
                                                                                            : !strcmp(
                                                                                        ArmyTypeStr,
                                                                                        "heavy tank")
                                                                                              ? ArmyDef::HeavyTank
                                                                                              : !strcmp(
                                                                                                ArmyTypeStr,
                                                                                                "destroyer")
                                                                                                ? ArmyDef::Destroyer
                                                                                                : !strcmp(
                                                                                                        ArmyTypeStr,
                                                                                                        "cruiser")
                                                                                                  ? ArmyDef::Cruiser
                                                                                                  : !strcmp(ArmyTypeStr,
                                                                                                            "battleship")
                                                                                                    ? ArmyDef::Battleship
                                                                                                    : !strcmp(ArmyTypeStr,
                                                                                                              "aircraft carrier")
                                                                                                      ? ArmyDef::AircraftCarrier
                                                                                                      : ArmyDef::InfantryD1905)
                                                               : ArmyDef::InfantryD1905;
                                if (ArmyElement->QueryIntAttribute("level", &ArmyLevel) !=
                                    TIXML_SUCCESS)
                                    ArmyLevel = 1;
                                if (ArmyElement->QueryIntAttribute("cards", &ArmyCards) !=
                                    TIXML_SUCCESS)
                                    ArmyCards = 0;
                                if (ArmyElement->QueryIntAttribute("designation", &Designation) !=
                                    TIXML_SUCCESS) {
                                    Designation = 0;
                                }
                                if (country != nullptr) {
                                    CArmy *Army = new CArmy();
                                    ArmyDef *ArmyDef = CObjectDef::Instance()->GetArmyDef(armyName,
                                                                                          country->Name);
                                    Army->Init(ArmyDef, country);
                                    Army->Designation = Designation;
                                    //以下是设置番号
                                    if (Designation != 0) {
                                        //如果是海军
                                        if (Army->IsNavy()) {
                                            country->ListDesignationsSea[Designation] = true;
                                        }
                                        else {
                                            //如果是空军
                                            if (Army->Is_airForce()) {
                                                country->ListDesignationsAir[Designation] = true;
                                            }
                                            else if (Army->Is_tank()) {
                                                country->ListDesignationsTank[Designation] = true;
                                            }
                                            else {
                                                country->ListDesignationsLand[Designation] = true;
                                            }
                                        }
                                    }
                                    else {
                                        //如果是海军
                                        if (Army->IsNavy()) {
                                            SetArmyDesignationSea(Army, country);
                                        }
                                        else {
                                            //如果是空军
                                            if (Army->Is_airForce()) {
                                                SetArmyDesignationAir(Army, country);
                                            }
                                            else if (Army->Is_tank()) {
                                                SetArmyDesignationTank(Army, country);
                                            }
                                            else {
                                                SetArmyDesignationLand(Army, country);
                                            }
                                        }
                                    }
                                    Army->Cards = ArmyCards;
                                    Army->Level = ArmyLevel;
                                    Army->Movement = 0;
                                    Area->AddArmy(Army, true);
                                    if (Army->HasCard(CArmy::Commander))
                                        country->CommanderAlive = true;
                                    Army->ResetMaxStrength(false);
                                }
                            }
                        }
                    }

                }
                else if (!strcmp(Name, "dialogue")) {
                    for (TiXmlElement *DialogElement = ListElement->FirstChildElement();
                         DialogElement !=
                         nullptr; DialogElement = DialogElement->NextSiblingElement()) {
                        DialogueDef *DialogueDef = new struct DialogueDef();
                        int Index, AtRound, Left;
                        DialogueDef->Commander = DialogElement->Attribute("commander");
                        if (DialogElement->QueryIntAttribute("index", &Index) != TIXML_SUCCESS)
                            Index = 1;
                        if (DialogElement->QueryIntAttribute("atround", &AtRound) !=
                            TIXML_SUCCESS)
                            AtRound = 1;
                        if (DialogElement->QueryIntAttribute("left", &Left) != TIXML_SUCCESS)
                            Left = 0;
                        if (Left != 0)
                            Left = 1;
                        DialogueDef->Index = Index;
                        DialogueDef->AtRound = AtRound;
                        DialogueDef->Left = Left;
                        DialogueList.push_back(DialogueDef);
                    }
                }
            }
        }
        _ZN13TiXmlDocumentD1Ev(&Xml);

        //初始化停战列表
        Init_armisticeStatus(g_GameManager.CampaignSeriesID);
        Init_policyType_economy(g_GameManager.CampaignSeriesID);
        Init_policyType_military(g_GameManager.CampaignSeriesID);

        //初始化石油转化率
        Init_conversionRate_ofOil_aboutAllCountry();
        Init_countryCardRound();
        //初始化事件
        if (g_GameManager.GameMode == Conquest) {
            Init_resolutions_theScript(g_GameManager.CampaignSeriesID);
        }


//        for (int i = 0; i < 4; ++i) {
//            Add_newArmy_toArea(517, ArmyDef::InfantryD1905);
//        }
//        for (int i = 0; i < 4; ++i) {
//            Add_newArmy_toArea(523, ArmyDef::InfantryD1905);
//        }
//        for (int i = 0; i < 4; ++i) {
//            Add_newArmy_toArea(550, ArmyDef::InfantryD1905);
//        }
//        for (int i = 0; i < 1; ++i) {
//            Add_newArmy_toArea(521, ArmyDef::Panzer);
//        }
//        for (int i = 0; i < 1; ++i) {
//            Add_newArmy_toArea(506, ArmyDef::InfantryD1905);
//        }
//        g_Scene.GetArea(506)->GetArmy(0)->Designation = 666;




//
//        g_Scene[511]->GetArmy(0)->Hp *= 0.1;
//        g_Scene[511]->GetArmy(1)->Hp *= 0.1;
//        g_Scene[511]->GetArmy(2)->Hp *= 0.1;
//        g_Scene[511]->GetArmy(3)->Hp *= 0.1;
//        Add_newArmy_toArea(754,ArmyDef::InfantryD1905);
//        Add_newArmy_toArea(735,ArmyDef::Fight_1914);

//        if (g_GameManager.GameMode == Conquest){
//            LoadEventXml(g_GameManager.CampaignSeriesID);
//        }

    }

}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 在指定地块添加指定军队。如果地块为空或者没有国家，不会添加军队。<p>
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * void <p>
     *=======================================================<p>
     * 【创建人】： ？？？ <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： ？？？<p>
     * 【修改时间】： ？？？
     */
void Add_newArmy_toArea(int areaID, int armyID){
    CArea* area = g_Scene[areaID];
    if(area == nullptr || area->Country == nullptr)
        return;
    ArmyDef *armyDef = CObjectDef::Instance()->GetArmyDef(armyID, area->Country->Name);
    CArmy* army = new CArmy();
    army->Init(armyDef,area->Country);
    Set_armyDesignaion(army, area->Country);
    area->AddArmy(army, false);
}
EventBase::EventBase() {
    strcpy(targetCountryName, "null");
    strcpy(theCountryName, "null");
    strcpy(theCountryID, "null");
    strcpy(theCountryName, "null");
    eventNum = 0;
    eventType = 0;
    startRound = 999;
    endRound = 9999;
    percentOfTrigger = 100;
    moneyForTriggerEvent = -1000;
    industryForTriggerEvent = -1000;
    oilForTriggerEvent = -1000;
    isFinished = false;
    tech = 0;
}

void
EventBase::Set_countryText(int eventNum, int eventType, char* theCountryName, char* theCountryID, char* targetCountryName, char* targetCountryID) {
    this->eventNum = eventNum;
    this->eventType = eventType;
    strcpy(this->theCountryName,theCountryName);
    strcpy(this->theCountryID,theCountryID);
    strcpy(this->targetCountryID,targetCountryID);
    strcpy(this->theCountryName,theCountryName);
}

void
EventBase::Set_condition(int startRound, int endRound, int percentOfTrigger, int money, int industry,
                         int oil, int tech, std::map<int,bool>* map_area ) {
    if (map_area != nullptr){
        this->map_area = *map_area;
    }
    this->startRound = startRound;
    this->endRound = endRound;
    this->moneyForTriggerEvent = money;
    this->industryForTriggerEvent = industry;
    this->oilForTriggerEvent = oil;
    this->tech = tech;
}


void CGameManager::ClearBattle() {

    ListCountry.clear();
    DefeatCountry.clear();
    DialogueList.clear();
}



/*
 * 设置阵营的底座颜色
 */
def_easytech(_ZN8CGameRes10RenderArmyEPKciiffimbf)

void CGameRes::RenderArmy(const char *CountryName, int Alliance, int ArmyCount, float x, float y,
                          int ArmyType, unsigned long Color, bool Sea, float Direction) {
    unsigned long color = 0xffffffff;
    switch (Alliance) {
        case 1:
            color = 0xff2a5caa;
            break;
        case 2:
            color = 0xff1d953f;
            break;
        case 3:
            color = 0xffed1941;
            break;
        case 4:
            color = 0xff72777b;
            break;
        case 5:
            color = 0xff00a6ac;
            break;
        case 6:
            color = 0xffffd400;
            break;
        case 7:
            color = 0xfffffae3;
            break;
        case 8:
            color = 0xff0000ff;
            break;
        case 9:
            color = 0xff596032;
            break;
        case 10:
            color = 0xff7077d8;

    }
    Image_unitbase_grey[0]->SetColor(color, -1);
    Image_unitbase_grey[1]->SetColor(color, -1);
    Image_unitbase_grey[2]->SetColor(color, -1);
    Image_unitbase_grey[3]->SetColor(color, -1);
    Alliance = 4;
//     auto armyImage =  this->GetArmyImage(CountryName,servicesType,Sea);
//    if (armyImage != nullptr){
//        int arrOffsetIndex = ArmyCount-1;
//        armyImage->Render(x,y);
//        armyImage->SetColor(color,-1);
//        auto baseImage = this->GetBaseImage(ArmyCount,Alliance);
//        baseImage->RenderEx(x,arrUnitDrawnOffset[arrOffsetIndex + 8],0.0,Direction,1.0);
//    }
    easytech(_ZN8CGameRes10RenderArmyEPKciiffimbf)(this, CountryName, Alliance, ArmyCount, x, y,
                                                   ArmyType, Color, Sea, Direction);
}

ecImage *CGameRes::GetBaseImage(int armyCount,int allance) {

    return this->Image_unitbase_grey[armyCount -1];
}

int CCountry::GetCardRounds(CARD_ID card) {
    return this->CardRound[card];
}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 执行指令。<p>
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * void <p>
     *=======================================================<p>
     * 【创建人】： 柒安 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： ？？？ <p>
     * 【修改时间】： ？？？
     */
void CCountry::DoAction() {
    ActionTimer = 0.0f;
    CArea *StartArea = g_Scene.GetArea(CAction.StartAreaID);
    CArea *TargetArea = g_Scene.GetArea(CAction.TargetAreaID);
    switch (CAction.ActionType) {
        case CountryAction::ArmyMoveAction: {
            if (CAction.ArmyIndex > 0) {
                StartArea->MoveArmyToFront(CAction.ArmyIndex, false);
                CAction.ArmyIndex = 0;
            }
            StartArea->MoveArmyTo(TargetArea);
        }
            break;

        case CountryAction::ArmyAttackAction: {
            bool NeedBattleAnimation = false;
            if (CAction.ArmyIndex > 0) {
                StartArea->MoveArmyToFront(CAction.ArmyIndex, false);
                CAction.ArmyIndex = 0;
            }
            CArmy *Army = StartArea->GetArmy(0);
            if (Army->BasicAbilities->ID == ArmyDef::AircraftCarrier) {
                g_Scene.AircraftCarrierBomb(CAction.StartAreaID, CAction.TargetAreaID);
                break;
            }
            if (g_GameSettings.BattleAnimation) {
                if (StartArea->Country == nullptr||StartArea->Country->AI) {
                    if (TargetArea->Country != nullptr)
                        NeedBattleAnimation = !TargetArea->Country->AI;
                }
                else {
                    NeedBattleAnimation = true;
                }
            }
            ((CGameState *) CStateManager::Instance()->GetStatePtr(Game))->StartBattale(
                    CAction.StartAreaID, CAction.TargetAreaID, 1, NeedBattleAnimation);
        }
            break;

        case CountryAction::ArmyMoveFrontAction: {
            if (CAction.ArmyIndex > 0)
                TargetArea->MoveArmyToFront(CAction.ArmyIndex, false);
        }
            break;

        case CountryAction::UseCardAction: {
            CardDef *Def = CObjectDef::Instance()->GetCardDef(CAction.CardID);
            if (CAction.CardID == AirborneForceCard) {
                g_Scene.Airborne(CAction.TargetAreaID);
            }
            else if (Def->Type == CardDef::AirForce) {
                g_Scene.BombArea(CAction.TargetAreaID,
                                 CAction.CardID == AirStrikeCard ? AirStrike : (CAction.CardID ==
                                                                                BomberCard ? Bomb
                                                                                           : NuclearBomb));
            }
            else {
                UseCard(CObjectDef::Instance()->GetCardDef(CAction.CardID), CAction.TargetAreaID,
                        CAction.ArmyIndex);
            }
        }
            break;

        case CountryAction::CameraMoveAction: {
            FinishAction();
        }
        case CountryAction::NoAction:
        case CountryAction::ArmyInactiveAction:
            break;
    }
}

void SplitArmy(CArea *area, ArmyDef::ArmyType *armyDef) {
    int divisionID = GetDivisionType(armyDef);
    CArmy *theArmy = area->Army[0];
    int armiesNum = 0;
    for (int i = 0; i < 3; ++i) {
        //如果不为0，则说明存储着师级单位的番号
        if (theArmy->OldDesignation[i] != 0) {
            armiesNum++;
        }
    }

    ArmyDef *newArmyDef = CObjectDef::Instance()->GetArmyDef(divisionID, area->Country->Name);
    CArmy *firstArmy = new CArmy();
    firstArmy->Init(newArmyDef, area->Country);
    int hpPercent = theArmy->Hp * 100 / theArmy->MaxHp;
    int OPercent = theArmy->Organization * 100 / theArmy->MaxOrganization;
    firstArmy->Cards = theArmy->Cards;
    firstArmy->Designation = theArmy->Designation;
    firstArmy->Hp = firstArmy->MaxHp * hpPercent / 100 * 0.8;
    firstArmy->Organization = firstArmy->MaxOrganization * OPercent / 100 * 0.8;
    firstArmy->Movement = 0;
    firstArmy->Active = false;
    //如果是2师军，经验*= 0.6
    firstArmy->Exp = theArmy->Exp * 0.6;
    if (armiesNum == 3) {
        firstArmy->Exp = theArmy->Exp * 0.5;
    }
    int theArmyExp = theArmy->Exp;
    int arr[armiesNum];

    for (int i = 0; i < armiesNum; ++i) {
        arr[i] = theArmy->OldDesignation[i];
    }


    area->RemoveArmy(theArmy);
    delete theArmy;
    area->AddArmy(firstArmy, false);
    for (int i = 1; i < armiesNum; ++i) {
        CArmy *army = new CArmy();
        army->Init(newArmyDef, area->Country);
        army->Hp = army->MaxHp * hpPercent * 0.8 / 100;
        army->Organization = army->MaxOrganization * OPercent * 0.8 / 100;
        army->Exp = theArmyExp * 0.1;
        army->Designation = arr[i];
        army->Movement = 0;
        army->Active = false;
        area->AddArmy(army, false);
    }

}

ArmyDef::ArmyType GetDivisionType(ArmyDef::ArmyType *armyDef) {
    ArmyDef::ArmyType return_id;
    switch (*armyDef) {
        case ArmyDef::InfantryA1905:
            return_id = ArmyDef::InfantryD1905; //返回1905步兵师
            break;
        default:
            break;
    }
    return return_id;
}

ArmyDef::ArmyType GetArmyType(ArmyDef::ArmyType *armyDef) {
    ArmyDef::ArmyType return_id;
    switch (*armyDef) {
        case ArmyDef::InfantryD1905:
            return_id = ArmyDef::InfantryA1905; //返回 1905步兵军
            break;
        default:
            break;
    }
    return return_id;
}

void ExpendArmy(CArea *area, int armies, ArmyDef::ArmyType *armyDef) {

    int averageHpPercent = 0;
    int averageOrganizationPercent = 0;

    //所有待合并单位的最大血量之和
    int allMaxHp = 0;
    //所有待合并单位最大组织度之和
    int allMaxO = 0;
    int allExp = 0;
    int allArmyHP = 0;
    int allArmyO = 0;
    //存储被合并的师级单位
    std::vector<CArmy *> listArmy;
    for (int i = 0; i < armies; ++i) {
        //不符合条件则退出
        if (area->Army[i]->IsA||area->Army[i]->BasicAbilities->ID != *armyDef) {
            break;
        }
        if (i == 3)
            break;
        listArmy.push_back(area->Army[i]);
    }
    CArmy *firstArmy = area->Army[0];
    CCountry *theCounty = area->Army[0]->Country;
    CArmy *newArmy = new CArmy();
    int armyID = GetArmyType(&firstArmy->BasicAbilities->ID);
    ArmyDef *newArmyDef = CObjectDef::Instance()->GetArmyDef(armyID, area->Country->Name);
    //初始化
    newArmy->Init(newArmyDef, theCounty);

    for (int i = 0; i < listArmy.size(); ++i) {
        allMaxHp += listArmy[i]->MaxHp;
        allMaxO += listArmy[i]->MaxOrganization;
        allArmyHP += listArmy[i]->Hp;
        allArmyO += listArmy[i]->Organization;
        allExp += listArmy[i]->Exp;
        if (!firstArmy->HasCard(CArmy::AntitankGun)&&listArmy[i]->HasCard(CArmy::AntitankGun)) {
            firstArmy->AddCard(CArmy::AntitankGun);
        }
        if (!firstArmy->Has_engineeringCorps() && listArmy[i]->Has_engineeringCorps()) {
            firstArmy->AddCard(CArmy::EngineeringCorps);
        }
        if (!firstArmy->HasCard(CArmy::LightArtillery) && listArmy[i]->HasCard(CArmy::LightArtillery)) {
            firstArmy->AddCard(CArmy::LightArtillery);
        }
        if (!firstArmy->HasCard(CArmy::Commander)&&listArmy[i]->HasCard(CArmy::Commander)) {
            firstArmy->AddCard(CArmy::Commander);
        }
    }

    averageHpPercent = allArmyHP * 100 / allMaxHp;
    averageOrganizationPercent = allArmyO * 100 / allMaxO;


    newArmy->Hp = newArmy->MaxHp * averageHpPercent * 0.8 / 100;
    newArmy->Organization = newArmy->MaxOrganization * 0.8 * averageOrganizationPercent / 100;
    newArmy->AddExp(allExp / (listArmy.size() + 1));


    //设置番号
    newArmy->Designation = firstArmy->Designation;
    newArmy->OldDesignation[0] = firstArmy->Designation;
    //保存旧番号
    for (int i = 1; i < listArmy.size(); ++i) {
        newArmy->OldDesignation[i] = area->Army[i]->Designation;
    }
    newArmy->IsA = true;
    newArmy->Cards = firstArmy->Cards;
    area->ClearArmy(listArmy.size());
    newArmy->Movement = 0;
    newArmy->Active = false;
    area->AddArmy(newArmy, false);
}

void Init_countryCardRound() {
    int size = g_GameManager.ListCountry.size();
    for (int i = 0; i < size; ++i) {
        CCountry *country = g_GameManager.ListCountry[i];
        int cardSize = NewCardDef.size();
        for (int j = 0; j < cardSize; ++j) {
            country->Arr_cardCD_inSetting[j] = NewCardDef[j]->Round;
        }
        for (int j = 0; j < cardSize; ++j) {
            country->Arr_cardCD_inTheTurn[j] = country->Arr_cardCD_inSetting[j];
        }
    }
}

void CCountry::UseCard(CardDef *Card, int TargetAreaID, int ArmyIndex) {
    //CanBuyCard   能否点击卡牌的勾
    int CardPrice = GetCardPrice(Card);
    int CardIndustry = GetCardIndustry(Card);
    if (CardPrice <= Money&&CardIndustry <= Industry) {
        //如果目标地块的id大于0，返回指定的地块id 否则返回空
        CArea *targetArea = TargetAreaID >= 0 ? g_Scene[TargetAreaID] : NULL;
        if (targetArea == nullptr)
            return;
        //管理所有的新卡牌
        if (IsNewCard(Card->ID)) {
            if (Arr_cardCD_inTheTurn[Card->ID] == 0 && CheckCardTargetArea(Card, TargetAreaID)) {
                if (Card->Type == CardDef::Army||Card->Type == CardDef::Navy) {
                    targetArea->DraftArmy(Card->ID);
                }
                else if (Card->Type == CardDef::Development && CheckCardTargetArea(Card, TargetAreaID)) {
                    if (Card->ID == ExpendArmyCard) {
                        ExpendArmy(targetArea, targetArea->ArmyCount, &targetArea->Army[0]->BasicAbilities->ID);

                    }
                    else if (Card->ID == SplitCard) {
                        SplitArmy(targetArea, &targetArea->Army[0]->BasicAbilities->ID);
                    }
                }
                else if(Card->Type == CardDef::Strategy && CheckCardTargetArea(Card, TargetAreaID)){

                    if (Card->ID == HARCard) {
                        targetArea->AddArmyCard(ArmyIndex, CArmy::HeavyArtillery);
                    }
                    else if (Card->ID == TDRCard) { //突击炮
                        targetArea->AddArmyCard(ArmyIndex, CArmy::TankDestroyer);
                    }
                    else if (Card->ID == TTRCard) { //后勤团
                        targetArea->AddArmyCard(ArmyIndex, CArmy::TransportationTroops);
                    }
                    else if (Card->ID == MARCard) { //自走炮
                        targetArea->AddArmyCard(ArmyIndex, CArmy::MoveArtillery);
                    }
                }
            }
            Arr_cardCD_inTheTurn[Card->ID] = Arr_cardCD_inSetting[Card->ID];
            Money -= CardPrice;
            Industry -= CardIndustry;
            return;
        }
        else if ((Card->Type == CardDef::Army||Card->Type == CardDef::Navy)&&
                 CheckCardTargetArea(Card, TargetAreaID)&&IsCardUnlock(Card)) {
            if (Arr_cardCD_inTheTurn[Card->ID] > 0)
                return;
            CArmy *Army = nullptr;
            switch (Card->ID) {
                case 0:
                    Army = targetArea->DraftArmy(0);
                    break;
                case 1:
                    Army = targetArea->DraftArmy(1);
                    break;
                case 2:
                    Army = targetArea->DraftArmy(2);
                    break;
                case 3:
                    Army = targetArea->DraftArmy(3);
                    break;
                case 4:
                    Army = targetArea->DraftArmy(4);
                    break;
                case 5:
                    Army = targetArea->DraftArmy(5);
                    break;
                case 6:
                    Army = targetArea->DraftArmy(6);
                    break;
                case 7:
                    Army = targetArea->DraftArmy(7);
                    break;
                case 8:
                    Army = targetArea->DraftArmy(8);
                    break;
                case 9:
                    Army = targetArea->DraftArmy(9);
                    break;
                default:
                    Army = targetArea->DraftArmy(0);
                    break;
            }
            if (Army == nullptr)
                return;
            // 海军勋章加成
            int NavyMedalLevel = GetWarMedalLevel(NavyMedal);
            if (NavyMedalLevel > 0) {
                if (Card->ID == InfantryCard)
                    Army->AddCard(CArmy::AntitankGun);
                if ((NavyMedalLevel > 1&&
                     (Card->ID == DestroyerCard||Card->ID == CruiserCard))||
                    (NavyMedalLevel > 2&&
                     (Card->ID == BattleShipCard||Card->ID == AircraftCarrierCard)))
                    Army->Upgrade();
            }
            // 步兵勋章加成
            if (Card->ID == InfantryCard) {
                int InfantryMedalLevel = GetWarMedalLevel(InfantryMedal);
                if (InfantryMedalLevel > 0) {
                    Army->Upgrade();
                    if (InfantryMedalLevel > 1) {
                        Army->Upgrade();
                        if (InfantryMedalLevel > 2) {
//                                    Army->AddCard(CArmy::LightArtillery);
                            Army->AddCard(CArmy::EngineeringCorps);
                        }
                    }
                }
            }

            // 炮兵勋章加成
            int ArtilleryMedalLevel = GetWarMedalLevel(ArtilleryMedal);
            if (ArtilleryMedalLevel > 0) {
                if (Card->ID == ArtilleryCard)
                    Army->Upgrade();
                if (ArtilleryMedalLevel > 1) {
                    if (Card->ID == RocketCard)
                        Army->Upgrade();
                    if (ArtilleryMedalLevel > 2&&
                        (Card->ID == ArtilleryCard||Card->ID == RocketCard))
                        Army->AddCard(CArmy::EngineeringCorps);
                }
            }

            // 装甲勋章加成
            int ArmourMedalLevel = GetWarMedalLevel(ArmourMedal);
            if (ArmourMedalLevel > 0) {
                if (Card->ID == ArmourCard)
                    Army->Upgrade();
                if (ArmourMedalLevel > 1) {
                    if (Card->ID == TankCard)
                        Army->Upgrade();
                    if (ArmourMedalLevel > 2&&Card->ID == MediumTankCard)
                        Army->Upgrade();
                }
            }

            Money -= CardPrice;
            Industry -= CardIndustry;
            //重置卡牌冷却时间
//                CardRound[Card->ID] = Card->Round;
            Arr_cardCD_inTheTurn[Card->ID] = Arr_cardCD_inSetting[Card->ID];
        }
        else if (Card->Type == CardDef::Development && CheckCardTargetArea(Card, TargetAreaID) &&
                 Arr_cardCD_inTheTurn[Card->ID] == 0) {
            switch (Card->ID) {
                case CityCard:
                    targetArea->Construct(CArea::city);
                    Arr_cardCD_inTheTurn[Card->ID] = Arr_cardCD_inSetting[Card->ID];
                    break;
                case IndustryCard:
                    targetArea->Construct(CArea::industry);
                    Arr_cardCD_inTheTurn[Card->ID] = Arr_cardCD_inSetting[Card->ID];
                    break;
                case AirportCard:
                    targetArea->Construct(CArea::airport);
                    Arr_cardCD_inTheTurn[Card->ID] = Arr_cardCD_inSetting[Card->ID];
                    break;
                case LandFortCard:
                    targetArea->InstallationType = CArea::fort;
                    Arr_cardCD_inTheTurn[Card->ID] = Arr_cardCD_inSetting[Card->ID];
                    break;
                case EntrenchmentCard:
                    targetArea->InstallationType = CArea::entrenchment;
                    Arr_cardCD_inTheTurn[Card->ID] = Arr_cardCD_inSetting[Card->ID];
                    break;
                case AntiaircraftCard:
                    targetArea->InstallationType = CArea::antiaircraft;
                    Arr_cardCD_inTheTurn[Card->ID] = Arr_cardCD_inSetting[Card->ID];
                    break;
                case RadarCard:
                    targetArea->InstallationType = CArea::radar;
                    Arr_cardCD_inTheTurn[Card->ID] = Arr_cardCD_inSetting[Card->ID];
                    break;
                default:
                    return;
            }
            _ZN9CSoundRes10PlayCharSEE10SND_EFFECT(&g_SoundRes, CSoundRes::aBuild_wav);
            Money -= CardPrice;
            Industry -= CardIndustry;
        }
        else if (Card->Type == CardDef::Strategy) {
            if (Card->ID == ResearchCard&&(Tech <= 4||TechTurn == 0)) {
                _ZN9CSoundRes10PlayCharSEE10SND_EFFECT(&g_SoundRes, CSoundRes::aLvup_wav);
                //设置研究时长
                TechTurn = 3;
                Money -= CardPrice;
                Industry -= CardIndustry;
            }
            else if (Card->ID == CommanderCard&&
                     CheckCardTargetArmy(Card, TargetAreaID, ArmyIndex)) {
                CommanderAlive = true;
                targetArea->AddArmyCard(ArmyIndex, CArmy::Commander);
                CArmy *TargetArmy = targetArea->Army[ArmyIndex];
                CommanderAbility Ability = GetCommanderAbility(GetCommanderLevel());
                if (TargetArmy != NULL) {
                    TargetArmy->AddStrength(Ability.Supply);
                    TargetArmy->ResetMaxStrength(false);
                }
                Money -= CardPrice;
                Industry -= CardIndustry;
                _ZN9CSoundRes10PlayCharSEE10SND_EFFECT(&g_SoundRes, CSoundRes::aBuff_wav);
                if (AI&&g_GameManager.GameMode != CGameManager::MultiPlayer) {
                    const char *CommanderName = GetCommanderName();
                    if (CommanderName != NULL) {
                        char s[128];
                        sprintf(s, "commander return %ld", lrand48() % 5 + 1);
                        CGameState *GameState = (CGameState *) CStateManager::Instance()->GetStatePtr(
                                Game);
                        GameState->ShowDialogue(s, CommanderName, false);
                    }

                }
            }
            else if (Card->ID == AceForcesCard && Arr_cardCD_inTheTurn[Card->ID] == 0 &&
                     targetArea != NULL) {
                targetArea->GetArmy(0)->Upgrade();
                _ZN9CSoundRes10PlayCharSEE10SND_EFFECT(&g_SoundRes, CSoundRes::aLvup_wav);
                Arr_cardCD_inTheTurn[Card->ID] = Arr_cardCD_inSetting[Card->ID];
                Money -= CardPrice;
                Industry -= CardIndustry;
            }
            else if (Card->ID == SupplyLineCard&&GetCardRounds(SupplyLineCard) == 0&&
                     targetArea != NULL) {
                for (int i = 0; i < targetArea->ArmyCount; i++)
                    targetArea->RevertArmyStrength(i);
                _ZN9CSoundRes10PlayCharSEE10SND_EFFECT(&g_SoundRes, CSoundRes::aSupply_wav);
                Arr_cardCD_inTheTurn[Card->ID] = Arr_cardCD_inSetting[Card->ID];
                Money -= CardPrice;
                Industry -= CardIndustry;
            }
            else if ((Card->ID == AGRCard||Card->ID == LARCard||Card->ID == ECRCard) &&
                     CheckCardTargetArmy(Card, TargetAreaID, ArmyIndex) &&
                     Arr_cardCD_inTheTurn[Card->ID] == 0) {
                switch (Card->ID) {
                    case AGRCard:
                        targetArea->AddArmyCard(ArmyIndex, CArmy::AntitankGun);
                        break;
                    case LARCard:
                        targetArea->AddArmyCard(ArmyIndex, CArmy::LightArtillery);
                        break;
                    case ECRCard:
                        targetArea->AddArmyCard(ArmyIndex, CArmy::EngineeringCorps);
                        break;
                    default:
                        return;
                }
                Arr_cardCD_inTheTurn[Card->ID] = Arr_cardCD_inSetting[Card->ID];
                _ZN9CSoundRes10PlayCharSEE10SND_EFFECT(&g_SoundRes, CSoundRes::aBuff_wav);
                Money -= CardPrice;
                Industry -= CardIndustry;
            }
        }
        else if (Card->Type == CardDef::AirForce && Arr_cardCD_inTheTurn[Card->ID] == 0) {
            if (CheckCardTargetArea(Card, TargetAreaID)) {
                if (Card->ID == AirborneForceCard) {
                    CCountry *TargetCountry = targetArea->Country;
                    if (TargetCountry != this) {
                        if (TargetCountry != NULL)
                            TargetCountry->RemoveArea(TargetAreaID);
                        AddArea(TargetAreaID);
                        targetArea->Country = this;
                        if (TargetCountry != NULL&&TargetCountry->IsConquested())
                            TargetCountry->BeConquestedBy(this);
                    }
                    targetArea->DraftArmy(ArmyDef::InfantryD1905);
                }
//                CardRound[Card->ID] = GetWarMedalLevel(AirForceMedal) > 2 ? 0 : Card->Round;
                Arr_cardCD_inTheTurn[Card->ID] = Arr_cardCD_inSetting[Card->ID];
                Money -= CardPrice;
                Industry -= CardIndustry;
            }
        }
    }
}

bool CCountry::IsCardUnlock(CardDef *carddef) {
    return NewCardDef[carddef->ID]->Tech <= this->Tech;
}

def_easytech(_ZN8CCountry19CheckCardTargetAreaEP7CardDefi)

bool CCountry::CheckCardTargetArea(CardDef *Card, int AreaID) {
    CArea *area = g_Scene.GetArea(AreaID);
    //安全地块不允许造兵
    if (g_GameManager.IsIntelligentAI && area->IsSafe) {
        return false;
    }
    //如果是新卡牌
    if (IsNewCard(Card->ID)) {
        if (area != nullptr&&area->Enable&&area->Country != nullptr&&area->Country == this) {
            if (Card->ID == SubmarineCard) {   //潜艇
                //检查这个地块能否被点击
                if (area->Sea == true&&area->Type == port&&area->ArmyCount < 4) {
                    return true;
                }
            }
            else if (Card->ID == BiochemicalForceCard) {  //生化部队
                //检查这个地块能否被点击
                if (area->Sea == false && area->ArmyCount < 4 &&
                    area->ConstructionType == CArea::industry && area->ConstructionLevel == 3) {
                    return true;
                }
            }
                //以下的卡是地块单位数量要求大于1 的
            else if (area->ArmyCount >= 1) {
                if (Card->ID == ExpendArmyCard) { //整编
                    //检查这个地块能否被点击
                    if (area->Sea == false  &&area->Country == g_GameManager.GetCurCountry()) {
                        CArmy *army = area->GetArmy(0);
                        int maxHp = army->GetMaxStrength();
                        //检查顶层单位
                        if (army->Hp >= maxHp / 2 && army->IsA == false) {
                            //符合的军队数量
                            int count = 1;
                            //需要检查的兵种id
                            int id = army->BasicAbilities->ID;
                            //如果不是步兵且不是装甲车
                            if (id != ArmyDef::InfantryD1905 && id != ArmyDef::Panzer) {
                                return false;
                            }
                            for (int i = 1; i < area->ArmyCount; ++i) {
                                //如果循环
                                if ((area->Army[i]->IsA ||
                                     area->Army[i]->BasicAbilities->ID != id) &&
                                    count < 2) {
                                    return false;
                                }
                                count++;
                            }
                            if (count >= 2)
                                return true;
                        }
                    }
                }
                else if (Card->ID == SplitCard) { //解散
                    if (area->Sea == false  &&area->Country == g_GameManager.GetCurCountry()) {
                        if (area->Army[0]->IsA) {
                            if (area->Army[0]->OldDesignation[2] != 0 && area->ArmyCount <= 2) {
                                return true;
                            }
                            else if (area->ArmyCount <= 3) {
                                return true;
                            }
                        }
                    }
                }
                else if (!area->Sea) {
                    CArmy* firstArmy = area->GetArmy(0);
                    //在海洋上不能建造 且只能是步兵单位建造
                    if (Card->ID == HARCard && !firstArmy->HasCard(CArmy::HeavyArtillery) && firstArmy->Is_infantry() ){
                        return true;
                    }
                    else if (Card->ID == TDRCard&& !firstArmy->HasCard(CArmy::TankDestroyer)  ) { //突击炮
                        return true;
                    }
                    else if (Card->ID == TTRCard&& !firstArmy->HasCard(CArmy::TransportationTroops) && firstArmy->BasicAbilities->Movement < 3&&!firstArmy->IsNavy()) { //后勤团
                        return true;
                    }
                    else if (Card->ID == MARCard&& !firstArmy->HasCard(CArmy::MoveArtillery) ) { //自走炮
                        return true;
                    }
                }
            }
            return false;
        }
    }
    else{
        if (Card->ID == InfantryCard){
            return area->ConstructionLevel > 2;
        }
        else{
            return easytech(_ZN8CCountry19CheckCardTargetAreaEP7CardDefi)(this, Card, AreaID);

        }

    }


}

/*
 * 购买卡牌
 */
bool CCountry::CanBuyCard(CardDef *Card) {
    //如果这个不是卡牌处于锁住状态
    if ((!IsCardUnlock(Card)
         // 或者 这个卡牌是指挥官卡牌的同时还指挥官已被使用了
         ||((Card->ID == CommanderCard)&&!CanUseCommander()))
        //或者这个是科技卡的同时 科技等级大于4或者科技冷却回合大于0
        ||((Card->ID == ResearchCard)&&((Tech > 4)||(TechTurn > 0)))
        //或者这个卡牌的冷却回合大于0
        ||((Arr_cardCD_inTheTurn[Card->ID] > 0)))
        return false;
    return IsEnoughMoney(Card)&&IsEnoughIndustry(Card);
}

//void CActionAssist::actionToNextArea(int mode, int targetAreaID, int ArmyIndex, int mystery) {
//    CArea *startArea = g_Scene.GetArea(targetAreaID);
//    if (startArea == nullptr)
//        return;
//    CArmy *startArmy = startArea->GetArmy(ArmyIndex);
//    int startAreaArmyCount = startArea->ArmyCount;
//    int startAreaValue = this->calcAreaValue(startArea); //这个地块的军队价值
//    int startAreaArmysStrength = GetAreaArmysStrengthNum(startArea, false);
//    int startAreaAroundEnemyStrengthNum = _ZN13CActionAssist11getNeighborEiiic(this, targetAreaID, 2010, 1, 1); //周围地块血量总数
//    this->TargetNode.ActionToNextAreaArmyIndex = ArmyIndex;
//    float coefficient = 0.0f;
//    std::vector<int> list_attack_to_choose;
//    bool arrToAttackAreas[g_Scene.GetNumAreas()];
//    for (int i = 0; i < g_Scene.GetNumAreas(); i++){
//        arrToAttackAreas[i] = false;
//    }
//    int r = 0;
//    switch (mode) {
//        case 0x1000: {
//            this->TargetNode.TargetValue = 0;
//            this->TargetNode.ActionToNextAreaTargetId = targetAreaID;
//            r = startAreaValue / 100 - startAreaArmyCount; //
//            if (startAreaAroundEnemyStrengthNum == 0 || startAreaAroundEnemyStrengthNum > startAreaArmysStrength) {
//                if (r != 0){
//                    if (r < 0) { //地块等级越低，执行这里的概率越大     单位数量越多，执行这里的概率越高（影响最大）  地块属性太低而防守兵力太多时，价值极低
//                        this->TargetNode.TargetValue = startAreaValue / (1 - r); //此时 1 - r必定为正整数
//                    }
//                    else {
//                        this->TargetNode.TargetValue = startAreaValue + r * startAreaValue; //此时地块价值高得惊人 如首都属性却只有一个防守单位
//                    }
//                    //目标地块单位为n（n>0）时，目标地块价值[100n,200n)时，才能触发这里的条件  原版设定中,不算上税收因素,首都上限不超过300 大城市不超过200
//                }else{
//                    this->TargetNode.TargetValue = startAreaValue;
//                }
//                int value = this->TargetNode.TargetValue;
//                int startArmyCountValue = startAreaArmyCount * 2;
//                if (startAreaAroundEnemyStrengthNum == 0)
//                    startArmyCountValue += 1;
//                this->TargetNode.TargetValue = value / startArmyCountValue; //军队数量越多，价值越低
//            }else{ //目标地块军队总血量 大于 周围地块敌军总血量
//                if (r != 0){
//                    int targetValue = this->TargetNode.TargetValue;
//                    if (r < 0) { //地块等级越低，执行这里的概率越大     单位数量越多，执行这里的概率越高（影响最大）
//                        this->TargetNode.TargetValue = startAreaValue / (1 - r); //此时 1 - r必定为正整数
//                    } else {
//                        this->TargetNode.TargetValue = startAreaValue + r * startAreaValue; //此时地块价值高得惊人 如首都属性却只有一个防守单位
//                    }
//                }else{
//                    this->TargetNode.TargetValue = startAreaValue;
//                }
//            }
//            if (startArea->ListCountry != nullptr) {
//                CCountry *startAreaCountry = startArea->ListCountry;
//                this->TargetNode.TargetIsPlayer = !startAreaCountry->AI;
//            }
//            else {
//                this->TargetNode.TargetIsPlayer = false;
//            }
//            return this->ActionToNextAreaTargetIdList.push_back(this->TargetNode);
//
//        }
//        case 0x2000: {//移动判定
//            _ZN13CActionAssist11getNeighborEiiic(this, targetAreaID, 2100, 0, 1); //初始化  this->SearchHead
//            int *arrAroundAreaID = GetNeighbourID; //目标地块周围的相邻地块
//            int mark = -1;
//            int first = 0;
//            while (true) {
//                this->TargetNode.TargetValue = 0;
//                if (*arrAroundAreaID != -1) {
//                    //循环所有相邻地块，检测能否移动
//                    bool can_move = g_Scene.CheckMoveable(targetAreaID, *arrAroundAreaID, ArmyIndex);
//                    if (can_move) {
//                        //如果能移动  设置进去
//                        this->TargetNode.ActionToNextAreaTargetId = *arrAroundAreaID;
//                        CArea *aroundArea = g_Scene.GetArea(*arrAroundAreaID);
//                        int aroundAreaValue = this->calcAreaValue(aroundArea);
//                        if (this->getAlliance(targetAreaID, aroundArea->ID, Self)) {//自己的领土上
//                            if (aroundArea->ArmyCount == 0 && startArea->ArmyCount > 1)
//                                //此时目标地块 已经变成了aroundArea   这里降低前往自己领土地块的比重
//                                this->TargetNode.TargetValue += aroundAreaValue / 2;
//                            coefficient = SetTargetIsPlayer(this, startArmy, startArea);
//                            //如果起始点
//                            this->TargetNode.TargetValue *= coefficient; //起始点单位的血量低于50%时，前往周围地块的概率下降（周围地块价值降低）
//                            this->ActionToNextAreaTargetIdList.push_back(this->TargetNode);
//                        }
//                        else if (this->getAlliance(targetAreaID, aroundArea->ID, Enemy) ||
//                                 this->getAlliance(targetAreaID, aroundArea->ID, NotAlly)) { //敌人的领土
//                            if (startAreaAroundEnemyStrengthNum > 0) {
//                                //周围地块价值因素影响最大，其次是起始地块单位，最后是起始地块单位总血量    最后减去周围所有敌军的总血量   + 80
//                                this->TargetNode.TargetValue += (startAreaArmysStrength + startAreaArmyCount * aroundAreaValue -startAreaAroundEnemyStrengthNum +80);
//                            }
//                            else {
//                                this->TargetNode.TargetValue += (aroundAreaValue +startAreaArmyCount *aroundAreaValue + 80);
//                            }
//                            //从海上攻击陆地的概率增加
//                            if ((!aroundArea->Sea) && startArea->Sea) {
//                                this->TargetNode.TargetValue += 80;
//                            }
//                            coefficient = SetTargetIsPlayer(this, startArmy, startArea);
//                            this->TargetNode.TargetValue *= coefficient;
//                            this->ActionToNextAreaTargetIdList.push_back(this->TargetNode);
//                        }
//                        else if (this->getAlliance(targetAreaID, aroundArea->ID, Ally)) {
//                            coefficient = SetTargetIsPlayer(this, startArmy, startArea);
//                            this->TargetNode.TargetValue *= coefficient;
//                            this->ActionToNextAreaTargetIdList.push_back(this->TargetNode);
//                        }
//                    }
//                }
//                arrAroundAreaID++;
//                if (arrAroundAreaID == &this->TotalSeaAreaCount) { //判断是否超出界限
//                    return;
//                }
//            }
//            break;
//        }
//        case 0x3000: {
//            if (startAreaArmyCount > 0) {
//                std::vector<CArea*> listCanAttackArea; //周围可攻击的地块
//                //添加起始地块可攻击的单位进去  此处为远程攻击
//                if (startArmy->Is_remoteAttack() || startArmy->Is_radiudAttack())
//                    Get_areas_inRange(startArea, listCanAttackArea);
//                else{   //此处为近战攻击
//                    int num = g_Scene.GetNumAdjacentAreas(startArea->ID);
//                    for (int i = 0; i < num; ++i) {
//                        listCanAttackArea.push_back(g_Scene.GetAdjacentArea(startArea->ID,i));
//                    }
//                }
//                int size = listCanAttackArea.size();
//                for (int i = 0; i < size; ++i) {
//                    CArea* canAttackArea = listCanAttackArea[i];
//                    this->TargetNode.ActionToNextAreaTargetId = canAttackArea->ID;
//                    this->TargetNode.TargetValue = calcAreaValue(canAttackArea);
//                    int targetValue = this->TargetNode.TargetValue;
//                    if (getAlliance(startArea->ID,canAttackArea->ID,Self))
//                        continue;
//                    if (getAlliance(startArea->ID,canAttackArea->ID,NotAlly))
//                        this->TargetNode.TargetValue -= 10000;
//                    if (canAttackArea->ArmyCount > 0){
//                        CArmy* canAttackArmy = canAttackArea->GetArmy(0);
//                        if (getAlliance(startArea->ID,canAttackArea->ID,NotAlly)){
//                            if (startAreaAroundEnemyStrengthNum > startAreaArmysStrength){ //周围敌军总血量大于起始地块军血量时 目标地块价值变低
//                                this->TargetNode.TargetValue += (startAreaArmyCount - 1)  * (targetValue + startArmy->Hp - canAttackArmy->Hp) + 120;
//                            }else if (startAreaAroundEnemyStrengthNum < startAreaArmysStrength){
//                                this->TargetNode.TargetValue += startAreaArmyCount  * (targetValue + startArmy->Hp - canAttackArmy->Hp) + 120;
//                            }
//                        }
//                    }
//                }
//            }
//
//        }
//
//    }
//}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     *   得到该地块指定远程单位射程范围内的所有地块。这些地块未必可攻击<p>
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * 返回范围内的指定地块。返回值记得释放。 <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： 李德邻<p>
     * 【创建时间】： ？？？<p>
     * 【修改时间】： 2022/7/4
     */
std::vector<CArea*>* CArea::Get_areas_inRange(int armyIndex){
    std::vector<CArea*>* listArea = new std::vector<CArea*>();
    CArmy* theArmy = this->GetArmy(armyIndex);
    int areas = g_Scene.GetNumAreas();
    float d;
    //如果是圆半径的攻击方式
    if (theArmy->Is_radiudAttack()){
        for (int i = 0; i < areas; ++i) {
            d = g_Scene.GetTwoAreasDistance(this->ID,g_Scene[i]->ID);
            //如果地块之间的距离在该单位的攻击半径之内，把这个地块添加进去
            if (d <= theArmy->GetAirstrikeRadius() && g_Scene[i]->Enable)
                listArea->push_back(g_Scene[i]);
        }
    }
   else  if(theArmy->Is_remoteAttack()){
        for (int k = 1; k < 2; ++k) {
            std::vector<CAreaOnRing*>* theRingInfo = g_areaSurroundingInfoManager.Get_theAreaSurroundingInfo(this->ID)->Get_ringInfo(k);
            if (theRingInfo == nullptr)
                break;
            int size = theRingInfo->size();
            for (int i = 0; i < size; ++i) {
                CArea* foundArea = g_Scene[(*theRingInfo)[i]->ID];
                if (foundArea != nullptr && foundArea->Enable)
                    listArea->push_back(g_Scene.GetArea((*theRingInfo)[i]->ID));
            }
        }

    }
    return listArea;
}
int SetTargetIsPlayer(CActionAssist* as,CArmy* targetArmy, CArea* targetArea){
    int coefficient = targetArmy->Hp / (targetArmy->MaxHp * 1.0) ;
    if (coefficient >= 0.5)
        coefficient = 1.0;
    if (targetArea->Country != nullptr){
        as->TargetNode.TargetIsPlayer = !targetArea->Country->AI;
        if (as->TargetNode.TargetIsPlayer){
            as->TargetNode.TargetValue += 40;
        }
    }
    else{
        as->TargetNode.TargetIsPlayer = false;
    }
    return coefficient;
}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 得到该地块所有单位的血量总和。<p>
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * void <p>
     *=======================================================<p>
     * 【创建人】： 馅饼<p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： 2022 <p>
     * 【修改时间】： ？？？
     */
int GetAreaArmysStrengthNum(CArea* area, bool isOnlyTop){
    if (area == nullptr){
        return -1;
    }
    int army_max_here = area->ArmyCount;
    if (army_max_here == 0){
        return 0;
    }
    if (isOnlyTop){
        army_max_here = 1;
    }
    int result = 0;
    for (int i = 0; i < army_max_here; i++){
        CArmy *army_tmp = area->GetArmy(i);
        if (army_tmp == NULL){
            return -1;
        }
        result += army_tmp->Hp;
    }
    return result;
}
/*
 * 设置新兵种
 */
def_easytech(_ZN10CObjectDef11LoadArmyDefEv)

void CObjectDef::LoadArmyDef() {
    Init_listCountryArmydef();
//    easytech(_ZN10CObjectDef11LoadArmyDefEv)(this);
    auto *InfantryD_1905 = new ArmyDef{"InfantryD_1905", ArmyDef::InfantryD1905,
                                       60, 2, 1, 6, 0, 0,
                                       0, 60, 35, 3, 10, 1, 1};
    Set_newArmyDef(InfantryD_1905);

    auto *PanzerD_1905 = new ArmyDef{"PanzerD_1905", ArmyDef::Panzer, 90, 3, 1, 7, 0, 0, 0,
                                     60, 35, 3, 8, 1, 0};
    Set_newArmyDef(PanzerD_1905);
    auto *TowedArtilleryD_1905 = new ArmyDef{"TowedArtilleryD_1905", ArmyDef::Artillery, 60, 1,
                                             1, 9, 0, 0, 0, 60, 35, 3, 8, 1, 0};
    Set_newArmyDef(TowedArtilleryD_1905);

    auto *MobileArtilleryD_1916 = new ArmyDef{"MobileArtilleryD_1918", ArmyDef::Rocket,
                                              60, 1,1, 10,
                                              0, 0, 0, 60, 35, 1, 8, 4, 0};
    Set_newArmyDef(MobileArtilleryD_1916);

    auto *LightTankD_1916 = new ArmyDef{"LightTankD_1916", ArmyDef::Tank
                                        , 120, 2, 2, 9,
                                        0, 0,
                                        0, 60, 35, 1, 8, 1, 0};
    Set_newArmyDef(LightTankD_1916);

    auto *MidTankD_1916 = new ArmyDef{"MidTankD_1916", (ArmyDef::ArmyType) 5, 180, 2, 4, 10, 0, 0, 0,
                                      60, 35, 1, 4, 1, 0};
    Set_newArmyDef(MidTankD_1916);

    auto *Destroyer = new ArmyDef{"Destroyer", (ArmyDef::ArmyType) 6,
                                  160, 1, 3, 9, 0, 0, 0, 60, 35,
                                  1, 4, 4, 0};
    Set_newArmyDef(Destroyer);

    auto *Cruiser = new ArmyDef{"Cruiser", (ArmyDef::ArmyType) 7,
                                240, 1, 3, 10, 0, 0, 0, 60, 35, 1,
                                4, 4, 0};
    Set_newArmyDef(Cruiser);

    auto *Battleship = new ArmyDef{"Battleship", (ArmyDef::ArmyType) 8,
                                   320, 1, 4, 11, 0, 0, 0, 60,
                                   35, 1, 4, 4, 0};
    Set_newArmyDef(Battleship);

    auto *AircraftCarrier = new ArmyDef{"AircraftCarrier", (ArmyDef::ArmyType) 9,
                                        220, 1, 1, 7, 0, 0,
                                        0, 60, 35, 1, 4, 4, 0};
    Set_newArmyDef(AircraftCarrier);

    auto *Carrier = new ArmyDef{"AntitankGun", (ArmyDef::ArmyType) 10, 60, 2, 1, 6, 0, 0, 0, 60, 35,
                                1, 4, 4, 0};
    Set_newArmyDef(Carrier);

    auto *AirStrike = new ArmyDef{"AirStrike", (ArmyDef::ArmyType) 11, 60, 2, 1, 6, 0, 0, 0, 60, 35,
                                  1, 4, 4, 0};
    Set_newArmyDef(AirStrike);

    auto *Bomber = new ArmyDef{"Bomber", (ArmyDef::ArmyType) 12, 60, 2, 1, 6, 0, 0, 0, 60, 35, 1, 4,
                               4, 0};
    Set_newArmyDef(Bomber);

    auto *Airborne = new ArmyDef{"Airborne", (ArmyDef::ArmyType) 13, 60, 2, 1, 6, 0, 0, 0, 60, 35,
                                 1, 4, 4, 0};
    Set_newArmyDef(Airborne);

    auto *NuclearBomb = new ArmyDef{"NuclearBomb", (ArmyDef::ArmyType) 14, 60, 2, 1, 6, 0, 0, 0, 60,
                                    35, 1, 4, 4, 0};
    Set_newArmyDef(NuclearBomb);

    auto *Submarine = new ArmyDef{"Submarine", (ArmyDef::ArmyType) 28, 60, 2, 1, 6, 0, 0, 0, 60, 35,
                                  1, 4, 4, 0};
    Set_newArmyDef(Submarine);

    auto *IntercontinentalMissile = new ArmyDef{"IntercontinentalMissile", (ArmyDef::ArmyType) 29,
                                                60, 2, 1, 6, 0, 0, 0, 60, 35, 1, 4, 4, 0};
    Set_newArmyDef(IntercontinentalMissile);

    auto *InfantryA_1905 = new ArmyDef{"InfantryA_1905", (ArmyDef::ArmyType) 30, 60, 2, 1, 6, 0, 0,
                                       0, 60, 2, 1, 4, 4, 0};
    Set_newArmyDef(InfantryA_1905);

    auto *HeavyTankD_1918 = new ArmyDef{"HeavyTankD_1918", (ArmyDef::ArmyType) 31, 60, 2, 1, 6, 0,
                                        0, 0, 60, 35, 1, 4, 4, 0};
    Set_newArmyDef(HeavyTankD_1918);
    auto *Fight_1914 = new ArmyDef{"Fight_1914", (ArmyDef::ArmyType) 32, 60, 2, 1, 6, 0,
                                   0, 0, 60, 35, 1, 4, 4, 0};
    Set_newArmyDef(Fight_1914);
    auto *Boom_1914 = new ArmyDef{"Boom_1914", (ArmyDef::ArmyType) 33, 60, 2, 1, 6, 0,
                                  0, 0, 60, 35, 1, 4, 4, 0};
    Set_newArmyDef(Boom_1914);
}

void Init_listCountryArmydef() {
    CountryArmyDef *newArmyDef = new CountryArmyDef();
    strcpy(newArmyDef->CountryName, "other");
    s_ListCountryArmydef.push_back(newArmyDef);
}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 得到指定国家指定军队id的armyDef。如果该国家的这个军队id未独立设定数据，会返回通用数据。<p>
     * 【参数说明】：<p>
     * armyDefID：请确定该值一定被记录。<p>
     * CountryName：请确定该值一定被记录。<p>
     * 【返回值说明】：<p>
     * 指定的armydef。 <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： ？？？<p>
     * 【修改时间】：？？？
     */
ArmyDef *Get_newArmyDef(int armyDefID, const char *CountryName) {
    int size = s_ListCountryArmydef.size();
    for (int i = 0; i < size; ++i) {
        if (strcmp(s_ListCountryArmydef[i]->CountryName, CountryName) == 0) {
            //如果找到了一定要有对应的armydef
            if(s_ListCountryArmydef[i]->mapArmydef.count(armyDefID))
                return s_ListCountryArmydef[i]->mapArmydef[armyDefID];
            break;
        }
    }
    //如果找不到返回默认设置
    return s_ListCountryArmydef[0]->mapArmydef[armyDefID];
}

void Add_newArmyDef_toList(ArmyDef *armydef, char *countryName) {
    if (strcmp(countryName, "other") == 0) {
        //设置到默认
        s_ListCountryArmydef[0]->mapArmydef[armydef->ID] = armydef;
        return;
    }
    //不是通用国家将执行下面这个代码
    int size = s_ListCountryArmydef.size();
    for (int i = 0; i < size; ++i) {
        //如果这个国家存在
        if (strcmp(s_ListCountryArmydef[i]->CountryName, countryName) == 0) {
            //替换数据
            s_ListCountryArmydef[i]->mapArmydef[armydef->ID] = armydef;
            return;
        }
    }
    //执行到这里，则这个国家并未找到，创建它
    CountryArmyDef *newArmyDef = new CountryArmyDef();
    strcpy(newArmyDef->CountryName,countryName);
    //先把通用国家的数据移植过来
    for (int i = 0; i < s_ListCountryArmydef[0]->mapArmydef.size(); ++i) {
        newArmyDef->mapArmydef[i] = s_ListCountryArmydef[0]->mapArmydef[i];
    }
    //再把指定的军队数据替换进去
    newArmyDef->mapArmydef[armydef->ID] = armydef;
    s_ListCountryArmydef.push_back(newArmyDef);

}

void ResetArmyDef(ArmyDef *armyDef, int extraHp, int extraMinAttack, int extraMaxAttack,
                  int extraMovement) {

    armyDef->Hp += extraHp;
    armyDef->MinAttack += extraMinAttack;
    armyDef->MaxAttack += extraMaxAttack;
    armyDef->Movement += extraMovement;
}

void Set_newArmyDef(ArmyDef *armydef) {
    //通用先加一遍

    Add_newArmyDef_toList(armydef, "other");
    if (armydef->ID == ArmyDef::Submarine) {
        //通过拷贝构造函数快速赋值
        ArmyDef *newArmydef = new ArmyDef(*armydef);
        ResetArmyDef(newArmydef, 20, 1, 1, 0);
    }
    else if (armydef->ID == ArmyDef::HeavyTank) {
        ArmyDef *newArmydef = new ArmyDef(*armydef);
        ResetArmyDef(newArmydef, 20, 1, 1, 0);
        Add_newArmyDef_toList(newArmydef, "de");
        newArmydef = new ArmyDef(*armydef);
        ResetArmyDef(newArmydef, 30, 0, 1, 0);
        Add_newArmyDef_toList(newArmydef, "ru");
    }
    else if (armydef->ID == ArmyDef::Tank) {
        ArmyDef *newArmydef = new ArmyDef(*armydef);
        ResetArmyDef(newArmydef, 10, 1, 0, 0);
        Add_newArmyDef_toList(newArmydef, "de");
        newArmydef = new ArmyDef(*armydef);
        ResetArmyDef(newArmydef, 30, 1, 0, 0);
        Add_newArmyDef_toList(newArmydef, "ru");
    }
    else if (armydef->ID == ArmyDef::Panzer) {
        ArmyDef *newArmydef = new ArmyDef(*armydef);
        ResetArmyDef(newArmydef, 10, 1, 0, 0);
        Add_newArmyDef_toList(newArmydef, "de");
        newArmydef = new ArmyDef(*armydef);
        ResetArmyDef(newArmydef, 10, 0, 0, 0);
        Add_newArmyDef_toList(newArmydef, "ru");
    }
    else if (armydef->ID == ArmyDef::InfantryD1905) {
        ArmyDef *newArmydef = new ArmyDef(*armydef);
        ResetArmyDef(newArmydef, 20, 0, 1, 0);
        Add_newArmyDef_toList(newArmydef, "ja");
         newArmydef = new ArmyDef(*armydef);
        ResetArmyDef(newArmydef, 20, 0, 1, 0);
        Add_newArmyDef_toList(newArmydef, "cn");
        newArmydef = new ArmyDef(*armydef);
        ResetArmyDef(newArmydef, 20, 0, 0, 0);
        Add_newArmyDef_toList(newArmydef, "ru");
    }
    else if (armydef->ID == ArmyDef::Artillery || armydef->ID == ArmyDef::Rocket) {
        ArmyDef *newArmydef = new ArmyDef(*armydef);
        ResetArmyDef(newArmydef, 0, 1, 1, 0);
        Add_newArmyDef_toList(newArmydef, "ru");
        newArmydef = new ArmyDef(*armydef);
        newArmydef = new ArmyDef(*armydef);
        ResetArmyDef(newArmydef, 0, 1, 1, 0);
        Add_newArmyDef_toList(newArmydef, "am");
        newArmydef = new ArmyDef(*armydef);
    }
    else if (armydef->ID == ArmyDef::AircraftCarrier) {
        ArmyDef *newArmydef = new ArmyDef(*armydef);
        ResetArmyDef(newArmydef, 0, 1, 1, 0);
        Add_newArmyDef_toList(newArmydef, "am");
        newArmydef = new ArmyDef(*armydef);
        ResetArmyDef(newArmydef, 0, 0, 1, 0);
        Add_newArmyDef_toList(newArmydef, "gb");
    }
}

def_easytech(_ZN10CObjectDef10GetArmyDefEiPKc)
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 得到指定国家指定军队id 的armyDef。请确保该军队id存在。如果该国家的这个军队id未独立设定数据，会返回通用数据。<p>
     * 【参数说明】：<p>
     * armyDefID：请确定该值一定被记录。<p>
     * CountryName：请确定该值一定被记录。<p>
     * 【返回值说明】：<p>
     * 指定的armydef，最好做非空判断。 <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： ？？？<p>
     * 【修改时间】： ？？？
     */
ArmyDef *CObjectDef::GetArmyDef(int ArmyType, const char *CountryName) {
//    if (servicesType >= NewArmyStartID){
    return Get_newArmyDef(ArmyType, CountryName);
//    }
//    return easytech(_ZN10CObjectDef10GetArmyDefEiPKc)(this, servicesType, CountryName);
}

def_easytech(_ZN10CObjectDef11LoadCardDefEv)


void CObjectDef::LoadCardDef() {

    easytech(_ZN10CObjectDef11LoadCardDefEv)(this);
    g_areaSurroundingInfoManager.Init();
    g_areaSurroundingInfoManager.InitListAreaSurroundingInfo();
    LoadMusic();
//    for (CardDef Card : CardDef28)
//        NewCardDef.push_back(Card);

    //设置各个国家的卡牌冷却回合在   Init_countryCardRound
    auto *Infantry = new CardDef{(string *) "InfantryD1905", (string *) "card_0.png",
                                 (string *) "InfantryD1905 Intro", CARD_ID::InfantryCard, CardDef::Army,
                                 75,0, 0, 1};
    NewCardDef.push_back(Infantry);
    auto *Panzer = new CardDef{(string *) "Panzer", (string *) "card_1.png",
                               (string *) "Panzer Intro", CARD_ID::ArmourCard, CardDef::Army,
                               140, 50, 0, 1};
    NewCardDef.push_back(Panzer);
    auto *Artillery = new CardDef{(string *) "Artillery", (string *) "card_2.png",
                                  (string *) "Artillery Intro", CARD_ID::ArtilleryCard, CardDef::Army,
                                  125, 40,0, 1};
    NewCardDef.push_back(Artillery);
    auto *Rocket = new CardDef{(string *) "Rocket", (string *) "card_3.png",
                               (string *) "Rocket Intro", CARD_ID::RocketCard, CardDef::Army,
                               240, 70, 3, 1};
    NewCardDef.push_back(Rocket);
    auto *Tank = new CardDef{(string *) "Tank", (string *) "card_4.png",
                             (string *) "Tank Intro",CARD_ID::TankCard, CardDef::Army,
                             220, 80, 0, 1};
    NewCardDef.push_back(Tank);
    auto *MediumTank = new CardDef{(string *) "MediumTank", (string *) "card_5.png",
                                   (string *) "MediumTank Intro", CARD_ID::MediumTankCard, CardDef::Army,
                                   300,150, 0, 3};
    NewCardDef.push_back(MediumTank);
    auto *Destroyer = new CardDef{(string *) "Destroyer", (string *) "card_6.png",
                                  (string *) "Destroyer Intro", (CARD_ID) DestroyerCard, CardDef::Navy,
                                  250,60, 3, 2};
    NewCardDef.push_back(Destroyer);
    auto *Cruiser = new CardDef{(string *) "Cruiser", (string *) "card_7.png",
                                (string *) "Cruiser Intro", (CARD_ID) 7, CardDef::Navy,
                                350, 100, 3,3};
    NewCardDef.push_back(Cruiser);
    auto *Battleship = new CardDef{(string *) "Battleship", (string *) "card_8.png",
                                   (string *) "Battleship Intro", (CARD_ID) 8, CardDef::Navy,
                                   450, 150, 3, 3};
    NewCardDef.push_back(Battleship);
    auto *AircraftCarrier = new CardDef{(string *) "AircraftCarrier", (string *) "card_9.png",
                                        (string *) "AircraftCarrier Intro", (CARD_ID) 9,CardDef::Navy,
                                        300, 150, 3, 4};
    NewCardDef.push_back(AircraftCarrier);
    auto *Airstrike = new CardDef{(string *) "Airstrike", (string *) "card_airstrike.png",
                                  (string *) "Airstrike Intro", (CARD_ID) 10, CardDef::AirForce,
                                  300, 150, 3, 4};
    NewCardDef.push_back(Airstrike);
    auto *Bomber = new CardDef{(string *) "Bomber", (string *) "card_bomber.png",
                               (string *) "Bomber Intro", (CARD_ID) 11, CardDef::AirForce,
                               300, 150,3, 4};
    NewCardDef.push_back(Bomber);
    auto *Airborneforce = new CardDef{(string *) "Airborneforce",
                                      (string *) "card_airborneforce.png",
                                      (string *) "Airborneforce Intro", (CARD_ID) 12,CardDef::AirForce,
                                      300, 150, 3, 4};
    NewCardDef.push_back(Airborneforce);
    auto *Nuclearbomb = new CardDef{(string *) "Nuclearbomb", (string *) "card_nuclearbomb.png",
                                    (string *) "Nuclearbomb Intro", (CARD_ID) 13,  CardDef::AirForce,
                                    300, 150, 3, 4};
    NewCardDef.push_back(Nuclearbomb);
    auto *City = new CardDef{(string *) "City", (string *) "card_city.png",
                             (string *) "City Intro",(CARD_ID) 14, CardDef::Development,
                             60, 0, 3, 1};
    NewCardDef.push_back(City);
    auto *Industry = new CardDef{(string *) "Indusry", (string *) "card_industry.png",
                                 (string *) "Indusry Intro", (CARD_ID) 15, CardDef::Development,
                                 50, 10, 3, 1};
    NewCardDef.push_back(Industry);
    auto *Airport = new CardDef{(string *) "Airport", (string *) "card_airport.png",
                                (string *) "Airport Intro", (CARD_ID) 16, CardDef::Development,
                                300,150, 3, 1};
    NewCardDef.push_back(Airport);
    auto *LandFort = new CardDef{(string *) "LandFort", (string *) "card_landfort.png",
                                 (string *) "LandFort Intro", (CARD_ID) 17, CardDef::Development,
                                 300, 150, 3, 4};
    NewCardDef.push_back(LandFort);
    auto *Entrenchment = new CardDef{(string *) "Entrenchment", (string *) "card_entrenchment.png",
                                     (string *) "Entrenchment Intro", (CARD_ID) 18,CardDef::Development,
                                     150, 0, 3, 1};
    NewCardDef.push_back(Entrenchment);
    auto *Antiaircraft = new CardDef{(string *) "Antiaircraft", (string *) "card_antiaircraft.png",
                                     (string *) "Antiaircraft Intro", (CARD_ID) 19, CardDef::Development,
                                     300, 150, 3, 1};
    NewCardDef.push_back(Antiaircraft);
    auto *Radar = new CardDef{(string *) "Radar", (string *) "card_radar.png",
                              (string *) "Radar Intro", (CARD_ID) 20, CardDef::Development,
                              300,150, 3, 3};
    NewCardDef.push_back(Radar);
    auto *Research = new CardDef{(string *) "Research", (string *) "card_research.png",
                                 (string *) "Research Intro", (CARD_ID) 21, CardDef::Strategy,
                                 300, 150, 3, 4};
    NewCardDef.push_back(Research);

//反坦克炮
    auto *AGRCard = new CardDef{(string *) "AGRCard", (string *) "card_35.png",
                                (string *) "AGRCard Intro", (CARD_ID) 22, CardDef::Strategy,
                                9, 30,0, 4};
    NewCardDef.push_back(AGRCard);

    auto *LARCard = new CardDef{(string *) "LARCard", (string *) "card_23.png",
                                (string *) "LARCard Intro", (CARD_ID) 23, CardDef::Strategy,
                                30,1, 0, 1};
    NewCardDef.push_back(LARCard);
    auto *ECRCard = new CardDef{(string *) "ECRCard", (string *) "card_24.png",
                                (string *) "ECRCard Intro", (CARD_ID) 24, CardDef::Strategy
                                ,  40,0, 0, 1};
    NewCardDef.push_back(ECRCard);
    auto *Commander = new CardDef{(string *) "Commander", (string *) "card_commander.png",
                                  (string *) "Commander Intro", (CARD_ID) 25, CardDef::Strategy
                                  , 40, 2, 3, 4};
    NewCardDef.push_back(Commander);
    auto *Supplyline = new CardDef{(string *) "Supplyline", (string *) "card_supplyline.png",
                                   (string *) "Supplyline Intro", (CARD_ID) 26, CardDef::Strategy
                                   ,180, 80, 4, 1};
    NewCardDef.push_back(Supplyline);
    auto *Aceforce = new CardDef{(string *) "Aceforce", (string *) "card_aceforce.png",
                                 (string *) "Aceforce Intro", (CARD_ID) 27, CardDef::Strategy
                                 , 180,80, 3, 4};
    NewCardDef.push_back(Aceforce);

    // 添加新卡牌
    auto *HeavyTankCard = new CardDef{(string *) "MediumTankCard", (string *) "card_32.png",
                                      (string *) "MediumTankCard Intro", (CARD_ID) 28,CardDef::Army
                                      , 300, 150, 3, 4};
    NewCardDef.push_back(HeavyTankCard);
    auto *BiochemicalForceCard = new CardDef{(string *) "BiochemicalForceCard",(string *) "card_29.png",
                                             (string *) "BiochemicalForceCard Intro", (CARD_ID) 29, CardDef::Army,
                                             9, 30, 1, 4};
    NewCardDef.push_back(BiochemicalForceCard);

    auto *SubmarineCard = new CardDef{(string *) "SubmarineCard", (string *) "card_28.png",
                                      (string *) "SubmarineCard Intro", (CARD_ID) 30, CardDef::Navy
                                      ,9, 30, 0, 4};
    NewCardDef.push_back(SubmarineCard);
    //从此开始
    auto *ExpendCard = new CardDef{(string *) "ExpendCard", (string *) "card_30.png",
                                   (string *) "ExpendCard Intro", (CARD_ID) 31, CardDef::Development
                                   , 9, 30, 0, 4};
    NewCardDef.push_back(ExpendCard);
    auto *SplitCard = new CardDef{(string *) "SplitCard", (string *) "card_31.png",
                                  (string *) "SplitCard Intro", (CARD_ID) 32, CardDef::Development
                                  ,9, 30, 0, 4};
    NewCardDef.push_back(SplitCard);
    auto *AirDefenseCard = new CardDef{(string *) "AirDefenseCard", (string *) "card_33.png",
                                       (string *) "AirDefenseCard Intro", (CARD_ID) 33,CardDef::AirForce
                                       , 9, 30, 0, 4};
    NewCardDef.push_back(AirDefenseCard);
    auto *HARCard = new CardDef{(string *) "HARCard", (string *) "card_34.png",
                                (string *) "HARCard Intro", (CARD_ID) 34, CardDef::Strategy
                                , 9, 30,0, 4};
    NewCardDef.push_back(HARCard);

    auto *TDRCard = new CardDef{(string *) "TDRCard", (string *) "card_36.png",
                                (string *) "TDRCard Intro", (CARD_ID) 35, CardDef::Strategy
                                , 1,1, 0, 4};
    NewCardDef.push_back(TDRCard);
    auto *TTRCard = new CardDef{(string *) "TTRCard", (string *) "card_37.png",
                                (string *) "TTRCard Intro", (CARD_ID) 36, CardDef::Strategy
                                , 9, 30,0, 4};
    NewCardDef.push_back(TTRCard);
    auto *MARCard = new CardDef{(string *) "MARCard", (string *) "card_38.png",
                                (string *) "MARCard Intro", (CARD_ID) 37, CardDef::Strategy
                                , 9, 30,0, 4};
    NewCardDef.push_back(MARCard);


}

CardDef *CObjectDef::GetCardDef(CARD_ID ID) {
    return NewCardDef[ID];
}

def_easytech(_ZN7GUICardC2Ev)

GUICard *BuildCard(CARD_ID ID, GUIRect LastCardRect, float CardWidth) {
    CObjectDef *objectDef = CObjectDef::Instance();
    if (objectDef) {
        auto *Card = new GUICard;
        easytech(_ZN7GUICardC2Ev)(Card);
        Card->Init(objectDef->GetCardDef(ID),
                   {LastCardRect.Pos[0] + CardWidth, LastCardRect.Pos[1], LastCardRect.Size[0],
                    LastCardRect.Size[1]});
        return Card;
    }
    return nullptr;
}
//此处无需改动
def_easytech(_ZN11GUICardList4InitERK7GUIRecti)
//此函数在进入剧本前被调用,一共被调用五次，分别对应五个卡牌类别
void GUICardList::Init(GUIRect const &Rect, int Type) {
    easytech(_ZN11GUICardList4InitERK7GUIRecti)(this, Rect, Type);
    GUIRect lastCardRect = data[data.size() - 1]->Rect;
    float CardWidth = *(float *) ((int *) this + 68 / 4);
    int cardID = 0;
    if (Type == CardDef::Army){
        for(auto card : s_ListNewCardIndex)
            delete card;
        s_ListNewCardIndex.clear();
    }


    for (auto & cardDef : NewCardDef) {
        //如果符合本次需要添加的类别，添加进去
        if (cardDef->Type == Type) {
            GUICard *Card;
            if (IsNewCard(cardDef->ID)) {
                Card = BuildCard(cardDef->ID, lastCardRect, CardWidth);
                AddChild(Card, true);
                data.push_back(Card);
            }
            s_ListNewCardIndex.push_back(new int[2]{Type, cardID});
            s_ArrCardType[Type][cardID++] = cardDef->ID;
        }
    }

}



//此处无需修改
def_easytech(_ZN10GUIBuyCard14ResetCardStateEv)

void GUIBuyCard::ResetCardState() {

    for (auto &arrIndex : s_ListNewCardIndex) {
        CCountry *Country = g_GameManager.GetCurCountry();

        int indexX = arrIndex[0];
        int indexY = arrIndex[1];
        CardDef *Card = NewCardDef[s_ArrCardType[indexX][indexY]];
        GUICard *GUI = CardListGUI[arrIndex[0]]->GetCard(arrIndex[1]);
        if (Country == g_GameManager.GetPlayerCountry()){
            GUI->SetPrice(Country->GetCardPrice(Card));
            GUI->SetIndustry(Country->GetCardIndustry(Card));

        }

        int *Enable = ((int *) GUI + 292 / 4);
        int *Tech = ((int *) GUI + 284 / 4);
        int *Round = ((int *) GUI + 70);
        *Round = Country->Arr_cardCD_inTheTurn[Card->ID];
        if (GUI != nullptr) {
            *Enable = 1;
            *Tech = 0;
            if (Card->Tech > Country->Tech) {
                *Enable = 0;
                *Tech = Card->Tech;
            }
            if (Country->IsEnoughMoney(Card)) {
                GUI->SetPriceColor(0xFF0F2632);
            }
            else {
                *Enable = 0;
                GUI->SetPriceColor(0xFF000080);
            }
            if (Country->IsEnoughIndustry(Card)) {
                GUI->SetIndustryColor(0xFF0F2632);
            }
            else {
                *Enable = 0;
                GUI->SetIndustryColor(0xFF000080);
            }
        }
    }
}

//此处无需修改
void GUIBuyCard::SelCard(int CardTab, int Index) {
    if (Index < 0)
        Index = 0;
    IntroText.SetText("");
    NameText.SetText("");
    SelCardID = s_ArrCardType[CardTab][Index];
    SetSelCardIntro();
    if (GetSelCard() != nullptr&&CanBuySelCard())
        ButtonOK->SetEnable(true);
    else
        ButtonOK->SetEnable(false);
}
//游戏存档相关，十分重要。如果国家、地块、军队等类有新成员，要记得存档时把它们的数据保存下来！
void CGameManager::SaveGame(const char *FileName) {
    ecFile *File = new ecFile();
    if ((GameMode == Campaign||GameMode == Conquest)&&
        File->Open(GetDocumentPath(FileName), "wb")) {
        SaveHeader Header{};
        time_t timer;
        time(&timer);
        tm *tm = localtime(&timer);
        strcpy(Header.VerificationCode, "YSAE");
        Header.FileVersion = 1;
        Header.GameMode = GameMode;
        Header.AlwaysZero = AlwaysZero;
        Header.MapID = MapID;
        memset(Header.AreasEnable, 0, sizeof(Header.AreasEnable));
        strcpy(Header.AreasEnable, AreasEnable);
        for (int i = 0; i != 8; i += 2) {
            memset(Header.PlayerCountryName[i], 0, sizeof(Header.PlayerCountryName[i]));
            strcpy(Header.PlayerCountryName[i], PlayerCountryName[i]);
        }
        memset(Header.BattleFileName, 0, sizeof(Header.BattleFileName));
        strcpy(Header.BattleFileName, BattleFileName);
        Header.Year = tm->tm_year + 1900;
        Header.CenterPos[0] = g_Scene.CCamera.CenterPos[0];
        Header.CenterPos[1] = g_Scene.CCamera.CenterPos[1];
        Header.Scale = g_Scene.CCamera.Scale;
        Header.CurrentCountryIndex = CurrentCountryIndex;
        Header.CurrentDialogueIndex = CurrentDialogueIndex;
        Header.AreaNum = 0;
        Header.CountryNum = ListCountry.size();
        Header.CurrentTurnNumMinusOne = CurrentTurnNumMinusOne;
        Header.RandomRewardMedal = RandomRewardMedal;
        Header.Month = tm->tm_mon + 1;
        Header.Day = tm->tm_mday;
        Header.Hour = tm->tm_hour;
        Header.Second = tm->tm_min;
        Header.CampaignSeriesID = CampaignSeriesID;
        Header.CampaignLevelID = CampaignLevelID;
        Header.VictoryTurn = VictoryTurn;
        Header.GreatVictoryTurn = GreatVictoryTurn;
        Header.isIntelligentAI = IsIntelligentAI; //AI增强
        Header.g_Seed = G_Seed;
        Header.GameDifficulty = GameDifficulty;
        Header.IsPause = IsPause;
        auto events  = g_commonEvent.Get_events_turnBegin();
        int f = 0;
        for(auto event : *events){
            Header.Arr_eventTurnBegin_ofCommonEvents_isFinished[f++] = event->isFinished;
        }
        SaveCountryInfo* CountriesInfo = new SaveCountryInfo[Header.CountryNum];
        for (int CountryIndex = 0; CountryIndex < Header.CountryNum; CountryIndex++) {
            ListCountry[CountryIndex]->SaveCountry(&CountriesInfo[CountryIndex]);
            Header.AreaNum += ListCountry[CountryIndex]->AreaIDList.size();
        }

        SaveAreaInfo* AreasInfo= new SaveAreaInfo [Header.AreaNum];
        for (int CountryIndex = 0, AreaInfoIndex = 0;
             CountryIndex < Header.CountryNum; CountryIndex++) {
            for (int AreaID : ListCountry[CountryIndex]->AreaIDList) {
                AreasInfo[AreaInfoIndex].Country = CountryIndex;
                g_Scene.GetArea(AreaID)->SaveAera(&AreasInfo[AreaInfoIndex++]);
            }
        }
        File->Write(&Header, sizeof(Header));
        File->Write(CountriesInfo, sizeof(SaveCountryInfo)* Header.CountryNum);
        File->Write(AreasInfo, sizeof(SaveAreaInfo) * Header.AreaNum);
        delete[] CountriesInfo;
        delete[] AreasInfo;
        File->Close();
    }
    delete File;
}


void GUITax::OnRender() {
    GUIRect Rect{};
    GUIElement::GetAbsRect(Rect);
    if (ImageCommander != nullptr) {
        Image_income_board2->Render(Rect.Pos[0], Rect.Pos[1] + 6.0f);
        ImageCommander->Render(Rect.Pos[0], Rect.Pos[1] + 191.0f);
        if (Country != nullptr&&Country->AI) {
            g_GameRes.RenderAICommanderMedal(1, Rect.Pos[0] + 60.0f, Rect.Pos[1] + 195.0f,
                                             Country->Name, Country->Alliance);
        }
        else {
            g_GameRes.RenderCommanderMedal(1, Rect.Pos[0] + 60.0f, Rect.Pos[1] + 195.0f,
                                           Country->GetCommanderLevel());
        }
    }
    else {
        Image_income_board->Render(Rect.Pos[0], Rect.Pos[1] + 7.0f);
    }
    if (InstalltionType > 0)
        Image_stamp[InstalltionType - 1]->Render(Rect.Pos[0] + 55.0f, Rect.Pos[1] + 36.0f);
    MoneyText.DrawText(Rect.Pos[0] + 50.0f, Rect.Pos[1] + 25.0f, 1);
    IndustryText.DrawText(Rect.Pos[0] + 50.0f, Rect.Pos[1] + 44.0f, 1);
// 下面的操作还要去GUITax::Ini
    sprintf(TempStr, "%d", g_Scene.GetSelectedArea()->Get_oil());
    OilTaxText->SetText(TempStr);
    OilTaxText->DrawText(Rect.Pos[0] + 50.0f, Rect.Pos[1] + 64.0f, 1);
    sprintf(TempStr, "%d", g_Scene.GetSelectedArea()->Get_cityGrowthRate());
    CityGrowthRateText->SetText(TempStr);
    CityGrowthRateText->DrawText(Rect.Pos[0] + 50.0f, Rect.Pos[1] + 84.0f, 1);
    sprintf(TempStr, "%d", g_Scene.GetSelectedArea()->Get_industryGrowthRate());
    IndustryGrowthRateText->SetText(TempStr);
    IndustryGrowthRateText->DrawText(Rect.Pos[0] + 50.0f, Rect.Pos[1] + 104.0f, 1);
    sprintf(TempStr, "%d", g_Scene.GetSelectedArea()->Get_oilGrowthRate());
    OilGrowthRateText->SetText(TempStr);
    OilGrowthRateText->DrawText(Rect.Pos[0] + 50.0f, Rect.Pos[1] + 124.0f, 1);
}

def_easytech(_ZN6GUITax4InitERK7GUIRect)

void GUITax::Init(const GUIRect &Rect) {
    easytech(_ZN6GUITax4InitERK7GUIRect)(this, &Rect);
    delete OilTaxText;
    delete CityGrowthRateText;
    delete IndustryGrowthRateText;
    delete OilGrowthRateText;
    OilTaxText = new ecText();
    OilTaxText->Init(&g_Num3);
    OilTaxText->SetColor(0xFF0F2632);
    CityGrowthRateText = new ecText();
    CityGrowthRateText->Init(&g_Num3);
    CityGrowthRateText->SetColor(0xFF0F2632);
    IndustryGrowthRateText = new ecText();
    IndustryGrowthRateText->Init(&g_Num3);
    IndustryGrowthRateText->SetColor(0xFF0F2632);
    OilGrowthRateText = new ecText();
    OilGrowthRateText->Init(&g_Num3);
    OilGrowthRateText->SetColor(0xFF0F2632);
}

def_easytech(_ZN7GUIGold4InitERK7GUIRect)

void GUIGold::Init(const GUIRect &Rect) {
    easytech(_ZN7GUIGold4InitERK7GUIRect)(this, &Rect);
    delete OilGoldText;
    OilGoldText = new ecText();
    OilGoldText->Init(&g_Num3);
    OilGoldText->SetColor(0xFF0F2632);
}

def_easytech(_ZN7GUIGold8OnRenderEv)

void GUIGold::OnRender() {
    easytech(_ZN7GUIGold8OnRenderEv)(this);
    GUIRect Rect{};
    GetAbsRect(Rect);
    sprintf(TempStr, "%d", g_GameManager.GetCurCountry()->Oil);
    OilGoldText->SetText(TempStr);
    OilGoldText->DrawText(Rect.Pos[0] + 107.0f, Rect.Pos[1] + 3.0f, 0);
}

def_easytech(_ZN8GUIBegin4InitERK7GUIRect)

void GUIBegin::Init(const GUIRect &Rect) {
    easytech(_ZN8GUIBegin4InitERK7GUIRect)(this, &Rect);
    delete OilCollectText;
    OilCollectText = new ecText();
    OilCollectText->Init(&g_Num4);
    OilCollectText->SetColor(0xFF0F2632);
}

def_easytech(_ZN8GUIBegin8OnRenderEv)

void GUIBegin::OnRender() {
    easytech(_ZN8GUIBegin8OnRenderEv)(this);
    GUIRect Rect{};
    GetAbsRect(Rect);
    sprintf(TempStr, "%d", g_GameManager.GetCurCountry()->Get_oilMultiplied_fromAreas());
    OilCollectText->SetText(TempStr);
    OilCollectText->DrawText(Rect.Pos[0] + 74.0f, Rect.Pos[1] + 153.0f, 0);
}

void CGameManager::RealLoadGame(const char *FileName) {

    ClearBattle();
    g_Scene.Init(AreasEnable, MapID);
    ecFile *File = new ecFile();
    const char *Path = GetDocumentPath(FileName);
    if (File->Open(Path, "rb")) {
        SaveHeader Header{};
        File->Read(&Header, sizeof(Header));
        SaveCountryInfo CountriesInfo[Header.CountryNum];
        File->Read(CountriesInfo, sizeof(CountriesInfo));
        SaveAreaInfo AreasInfo[Header.AreaNum];
        File->Read(AreasInfo, sizeof(AreasInfo));
        File->Close();
        for (int i = 0; i < Header.CountryNum; i++) {
            CCountry *country = new CCountry();
            country->Init(CountriesInfo[i].ID, CountriesInfo[i].Name);
            country->LoadCountry(&CountriesInfo[i]);
            country->Oil = CountriesInfo[i].Oil;   // ÐÂÔö
            this->ListCountry.push_back(country);
            if (country->Alliance != 4)
                DefeatCountry.push_back(country);
        }
        Init_resolutions_theScript(Header.CampaignSeriesID);
        for (int j = 0; j < g_GameManager.ListCountry.size(); ++j) {
            SaveCountryInfo *countryInfo = &CountriesInfo[j];
            CCountry *country = g_GameManager.ListCountry[j];
            for (int i = 0; i < countryInfo->eventOccupyAreasNum; ++i) {
                country->List_event_moveArmyTo[i]->isFinished = countryInfo->arr_eventFinished_moveArmyTo[i];
            }
            for (int i = 0; i < countryInfo->eventTurnBeginsNum; ++i) {
                country->List_event_turnBegin[i]->isFinished = countryInfo->arr_eventFinished_turnBegin[i];
            }
            for (int i = 0; i < countryInfo->eventResolutionsNum; ++i) {
                country->List_resolution[i]->isFinished = countryInfo->arr_eventFinished_resolution[i];
            }
        }
        int f = 0;
        auto events = g_commonEvent.Get_events_turnBegin();
        for(auto event :*events){
            event->isFinished = Header.Arr_eventTurnBegin_ofCommonEvents_isFinished[f++];
        }
        for (int i = 0; i < Header.AreaNum; i++) {
            CArea *Area = g_Scene.GetArea(AreasInfo[i].ID);
            Area->Country = ListCountry[AreasInfo[i].Country];
            Area->LoadArea(&AreasInfo[i]);
            Area->Country->AddArea(Area->ID);
        }
        CurrentCountryIndex = Header.CurrentCountryIndex;
        CampaignSeriesID = Header.CampaignSeriesID;
        CampaignLevelID = Header.CampaignLevelID;
        CurrentTurnNumMinusOne = Header.CurrentTurnNumMinusOne;
        RandomRewardMedal = Header.RandomRewardMedal;
        CurrentDialogueIndex = Header.CurrentDialogueIndex;
        VictoryTurn = Header.VictoryTurn;
        GreatVictoryTurn = Header.GreatVictoryTurn;
        IsIntelligentAI = Header.isIntelligentAI; //Ai增强
        G_Seed = Header.g_Seed;
        GameDifficulty = Header.GameDifficulty;
        IsPause = Header.IsPause;
        g_Scene.CCamera.SetPos(Header.CenterPos[0], Header.CenterPos[1], false);
        g_Scene.CCamera.Scale = Header.Scale;

    }
    delete File;
}

// ÐÂÔö
def_easytech(_ZN8CCountry16CollectIndustrysEv)

void CCountry::SaveCountry(SaveCountryInfo *countryInfo) {
    countryInfo->Money = Money;
    countryInfo->Industry = Industry;
    countryInfo->Oil = Oil;
    countryInfo->Tech = Tech;
    countryInfo->TechTurn = TechTurn;
    countryInfo->AI = AI;
    memcpy(countryInfo->CardRound, CardRound, sizeof(CardRound));
    strcpy(countryInfo->ID, this->ID);
    strcpy(countryInfo->Name, this->Name);
    countryInfo->R = R;
    countryInfo->G = G;
    countryInfo->B = B;
    countryInfo->A = A;
    countryInfo->Alliance = Alliance;
    countryInfo->DefeatType = DefeatType;
    countryInfo->TaxRate = TaxRate;
    memcpy(countryInfo->DestroyCount, DestroyCount, sizeof(DestroyCount));
    memset(countryInfo->CommanderName, 0, sizeof(countryInfo->CommanderName));
    if (PvCommanderDef != nullptr)
        strcpy(countryInfo->CommanderName, PvCommanderDef->name.c_str());
    countryInfo->CommanderTurn = this->CommanderTurn;
    countryInfo->CommanderAlive = this->CommanderAlive;
    memcpy(countryInfo->WarMedal, WarMedal, sizeof(WarMedal));
    countryInfo->BorrowedLoan = this->BorrowedLoan;
    countryInfo->Defeated = this->Defeated;
    countryInfo->IsMaritime = this->IsCoastal;
    int listArmisticeStatusCount = 0;
    for (int i = 0; i < g_GameManager.ListCountry.size(); ++i) {
        if (ListArmisticeStatus[g_GameManager.ListCountry[i]->ID]) {
            strcpy(countryInfo->listArmisticeStatus[i].CountryName, g_GameManager.ListCountry[i]->ID);
            countryInfo->listArmisticeStatus[i].Round = ListArmisticeStatus[g_GameManager.ListCountry[i]->ID];
            listArmisticeStatusCount++;
        }
    }
    countryInfo->listArmisticeStatusCount = listArmisticeStatusCount;
    countryInfo->listArrDesignationsLand_Count = this->ListDesignationsLand.size();
    countryInfo->listArrDesignationsAir_Count = this->ListDesignationsAir.size();
    countryInfo->listArrDesignationsSea_Count = this->ListDesignationsSea.size();
    countryInfo->listArrDesignationsTank_Count = this->ListDesignationsTank.size();
    for (int i = 0; i < countryInfo->listArrDesignationsLand_Count; ++i) {
        countryInfo->arr_land[i] = this->ListDesignationsLand[i];
    }
    for (int i = 0; i < countryInfo->listArrDesignationsAir_Count; ++i) {
        countryInfo->arr_air[i] = this->ListDesignationsAir[i];
    }
    for (int i = 0; i < countryInfo->listArrDesignationsSea_Count; ++i) {
        countryInfo->arr_sea[i] = this->ListDesignationsSea[i];
    }
    for (int i = 0; i < countryInfo->listArrDesignationsTank_Count; ++i) {
        countryInfo->arr_tank[i] = this->ListDesignationsTank[i];
    }
    countryInfo->MaxArmyDesignationsLand = this->MaxArmyDesignationsLand;
    countryInfo->MaxArmyDesignationsAir = this->MaxArmyDesignationsAir;
    countryInfo->MaxArmyDesignationsSea = this->MaxArmyDesignationsSea;
    countryInfo->MaxArmyDesignationsTank = this->MaxArmyDesignationsTank;
    countryInfo->eventOccupyAreasNum = this->List_event_moveArmyTo.size();
    countryInfo->eventTurnBeginsNum = this->List_event_turnBegin.size();
    countryInfo->eventResolutionsNum = this->List_resolution.size();
    countryInfo->SpecificMark = this->SpecialMark;
    countryInfo->OilConversionRate = this->OilConversionRate;
    memcpy(countryInfo->New_Card_CDRound_Current, Arr_cardCD_inTheTurn,
           sizeof(Arr_cardCD_inTheTurn));
    memcpy(countryInfo->New_Card_CDRound_Each, Arr_cardCD_inSetting,
           sizeof(Arr_cardCD_inSetting));
    for (int i = 0; i < countryInfo->eventOccupyAreasNum; ++i) {
        countryInfo->arr_eventFinished_moveArmyTo[i] = this->List_event_moveArmyTo[i]->isFinished;
    }
    for (int i = 0; i < countryInfo->eventTurnBeginsNum; ++i) {
        countryInfo->arr_eventFinished_turnBegin[i] = this->List_event_turnBegin[i]->isFinished;
    }
    for (int i = 0; i < countryInfo->eventResolutionsNum; ++i) {
        countryInfo->arr_eventFinished_resolution[i] = this->List_resolution[i]->isFinished;
    }
    countryInfo->PolicyType_military = this->PolicyType_military;
    countryInfo->PolicyType_economy = this->PolicyType_economy;
    countryInfo->IsNewActionOver = this->IsNewActionOver;


}
/**
     *<p>=======================================================<p>
     * 【功能描述】：
     * <p> 检测一个地块 指定范围内是否有超过指定数量的敌军。
     * <p>
     * 【参数说明】：
     * <p> ringCount：指定的圈层数，请在传参前做好越界判断。
     * <p> enemyCount：指定应该超过的敌军数
     * <p> alliance：不等于它则视为敌军；
     * <p>
     * 【返回值说明】：
     * <p> 判断结果
     *<p>=======================================================<p>
     * 【创建人】：李德邻 <p>
     * 【修改人】：李德邻 <p>
     * 【创建时间】：？？？<p>
     * 【修改时间】：2022/10/11
     */
bool CArea::Is_existMilitaryThreat(int ringCount, int enemyCount, int alliance) {

    auto areaCircleInfo = g_areaSurroundingInfoManager.Get_theAreaSurroundingInfo(this->ID);
    //循环指定的圈层数
    for (int i = 0; i < ringCount; ++i) {
        std::vector<CAreaOnRing*>* theRingInfo_byFor = areaCircleInfo->Get_ringInfo(i);
        if(theRingInfo_byFor == nullptr)
            continue;
        //循环当前圈层的地块数
        int areas_theRing = theRingInfo_byFor->size();
        for (int j = 0; j < areas_theRing; ++j) {
            int foundAreaID = (*theRingInfo_byFor)[j]->ID;
            CArea* foundArea = g_Scene[foundAreaID];
            if (g_Scene.Is_waring(this->ID,foundArea->ID)){
                if (foundArea->ArmyCount > enemyCount){
                    return true;
                }
            }
        }
    }
    return false;
}




/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 检测是否有满状态步兵或者机步（均要求50%血量以上）且周围有地块可移动过去。<p>
     * 【参数说明】：<p>
     * isAutomaticRemoval：是否让满状态单位自动移走到可移动地块上。<p>
     * 【返回值说明】：<p>
     * 如果有可移动地块，返回其id。否则，返回-1 <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： 李德邻<p>
     * 【创建时间】： ？？？<p>
     * 【修改时间】： 2022/10/10
     */
int CArea::Check_moveBestStatusArmyToOtherArea(bool isAutomaticRemoval) {

    for (int i = 0; i < this->ArmyCount; ++i) {
        CArmy* foundArmy_byFor = this->GetArmy(i);
        //如果这个军队是健康状态
        if (foundArmy_byFor->Is_wounded() == false && (foundArmy_byFor->Is_infantry() || foundArmy_byFor->Is_panzer())){
            int areaID_passedAtAround = g_Scene.Get_passedArea_fromAdjcentAreas(this, foundArmy_byFor);
            //该单位必须可以向周围地块移动
            if(areaID_passedAtAround == -1){
                continue;
            }
            else{
                if (isAutomaticRemoval == false)
                    return areaID_passedAtAround;
                auto theAreaSurroundingInfo = g_areaSurroundingInfoManager.Get_theAreaSurroundingInfo(this->ID);
                //指定优先查找的单位
                std::vector<  CountryArmyDef::servicesType> list_foundArmyType_priority;
                if (foundArmy_byFor->Is_artillery() || foundArmy_byFor->Is_remoteAttack())
                    list_foundArmyType_priority.push_back(CountryArmyDef::Tank);
                else if(foundArmy_byFor->Is_panzer())
                    list_foundArmyType_priority.push_back(CountryArmyDef::Infantry);
                else if(foundArmy_byFor->Is_tank()){
                    list_foundArmyType_priority.push_back(CountryArmyDef::Tank);
                    list_foundArmyType_priority.push_back(CountryArmyDef::Panzer);
                }
                int areaID_hasExistNearestArmy = g_newAiAction.Get_areaID_existNearestArmy(this->ID, &list_foundArmyType_priority, 6);
                //如果没找到敌情就随便移动到周围的可自由通行地块上
                if (areaID_hasExistNearestArmy == -1){
                    auto area_passed = g_Scene.GetArea(areaID_passedAtAround);
                    this->MoveArmyTo(area_passed, foundArmy_byFor, false);
                    return area_passed->ID;
                }
                //找到敌情后，往有敌情的地方移动
                else{
                    auto list_moveLine =g_areaSurroundingInfoManager.Get_shortestPath(foundArmy_byFor->Country->PolicyType_military, this->ID, areaID_hasExistNearestArmy, i);
                    //如果移动不过去，就随便移动到周围的可自由通行地块上
                    if (list_moveLine == nullptr){
                        auto area_passed = g_Scene.GetArea(areaID_passedAtAround);
                        this->MoveArmyTo(area_passed, foundArmy_byFor, false);
                        return area_passed->ID;
                    }
                   g_newAiAction.Try_TheArmy_toTargetArea_alongOptimalRoute(this->ID, foundArmy_byFor, *list_moveLine);
                    delete list_moveLine;
                    return areaID_passedAtAround;
                }
            }
        }

    }
    return -1;
}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 指定军队沿着指定的一条线移动 <p>
     * 【参数说明】：<p>
     * linePassed：请确定列表中的每个地块都可以移动过去。<p>
     * 【返回值说明】：<p>
     * void <p>
     *=======================================================<p>
     * 【创建人】：李德邻 <p>
     * 【修改人】： 李德邻 <p>
     * 【创建时间】：？？？<p>
     * 【修改时间】： 2022/7/3
     */
void CArea::Army_followTheRouteToTargetArea(CArmy *theArmy, std::vector<int>& linePassed) {

//
//    int size = linePassed.size();
//    if(size <= 0)
//        return;
//    CArea* firstArea = g_Scene[linePassed[0]];
//
//        this->MoveArmyTo(firstArea, theArmy);
//    for (int i = 0; i < linePassed.size()-1; ++i) {
//        CArea* targetArea = g_Scene[linePassed[i]];
//      if(  g_Scene.CheckMoveable(targetArea->ID,g_Scene[linePassed[i+1]]->ID,targetArea->GetArmyIdx(theArmy)) == false)
//          break;
//        targetArea->MoveArmyTo(g_Scene[linePassed[i+1]],theArmy);
//    }
}

void ConvertToChar(char *arrChar, int num) {

}

/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 是否是可自由通行地块和满行动力仍不可前往的地块。不判断该地块军队数量。<p>
     * 【参数说明】：<p>
     * passedArea：这个指针一定不为空。<p>
     * 【返回值说明】：<p>
     * 判断结果 <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： 2022/7/13<p>
     * 【修改时间】： ？？？
     */
bool CScene::Is_passedArea(CArea* passedArea,CArmy* theArmy){
    if (g_Scene.Is_freeArea(theArmy->Country, passedArea) && theArmy->BasicAbilities->Movement >=
                                                             passedArea->Get_consumedMovement()){
        return true;
    }
    return false;
}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 盟友陆地地块、单位满行动力仍不可前往的陆地地块（海洋地块统一算可前往），会被直接筛除。<p>
     * 如果单位是海军，则陆地地块会被直接筛除。<p>
     * 得到通往补给点的所有路线。<p>
     *  如果是海军，那么陆地地块直接被筛除。<p>
     * 【参数说明】： <p>
     * routeType：1为行军路线。筛除不为己方地块而单位数量大于0的地块。2为攻击路线。<p>
     * armyIndex：请在确保不会越界。<p>
     * theArea：请确保不为空指针.该地块为待军队所在地块。<p>
     * ignoreID：忽略的地块ID，用于add_newLine函数<p>
     * 【返回值说明】：<p>
     * void <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： 2022/7/14<p>
     * 【修改时间】： 2022/7/16
     */
void Auto_splitJointLinePassed(CArmy* theArmy,int routeType,std::vector<std::vector<int>*>& list_linePassed,
                               std::vector<std::vector<CAreaOnRoute>*>& list_ringOfShortestPath,int ignoreID){

    if (list_ringOfShortestPath.size() <= 0)
        return;
    int* line;
    int maxJoinRingCOUnt;
    //高级ai的拼接范围更大一点。
    if (g_GameManager.IsIntelligentAI){
        line = new int[4]{-1,-1,-1,-1};
        maxJoinRingCOUnt = 4;
    }else{
        line = new int[3]{-1,-1,-1};
        maxJoinRingCOUnt = 3;
    }
    int currentRingNumber = 0;
    for (int i = 0; i < list_ringOfShortestPath[0]->size(); ++i) {
        CAreaOnRoute* areawOnRoute = &(*list_ringOfShortestPath[0])[i];
        recursion(line, maxJoinRingCOUnt, list_ringOfShortestPath.size(),currentRingNumber, list_linePassed, theArmy, routeType, areawOnRoute,ignoreID);
    }
    delete[] line;
}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 递归拼接路线<p>
     * 【参数说明】：<p>
     * line：容量和maxJointRingCount保持一致。<p>
     * maxJointRingCount：每条路线最大连接的圈层数。建议为3.<p>
     * foundAreaOnRing：这个地块是还未筛选出的。进入函数后进行筛选甄别。<p>
     * 【返回值说明】：<p>
     * void <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】：2022/7/13<p>
     * 【修改时间】： 2022/7/15
     */
void recursion(int* line, int maxJointRingCount, int maxRingCout, int currentRIngNumber, std::vector<std::vector<int>*>& list_linePassed, CArmy* theArmy, int routeType, CAreaOnRoute* foundAreaOnRing,int ignoreID){

    //对传进来的地块作筛选
    int foundAreaOnRing_ID = foundAreaOnRing->ID;
    CArea* foundArea = g_Scene.GetArea(foundAreaOnRing_ID);
    if (foundArea == nullptr || foundArea->ID == ignoreID)
        return;
    if (theArmy == nullptr)
        return;
    if(g_Scene.Is_passedArea(foundArea, theArmy) &&
       foundArea->Get_consumedMovement() <= theArmy->BasicAbilities->Movement ){
        //如果是海军，那么陆地地块直接被筛除
        if (foundArea->Sea == false){
            if (theArmy->IsNavy()){
                return;
            }
        }
        //如果是我方地块且单位满了，而无单位可以移动到其他地块上的可能，那么筛除。
        if (foundArea->Country != nullptr && foundArea->Country == theArmy->Country
            && foundArea->ArmyCount >= 4 && foundArea->Check_moveBestStatusArmyToOtherArea(false) == -1){
            return;
        }
        //行军路线
        if (routeType == 1){
            //如果是敌军的地块，那么敌军数不能大于0
            if (foundArea->Country != nullptr && foundArea->Country != theArmy->Country){
                if (foundArea->ArmyCount > 0)
                    return;
            }
        }
            //进攻路线
        else if (routeType == 2){

        }
        //甄别出来后才能添加到路线中
        line[currentRIngNumber] = foundAreaOnRing_ID;

        currentRIngNumber++;
        //如果到底了，拼接
        if (currentRIngNumber >= maxRingCout || currentRIngNumber >= maxJointRingCount){
            std::vector<int>* newLine = new std::vector<int>();
            for (int i = 0; i < maxJointRingCount; ++i) {
                int id = line[i];
                if (id == -1)
                    break;
                newLine->push_back(id);
            }
            list_linePassed.push_back(newLine);
            return;
        }
        for (int i = 0; i < foundAreaOnRing->List_areaOnRoute_linkNextRing.size(); ++i) {
            int foundID = foundAreaOnRing->List_areaOnRoute_linkNextRing[i]->ID;
            CArea* passedArea = g_Scene.GetArea(foundID);
            if (passedArea == nullptr)
                continue;
            CAreaOnRoute* newFoundAreawOnRoute = &(*foundAreaOnRing->List_areaOnRoute_linkNextRing[i]);
            //如果是急行军或者行军
            recursion(line, maxJointRingCount, maxRingCout,currentRIngNumber, list_linePassed, theArmy, routeType, newFoundAreawOnRoute,ignoreID);
        }
    }
}

/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 如果线路中有满单位，会让满单位地块移动到其他地块。<p>
     * 如果线路中有海洋地块且自身为陆地单位，会检测是否有运输船单位，如果没有，尝试建造。否则跳过。<p>
     * 【参数说明】：<p>
     * list_linePassed：里面的地块id一定不为空地块。<p>
     * 【返回值说明】：<p>
     * 返回尝试结果。 <p>
     *=======================================================<p>
     * 【创建人】：李德邻 <p>
     * 【修改人】：李德邻<p>
     * 【创建时间】： ？？？<p>
     * 【修改时间】：2022/7/28
     */
bool CArea::Try_armyToSuppliedArea(CArmy *theArmy, std::vector<std::vector<int>*>& list_linePassed) {
    if (list_linePassed.size() <= 0)
        return false;

    std::vector<int>* linePassed;
    std::vector<CArea*> list_needMove;
    bool isBestLine = true;

    for (int i = 0; i < list_linePassed.size(); ++i) {
        linePassed = list_linePassed[i];
        isBestLine = true;
        for (int j = 0; j < linePassed->size(); ++j) {
            CArea* passedArea = g_Scene.GetArea((*linePassed)[j]);
            if(passedArea->ArmyCount >= 4){
                list_needMove.push_back(passedArea);
            }
            if (passedArea->Sea && theArmy->IsNavy() == false &&theArmy->Has_engineeringCorps() == false){
                //如果没有运输船，80%几率装备运输船
                if (g_GameManager.GetRand() % 10 >= 2){
                    theArmy->AddCard(CArmy::EngineeringCorps);
                    theArmy->Country->Money -= 10;
                }
                else{
                    isBestLine = false;
                    break;
                }
            }
        }
        //循环完没有遇到什么问题，这条线路可以直接走了
        if (isBestLine)
            break;
        else
            list_needMove.clear();
    }
    for (int j = 0; j < list_needMove.size(); ++j) {
        list_needMove[j]->Check_moveBestStatusArmyToOtherArea(true);
    }
    this->Army_followTheRouteToTargetArea(theArmy, *linePassed);
    return isBestLine? true:false;
}

bool Is_inSpecifiedRangeOfAreas(int theAreaID) {
    if(theAreaID >=0 && theAreaID < s_Count_theMapAreas)
        return true;
    return false;
}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 判断目标地块是否自由通行地块。条件如下：<p>
     * 1：我方地块。<p>
     * 2：中立国地块。<p>
     * 3：海洋地块。<p>
     * 【参数说明】：<p>
     * startCountry：这个指针一定不为空。<p>
     * 【返回值说明】：<p>
     * 结果 <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】：2022/7/8<p>
     * 【修改时间】： ？？？
     */
bool CScene::Is_freeArea(CCountry* startCountry, CArea* targetArea) {
    if( targetArea->Enable == false || targetArea == nullptr)
        return false;
    if(targetArea->Sea)
        return true;
    if (targetArea->Country == nullptr)
        return true;
    if (targetArea->Country == startCountry)
        return true;
    if (targetArea->Country->Alliance == 4)
        return true;
    return false;
}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 获得周围一圈地块，该单位一定可以移动过去的地块。<p>
     * 【参数说明】：<p>
     * startArea：请确定该参数一定不为空。<p>
     * 【返回值说明】：<p>
     * 如果没找到返回-1，否则返回找到的地块id。返回的地块单位数量一定小于4 <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： 2022/8/1 <p>
     * 【修改时间】： ？？？
     */
int CScene::Get_passedArea_fromAdjcentAreas(CArea* startArea, CArmy* theArmy) {
    int areas = g_Scene.GetNumAdjacentAreas(startArea->ID);
    for (int i = 0; i < areas; ++i) {
        CArea* adjacentArea = g_Scene.GetAdjacentArea(startArea->ID, i);
        if(adjacentArea == nullptr)
            continue;
        if(g_Scene.Is_passedArea(adjacentArea, theArmy) && adjacentArea->ArmyCount < 4)
            return adjacentArea->ID;
    }
    return -1;
}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 判断两个地块的国家是否处于交战状态。<p>
     * 注意：如果指针为空，会返回false。<p>
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * 返回结果 <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： 2022/8/2 <p>
     * 【修改时间】： ？？？
     */
bool CScene::Is_waring(int startAreaID, int targetAreaID) {
    if(this->Is_nonAlly(startAreaID,targetAreaID) ){
        //此时两个地块都一定有国家
        CCountry* startCountry = g_Scene.GetArea(startAreaID)->Country;
        CCountry* targetCountry = g_Scene.GetArea(targetAreaID)->Country;
        if (this->Is_armisticeStatus(startCountry,targetCountry) == false)
            return true;
    }
    return false;
}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 判断这个地块是否位于迷雾中。<p>
     * 【参数说明】：<p>
     * targetArea:该指针不得为空。<p>
     * allance:共享视野的阵营<p>
     * 【返回值说明】：<p>
     * 判断结果 <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： 2022/8/21 <p>
     * 【修改时间】： ？？？
     */
bool CScene::Is_inWarFog(CArea* targetArea,int allance) {
    if (s_OpenWarFog== false)
        return false;
    if(targetArea->Country->Alliance == allance){
        return false;
    }
    else {
        return !targetArea->Is_existTheCamp(1, allance, 0, 1);
    }
}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 判断一个地块是否已经安全打开。即检查是否非空，是否打开。
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * 返回结果 <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： 2022/8/29 <p>
     * 【修改时间】： ？？？
     */
bool CScene::Is_safedOpen(CArea* area) {
    if(area == nullptr || area->Enable == false)
        return false;
    return true;
}


/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 从传进去的地块数组中 获取存在高价值的虚弱打击对象的地块(只统计顶层单位）。<p>
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * 如果没找到，返回-1.否则返回该地块。 <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： 2022/8/2 <p>
     * 【修改时间】： ？？？
     */
int NewAiAction::Get_areaID_ofHighestValueWeakTargetAround(std::vector<int>& list_area_existEnemy_inRange) {
    std::vector<int> list_areaID_existArmy;
    std::vector<int> list_areaID_armyValue;
    for (int i = 0; i < list_area_existEnemy_inRange.size(); ++i) {
        CArea* foundArea_byFor = g_Scene.GetArea(list_area_existEnemy_inRange[i]);
        list_areaID_existArmy.push_back(foundArea_byFor->ID);
        list_areaID_armyValue.push_back(foundArea_byFor->GetArmy(0)->Get_armyValue());
    }
    g_algorithm.QuickSort_value(0,list_areaID_existArmy.size()-1,list_areaID_armyValue,list_areaID_existArmy,false);
//    g_algorithm.QuickSort_armyValue(0, list_areaID_existArmy.size() - 1, false, list_areaID_existArmy);
    return list_areaID_existArmy[0];
}


void QuickSort_consume(std::vector<std::vector<int>*>& list_linePassed, int leftIndex, int rightIndex, bool isSmallToLarge, std::vector<int>& list_consumeOfLine) {
    if (leftIndex >= rightIndex) return;
    //左下标的数作为基准值
    int baseValue = list_consumeOfLine[0];
    std::vector<int>* baseValue1 = list_linePassed[0];
    //由于temp可能会被多次使用，所以为了避免在循环中被反复开辟，可以提前声明。
    std::vector<int>* temp_line ;
    int temp_consume;
    int i, j;
    i = leftIndex;
    j = rightIndex;
    //如果左边下标小于右边下标，则进行循环查找
    while (i < j) {
        int consumedmovement_left = list_consumeOfLine[i];
        int consumedmovement_right =  list_consumeOfLine[j];
        if(isSmallToLarge) {
            while (consumedmovement_right >= baseValue && i < j) j--;
            while (consumedmovement_left <= baseValue && i < j) i++;
        }
        else{
            while (consumedmovement_right <= baseValue && i < j)j--;
            while (consumedmovement_left >= baseValue && i < j) i++;
        }
        if (i < j) {
            temp_line = list_linePassed[i];
            list_linePassed[i] = list_linePassed[j];
            list_linePassed[j] = temp_line;
            temp_consume = list_consumeOfLine[i];
            list_consumeOfLine[i] = list_consumeOfLine[j];
            list_consumeOfLine[j] = temp_consume;
        }
            //此时必定相碰，交换基准值,使其回到“中间位”
        else {
            list_consumeOfLine[leftIndex] = list_consumeOfLine[i];
            list_consumeOfLine[i] = baseValue;
            list_linePassed[leftIndex] = list_linePassed[i];
            list_linePassed[i] = baseValue1;
        }
    }
    QuickSort_consume(list_linePassed, leftIndex, i - 1, true, list_consumeOfLine);//递归左边
    QuickSort_consume(list_linePassed, i + 1, rightIndex, true, list_consumeOfLine);//递归右边
}


/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 传入数条线路，根据每条线路的移动力总消耗排成从小到大或从大到小的顺序。<p>
     * 满单位地块会降低优先级<p>
     * 如果该单位是陆地单位，海洋地块会降低优先级。<p>
     * 【参数说明】：<p>
     * isSmallToLarge：如果为true，则从小到大排序。否则，从大到小排序。<p>
     * list_linePassed：请确定不为空指针。<p>
     * theArmy：请确定不为空指针。<p>
     * 【返回值说明】：<p>
     * void <p>
     *=======================================================<p>
     * 【创建人】： 李德邻<p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： 2022/7/7<p>
     * 【修改时间】： 2022/7/14
     */
void Reorder_linesPassed(std::vector<std::vector<int>*>& list_linePassed, bool isSmallToLarge,CArmy* theArmy) {
    if (list_linePassed.size() <= 0)
        return;
    //记录每一条线路的移动力消耗
    std::vector<int> list_consumeOfLine ;
    for (int i = 0; i < list_linePassed.size(); ++i) {
        int consumeTotal = 0;
        for (int j = 0; j < list_linePassed[i]->size(); ++j) {
            CArea* passedArea = g_Scene.GetArea((*list_linePassed[i])[j]);
            if(passedArea == nullptr)
                continue;
            //如果这个地块满单位，那么降低优先级
            if (passedArea->ArmyCount >= 4)
                consumeTotal += 2;
            consumeTotal += passedArea->Get_consumedMovement(theArmy);
        }
        list_consumeOfLine.push_back(consumeTotal);
    }
    QuickSort_consume(list_linePassed, 0,
                      list_linePassed.size() - 1, isSmallToLarge, list_consumeOfLine);

}
//初始化 全局数组 sArr_foundArea
void Init_sArr_foundArea() {
    for (int i = 0; i < s_Count_theMapAreas; ++i) {
        sArr_foundArea[i] = false;
    }
}

void Init_policyType_military(int scripNum) {
    switch (scripNum) {
        case 0:
            Init_policyType_military_conquest1();
            break;
        default:
            break;
    }
}



void Init_policyType_economy_conquest1() {
    for(auto country : g_GameManager.ListCountry){
        country->PolicyType_economy = 4;
    }
}

void Init_policyType_economy(int scripNum) {
    switch (scripNum) {
        case 1:
            Init_policyType_economy_conquest1();
            break;
        default:
            break;
    }
}

void Init_policyType_military_conquest1() {
    for(auto country : g_GameManager.ListCountry){
        country->PolicyType_military = 2;
    }
}

void CCountry::LoadCountry(const SaveCountryInfo *countryInfo) {
    Money = countryInfo->Money;
    Industry = countryInfo->Industry;
    Oil = countryInfo->Oil;
    Tech = countryInfo->Tech;
    TechTurn = countryInfo->TechTurn;
    AI = countryInfo->AI;
    memcpy(CardRound, countryInfo->CardRound, sizeof(CardRound));
    strcpy(ID, countryInfo->ID);
    strcpy(Name, countryInfo->Name);
    R = countryInfo->R;
    G = countryInfo->G;
    B = countryInfo->B;
    A = countryInfo->A;
    Alliance = countryInfo->Alliance;
    DefeatType = countryInfo->DefeatType;
    TaxRate = countryInfo->TaxRate;
    memcpy(DestroyCount, countryInfo->DestroyCount, sizeof(DestroyCount));
    SetCommander(countryInfo->CommanderName);
    CommanderTurn = countryInfo->CommanderTurn;
    CommanderAlive = countryInfo->CommanderAlive;
    memcpy(WarMedal, countryInfo->WarMedal, sizeof(WarMedal));
    BorrowedLoan = countryInfo->BorrowedLoan;
    Defeated = countryInfo->Defeated;
    IsCoastal = countryInfo->IsMaritime;
    for (int i = 0; i < countryInfo->listArmisticeStatusCount; ++i) {
        ListArmisticeStatus[countryInfo->listArmisticeStatus[i].CountryName] = countryInfo->listArmisticeStatus[i].Round;
    }
    for (int i = 0; i < countryInfo->listArrDesignationsLand_Count; ++i) {
        this->ListDesignationsLand.push_back(countryInfo->arr_land[i]);
    }
    for (int i = 0; i < countryInfo->listArrDesignationsAir_Count; ++i) {
        this->ListDesignationsAir.push_back(countryInfo->arr_air[i]);
    }
    for (int i = 0; i < countryInfo->listArrDesignationsSea_Count; ++i) {
        this->ListDesignationsSea.push_back(countryInfo->arr_sea[i]);
    }
    for (int i = 0; i < countryInfo->listArrDesignationsTank_Count; ++i) {
        this->ListDesignationsTank.push_back(countryInfo->arr_tank[i]);
    }
    this->MaxArmyDesignationsTank = countryInfo->MaxArmyDesignationsTank;
    this->MaxArmyDesignationsLand = countryInfo->MaxArmyDesignationsLand;
    this->MaxArmyDesignationsSea = countryInfo->MaxArmyDesignationsSea;
    this->MaxArmyDesignationsAir = countryInfo->MaxArmyDesignationsAir;
    this->SpecialMark = countryInfo->SpecificMark;
    this->OilConversionRate = countryInfo->OilConversionRate;
    memcpy(Arr_cardCD_inTheTurn, countryInfo->New_Card_CDRound_Current,
           sizeof(Arr_cardCD_inTheTurn));
    memcpy(Arr_cardCD_inSetting, countryInfo->New_Card_CDRound_Each,
           sizeof(Arr_cardCD_inSetting));
    this->PolicyType_military = countryInfo->PolicyType_military;
    this->PolicyType_economy = countryInfo->PolicyType_economy;
    this->IsNewActionOver = countryInfo->IsNewActionOver;

}

CCountry::~CCountry() {
    delete this->PvCommanderDef;
    this->PvPlayer = nullptr;
    this->AOECardCurTargetID = nullptr;
    for (int i = 0; i < List_event_turnBegin.size(); ++i) {
        delete List_event_turnBegin[i];
    }
    for (int i = 0; i < List_event_moveArmyTo.size(); ++i) {
        delete List_event_moveArmyTo[i];
    }
    for (int i = 0; i < List_resolution.size(); ++i) {
        delete List_resolution[i];
    }
}
//CActionNode *CActionAI::setCpuDriverbyId(int CountryIndex, int) {
//    Action = false;
//    CActionAssist *ActionAssist = CActionAssist::Instance();
//    ActionAssist->ActionToNextAreaTargetIdList.clear();
//    setActionNodeClear();
//    CurrentCountry = g_GameManager.GetCountryByIndex(CountryIndex);
//    if (CurrentCountry != nullptr && !CurrentCountry->IsConquested()) {
//        memcpy(CurrentCountryName, CurrentCountry->Name, sizeof(CurrentCountryName));
//        getAiPercent(CountryIndex);
//        CurrentCountryIndex = CountryIndex;
//        TurnBeginMoneyQuarter = CurrentCountry->Money / 4;
//        TurnBeginIndustryQuarter = CurrentCountry->Industry / 4;
//        analyseCompositePower();
//        int rand = lrand48() % 100;
//        purChaseCard(
//                rand <= 50 ? InfantryCard :
//                rand <= 65 ? DestroyerCard :
//                rand <= 70 ? AssaultArtCard :
//                rand <= 75 ? CityCard :
//                rand <= 83 ? AirStrikeCard :
//                rand <= 93 ? NuclearBombCard : LandFortCard
//        );
//        sortArmyToFront();
//        moveAndAttack();
//        if (Action)
//            return &ActionAssist->ActionNode;
//        CurrentCountryIndex = -1;
//    } else {
//        ActionAssist->ActionNode.ActionType = CActionNode::ArmyInactiveAction;
//    }
//    return nullptr;
//}
//void CCountry::Update(float time) {
//    //如果正在等待摄像头
//    if(this->ActionWaitCamera){
//        //符合两个条件：则不再等待摄像头
//        //1:行动的延迟时间小于等于0 或者 当前摄像头没有移动
//        if ( g_Scene.IsMoving() ||this->ActionDelayTime <= 0.0 ){
//                return;
//        }
//        //2:当前行动时间已经超过或达到了摄像头的等待时间（不允许等待太久）
//        else{
//            this->ActionTimer = this->ActionTimer + time;
//            if (this->ActionTimer < this->ActionWaitCamera)
//                return;
//        }
//        //不再等待，执行因等待摄像头还未进行的操作指令
//        this->ActionWaitCamera = false;
//        this->DoAction();
//    }
//    //重置上一个指令的部分数据
//    else{
//        if(this->CAction.ActionType == CountryAction::ArmyMoveAction){
//            //等待动画
//            if (g_Scene.GetArea( this->CAction.TargetAreaID)->ArmyMovingIn != nullptr || this->ActionTimer <= 0.6)
//                return;
//            this->FinishAction();
//        }
//        auto action = this->CAction.ActionType;
//        if ( action == CountryAction::ArmyMoveFrontAction ||action == CountryAction::ArmyAttackAction || action == CountryAction::UseCardAction || this->ActionTimer > 0.6)
//            this->FinishAction();
//    }
//}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 国家指令开始执行前，进行初始化工作。设置完成后，部分指令会自动调用DoAction函数。<p>
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * void <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： 2022/8/6 <p>
     * 【修改时间】： ？？？
     */
//void CCountry::Action(const CountryAction& countryAction) {
// this->CAction.ActionType = countryAction.ActionType;
// this->CAction.CardID = countryAction.CardID;
// this->CAction.StartAreaID = countryAction.StartAreaID;
// this->CAction.TargetAreaID = countryAction.TargetAreaID;
// this->CAction.ArmyIndex = countryAction.ArmyIndex;
//    switch (this->CAction.ActionType) {
//        case CountryAction::ArmyMoveAction:{
//            if (this->IsLocalPlayer()){
//                this->DoAction();
//            }
//        }
//            break;
//        case CountryAction::ArmyAttackAction:{
//            if (this->IsLocalPlayer()){
//                this->DoAction();
//            }
//            else{
//                g_Scene.MoveCameraBetweenArea(this->CAction.StartAreaID,this->CAction.TargetAreaID);
//                this->ActionWaitCamera = 1;
//                this->ActionDelayTime = 0.0;
//            }
//        }
//            break;
//        case CountryAction::ArmyInactiveAction:{
//            CArea* targetArea = g_Scene.GetArea(this->CAction.TargetAreaID);
//            //军队不再活跃
//            targetArea->SetArmyActive(this->CAction.ArmyIndex,false);
//            this->FinishAction();
//        }
//            break;
//        case CountryAction::ArmyMoveFrontAction:
//        case CountryAction::UseCardAction:
//            if (this->IsLocalPlayer()){
//                this->DoAction();
//            }
//            break;
//        case CountryAction::CameraMoveAction:
//            break;
//        default:
//            break;
//    }
//}

void CArea::SaveAera(SaveAreaInfo *AreaInfo) {
    AreaInfo->ID = ID;
    AreaInfo->ConstructionType = ConstructionType;
    AreaInfo->ConstructionLevel = ConstructionLevel;
    AreaInfo->InstallationType = InstallationType;
    AreaInfo->ArmyCount = 0;
    memset(AreaInfo->ArmyInfo, 0, sizeof(AreaInfo->ArmyInfo));
    if (ArmyDrafting != nullptr)
        ArmyDrafting->SaveArmy(&AreaInfo->ArmyInfo[AreaInfo->ArmyCount++]);
    if (ArmyMovingIn != nullptr)
        ArmyMovingIn->SaveArmy(&AreaInfo->ArmyInfo[AreaInfo->ArmyCount++]);
    if (ArmyMovingToFront != nullptr)
        ArmyMovingToFront->SaveArmy(&AreaInfo->ArmyInfo[AreaInfo->ArmyCount++]);
    for (CArmy *Army : this->Army) {
        if (Army != nullptr)
            Army->SaveArmy(&AreaInfo->ArmyInfo[AreaInfo->ArmyCount++]);
    }
    AreaInfo->safe = IsSafe;
    AreaInfo->CityGrowthRate = CityGrowthRate;
    AreaInfo->IndustryGrowthRate = IndustryGrowthRate;
    AreaInfo->OilGrowthRate = OilGrowthRate;

}

void CArea::LoadArea(const SaveAreaInfo *AreaInfo) {
    ID = AreaInfo->ID;
    ConstructionType = (enum ConstructionType) (AreaInfo->ConstructionType);
    ConstructionLevel = AreaInfo->ConstructionLevel;
    InstallationType = (enum InstallationType) (AreaInfo->InstallationType);
    if (Country != nullptr) {
        for (int i = 0; i < AreaInfo->ArmyCount; i++) {
            CArmy *Army = new CArmy();
            Army->Init(CObjectDef::Instance()->GetArmyDef(AreaInfo->ArmyInfo[i].ID, Country->Name),
                       Country);
            Army->LoadArmy(&AreaInfo->ArmyInfo[i]);
            AddArmy(Army, true);
        }
    }

    this->IsSafe = AreaInfo->safe;
    CityGrowthRate = AreaInfo->CityGrowthRate;
    IndustryGrowthRate = AreaInfo->IndustryGrowthRate;
    OilGrowthRate = AreaInfo->OilGrowthRate;

}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     *  得到移动该陆地地块所需移动力。<p>
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * 返回移动到该地块所消耗的移动力。如果该地块为海洋，返回为1. <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： ？？？<p>
     * 【修改时间】： ？？？
     */
int CArea::Get_consumedMovement() {
    switch (this->Type) {
        case forest:
            return 2;
        case hilly:
            return 3;
        case desert:
            return 3;
        default:
            return 1;
    }
}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 这个重载会判断这个地块是否是海陆状况，如果是海洋，会返回传入单位的最大移动力。<p>
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * 返回的行动力 <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： 2022/7/8<p>
     * 【修改时间】： ？？？
     */
int CArea::Get_consumedMovement(CArmy* army) {
    if (this->Sea)  return army->BasicAbilities->Movement;
    switch (this->Type) {
        case forest:
            return 2;
        case hilly:
            return 3;
        case desert:
            return 3;
        default:
            return 1;
    }
}

/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 检测这个地块的建筑是否符合撤退单位的陆地补给条件。<p>
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * void <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： 2022/7/6 <p>
     * 【修改时间】： ？？？
     */
bool CArea::Is_supplyCity() {
    if (this->Sea == false ){
        if (this->ConstructionLevel > 1 || this->Type == large_city || this->Type == capital){
            return true;
        }
    }
    return false;
}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 检测这个地块的建筑是否符合撤退单位的海洋补给条件。<p>
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * void <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： 2022/7/6 <p>
     * 【修改时间】： ？？？
     */
bool CArea::Is_supplyPort() {
    if (this->Sea){
        if(this->Type == port){
            return true;
        }
    }
    return false;
}


int CArea::Get_countOfDesigantions() {
    int selfCount = 0;
    for (int i = 0; i < this->ArmyCount; ++i) {
        CArmy* army_byFOr = this->GetArmy(i);
        if(army_byFOr->IsA)
            selfCount += army_byFOr->Get_count_oldDesignation();
        else
            selfCount ++;
    }
    return selfCount;
}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 判断这个地块周围指定范围内 是否存在非本阵营且单位数大于指定数字的地块。<p>
     * 【参数说明】：<p>
     * alliance：等于它，返回true <p>
     * 【返回值说明】：<p>
     * 如果存在，返回符合的地块ID。否则返回-1。<p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： 2022/8/1 <p>
     * 【修改时间】： ？？？
     */
bool CArea::Is_existNoAlly(int ringCount, int armys_noAlly) {
    auto areaCircleInfo = g_areaSurroundingInfoManager.Get_theAreaSurroundingInfo(this->ID);
    //循环指定的圈层数
    for (int i = 0; i < ringCount; ++i) {
        std::vector<CAreaOnRing*>* theRingInfo_byFor = areaCircleInfo->Get_ringInfo(i);
        if(theRingInfo_byFor == nullptr)
            continue;
        //循环当前圈层的地块数
        int areas_theRing = theRingInfo_byFor->size();
        for (int j = 0; j < areas_theRing; ++j) {
            int foundAreaID = (*theRingInfo_byFor)[j]->ID;
            CArea* foundArea = g_Scene[foundAreaID];
            if (foundArea == nullptr ||foundArea->Enable == false ||Is_inSpecifiedRangeOfAreas(foundArea->ID) == false)
                continue;
            if (foundArea->Country != nullptr && foundArea->Country->Alliance == this->Country->Alliance && foundArea->ArmyCount > armys_noAlly){
                return foundArea-ID;
            }
        }
    }
    return -1;
}

//是否存在指定单位的军种类型
bool CArea::Is_existTheServicesType(std::vector< CountryArmyDef::servicesType>&  list_armyType) {
    if(list_armyType.size() <= 0)
        return false;
    bool found = false;
    for (int i = 0; i < this->ArmyCount; ++i) {
        auto army_byFOr = this->GetArmy(i);
        for (int j = 0; j < list_armyType.size(); ++j) {
            auto armyType_byFor = list_armyType[j];
            if (armyType_byFor == CountryArmyDef::Infantry){
                if (army_byFOr->Is_infantry())
                    found = true;
            }
            else if (armyType_byFor == CountryArmyDef::Panzer){
                if (army_byFOr->Is_panzer())
                    found = true;
            }
            else if (armyType_byFor == CountryArmyDef::Artillery){
                if (army_byFOr->Is_artillery())
                    found = true;
            }
            else if (armyType_byFor == CountryArmyDef::Tank){
                if (army_byFOr->Is_tank())
                    found = true;
            }
            else if (armyType_byFor == CountryArmyDef::AirForce){
                if (army_byFOr->Is_airForce())
                    found = true;
            }
            else if (armyType_byFor == CountryArmyDef::Navy){
                if (army_byFOr->IsNavy())
                    found = true;
            }
        }
        if (found)
            break;
    }
    return found;
}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 检测这个地块周围一圈是否有可移动单位过去的地块.条件如下：<p>
     * 1：我方地块。<p>
     * 2：中立国地块。<p>
     * 3：海洋地块。<p>
     * 4：单位小于4.<p>
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * 存在则返回地块id，否则返回-1<p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： 2022/8/1 <p>
     * 【修改时间】： ？？？
     */
int CArea::Is_hasExport() {
    int aroundAreas = g_Scene.GetNumAdjacentAreas(this->ID);
    //循环周围地块
    for (int j = 0; j < aroundAreas; ++j) {
        CArea* foundArea_byFor = g_Scene.GetAdjacentArea(this->ID, j);
        if (g_Scene.Is_freeArea(this->Country, foundArea_byFor) && foundArea_byFor->ArmyCount < 4){
            return foundArea_byFor->ID;
        }
    }
    return -1;
}


void CArmy::SaveArmy(SaveArmyInfo *ArmyInfo) {
    ArmyInfo->ID = BasicAbilities->ID;
    ArmyInfo->HP = Hp;
    ArmyInfo->MaxHP = MaxHp;
    ArmyInfo->Movement = Movement;
    ArmyInfo->Cards = Cards;
    ArmyInfo->Level = Level;
    ArmyInfo->Exp = Exp;
    ArmyInfo->Morale = Morale;
    ArmyInfo->MoraleUpTurn = MoraleUpTurn;
    ArmyInfo->Direction = Direction;
    ArmyInfo->Active = Active;
    ArmyInfo->Seed = Seed;
    ArmyInfo->Designation = Designation;
    ArmyInfo->IsA = IsA;
    ArmyInfo->MaxOrganization = MaxOrganization;
    ArmyInfo->Organization = Organization;
    for (int i = 0; i < 3; ++i) {
        ArmyInfo->OldDesignation[i] = OldDesignation[i];
    }
}

void CArmy::LoadArmy(const SaveArmyInfo *ArmyInfo) {
    Hp = ArmyInfo->HP;
    MaxHp = ArmyInfo->MaxHP;
    Cards = ArmyInfo->Cards;
    Level = ArmyInfo->Level;
    Exp = ArmyInfo->Exp;
    Morale = ArmyInfo->Morale;
    MoraleUpTurn = ArmyInfo->MoraleUpTurn;
    Direction = ArmyInfo->Direction;
    Movement = ArmyInfo->Movement;
    if (Movement < 0)
        Movement = 0;
    Active = ArmyInfo->Active;
    Seed = ArmyInfo->Seed;
    Designation = ArmyInfo->Designation;
    IsA = ArmyInfo->IsA;
    MaxOrganization = ArmyInfo->MaxOrganization;
    Organization = ArmyInfo->Organization;
    for (int i = 0; i < 3; ++i) {
        OldDesignation[i] = ArmyInfo->OldDesignation[i];
    }
    ResetMaxStrength(false);
}

bool CArmy::Has_engineeringCorps() {
    return this->HasCard(CArmy::EngineeringCorps);
}
//判断是否是伤兵
bool CArmy::Is_wounded() {
    return this->GetNumDices() <= 3 ? true : false;
}

int CArmy::Get_count_oldDesignation() {
    if (this->IsA == false)
        return 0;
    int count = 0;
    for (int i = 0; i < 3; ++i) {
        if (this->OldDesignation[i] != 0)
            count++;
    }
    return count;
}

bool CArmy::Is_panzer() {
    if (this->BasicAbilities->ID == ArmyDef::Panzer) {
        return true;
    }
    return false;
}

bool CArmy::Is_infantry() {
    if (this->BasicAbilities->ID == ArmyDef::InfantryD1905||this->BasicAbilities->ID == ArmyDef::InfantryA1905) {
        return true;
    }
    return false;
}
//近距离火炮
bool CArmy::Is_artillery() {
    if (this->BasicAbilities->ID == ArmyDef::Artillery) {
        return true;
    }
    return false;
}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 当单位的等级为3级时，价值超过下个更高价值单位的1级。<p>
     * 当单位的等级为4级时，价值超过下个更高价值单位的2级。<p>
     * 单位每有一个附属团，增加50价值。指挥官额外+200。<p>
     * 单位血量小于15%，价值再增加200,否则小于50%，价值加150.<p>
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * void <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： 2022/8/2 <p>
     * 【修改时间】： ？？？
     */
int CArmy::Get_armyValue() {
    int value = 0;
    if (this->Is_infantry())
        value+= 100;
    else if (this->Is_panzer())
        value+= 200;
    else if(this->IsNavy()){
        value+=200;
        if (this->BasicAbilities->ID == ArmyDef::AircraftCarrier)
            value += 150;
    }
    else if(this->Is_tank())
        value+=300;
    else if(this->Is_artillery())
        value+= 400;
    else if(this->Is_airForce())
        value+= 500;
    switch (this->Level) {
        case 1:
            value+=10;
            break;
        case 2:
            value+=50;
            break;
        case 3:
            value+=110;
            break;
        case 4:
            value+=160;
            break;
        default:
            break;
    }
    value += this->Get_countOfCard() * 50;
    if(this->HasCard(Commander))
        value+=200;
    if (this->GetNumDices() < 3)
        value += 200;
    else if (this->GetNumDices() < 5)
        value += 150;
    return value;
}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 是否是精锐部队。<p>
     * 条件： 单位等级大于等于2  或者是坦克且等级大于等于1   或者是军舰
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * 返回结果<p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： 2022/9/2 <p>
     * 【修改时间】： ？？？
     */
bool CArmy::Is_eliteArmy() {
    return (this->Level >= 2 || (this->Is_tank() && this->Level >= 1) || this->IsNavy());
}

bool CArmy::Is_collapsed() {
  return  this->Organization <=  this->BasicAbilities->Collapse;
}


void CAreaSurroundingInfo::Set_ringNum_areaOnRing(int areaOnRing_ID, int ringNum) {
    (Map_ringNum_inRingArea)[areaOnRing_ID] = ringNum;
}

void CAreaSurroundingInfo::Add_ringInfo(std::vector<CAreaOnRing*>* circleInfo) {
    ListRingInfo.push_back(circleInfo);
}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 得到该地块指定圈层的所有地块信息. <p>
     * 【参数说明】：<p>
     * ringNumber：指定的圈层号（下标）<p>
     * 【返回值说明】：<p>
     * 返回该地块的指定圈层信息，该指针不可被释放！如果输入的圈层超出最大下标，返回空。 <p>
     *=======================================================<p>
     * 【创建人】：李德邻 <p>
     * 【修改人】：？？？<p>
     * 【创建时间】：？？？<p>
     * 【修改时间】：？？？
     */
std::vector<CAreaOnRing*>* CAreaSurroundingInfo::Get_ringInfo(int ringNumber) {
    //圈层存储的地块数是有可能为0的。
    if (ringNumber >= this->ListRingInfo.size()|| this->ListRingInfo.size() <= 0)
        return nullptr;
    return ListRingInfo[ringNumber];
}


/**
     *=======================================================<p>
     * 【功能描述】：<p>
     *  得到与目标地块之间的直接最短路径中的地块。这些返回的地块还需要作二次筛选组成一条条路线，且有的地块并不支持一些单位通过。<p>
     * 【参数说明】：<p>
     * targetAreaID：目的地。请在传参前确认该地块存在于该CAreaSurroundingInfo中。<p>
     * 【返回值说明】：<p>
     * 每个元素是每个圈层的最短路径地块(不包括起始地块）。 目标地块位于最后一个元素。<p>
     * 返回的地块是一个多叉树，高级AI状态下深度为3，否则为2。用完记得释放。<p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： 李德邻<p>
     * 【创建时间】：？？？<p>
     * 【修改时间】：2022/7/13
     */
std::vector<std::vector<CAreaOnRoute>*>* CAreaSurroundingInfo::Get_ringOfShortestPath(int targetAreaID) {
    CAreaOnRing* c_target_areaOnRing = this->Get_areaOnRing(targetAreaID);

    bool arrIsFoundArea[s_Count_theMapAreas]{};
    arrIsFoundArea[targetAreaID] = true;
    //存放下一递归(从补给点往圆心（出发点）递归的地块
    std::vector<CAreaOnRing*> list_area_nextRecursion;
    //存放位于最短路线上的地块
    std::vector<std::vector<CAreaOnRoute>*>* list_ringOfShortestPath = new std::vector<std::vector<CAreaOnRoute>*>();
    //首次遍历从目的地开始
    list_area_nextRecursion.push_back(c_target_areaOnRing);

    //当前圈层号  从目标地块开始向原点遍历
    int theRingNUmber = this->Get_ringNumber_targetAreaOnRing(targetAreaID) ;

    //初始化
    for (int i = 0; i < theRingNUmber+1; ++i) {
        std::vector<CAreaOnRoute>* list_areaOnRoute = new std::vector<CAreaOnRoute>();
        list_ringOfShortestPath->push_back(list_areaOnRoute);
    }

    NextRecursion(list_ringOfShortestPath, list_area_nextRecursion, arrIsFoundArea, theRingNUmber + 1);

    return list_ringOfShortestPath;
}



/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 根据id得到位于该areaSurrounding中的areaOnRing。<p>
     * 【参数说明】：<p>
     * areaOnRing_ID：请确定该地块一定存在于这个areaSurrounding中。<p>
     * 【返回值说明】：<p>
     * 结果 <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： ？？？<p>
     * 【修改时间】： ？？？
     */
CAreaOnRing* CAreaSurroundingInfo::Get_areaOnRing(int areaOnRing_ID) {
    int ringNum = (Map_ringNum_inRingArea)[areaOnRing_ID];
    int indexAtRing = (Map_index_areaOnRingAtRing)[areaOnRing_ID];
    return (*this->ListRingInfo[ringNum])[indexAtRing];
}


/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 从目标地块不断向原点递归遍历。 <p>
     * 【参数说明】：<p>
     * list_area_nextRecursion：存放下一个递归需要遍历的地块。<p>
     * lastRingNumber：遍历下一圈前，输入当前的圈层号。<p>
     * 【返回值说明】：<p>
     * void <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： 李德邻<p>
     * 【创建时间】： ？？？<p>
     * 【修改时间】： 2022/7/13
     */
void CAreaSurroundingInfo::NextRecursion(std::vector<std::vector<CAreaOnRoute>*>* list_ringOfShortestPath, std::vector<CAreaOnRing*>& list_area_nextRecursion, bool* arrIsFoundArea, int lastRingNumber ) {

    //本圈层号 应是：
    int theRingNUmber = lastRingNumber - 1;;
    // 小于0，说明已经到了原点
    if (theRingNUmber < 0){
        return;
    }
    int total_area_nextRecursion = list_area_nextRecursion.size();
    //决定多叉树的深度
    int max = 2;
    if (g_GameManager.IsIntelligentAI)
        max = 3;
    //存放下一个递归
    std::vector<CAreaOnRing*> listNextRecursionArea ;
    /*
     * 循环本次需要遍历的地块，如果存在于本圈层，说明为最短路径。则添加到最短路径列表。
     * 并且遍历它们的上一链接的地块，添加到下一个递归的地块列表中。
     */
    for (int i = 0; i < total_area_nextRecursion; ++i) {
        CAreaOnRing* areaOnRing_theRing = list_area_nextRecursion[i];
        if (this->Get_ringNumber_targetAreaOnRing(areaOnRing_theRing->ID) == theRingNUmber){
            CAreaOnRoute areaOnRoute ;
            areaOnRoute.ID = areaOnRing_theRing->ID;
            //newAreaOnRing的ListAreaID_linkNextRing 添加地块  用于之后的线路链接
            if(theRingNUmber < list_ringOfShortestPath->size() - 1 && theRingNUmber < max){
                for (int j = 0; j < (*list_ringOfShortestPath)[theRingNUmber + 1]->size(); ++j) {
                    for (int k = 0; k < areaOnRing_theRing->List_areaID_linkNextRing.size() ; ++k) {
                        //配对成功
                        if (areaOnRing_theRing->List_areaID_linkNextRing[k] == (*(*list_ringOfShortestPath)[theRingNUmber + 1])[i].ID ){
                            areaOnRoute.List_areaOnRoute_linkNextRing.push_back(&(*(*list_ringOfShortestPath)[theRingNUmber + 1])[i]);
                            break;
                        }
                    }
                }
            }
            (*list_ringOfShortestPath)[theRingNUmber]->push_back(areaOnRoute);
            //循环这个地块与上一圈层链接的地块
            int linkLastAreas = areaOnRing_theRing->List_areaID_linkLastRing.size();
            for (int j = 0; j < linkLastAreas; ++j) {
                int lastAreaID = (areaOnRing_theRing->List_areaID_linkLastRing)[j];
                //如果这个地块被遍历过，那么就开始下一循环
                if (arrIsFoundArea[lastAreaID])
                    continue;
                arrIsFoundArea[lastAreaID] = true;
                //添加到下一循环中
                listNextRecursionArea.push_back(this->Get_areaOnRing(lastAreaID));
            }
        }
    }
    NextRecursion(list_ringOfShortestPath, listNextRecursionArea, arrIsFoundArea, theRingNUmber);
    return;
}



void CAreaSurroundingInfo::Set_index_ofAreaOnRingAtRing(int areaInRing_ID, int indexAtRing) {
    ( Map_index_areaOnRingAtRing)[areaInRing_ID] = indexAtRing;
}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 返回这个地块的所在圈层号。<p>
     * 【参数说明】：<p>
     * targetArea：请确定该地块一定在记录中。<p>
     * 【返回值说明】：<p>
     * 参数地块所在的圈层号 <p>
     *=======================================================<p>
     * 【创建人】：李德邻 <p>
     * 【修改人】：？？？<p>
     * 【创建时间】：？？？<p>
     * 【修改时间】： ？？？
     */
int CAreaSurroundingInfo::Get_ringNumber_targetAreaOnRing(int targetArea_ID) {
    return (Map_ringNum_inRingArea)[targetArea_ID];
}

int CAreaSurroundingInfo::Get_ringCount() {
    return ListRingInfo.size();
}

//得到与指定链接地块所共同接壤的所有地块。获取值记得做空值检查。
std::vector<int>* CJointlyLinkWithAroundAreasInfo::Get_areas_jointLyLink_withEachArea(int adjacentIndex) {
    if (adjacentIndex >= this->List_areas_jointlyLink_withEachArea->size())
        return nullptr;
    return &(*this->List_areas_jointlyLink_withEachArea)[adjacentIndex];
}

CJointlyLinkWithAroundAreasInfo::CJointlyLinkWithAroundAreasInfo() {

}

CJointlyLinkWithAroundAreasInfo::CJointlyLinkWithAroundAreasInfo(int id) {
    this->ID = id;
}

void CJointlyLinkWithAroundAreasInfo::Init_ListAreasJointlyLink(std::vector<std::vector<int>>* list_areas_jointlyLink) {
    this->List_areas_jointlyLink_withEachArea = list_areas_jointlyLink;
}

/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 如果线路中有满单位，会让满单位地块移动到其他地块。<p>
     * 如果线路中有海洋地块且自身为陆地单位，会检测是否有运输船单位，如果没有，尝试建造。否则中止行动。<p>
     * 单位走完这个路线后（即使遇到没办法走的地块&理论上已经是最优路线），状态为设置为false
     * 【参数说明】：<p>
     * startAreaID：一定不为空。<p>
     * theArmy：一定是开始地块上的单位。<p>
     * OptimalRoute：里面的地块id一定不为空地块。如果单位已满，一定有单位可移动走。并且所有地块理论上可走。<p>
     * 【返回值说明】：<p>
     * 返回尝试结果。 <p>
     *=======================================================<p>
     * 【创建人】：李德邻 <p>
     * 【修改人】：???<p>
     * 【创建时间】： 2022/8/1<p>
     * 【修改时间】：???
     */
void NewAiAction::Try_TheArmy_toTargetArea_alongOptimalRoute(int startAreaID, CArmy* theArmy, std::stack<int>& OptimalRoute) {
    int areas = OptimalRoute.size();
    int lastAreaID = startAreaID;
    for (int i = 0; i < areas; ++i) {
        CArea* areaPassed_byFor = g_Scene.GetArea(OptimalRoute.top());
        OptimalRoute.pop();
        if (areaPassed_byFor->Sea && theArmy->Has_engineeringCorps() == false){
            int r = GetRand(theArmy)%100 + 1;
            auto ecrCard = CObjectDef::Instance()->GetCardDef(CARD_ID::ECRCard);
            CCountry* armyCountry = theArmy->Country;
            //ai即将下水 上运输船的概率
            if(r <= 80 && armyCountry->Money >= ecrCard->Price && armyCountry->Industry >= ecrCard->Industry){
                theArmy->AddCard(CArmy::EngineeringCorps);
                armyCountry->Money -= theArmy->Country->GetCardPrice(ecrCard);
                armyCountry->Industry -= theArmy->Country->GetCardIndustry(ecrCard);
            }
            else{
                break;
            }
        }
        if (areaPassed_byFor->ArmyCount >= 4){
            //因为它必定可以移动走，所以把堵住的单位移走
           areaPassed_byFor->Check_moveBestStatusArmyToOtherArea(true);
        }
        CArea* startArea = g_Scene.GetArea(lastAreaID);
        int armyIndex = startArea->GetArmyIdx(theArmy);
        if( g_Scene.CheckMoveable(lastAreaID,areaPassed_byFor->ID,armyIndex)){
            startArea->MoveArmyTo(areaPassed_byFor, theArmy, true);
            lastAreaID = areaPassed_byFor->ID;
        }
        else{
            break;
        }
    }
    //军队状态设置为false
    theArmy->Active = false;
}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 得到距离最近的敌军地块ID。
     * 【参数说明】：<p>
     * startAreaID：该地块一定存在国家。<p>
     * list_servicesType_toBePrioritySearch：优先查找指定的单位。<p>
     * maxRingCount_toPrioritySearch：如果在指定圈层内仍未找到指定的优先单位，则查找普通单位。<p>
     * 【返回值说明】：<p>
     * 查找到的地块id。没找到返回-1 <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： 2022/8/1 <p>
     * 【修改时间】： 2022/10/9
     */
int NewAiAction::Get_areaID_existNearestArmy(int startAreaID, std::vector<CountryArmyDef::servicesType>* list_servicesType_toBePrioritySearch, int maxRingCount_toPrioritySearch) {
    auto theAreaSurroundingInfo = g_areaSurroundingInfoManager.Get_theAreaSurroundingInfo(startAreaID);
    int ringCount = theAreaSurroundingInfo->Get_ringCount();
    if (maxRingCount_toPrioritySearch > ringCount)
        maxRingCount_toPrioritySearch = ringCount;
    int nearestID = -1;
    bool isFound = false;
    CCountry* startCountry = g_Scene.GetArea(startAreaID)->Country;
    for (int ringNumber_byFor = 0; ringNumber_byFor < ringCount; ++ringNumber_byFor) {
        auto theRingInfo_byFor = theAreaSurroundingInfo->Get_ringInfo(ringNumber_byFor);
        for (int j = 0; j < theRingInfo_byFor->size(); ++j) {
            CArea* foundArea_byFor = g_Scene.GetArea((*theRingInfo_byFor)[j]->ID);
            if (foundArea_byFor != nullptr && foundArea_byFor->Enable &&  foundArea_byFor->Country != nullptr){
                //只查找属于交战状态下的地块
                if (g_Scene.Is_waring(startAreaID,foundArea_byFor->ID)){
                    if (foundArea_byFor->ArmyCount > 0 && nearestID == -1){
                        nearestID = foundArea_byFor->ID;
                    }
                    //超过指定优先单位的最大圈层上限则只查找普通单位
                    if(ringNumber_byFor < maxRingCount_toPrioritySearch &&
                       foundArea_byFor->Is_existTheServicesType(*list_servicesType_toBePrioritySearch)) {
                        nearestID = foundArea_byFor->ID;
                        isFound = true;
                        break;
                    }
                }
            }
        }
        if(isFound || (ringNumber_byFor >= maxRingCount_toPrioritySearch && nearestID != -1))
            break;
    }
    return nearestID;
}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 得到走完该路线所需的移动力。<p>
     * 【参数说明】：<p>
     * army_preparing：该单位最大行动力是海运时所需的移动力。<p>
     * 【返回值说明】：<p>
     * 走完该路线所需的移动力 <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： 2022/8/2 <p>
     * 【修改时间】： ？？？
     */
int NewAiAction::Get_consume_aboutTheline(std::vector<int>& list_line, CArmy* army_preparing) {
    if (list_line.size() <= 0)
        return 0;
    int areas = list_line.size();
    int consume = 0;
    for (int i = 0; i < areas; ++i) {
        CArea* areaPassed_byFor = g_Scene.GetArea(list_line[i]);
        consume += areaPassed_byFor->Get_consumedMovement(army_preparing);
    }
    return consume;
}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 找到最近的一个敌国地块。.<p>
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * 如果没找到，返回-1 <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： 2022/10/9 <p>
     * 【修改时间】： ？？？
     */
int NewAiAction::Get_areaID_ofNearestEnemy(int startAreaID, int maxRingCount) {
    auto theSurroundingInfo = g_areaSurroundingInfoManager.Get_theAreaSurroundingInfo(startAreaID);
    for (int i = 0; i < theSurroundingInfo->Get_ringCount(); ++i) {
        auto theRing_byFOr = theSurroundingInfo->Get_ringInfo(i);
        int areaID = -1;
        for (int j = 0; j < theRing_byFOr->size(); ++j) {
            CArea* area_byFor = g_Scene.GetArea((*theRing_byFOr)[j]->ID);
            if (g_Scene.Is_waring(startAreaID, area_byFor->ID)){
                areaID = area_byFor->ID;
                if(g_GameManager.GetRand() % 100 +1 < 50){
                    return areaID;
                }
            }
        }
        if (areaID != -1){
            return areaID;
        }
    }
    return -1;
}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 得到周围一定圈层，最高价值的地块。同时敌军越少，价值也越大。<p>
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * 返回结果 <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： 2022 <p>
     * 【修改时间】： ？？？
     */
int NewAiAction::Get_areaID_ofHighestMilitaryValueAround(int startAreaID, int maxRingCount) {
    std::vector<int> list_areaID;
    std::vector<int> list_areaValue;
    auto info = g_areaSurroundingInfoManager.Get_theAreaSurroundingInfo(startAreaID);
    int ringCount = (*info).Get_ringCount();
    if (maxRingCount>ringCount)
        maxRingCount = ringCount;
    for (int ringNumber = 0; ringNumber < maxRingCount; ++ringNumber) {
        auto theRing_byFOr = info->Get_ringInfo(ringNumber);
        for (int i = 0; i < theRing_byFOr->size(); ++i) {
            auto area = g_Scene.GetArea((*theRing_byFOr)[i]->ID);
            if (g_Scene.Is_waring(startAreaID,area->ID)){
                list_areaID.push_back(area->ID);
                list_areaValue.push_back(area->Get_valueMilitary(ringNumber));
            }
        }
    }
    if(list_areaID.size() == 0)
        return -1;
//    std::vector<int> list;
//    for (int i = 0; i < list_area.size(); ++i) {
//        int num = info->Get_ringNumber_targetAreaOnRing(list_area[i]->ID);
//        list.push_back(list_area[i]->Get_valueMilitary(num));
//    }
    //快排没问题，但是好像有显示bug
//    g_algorithm.QuickSort_areaValue(0, list_area.size() - 1, false, list_area, *info);
    g_algorithm.QuickSort_value(0,list_areaID.size()-1,list_areaValue,list_areaID,false);
//
//    g_algorithm.QuickSort_value(0, list.size() - 1, list);

    return list_areaID[0];
}

int NewAiAction::Get_areaID_ofBestRemoteArmyAttackEnemy(int startAreaID, int maxRingCount) {
    auto startArea = g_Scene[startAreaID];
    auto theSurroundingInfo = g_areaSurroundingInfoManager.Get_theAreaSurroundingInfo(startAreaID);
    for (int i = 0; i < theSurroundingInfo->Get_ringCount(); ++i) {
        auto theRing_byFOr = theSurroundingInfo->Get_ringInfo(i);
        int areaID = -1;
        for (int j = 0; j < theRing_byFOr->size(); ++j) {
            CArea* area_byFor = g_Scene.GetArea((*theRing_byFOr)[j]->ID);
            if (g_Scene.Is_waring(startAreaID, area_byFor->ID) && area_byFor->ArmyCount > 0){
                auto  enemyRingInfo = g_areaSurroundingInfoManager.Get_theAreaSurroundingInfo(area_byFor->ID);
                auto secondRingInfo = enemyRingInfo->Get_ringInfo(1);
                for (int k = 0; k < secondRingInfo->size(); ++k) {
                    auto area_ofSecondRing = g_Scene.GetArea((*secondRingInfo)[k]->ID);
                    if (area_ofSecondRing->Country != nullptr && area_ofSecondRing->ID != startAreaID && area_ofSecondRing->Country == startArea->Country &&
                    area_ofSecondRing->Is_existMilitaryThreat(1,0,startArea->Country->Alliance) == false) {
                        areaID = area_ofSecondRing->ID;
                        if (g_GameManager.GetRand() % 100 + 1 < 30) {
                            return areaID;
                        }
                    }
                }
            }
        }
        if (areaID != -1){
            return areaID;
        }
    }
    return -1;
}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 获取所有受到军事威胁的地块，然后按照军事价值建造1个单位。<p>
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * void <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： 2022/10/11 <p>
     * 【修改时间】： ？？？
     */
void NewAiAction::Add_newArmy_toImportantTown(CCountry* theCountry) {
    std::vector<int> list_areaID;
    std::vector<int> list_value;

    for(auto areaID : theCountry->AreaIDList){
        auto area = g_Scene.GetArea(areaID);
        if (area->Is_supplyCity() && area->Is_existMilitaryThreat(3,0,theCountry->Alliance) && area->ArmyCount < 1){
            list_areaID.push_back(area->ID);
            list_value.push_back(area->Get_valueMilitary(-1));
        }
    }
    g_algorithm.QuickSort_value(0,list_value.size()-1,list_value,list_areaID,false);
    auto infantryCard = CObjectDef::Instance()->GetCardDef(CARD_ID::InfantryCard);
    auto tankCard = CObjectDef::Instance()->GetCardDef(CARD_ID::TankCard);
    auto armourCard = CObjectDef::Instance()->GetCardDef(CARD_ID::ArmourCard);
//价值垫底就不管了
   int infantrysPrice = infantryCard->Price * (list_areaID.size()-1);
    int infantryIndustry = infantryCard->Industry * (list_areaID.size()-1);
    for (int i = 0; i < list_areaID.size(); ++i) {

        auto area = g_Scene.GetArea(list_areaID[i]);
        if (theCountry->Money - infantrysPrice >= tankCard->Price * 2 &&
            theCountry->Money >= tankCard->Price && theCountry->Industry >= tankCard->Industry) {
            if (theCountry->CheckCardTargetArea(tankCard, area->ID)) {
                Add_newArmy_toArea(area->ID, ArmyDef::Tank);
                theCountry->Money -= tankCard->Price;
                theCountry->Industry -= tankCard->Industry;
            }
        }
        else if (theCountry->Money - infantrysPrice >= armourCard->Price * 1.5 &&
                 theCountry->Money >= armourCard->Price &&
                 theCountry->Industry >= armourCard->Industry) {
            if (theCountry->CheckCardTargetArea(armourCard, area->ID)) {
                Add_newArmy_toArea(area->ID, ArmyDef::Panzer);
                theCountry->Money -= armourCard->Price;
                theCountry->Industry -= armourCard->Industry;
            }

        }
        else if (theCountry->Money >= infantryCard->Price &&
                 theCountry->Industry >= infantryCard->Industry) {
            if (theCountry->CheckCardTargetArea(infantryCard, area->ID)) {
                Add_newArmy_toArea(area->ID, ArmyDef::InfantryD1905);
                theCountry->Money -= infantryCard->Price;
                theCountry->Industry -= infantryCard->Industry;
            }
        }
    }
}


CCountry* CGameManager::GetCurCountry(){
    if(this->CurrentCountryIndex < 0 || this->CurrentCountryIndex >= this->ListCountry.size())
        this->CurrentCountryIndex = 0;
    return this->GetCountryByIndex(this->CurrentCountryIndex);
}

/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 得到远程单位射击范围内的所有敌军。这些敌军都一定可以攻击。<p>
     * 【参数说明】：<p>
     * startAreaID：不能为空值。<p>
     * 【返回值说明】：<p>
     * 记得释放。 <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： 2022/8/2 <p>
     * 【修改时间】： ？？？
     */
std::vector<int>* CArea::Get_areas_existEnemy_inRange( int armyIndex) {
    std::vector<int>* list_area_existEnemy_inRange = new std::vector<int>();
    auto army = this->GetArmy(armyIndex);
    auto list_area_inRange =  this->Get_areas_inRange(armyIndex);
    for (int i = 0; i < list_area_inRange->size(); ++i) {
        CArea* foundArea_byFOr = (*list_area_inRange)[i];
        //该地块可以被攻击则添加进去
        if (g_Scene.CheckAttackable(this->ID,foundArea_byFOr->ID,armyIndex))
            list_area_existEnemy_inRange->push_back(foundArea_byFOr->ID);
    }
    delete list_area_inRange;
    return list_area_existEnemy_inRange;
}
//为单位恢复血量和组织度
void CArea::Recover_armyState() {
    int cityLevel = this->GetCityLevel();
    int industryLevel = this->GetIndustryLevel();
    int conscructLevel = 0;
    int replenishment = 0;
    if (cityLevel > industryLevel) {
        conscructLevel = cityLevel;
    }
    else {
        conscructLevel = industryLevel;
    }
    switch (this->Type) {
        case capital:
            replenishment = 8;
            break;
        case large_city:
            replenishment = 6;
            break;
        case normal_city :
            replenishment = 4;
            break;
        case port:
            replenishment = 5;
            break;
        default:
            replenishment = 2;
            break;
    }
    replenishment += 3 * conscructLevel;
    for (int i = 0; i < this->ArmyCount; ++i) {
        CArmy *theArmy = this->Army[i];
        //如果是港口并且是海军
        if (this->Type == port&&theArmy->IsNavy()) {
            //回血量*2
            replenishment *= 2;
        }
        else if (theArmy->BasicAbilities->ID == ArmyDef::IntercontinentalMissile) {
            //如果是洲际导弹发射车，回血量是2
            replenishment = 2;
        }
        theArmy->AddStrength(replenishment);
        if (g_GameManager.IsIntelligentAI && (theArmy->Country->AI || g_GameManager.Is_spectetorMode())) {
            if (this->Retreat_missileArmy(theArmy)) {
                if (i != 0) {
                    i--;
                }
                continue;
            }
        }
       int newReplenishment = 0;
        //如果存在指挥官
        if (theArmy->HasCard(CArmy::Commander)) {
            int level1 = this->Country->GetCommanderLevel();
            if (GetCommanderAbility(level1).Supply > GetArmySupply(level1)) {
                newReplenishment += GetCommanderAbility(level1).Supply;
            }
            else {
                newReplenishment += GetArmySupply(level1);
            }
        }
        else{
            theArmy->AddStrength(newReplenishment);
        }
        theArmy->TurnEnd();
    }
}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 这个地块周围是否存在某个阵营,且地块的军队数必须符合一定数量或存在某种建筑。<p>
     * 【参数说明】：<p>
     * construction：如果为1，则表示存在雷达。<p>
     * 【返回值说明】：<p>
     * 判断结果 <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： 2022 <p>
     * 【修改时间】： ？？？
     */
bool CArea::Is_existTheCamp(int ringCount,int allanceOfTheCamp,int armyCount,int construction) {
    auto areaCircleInfo = g_areaSurroundingInfoManager.Get_theAreaSurroundingInfo(this->ID);
    //循环指定的圈层数
    for (int i = 0; i < ringCount; ++i) {
        std::vector<CAreaOnRing*>* theRingInfo_byFor = areaCircleInfo->Get_ringInfo(i);
        if(theRingInfo_byFor == nullptr)
            continue;
        //循环当前圈层的地块数
        int areas_theRing = theRingInfo_byFor->size();
        for (int j = 0; j < areas_theRing; ++j) {
            int foundAreaID = (*theRingInfo_byFor)[j]->ID;
            CArea* foundArea = g_Scene[foundAreaID];
            if (foundArea == nullptr ||foundArea->Enable == false ||Is_inSpecifiedRangeOfAreas(foundArea->ID) == false)
                continue;
            if (foundArea->Country != nullptr && foundArea->Country->Alliance == allanceOfTheCamp){
                if (foundArea->ArmyCount > armyCount){
                    return true;
                }
                else{
                    switch (construction) {
                        case 1:
                            if (foundArea->InstallationType == radar)
                                return true;
                            break;
                        default:
                            break;
                    }
                }
            }
        }
    }
    return false;
}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 判断这个地块是否存在精锐部队。<p>
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * 返回结果 <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： 2022/9/2 <p>
     * 【修改时间】： ？？？
     */
bool CArea::Exist_eliteArmy() {
    for (int i = 0; i < this->ArmyCount; ++i) {
        if (this->GetArmy(i)->Is_eliteArmy())
            return true;
    }
    return false;
}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 得到这个地块的军事价值。和经济，属性挂钩。且单位越少，价值也会在增加。<p>
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * void <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： 2022 <p>
     * 【修改时间】： ？？？
     */
int CArea::Get_valueMilitary(int ringNumber) {

    int value = 0;
    if (this->Type != normal){
        if(this->Type == capital){
            value += 200;
        }
        else if (this->Type == large_city){
            value +=130;
        }
        else if (this->Type == normal_city){
            value += 50;
        }
        if(this->ArmyCount < 2)
            value += 50;
        if(this->ArmyCount < 1)
            value+= 200;
        if (this->CheckEncirclement()){
            value += 150;
        }
        if (ringNumber != -1){
            if (ringNumber <= 3){
                value += (4-ringNumber)*100;
            }
        }
    }
    if (this->ConstructionLevel > 1)
        value += 20;
    value += this->GetRealTax();
    value += this->GetIndustry()*2;

    value += this->Get_oil();

    return value;
}


void NewCountryAction::Start_countryAction(CCountry* actionCountry) {
    this->ActionCountry = actionCountry;
    this->Init_Array();
    //先把相关数据提前收集好
    this->Collect_data();
    //第一步、精锐前移至有利战机地块
    if (g_GameManager.IsIntelligentAI){
        this->EliteArmyTo_opportunityArea();
    }
    //第二部，普通单位填满尚未填满的有利战地地块
    this->NormalArmyTo_opportunityArea();

    //第二步、陆空协同作战
//    this->AirGroundOperation();


}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 收集行动前所需的各种信息。<p>
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * void <p>
     *=======================================================<p>
     * 【创建人】：李德邻 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： 2022/7/30 <p>
     * 【修改时间】： ？？？
     */
void NewCountryAction::Collect_data() {
    //全局数组arr_foundArea 将会用于记录地块是否为前线地块
    Init_sArr_foundArea();
    auto ss = this->ActionCountry->AreaIDList;
    for(int i : this->ActionCountry->AreaIDList ){
        CArea* foundArea_byFor = g_Scene.GetArea(i);
        this->Collect_area_frontLine(foundArea_byFor);
        this->Collect_arae_existArmy(foundArea_byFor);
    }
    for (int i : this->List_area_frontLine) {
        CArea* foundArea_byFor = g_Scene.GetArea(i);
        this->Collect_area_opportunityForAttack(foundArea_byFor);
    }
}
//收集伤兵地块
void NewCountryAction::Collect_area_wounded(CArea* collectedArea) {
    for (int i = 0; i < collectedArea->ArmyCount; ++i) {
        if (collectedArea->GetArmy(i)->Is_wounded()){
            this->List_area_wounded.push_back(collectedArea->ID);
            break;
        }
    }
}
//收集前线地块
void NewCountryAction::Collect_area_frontLine(CArea* collectArea) {
    int size = g_Scene.GetNumAdjacentAreas(collectArea->ID);
    for (int i = 0; i < size; ++i) {
        CArea* aroundArea_byFOr = g_Scene.GetAdjacentArea(collectArea->ID,i);
        if (aroundArea_byFOr->Is_existMilitaryThreat(1,-1,this->ActionCountry->Alliance)){
            this->List_area_frontLine.push_back(collectArea->ID);
            this->Arr_area_isFrontLine[collectArea->ID] = true;
            break;
        }
    }
}
//收集我方存在军队的地块
void NewCountryAction::Collect_arae_existArmy(CArea* collectArea) {
    if(collectArea->ArmyCount > 0){
        this->List_area_existArmy.push_back(collectArea->ID);
        this->Collect_area_wounded(collectArea);
        this->Collect_area_existEliteArmy(collectArea);
        this->Collect_area_existAirForce(collectArea);
    }
}
///**
//     *=======================================================<p>
//     * 【功能描述】：<p>
//     * 负责游戏场景的所有绘制。<p>
//     * 【参数说明】：<p>
//     * 【返回值说明】：<p>
//     * void <p>
//     *=======================================================<p>
//     * 【创建人】： 柒安 <p>
//     * 【修改人】： 李德邻<p>
//     * 【创建时间】： ？？？ <p>
//     * 【修改时间】： 2022/8/12
//     */
//void CScene::Render() {
//    //TODO 新地图（地块大于等于原版地块数）可以把这个注释的函数打开。
//    if (AreaList.empty())
//        return;
//    float xFactor = g_contenscalefactor ;
//    CCountry *PlayerCountry = g_GameManager.GetPlayerCountry();
//    CCamera.Apply();
//    CBackground->Render(&CCamera);
//    int SelectedAreaID = -1;
//    if (AreaSelected != nullptr && AreaSelected->ID >= 0 && !g_GameManager.Get_skipMode()) {
//        SelectedAreaID = AreaSelected->ID;
//        if (AreaSelected->Country != nullptr)
//            ZoneImage[SelectedAreaID]->SetColor(*(int *) &AreaSelected->Country->R, -1);
//        else
//            ZoneImage[SelectedAreaID]->SetColor(0x80FF0000, -1);
//        ZoneImage[SelectedAreaID]->SetAlpha(FlashTimer, -1);
//        ZoneImage[SelectedAreaID]->RenderEx(AreasData[SelectedAreaID].AreaPos[0], AreasData[SelectedAreaID].AreaPos[1],
//                                            0.0, xFactor, 0.0);
//    }
////    for (int i = 0; i < s_Count_theMapAreas; i+=2) {
////        if (i >= 0 && !g_GameManager.Get_skipMode()) {
////            ZoneImage[SelectedAreaID]->SetTextureRect(AreasData[SelectedAreaID].AreaPos[0],AreasData[SelectedAreaID].AreaPos[1],
////                                                      AreasData[SelectedAreaID].AreaSize[0]/2,AreasData[SelectedAreaID].AreaSize[1]/2);
////
////            ZoneImage[i]->SetColor(0xFF8080FF, -1);
////            ZoneImage[i]->SetAlpha(FlashTimer, -1);
////            ZoneImage[i]->RenderEx(AreasData[i].AreaPos[0], AreasData[i].AreaPos[1], 0.0, xFactor, 0.0);
////        }
////    }
//    for (int i : FightFlashAreaID) {
//        if (i >= 0 && !g_GameManager.Get_skipMode()) {
//            ZoneImage[SelectedAreaID]->SetTextureRect(AreasData[SelectedAreaID].AreaPos[0],AreasData[SelectedAreaID].AreaPos[1],
//                                                      AreasData[SelectedAreaID].AreaSize[0]/2,AreasData[SelectedAreaID].AreaSize[1]/2);
//
//            ZoneImage[i]->SetColor(0xFF8080FF, -1);
//            ZoneImage[i]->SetAlpha(FlashTimer, -1);
//            ZoneImage[i]->RenderEx(AreasData[i].AreaPos[0], AreasData[i].AreaPos[1], 0.0, xFactor, 0.0);
//        }
//    }
//    for (int i = 0; i < s_Count_theMapAreas; i+=2) {
//        CArea *RenderArea = RenderAreaList[i + 1];
//        if (RenderArea->ID != SelectedAreaID && CCamera.IsRectInCamera(AreasData[RenderArea->ID].AreaPos[0],
//                                                                       AreasData[RenderArea->ID].AreaPos[1],
//                                                                       AreasData[RenderArea->ID].AreaSize[0],
//
//                                                                       AreasData[RenderArea->ID].AreaSize[1]) &&
//            ZoneImage[RenderArea->ID] != nullptr && !RenderArea->Sea) {
//            if (!RenderArea->Enable) {
//                ZoneImage[RenderArea->ID]->SetColor(-1, -1);
//            } else if (RenderArea->Country != nullptr) {
//                ZoneImage[RenderArea->ID]->SetColor(*(int *) &RenderArea->Country->R, -1);
//                if (NewRoundFlashing && RenderArea->Country == g_GameManager.GetCurCountry())
//                    ZoneImage[RenderArea->ID]->SetAlpha(FlashTimer, -1);
//            }
//            ZoneImage[RenderArea->ID]->RenderEx(AreasData[RenderArea->ID].AreaPos[0],
//                                                AreasData[RenderArea->ID].AreaPos[1],
//                                                0.0, xFactor, 0.0);
//        }
//    }
//    for (int i = 0; i < s_Count_theMapAreas; i+=2) {
//        CArea *RenderArea = RenderAreaList[i + 1];
//        if (CCamera.IsRectInCamera(AreasData[RenderArea->ID].AreaPos[0], AreasData[RenderArea->ID].AreaPos[1],
//                                   AreasData[RenderArea->ID].AreaSize[0], AreasData[RenderArea->ID].AreaSize[1])) {
//            RenderArea->RenderBuilding();
//        }
//    }
//    for (int i = 0; i < s_Count_theMapAreas; i+=2) {
//        CArea *RenderArea = RenderAreaList[i + 1];
//        if (CCamera.IsRectInCamera(AreasData[RenderArea->ID].AreaPos[0], AreasData[RenderArea->ID].AreaPos[1],
//                                   AreasData[RenderArea->ID].AreaSize[0], AreasData[RenderArea->ID].AreaSize[1])) {
//            RenderArea->Render();
//        }
//    }
//    for (int i = 0; i < s_Count_theMapAreas; i+=2) {
//        CArea *RenderArea = RenderAreaList[i + 1];
//        if (CCamera.IsRectInCamera(AreasData[RenderArea->ID].AreaPos[0], AreasData[RenderArea->ID].AreaPos[1],
//                                   AreasData[RenderArea->ID].AreaSize[0], AreasData[RenderArea->ID].AreaSize[1])) {
//            if (RenderArea->ArmyCount > 0) {
//                CArmy *Army = RenderArea->GetArmy(0);
//                bool IsEnemy = PlayerCountry != nullptr && PlayerCountry->Alliance != Army->Country->Alliance;
//            }
//        }
//    }
//    for (int i = 0; i < s_Count_theMapAreas; i+=2) {
//        CArea *RenderArea = RenderAreaList[i + 1];
//        if (RenderArea->TargetType == 1) {
//            g_GameRes.Image_arrowshadow->RenderEx(RenderArea->ArmyPos[0] - VerticalArrowTimer * 0.5,
//                                                  RenderArea->ArmyPos[1] + VerticalArrowTimer * 0.5, 0.0, 2.0, 0.0);
//            g_GameRes.Image_arrow_green->Render(RenderArea->ArmyPos[0], RenderArea->ArmyPos[1] + this->VerticalArrowTimer);
//        } else if (RenderArea->TargetType == 2) {
//            g_GameRes.Image_arrowshadow->RenderEx(RenderArea->ArmyPos[0] - VerticalArrowTimer * 0.5,
//                                                  RenderArea->ArmyPos[1] + VerticalArrowTimer * 0.5, 0.0, 2.0, 0.0);
//            g_GameRes.Image_arrow_yellow->Render(RenderArea->ArmyPos[0], RenderArea->ArmyPos[1] + this->VerticalArrowTimer);
//        }
//    }
//    CBackground->RenderBox(&CCamera, TopLeftPos[0], TopLeftPos[1], Size[0], Size[1]);
//    if (AreaSelected != nullptr) {
//        if (AreaSelected->ConstructionType == CArea::airport && AreaSelected->Country != nullptr) {
//            ecGraphics::Instance()->ecGraphics::SetBlendMode(2);
//            ecGraphics::Instance()->RenderCircle(AreaSelected->ConstructionPos[0], AreaSelected->ConstructionPos[1],
//                                                 AreaSelected->Country->AirstrikeRadius(), 0x4F000000);
//        } else if (AreaSelected->Sea && AreaSelected->ArmyCount > 0 && AreaSelected->GetArmy(0)->BasicAbilities->ID == ArmyDef::AircraftCarrier) {
//            ecGraphics::Instance()->ecGraphics::SetBlendMode(2);
//            ecGraphics::Instance()->RenderCircle(AreaSelected->ArmyPos[0], AreaSelected->ArmyPos[1], AreaSelected->Country->AirstrikeRadius(), 0x4F800000);
//        }
//        for (int i = 0; i < s_Count_theMapAreas; ++i) {
//            CArea* foundArea_byFOr = g_Scene.GetArea(g_Scene.GetAdjacentArea(AreaSelected->ID,i)->ID);
//            if (foundArea_byFOr->TargetType == 3)
//                g_GameRes.Image_arrow_blue->Render4VC(foundArea_byFOr->ArrowEndOffset[0], foundArea_byFOr->ArrowEndOffset[1],
//                                                      foundArea_byFOr->ArrowEndOffsetMinus[0], foundArea_byFOr->ArrowEndOffsetMinus[1],
//                                                      foundArea_byFOr->ArrowStartOffsetMinus[0], foundArea_byFOr->ArrowStartOffsetMinus[1],
//                                                      foundArea_byFOr->ArrowStartOffset[0], foundArea_byFOr->ArrowStartOffset[1], 0, ArrowTimer);
//            else if(foundArea_byFOr->TargetType == 4)
//                g_GameRes.Image_arrow_red->Render4VC(foundArea_byFOr->ArrowEndOffset[0], foundArea_byFOr->ArrowEndOffset[1],
//                                                     foundArea_byFOr->ArrowEndOffsetMinus[0], foundArea_byFOr->ArrowEndOffsetMinus[1],
//                                                     foundArea_byFOr->ArrowStartOffsetMinus[0], foundArea_byFOr->ArrowStartOffsetMinus[1],
//                                                     foundArea_byFOr->ArrowStartOffset[0], foundArea_byFOr->ArrowStartOffset[1], 0, ArrowTimer);
//        }
//    }
//    for (CMedal *Medal : MedalList)
//        Medal->Render();
//    if (CBomber != nullptr && !g_GameManager.Get_skipMode())
//        CBomber->Render();
//    for (CWarGas *Gas : g_GasList)
//        if (Gas->IsActive)
//            Gas->Render();
//}

//收集我方存在精英部队的地块
void NewCountryAction::Collect_area_existEliteArmy(CArea* collectArea) {
    for (int i = 0; i < collectArea->ArmyCount; ++i) {
        if (collectArea->GetArmy(i)->Is_eliteArmy()){
            this->Arr_area_existEliteArmy[collectArea->ID] = true;
            this->Map_area_existEliteArmy[collectArea->ID]++;
        }
    }
//     if (collectArea->Exist_eliteArmy()){
//         this->List_area_existEliteArmy.push_back(collectArea->ID);
//         this->Arr_area_existEliteArmy[collectArea->ID] = true;
//     }
}
//收集我方存在空军的地块
void NewCountryAction::Collect_area_existAirForce(CArea* collectArea) {
    for (int i = 0; i < collectArea->ArmyCount; ++i) {
        if (collectArea->GetArmy(i)->Is_airForce()){
            this->List_area_existAirForce.push_back(collectArea->ID);
            break;
        }
    }
}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 收集我方进攻敌军地块的有利地块。要求符合以下条件：<p>
     * 1：敌我兵力比小于100%。<p>
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * void <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： 2022/7/31<p>
     * 【修改时间】： 2022/8/21
     */
void NewCountryAction::Collect_area_opportunityForAttack(CArea* collectArea) {
    int selfCount = 0;
    int enemyCount = 0;
    selfCount = collectArea->Get_countOfDesigantions();

    int count_aroundAreas = g_Scene.GetNumAdjacentAreas(collectArea->ID);
    for (int i = 0; i < count_aroundAreas ; ++i) {
        CArea* aroundArea_byFOr = g_Scene.GetAdjacentArea(collectArea->ID,i);
        if (g_Scene.Is_waring(collectArea->ID, aroundArea_byFOr->ID)){
            //高级AI有70%概率猜中隐藏的军级单位
            if (g_GameManager.IsIntelligentAI){
                int realCount = aroundArea_byFOr->Get_countOfDesigantions();
                int tempCount = realCount - enemyCount;
                for (int j = 0; j < tempCount; ++j) {
                    int r = GetRand()%100 +1;
                    if (r >= 70)
                        enemyCount++;
                }
            }
            else{
                if (aroundArea_byFOr->ArmyCount > 0 ){
                    if (aroundArea_byFOr->GetArmy(0)->IsA)
                        enemyCount+=aroundArea_byFOr->GetArmy(0)->Get_count_oldDesignation()-1;
                    enemyCount += aroundArea_byFOr->ArmyCount;
                }
                else{
                    enemyCount += aroundArea_byFOr->ArmyCount;
                }
            }
        }
            //匹配周围是前线地块的我方地块
        else if(aroundArea_byFOr->Country == this->ActionCountry && this->Arr_area_isFrontLine[aroundArea_byFOr->ID]){
            selfCount+= aroundArea_byFOr->Get_countOfDesigantions();
        }
    }
    //注意：除数不能为0
    if(selfCount == 0)
        selfCount = 1;
    //敌我比
    int radio = enemyCount * 100 / selfCount;
    if (radio <= 100){
        this->List_area_opportunityForAttack.push_back(collectArea->ID);
        this->Arr_area_isOpportunityForAttack[collectArea->ID] = true;
    }
}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 非前线的精锐部队向存在有利战机之地块前进。并要求：<p>
     * 1：精锐部队抵达后，移动力必须大于等于1。<p>
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * void <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： 2022/7/31<p>
     * 【修改时间】： ？？？
     */
void NewCountryAction::EliteArmyTo_opportunityArea() {
    for(auto areaID : this->List_area_existEliteArmy){
        //精英部队必须不位于前线
        if (this->Arr_area_isFrontLine[areaID] == false){
            CArea* area_existEliteArmy_byFor = g_Scene.GetArea(areaID);
            this->ChoosedAreaID = areaID;
            //循环精英单位地块的所有单位
            for (int i = 0; i < area_existEliteArmy_byFor->ArmyCount;) {
                CArmy* foundArmy_byFor = area_existEliteArmy_byFor->GetArmy(i);
                this->ChoosedArmyIndex = i;
                if (foundArmy_byFor->Is_eliteArmy()){
                    //没有比这个更近的有利战机地块了
                    int areaID_opportunityForAttack = this->Find_nearestAreaID_opportunityForAttack(area_existEliteArmy_byFor, foundArmy_byFor, foundArmy_byFor->Movement-1);
                    //如果指定范围没找到，直接下一个单位。
                    if (areaID_opportunityForAttack == -1){
                        i++;
                        continue;
                    }
                    //这个单位现在必定行动成功
                    this->Map_area_existEliteArmy[area_existEliteArmy_byFor->ID]--;
                    if (this->Map_area_existEliteArmy[area_existEliteArmy_byFor->ID]==0)
                        this->Arr_area_existEliteArmy[area_existEliteArmy_byFor->ID] = false;
                    std::stack<int>* shortestPath =  g_areaSurroundingInfoManager.Get_shortestPath(2, area_existEliteArmy_byFor->ID, areaID_opportunityForAttack,i);
//                    int consume_line = g_newAiAction.Get_consume_aboutTheline(*shortestPath,foundArmy_byFor);
                    g_newAiAction.Try_TheArmy_toTargetArea_alongOptimalRoute(area_existEliteArmy_byFor->ID, foundArmy_byFor, *shortestPath);
                    delete shortestPath;
                }

            }
        }
    }
}

//
void NewCountryAction::QuickSort_area_opportunityForAttack() {

}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 查找最近的一个有利战机地块。该地块单位数量一定小于4.<p>
     * 【参数说明】：<p>
     * ringCount：循环的范围<p>
     * 【返回值说明】：<p>
     * 最近的有利战机地块ID。如果没找到，返回-1<p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： 2022/7/31 <p>
     * 【修改时间】： 2022/9/1
     */
int NewCountryAction::Find_nearestAreaID_opportunityForAttack(CArea* startArea, CArmy* findingArmy, int ringCount) {
    CAreaSurroundingInfo* startArea_surroundingInfo = g_areaSurroundingInfoManager.Get_theAreaSurroundingInfo(startArea->ID);
    for (int i = 0; i < ringCount; ++i) {
        std::vector<CAreaOnRing*>* ringInfo = startArea_surroundingInfo->Get_ringInfo(i);
        int areas_inTheRing = ringInfo->size();
        for (int j = 0; j < areas_inTheRing; ++j) {
            int foundAreaID = (*ringInfo)[j]->ID;
            if (this->Arr_area_isOpportunityForAttack[foundAreaID] && g_Scene.GetArea(foundAreaID)->ArmyCount < 4){
                return foundAreaID;
            }
        }
    }
    return -1;
}
bool NewCountryAction::Arr_area_isOpportunityForAttack[s_Count_theMapAreas]{};
bool NewCountryAction::Arr_area_isFrontLine[s_Count_theMapAreas]{};
bool NewCountryAction::Arr_area_existEliteArmy[s_Count_theMapAreas]{};

NewCountryAction::NewCountryAction() {
//    //避免多次重复“扩容”带来的性能开销。
//    this->List_area_existArmy.resize(40);
//    this->List_area_frontLine.resize(20);
//    this->List_area_existEliteArmy.resize(20);
//    this->List_area_opportunityForAttack.resize(10);
}

void NewCountryAction::Init_Array() {
    for (int i = 0; i < s_Count_theMapAreas; ++i) {
        this->Arr_area_isOpportunityForAttack[i] = false;
        this->Arr_area_isFrontLine[i] = false;
        this->Arr_area_existEliteArmy[i] = false;
    }

}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 协同作战陆空：<p>
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * void <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： 2022/8/2 <p>
     * 【修改时间】： ？？？
     */
void NewCountryAction::AirGroundOperation() {
    for (int i = 0; i < this->List_area_existAirForce.size(); ++i) {
        if (this->List_area_existAirForce[i] == -1)
            continue;
        this->ChoosedAreaID = this->List_area_existAirForce[i];
        CArea* area_existAirForce_byFor = g_Scene.GetArea(this->ChoosedAreaID);
        //战斗前的军队数量
        int startArmyCount = area_existAirForce_byFor->ArmyCount;
        for (int j = 0; j < startArmyCount; ) {
            auto airForce_byfor = area_existAirForce_byFor->GetArmy(j);
            //血量得在25%及其以上
            if(airForce_byfor->Is_airForce() && airForce_byfor->GetNumDices() > 3){
                this->ChoosedArmyIndex = j;
                this->Adjust_armyToTopOfArea();
                std::vector<int>* list_area_existEnemy_inRange = area_existAirForce_byFor->Get_areas_existEnemy_inRange(this->ChoosedArmyIndex);
                //这个地块一定可以攻击
                int areaID_attackTarget = g_newAiAction.Get_areaID_ofHighestValueWeakTargetAround(*list_area_existEnemy_inRange);
                delete list_area_existEnemy_inRange;
                CArea* targetArea = g_Scene.GetArea(areaID_attackTarget);
                CArea* startArea = g_Scene.GetArea(this->ChoosedAreaID);
                //开始攻击
                ((CGameState *) CStateManager::Instance()->GetStatePtr(Game))->StartBattale(
                        startArea->ID,targetArea->ID, 1,false);
            }
            //数量没变动才开始下一循环
            if (startArmyCount == area_existAirForce_byFor->ArmyCount)
                ++j;
            else
                startArmyCount = area_existAirForce_byFor->ArmyCount;
        }
    }
    //普通AI：只要飞机血量还在30%以上无脑进攻

}
//把当前选中的单位移动到地块上方。
void NewCountryAction::Adjust_armyToTopOfArea() {
    auto area_needAdjust = g_Scene.GetArea(this->ChoosedAreaID);
    if (area_needAdjust->ArmyCount == 1)
        return;
    CArmy* tempArmy = area_needAdjust->GetArmy(0);
    area_needAdjust->Army[0] = area_needAdjust->GetArmy(this->ChoosedArmyIndex);
    area_needAdjust->Army[this->ChoosedArmyIndex] = tempArmy;
    this->ChoosedArmyIndex = 0;
}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 普通单位填满尚未填满的有利战地地块<p>
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * void <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： 2022/9/1 <p>
     * 【修改时间】： 2022/9/2
     */
void NewCountryAction::NormalArmyTo_opportunityArea() {
    for(auto opportunityArea_ID : this->List_area_opportunityForAttack){
        CArea* area_opportunityForAttack = g_Scene.GetArea(opportunityArea_ID);
        if (area_opportunityForAttack->ArmyCount < 4){
            this->ChoosedAreaID = opportunityArea_ID;
            while (area_opportunityForAttack->ArmyCount < 4){
                int areaID_nearestArmy = this->Find_areaID_nearestArmy(area_opportunityForAttack, 3);
                //如果找不到，就说明周围已经没有合适的单位了，直接退出循环。
                if (areaID_nearestArmy == -1)
                    break;
                std::stack<int>* shortestPath =  g_areaSurroundingInfoManager.Get_shortestPath(2, areaID_nearestArmy, opportunityArea_ID,this->ChoosedArmyIndex);
                CArmy* nearestArmy = g_Scene.GetArea(areaID_nearestArmy)->GetArmy(this->ChoosedArmyIndex);
//                int consume_line = g_newAiAction.Get_consume_aboutTheline(*shortestPath,nearestArmy);
                g_newAiAction.Try_TheArmy_toTargetArea_alongOptimalRoute(areaID_nearestArmy, nearestArmy, *shortestPath);
                delete shortestPath;
            }
        }
    }
}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 查找指定范围内最近的一个单位。该单位的移动力必须大于圈层数，这才能保证到达目的地后行动力大于等于1.<p>
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * 如果没找到，返回-1 <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： 2022/9/1 <p>
     * 【修改时间】： ？？？
     */
int NewCountryAction::Find_areaID_nearestArmy(CArea* startArea, int ringCount) {
    CAreaSurroundingInfo* startArea_surroundingInfo = g_areaSurroundingInfoManager.Get_theAreaSurroundingInfo(startArea->ID);
    for (int i = 0; i < ringCount; ++i) {
        std::vector<CAreaOnRing*>* ringInfo = startArea_surroundingInfo->Get_ringInfo(i);
        if(ringInfo == nullptr)
            break;
        int areas_inTheRing = ringInfo->size();
        for (int j = 0; j < areas_inTheRing; ++j) {
            int foundAreaID = (*ringInfo)[j]->ID;
            CArea* foundArea_byFor = g_Scene.GetArea(foundAreaID);
            for (int k = 0; k < foundArea_byFor->ArmyCount; ++k) {
                CArmy* foundArmy_byFor = foundArea_byFor->GetArmy(k);
                if ((foundArmy_byFor->Is_fightPlane() == false || foundArmy_byFor->Is_remoteAttack() == false )&& foundArmy_byFor->Movement > i+1){
                    this->ChoosedArmyIndex = k;
                    return foundAreaID;
                }
            }
        }
    }
    return -1;
}

/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 传入一系列地块，获取每个地块最上方单位的军队价值，然后进行排列。<p>
     * 【参数说明】：<p>
     * list_areaID：请确定里面的地块都有单位存在。<p>
     * 【返回值说明】：<p>
     * void <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： 2022/8/4 <p>
     * 【修改时间】： ？？？
     */
void Algorithm::QuickSort_armyValue(int leftIndex, int rightIndex, bool isSmallToLarge, std::vector<int>& list_areaID) {
    if (leftIndex >= rightIndex)
        return;
    int temp;
    CArea* baseArea = g_Scene.GetArea(list_areaID[leftIndex]);
    int baseArmyValue = baseArea->GetArmy(0)->Get_armyValue();
    int i = leftIndex;
    int j = rightIndex;
    while (i < j){
        if (isSmallToLarge){
            while (g_Scene.GetArea( list_areaID[j])->GetArmy(0)->Get_armyValue() >= baseArmyValue && i < j) j--;
            while (g_Scene.GetArea( list_areaID[i])->GetArmy(0)->Get_armyValue() <= baseArmyValue && i < j) i++;
        }
        else{
            while (g_Scene.GetArea( list_areaID[j])->GetArmy(0)->Get_armyValue() <= baseArmyValue && i < j) j--;
            while (g_Scene.GetArea( list_areaID[i])->GetArmy(0)->Get_armyValue() >= baseArmyValue && i < j) i++;
        }
        if (i < j){
            temp = list_areaID[i];
            list_areaID[i] = list_areaID[j];
            list_areaID[j] = temp;
        }
        else{
            list_areaID[leftIndex] = list_areaID[i];
            list_areaID[i] = baseArmyValue;
        }
    }
    QuickSort_armyValue(leftIndex, j - 1, isSmallToLarge, list_areaID);
    QuickSort_armyValue(j + 1, rightIndex, isSmallToLarge, list_areaID);

}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 通过迪杰斯特拉算法获取起始点到目标点的最短路径走法。。<p>
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * void <p>
     *=======================================================<p>
     * 【创建人】： 李德邻 <p>
     * 【修改人】： ？？？<p>
     * 【创建时间】： 2022 <p>
     * 【修改时间】： 2022/10/9
     */
void
Algorithm::Dijkstra(int pathType, int startAreaID, int targetAreaID, int armyIndex, int maxRIngCount) {
    //初始化
    for (int i = 0; i < s_Count_theMapAreas; ++i) {
        Arr_distance_ofStartAreaToTargetArea[i] = 9999;
        sArr_foundArea[i] = false;
        Arr_lastAreaID_inShortestPath[i] = -1;
    }

    Arr_distance_ofStartAreaToTargetArea[startAreaID] = 0;
    int tempTargetAreaID = startAreaID;
    CArea* startArea = g_Scene.GetArea(startAreaID);
    CArmy* theArmy = startArea->GetArmy(armyIndex);
    this->Set_lastID_inShortestPath(startAreaID, theArmy, pathType, startArea);
    CAreaSurroundingInfo* info_startAreaSurrounding = g_areaSurroundingInfoManager.Get_theAreaSurroundingInfo(startAreaID);
    int count_maxLimit = info_startAreaSurrounding->Get_ringCount();
    //有时候最优路线并不一定在这些圈层内，所以可以再查找三圈
    if(maxRIngCount+4 > count_maxLimit)
        maxRIngCount = count_maxLimit;
    bool found = false;

    int continueSearchCount = 4;
    //循环周围圈层，直到找到目标地块。
    for (int i = 0; i < maxRIngCount; ++i) {
        std::vector<CAreaOnRing*>* theRingInfo_byFor = info_startAreaSurrounding->Get_ringInfo(i);
        int count_areas_InTheRing = theRingInfo_byFor->size();
        for (int j = 0; j < count_areas_InTheRing; ++j) {
            int newTargetAreaID = (*theRingInfo_byFor)[j]->ID;
            if (g_Scene.GetArea(newTargetAreaID)->Enable){
                this->Set_lastID_inShortestPath(newTargetAreaID, theArmy, pathType, startArea);
            }

        }
    }
}

void
Algorithm::QuickSort_areaValue(int leftIndex, int rightIndex, bool isSmallToLarge, std::vector<CArea*>& list_area, CAreaSurroundingInfo& startInfo) {
    if (leftIndex >= rightIndex)
        return;
    CArea* temp;
    CArea* baseArea =list_area[leftIndex];
    int baseArmyValue = baseArea->Get_valueMilitary(startInfo.Get_ringNumber_targetAreaOnRing(baseArea->ID));
    int i = leftIndex;
    int j = rightIndex;
    while (i < j){
       int ringNumber_j = startInfo.Get_ringNumber_targetAreaOnRing(list_area[j]->ID);
        int ringNumber_i = startInfo.Get_ringNumber_targetAreaOnRing(list_area[i]->ID);
        if (isSmallToLarge){
            while (list_area[j]->Get_valueMilitary(ringNumber_j) >= baseArmyValue && i < j) j--;
            while (list_area[i]->Get_valueMilitary(ringNumber_i) <= baseArmyValue && i < j) i++;
        }
        else{
            while (list_area[j]->Get_valueMilitary(ringNumber_j) <= baseArmyValue && i < j) j--;
            while (list_area[i]->Get_valueMilitary(ringNumber_i) >= baseArmyValue && i < j) i++;
        }
        if (i < j){
            temp = list_area[i];
            list_area[i] = list_area[j];
            list_area[j] = temp;
        }
    }
    list_area[leftIndex] = list_area[i];
    list_area[i] = baseArea;
    QuickSort_areaValue(leftIndex, i - 1, isSmallToLarge, list_area, startInfo);
    QuickSort_areaValue(i + 1, rightIndex, isSmallToLarge, list_area, startInfo);

}

void
Algorithm::QuickSort_value(int leftIndex, int rightIndex, std::vector<int>& list_value, std::vector<int>& list_areaID, bool isSmaliToLarge) {
    if (leftIndex >= rightIndex) return;
    int baseValue = list_value[leftIndex];
    int baseAreaID = list_areaID[leftIndex];
    int tempNUm = 0;
    int tempAreaID = 0;
    int i;
    int j;
    i = leftIndex;
    j = rightIndex;
    while (i < j) {
        if(isSmaliToLarge){
            while (list_value[j] >= baseValue && i < j)j--;
            while (list_value[i] <= baseValue && i < j)i++;
        }
        else{
            while (list_value[j] <= baseValue && i < j)j--;
            while (list_value[i] >= baseValue && i < j)i++;
        }
        if (i < j) {
            tempNUm = list_value[i];
            tempAreaID = list_areaID[i];
            list_value[i] = list_value[j];
            list_areaID[i] = list_areaID[j];
            list_value[j] = tempNUm;
            list_areaID[j] = tempAreaID;
        }
    }
    list_value[leftIndex] = list_value[i];
    list_areaID[leftIndex] = list_areaID[i];
    list_value[i] = baseValue;
    list_areaID[i] = baseAreaID;
    QuickSort_value(leftIndex, i - 1, list_value, list_areaID, false);
    QuickSort_value(i + 1, rightIndex, list_value, list_areaID, false);

}


void NewButton::Add_newButtons(GUIManager * instance) {

        this->map_newButton[aiTypeText] = instance->AddButton("","",{4,20,80,50}, nullptr, &g_Font1);


        this->map_newButton[oilText] = instance->AddButton("","",{5,7,50,50}, nullptr, &g_Num3);


        this->map_newButton[versionText] = instance->AddButton("","",{4,50,100,50}, nullptr, &g_Font1);


        this->map_newButton[gameDifficultyText] = instance->AddButton("","",{4,35,80,50}, nullptr, &g_Font1);


        this->map_newButton[resolutionButton] = instance->AddButton("buttonboard_red.png","buttonboard_red.png",{150,0,50,50},0, 0);
    this->map_newButton[spectatorModeButton] = instance->AddButton("buttonboard_red.png","buttonboard_red.png",{200,0,50,50},0, 0);
    this->map_newButton[spectatorModeText] = instance->AddButton("","",{260,0,50,50}, nullptr, &g_Font1);

}

GUIButton* NewButton::Get_newButton(NewButton::ButtonType buttonType) {
    return this->map_newButton[buttonType];
}

//关闭某个事件。如果是0（common）类型，则直接从全局事件中查找关闭   建议自己写一个二分查找,因为事件是有序排列的
void
CommonEvent::Close_event(const char* countyID, int eventNum, int eventType1, int resolutionBtnNum) {
    if (eventType1 == eventType::common){
        for (int i = 0; i < list_event_TurnBegin.size(); ++i) {
            if ((*Get_events_turnBegin())[i]->eventNum == eventNum){
                (*Get_events_turnBegin())[i]->isFinished = true;
                return;
            }
        }
    }
    else{
        CCountry* theCOuntry = g_GameManager.FindCountry(countyID);
        if (theCOuntry!= nullptr){
            if (eventType1 == CommonEvent::armyToArea){
                for (int i = 0; i < theCOuntry->List_event_moveArmyTo.size(); ++i) {
                    if(theCOuntry->List_event_moveArmyTo[i]->eventNum == eventNum){
                        theCOuntry->List_event_moveArmyTo[i]->isFinished = true;
                        return;
                    }
                }
            }
            else if(eventType1 == CommonEvent::turnBegin){
                for (int i = 0; i < theCOuntry->List_event_turnBegin.size(); ++i) {
                    if(theCOuntry->List_event_turnBegin[i]->eventNum == eventNum){
                        theCOuntry->List_event_turnBegin[i]->isFinished = true;
                        return;
                    }
                }
            }
            else if (eventType1 == CommonEvent::resolution){
                for (int i = 0; i < theCOuntry->List_resolution.size(); ++i) {
                    if(theCOuntry->List_resolution[i]->eventNum == eventNum){
                        theCOuntry->List_resolution[i]->isFinished = true;
                        theCOuntry->List_resolution[i]->Trigger_resolution(eventNum, resolutionBtnNum, theCOuntry);
                        return;
                    }
                }
            }
        }
    }
}

void EventResolution::NothingHappened() {

}
void EventResolution::Set_btnText(int btnCount,const char* btn1,const char* btn2,const char* btn3){
    this->btnCount = btnCount;
    strcpy(this->btn1,btn1);
    strcpy(this->btn2,btn2);
    strcpy(this->btn3,btn3);
}

void
EventResolution::Set_resolutionText(const char* title, const char* content, const char* result, const char* note) {
    strcpy(this->title,title);
    strcpy(this->content,content);
    strcpy(this->result,result);
    strcpy(this->note,note);

}

void EventResolution::Trigger_resolution(int event, int btnNum, CCountry* theCounry) {

}

