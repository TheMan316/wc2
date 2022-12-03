//
// Created by KK on 2021/9/9.
//

#ifndef WC2_CLI_H
#define WC2_CLI_H
#include <CStateManager/CGameState.h>
#include <CActionAI.h>


struct CommonEvent{
    enum eventType{
        common = 0,
        turnBegin = 1,
        armyToArea = 2,
        resolution = 3
    };
private:
    std::vector<EventTurnBegin*> list_event_TurnBegin;
//所有函数都自带指针检查
public:
    void Close_event(const char* countyID, int eventNum, int eventType1, int resolutionBtnNum);
    void Occupy_areas_ofTheCountry(CCountry* targetCOuntry, CCountry* attackCountry, int occupyEachAreaPercent);
    void Add_areas_toTheCountry(CCountry* country, std::vector<int>& listArea);
    void Change_armysMorale_ofTheCountry(CCountry* country, int Morale, int MoraleUpTurn);
    void ToWar(const char* id1,const char* id2);
    void Add_eventTurnBegin_toSelf(EventTurnBegin* event);
    void Add_resources_toTheCountry(CCountry* country, int money, int industry, int oil);
    std::vector<EventTurnBegin*>* Get_events_turnBegin();
    void Creat_newCountry(const char* ID,const char* name,int money,int industry, int oil,int defeatType,int alliance,double taxfactor,
                          int r,int g,int b,int a,bool isAI,int tech, const char* commanderName,bool isCoastal,int oilConversionRate);
    void Dispose();
    static bool Can_triggere_resolution(EventBase* event, CCountry* theCountry);
    static bool Can_triggere_turnBeginEvent(EventBase* event, CCountry* theCountry);
    static bool Can_triggere_armyMoveToEvent(EventBase* event, int targetAreaID, const char* targetCountryID, CCountry* theCountry);
};

struct CommanderAbility{
    int Assault;
    int Shield;
    int Strength;
    int Supply;
    int MaxStrength;
    int Interval;
};
struct  NewArmyAbility{
    int Assault;
    int Shield;
    int RecoverHp_eachTurn;
    int RecoverOgranization_eachTurn;
    int Increase_percentageOfMaxHP;
    int Increase_percentageOfMaxOgranization;

};


struct ecEffectResManager {

#ifdef __cplusplus

    static ecEffectResManager *Instance();

    void LoadTextureRes(char const*);

    void LoadEffectRes(char const*);

#ifdef extends_ecEffectResManager
    extends_ecEffectResManager
#endif
#endif
};




const CommanderAbility& GetCommanderAbility(int Level);
const NewArmyAbility& Get_armyAbility(int Level);
bool Can_breakTargetArmor(CArmy* startArmy,CArmy* targetArmy);
int Get_attackValue_formStartArmyAttackTargetArmy(CFight* fight, CArmy* startArmy, CArmy* targetArmy, bool isStart);
void Increase_damage_accordingToRestraintRelationship(CArmy* startArmy, CArmy* targetArmy, int& targetAttackTotal, int& startAttackTotal);
void Set_effect_byAreaType(CArea* targetArea, ArmyDef::ArmyType& targetArmy_ID, int& targetAttackTotal);
void SetCounter(CFight* fight,ArmyDef::ArmyType& startArmy_ID,ArmyDef::ArmyType& targetArmy_ID,CArmy* startArmy,CArmy* targetArmy,CArea* targetArea);
bool Is_dodged(CArmy* startArmy, CArmy* targetArmy);
int Get_blow(CArmy* army, int dicesNum);
//自动建设
void Auto_construction(CArea* area);

void SetRandSeed(unsigned int seed);
void SetArmySeed(CArmy* army);

//初始化所有国家的石油转化率
void Init_conversionRate_ofOil_aboutAllCountry();

int GetArmyAtkBuff(int level);
int GetArmyDfsBuff(int level);
int GetArmySupply(int level);

unsigned int GetRand(CArmy* army);
void QuickSort_consume(std::vector<std::vector<int>*>& list_linePassed, int leftIndex, int rightIndex, bool isSmallToLarge, std::vector<int>& list_consumeOfLine);

//读取事件xml
void LoadEventXml(const char *FileName);

void Set_armyBuff(CFight* fight, CArmy* army, CArea* area, bool isStartArmy);
void DrawImage_FightResult(CFight* fight);


//检测这个地块周围(一圈)是否有超过指定数量的敌军
bool IsAroundEnemy(CArea* targetArea,int investigateNum,int allance);


//检查这个需要溃败的军队是否溃败成功
bool Is_successful_colliapsedArmyRetreatSafeArea(CArea *theArea, CArmy* theArmy, int morale, int moraleUpTurn, int movement);


std::vector<int> Get_linkedAreas_byAreasOfNextRing(std::vector<int> & byAreasOfNextRing,bool* foundArea);

void Init_sArr_foundArea();
void Reorder_linesPassed(std::vector<std::vector<int>*>& list_linePassed, bool isSmallToLarge,CArmy* theArmy);


void recursion(int* line, int maxJointRingCount, int maxRingCout, int currentRIngNumber, std::vector<std::vector<int>*>& list_linePassed, CArmy* theArmy, int routeType, CAreaOnRoute* foundAreaOnRing,int ignoreID);
void Set_routePassed(CArea* theArea, std::vector<std::vector<CAreaOnRing*>> &list_linePassed, std::vector<std::vector<CAreaOnRing*>*>* list_ringOfShortestPath, int armyIndex, int routeType);

void Auto_splitJointLinePassed(CArmy* theArmy,int routeType,std::vector<std::vector<int>*>& list_linePassed,
                               std::vector<std::vector<CAreaOnRoute>*>& list_ringOfShortestPath,int ignoreID);
//查找这个地块周围是否有出口
bool IsFoundOut(CArea* targetArea);
//设置ai使用卡牌的逻辑
void Set_AI_DraftLogic(CArea* area, int money,int industry,int r,int ID,int level,int cards);
//Ai使用新卡牌
void AI_Uses_NewCard(CArea* theArea);
//读取自定义本地音乐
void LoadMusic();


//检查地块是否在规定的范围中
bool Is_inSpecifiedRangeOfAreas(int theAreaID);
//使Ai的部队在特定情况下撤退


//设置步兵军
void ExpendArmy(CArea* area,int armies,ArmyDef::ArmyType* armyDef);
void SplitArmy(CArea* area, ArmyDef::ArmyType* armyDef);
//返回一个师类型
ArmyDef::ArmyType GetDivisionType(ArmyDef::ArmyType* armyDef);
//返回一个军类型
ArmyDef::ArmyType GetArmyType(ArmyDef::ArmyType* armyDef);
void Init_countryCardRound();
//重设军队信息
void Set_newArmyDef(ArmyDef* armydef);
//包含在GetArmyDef中
ArmyDef* Get_newArmyDef(int ID, const char* CountryName);
void Init_listCountryArmydef();
void Add_newArmy_toArea(int areaID, int armyID);
void Add_newArmyDef_toList(ArmyDef* armydef, char* countryName);
void ResetArmyDef(ArmyDef* armyDef,int extraHp,int extraMinAttack,int extraMaxAttack,int extraMovement);

extern std::vector<CardDef *> NewCardDef;
//初始化停战列表
void Init_armisticeStatus(int scriptNum);
void Init_armisticeStatus_conquest1();
void Init_policyType_military(int scripNum);
void Init_policyType_economy(int scripNum);
void Init_policyType_military_conquest1();
void Init_policyType_economy_conquest1();


//给军队设置番号
void InitArrArmyDesignations(CCountry* country);
void SetArmyDesignationLand(CArmy* army,CCountry* country);
void SetArmyDesignationSea(CArmy* army,CCountry* country);
void SetArmyDesignationAir(CArmy* army,CCountry* country);
void SetArmyDesignationTank(CArmy* army,CCountry* country);
void GamePause();
void GameStart();
void SetEventsValue_Save( EventTurnBegin* eventBase, SaveEventGeneral& eventGeneral);
void SetEventsValue_Load( SaveEventGeneral& eventGeneral, EventTurnBegin* eventBase);

void Set_armyDesignaion(CArmy* army, CCountry* country);



int GetAreaArmysStrengthNum(CArea* area, bool isOnlyTop);
//设置自定义卡牌介绍文本
void SetCardText(CCountry* country);
int SetTargetIsPlayer(CActionAssist* as,CArmy* targetArmy, CArea* targetArea);

#endif //WC2_CLI_H