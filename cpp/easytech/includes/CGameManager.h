#ifndef EASYTECH_CGAMEMANAGER_H
#define EASYTECH_CGAMEMANAGER_H

#include <zconf.h>
#include "CObjectDef.h"
#include "cxxlist.h"
#include "cxxvector.h"
extern float g_contenscalefactor;
extern bool g_AllowSpectatorMode;
struct CAreaOnRing{
    int ID;
    //以原点为起点
    std::vector<int> List_areaID_linkLastRing;
    //以原点为起点
    std::vector<int> List_areaID_linkNextRing;
};
struct CAreaOnRoute{
    int ID;
    //链接的下一圈地块
    std::vector<CAreaOnRoute*> List_areaOnRoute_linkNextRing;
};
//记录每个地块的周围各圈层的地块情况(每个地块1个）
struct CAreaSurroundingInfo{
private:
    //存放每个圈层的信息
    std::vector<std::vector<CAreaOnRing*>*> ListRingInfo;
    //存放每个圈层地块的所在圈层号
    std::map<int,int> Map_ringNum_inRingArea;
    //存放每个圈层地块所在圈层的下标
    std::map<int,int> Map_index_areaOnRingAtRing;
public:
    //返回圈层总数
    int Get_ringCount();

    int Get_ringNumber_targetAreaOnRing(int targetArea_ID);
    void Add_ringInfo(std::vector<CAreaOnRing*>* circleInfo);
    //记录该圈层地块所在的圈层号
    void Set_ringNum_areaOnRing(int areaOnRing_ID, int ringNum);
    //记录该圈层地块在其圈层的下标位置
    void Set_index_ofAreaOnRingAtRing(int areaInRing_ID, int indexAtRing);

    std::vector<CAreaOnRing*>* Get_ringInfo(int ringNumber);
    //通过指定的ID得到 areaOnRing 请在输入参数前对其进行检查
    CAreaOnRing* Get_areaOnRing(int areaOnRing_ID);

    std::vector<std::vector<CAreaOnRoute>*>* Get_ringOfShortestPath(int targetAreaID);
    //从指定地块向上递归不断获取链接上一个圈层的地块
    void  NextRecursion(std::vector<std::vector<CAreaOnRoute>*>* list_ringOfShortestPath, std::vector<CAreaOnRing*>& list_area_nextRecursion, bool* arrIsFoundArea, int lastRingNumber);

};
struct EventBase{
public:
    char theCountryName[16];
    char theCountryID[16];
    char targetCountryName[16];
    char targetCountryID[16];
    int eventNum;
    int eventType;
    int startRound;
    int endRound;
    int percentOfTrigger;//每回合触发的概率
    bool isFinished;//事件是否已触发
    //设置触发条件
    int moneyForTriggerEvent;
    int industryForTriggerEvent;
    int oilForTriggerEvent;
    int tech; //触发的科技要求
    std::map<int,bool> map_area;
    EventBase();
    //设置有关国家的信息
    void Set_countryText(int eventNum, int eventType, char* theCountryName, char* theCountryID, char* targetCountryName = "null", char* targetCountryID = "null");
    //设置触发条件以及失效时间
    void
    Set_condition(int startRound, int endRound, int percentOfTrigger= 100, int money = -1000, int industry = -1000, int oil = -1000, int tech = 0, std::map<int,bool>* map_area = nullptr);

};
struct CCountry;
//决议事件
struct EventResolution:public EventBase{
    char btn1[16];
    char btn2[16];
    char btn3[16];
    char title[50];
    char content[1024];
    char note[256];
    char result[256];
    int btnCount;
    void NothingHappened();
    virtual void Set_btnText(int btnCount,const char* btn1,const char* btn2 = "null",const char* btn3 = "null");
    virtual void Set_resolutionText(const char* title,const char* content,const char* result,const char* note);
    //每个派生类自己实现这个函数
    virtual void Trigger_resolution(int event, int btnNum, CCountry* theCounry);
};
//回合事件
struct EventTurnBegin:public EventBase{

};
struct EventMoveArmyTo: public EventBase {
};
struct CJointlyLinkWithAroundAreasInfo{
private:
    int ID;
    //与周围各个连接地块  共同接壤的地块
    std::vector<std::vector<int>>* List_areas_jointlyLink_withEachArea;
public:
    std::vector<int>* Get_areas_jointLyLink_withEachArea(int adjacentIndex);
    CJointlyLinkWithAroundAreasInfo();
    CJointlyLinkWithAroundAreasInfo(int id);
    void Init_ListAreasJointlyLink(std::vector<std::vector<int>>* list_areas_jointlyLink);
};








struct  SaveEventGeneral{
    int targetAreaID;
    char targetCountryName[16];
    char theCountryName[16];
    char theCountryID[16];
    char targetCountryID[16];
    char btn1[16];
    char btn2[16];
    char btn3[16];
    char btn4[16];
    int eventNum;
    int armies;
    int areas;
    int money;
    int industry;
    int oil;
    int startRound;
    int endRound;
    int percentOfTrigger;//每回合触发的概率
    bool isFinished;
    int eventType;
    char title[50];
    char content[1024];
    char effect[124];
    char notice[124];
    char newCountryID[16];
    char newCountryName[16];
    int addMoney;
    int addIndustry;
    int addOil;
};




enum WARMEDAL_ID {
    InfantryMedal, AirForceMedal, ArtilleryMedal, ArmourMedal, NavyMedal, CommerceMedal
};

struct CountryAction {
    enum ActionType {
        NoAction,
        ArmyMoveAction,
        ArmyInactiveAction,
        ArmyAttackAction,
        ArmyMoveFrontAction,
        UseCardAction,
        CameraMoveAction
    } ActionType;
    enum CARD_ID CardID;
    int StartAreaID;
    int TargetAreaID;
    int ArmyIndex;
};
enum BomberActionType {
    NoAction, AirStrike, Bomb, NuclearBomb
};
struct SaveCountryInfo;

//添加新东西别忘了去 Creat_newCountry 初始化
struct CCountry {
    int Alliance;
    //军队是1，陆地是0
    int DefeatType;
    std::list<int> AreaIDList;
    std::list<int> CapitalIDList;
    int Money;
    int Industry;
    float TaxRate; //盟友税率固定为1的情况在InitBattle函数中取消了
    char ID[16];
    char Name[16];
    unsigned char R;
    unsigned char G;
    unsigned char B;
    unsigned char A;
    bool AI;
    bool Defeated;
    struct CountryAction CAction;
    float ActionTimer;
    float ActionDelayTime;
    bool ActionWaitCamera;
    int Tech;
    int TechTurn;
    int CardRound[28];
    list(int) AOECardTargetIDList;
    void *AOECardCurTargetID;
    int DestroyCount[10];
    struct CommanderDef *PvCommanderDef;
    int CommanderTurn;
    bool CommanderAlive;
    int WarMedal[6];
    bool BorrowedLoan;
    void *PvPlayer;
    //石油
    int Oil;
    bool IsCoastal;
    std::map <std::string ,int> ListArmisticeStatus;
//存放已有过的番号
    std::vector<bool>  ListDesignationsLand;
    std::vector<bool>  ListDesignationsSea;
    std::vector<bool>  ListDesignationsAir;
    std::vector<bool>  ListDesignationsTank;
    //下一个单位的番号
    int MaxArmyDesignationsLand;
    int MaxArmyDesignationsSea;
    int MaxArmyDesignationsAir;
    int MaxArmyDesignationsTank;
    std::vector<EventTurnBegin*> List_event_turnBegin;
    std::vector<EventMoveArmyTo*> List_event_moveArmyTo;
    std::vector<EventResolution*> List_resolution;
    int SpecialMark; //某某事触发时可用来赋值已进行后续其它相关事件的互动
    int OilConversionRate; //石油转化率
    int Arr_cardCD_inTheTurn[128];
    int Arr_cardCD_inSetting[128];
    short PolicyType_military;//AI军事行动的策略类型
    short PolicyType_economy; //AI经济行动的策略类型
    bool IsNewActionOver; //TODO 新ai逻辑结束了吗(未完成）
//#ifdef __cplusplus~#endif  之间的代码只有语言是cpp的时候才存在。具体来说就是不让反编译器看见这部分代码.
#ifdef __cplusplus
    ~CCountry();
    static const int NeutralID = 4;
    //新函数
    int Get_oilMultiplied_fromAreas();
    //新函数
    void Collect_oilMultiplied_toTheReserve();
    void Init(const char *ID, const char *Name);

    void SaveCountry(SaveCountryInfo *);

    void LoadCountry(const SaveCountryInfo *);

    void Action(const CountryAction & countryAction);

    void AddArea(int AreaID);

    void AddDestroy(int ArmyType);

    float AirstrikeRadius();

    void BeConquestedBy(CCountry *);

    bool CanBuyCard(CardDef *);

    bool CanUseCommander();

    bool CheckCardTargetArea(CardDef *, int id);

    bool CheckCardTargetArmy(CardDef *, int AreaID, int ArmyIndex);

    void CollectIndustrys();

    void CollectTaxes();

    void CommanderDie();

    void DoAction();

    int FindAdjacentAreaID(int AreaID, bool HasArmy);

    int FindAdjacentLandAreaID(int AreaID, bool HasArmy);

    bool FindArea(int AreaID);
//行动类型设置为NoAction
    void FinishAction();

    int GetCardIndustry(CardDef *);

    int GetCardPrice(CardDef *);

    int GetCardRounds(CARD_ID);

    int GetCommanderLevel();

    const char *GetCommanderName();

    int GetCurCardTarget();

    int GetFirstCapital();

    int GetHighestValueArea();

    int GetIndustrys();

    float GetMinDstToAirport(int AreaID);

    int GetNumAirport();

    int GetTaxes();

    int GetWarMedalLevel(WARMEDAL_ID);

    bool IsActionFinish();

    bool IsCardUnlock(CardDef *);

    bool IsConquested();

    bool IsEnoughIndustry(CardDef *);

    bool IsEnoughMoney(CardDef *);

    bool IsLocalPlayer();

    void NextCardTarget();

    void RemoveArea(int AreaID);

    void SetCardTargets(CardDef *);

    void SetCommander(const char *name);

    void SetWarMedalLevel(WARMEDAL_ID, int level);

    void TurnBegin();

    void TurnEnd();

    void Update(float time);

    void UseCard(CardDef *, int TargetAreaID, int ArmyIndex);

#ifdef extends_CCountry
    extends_CCountry
#endif
#endif
};

__BEGIN_DECLS
void _ZN8CCountryC1Ev(struct CCountry *self);

void _ZN8CCountryD1Ev(struct CCountry *self);

void _ZN8CCountry4InitEPKcS1_(struct CCountry *self, const char *ID, const char *Name);

void _ZN8CCountry11SaveCountryEP15SaveCountryInfo(struct CCountry *self, struct SaveCountryInfo *);

void _ZN8CCountry11LoadCountryEPK15SaveCountryInfo(struct CCountry *self,
                                                   const struct SaveCountryInfo *);

void _ZN8CCountry6ActionERK13CountryAction(struct CCountry *self, const struct CountryAction *);

void _ZN8CCountry7AddAreaEi(struct CCountry *self, int AreaID);

void _ZN8CCountry10AddDestroyEi(struct CCountry *self, int ArmyType);

float _ZN8CCountry15AirstrikeRadiusEv(struct CCountry *self);

void _ZN8CCountry14BeConquestedByEPS_(struct CCountry *self, struct CCountry *);

bool _ZN8CCountry10CanBuyCardEP7CardDef(struct CCountry *self, struct CardDef *);

bool _ZN8CCountry15CanUseCommanderEv(struct CCountry *self);

bool _ZN8CCountry19CheckCardTargetAreaEP7CardDefi(struct CCountry *self, struct CardDef *, int id);

bool
_ZN8CCountry19CheckCardTargetArmyEP7CardDefii(struct CCountry *self, struct CardDef *, int AreaID,
                                              int ArmyIndex);

void _ZN8CCountry16CollectIndustrysEv(struct CCountry *self);

void _ZN8CCountry12CollectTaxesEv(struct CCountry *self);

void _ZN8CCountry12CommanderDieEv(struct CCountry *self);

void _ZN8CCountry8DoActionEv(struct CCountry *self);

int _ZN8CCountry18FindAdjacentAreaIDEib(struct CCountry *self, int AreaID, bool HasArmy);

int _ZN8CCountry22FindAdjacentLandAreaIDEib(struct CCountry *self, int AreaID, bool HasArmy);

bool _ZN8CCountry8FindAreaEi(struct CCountry *self, int AreaID);

void _ZN8CCountry12FinishActionEv(struct CCountry *self);

int _ZN8CCountry15GetCardIndustryEP7CardDef(struct CCountry *self, struct CardDef *);

int _ZN8CCountry12GetCardPriceEP7CardDef(struct CCountry *self, struct CardDef *);

int _ZN8CCountry13GetCardRoundsE7CARD_ID(struct CCountry *self, enum CARD_ID);

int _ZN8CCountry17GetCommanderLevelEv(struct CCountry *self);

const char *_ZN8CCountry16GetCommanderNameEv(struct CCountry *self);

int _ZN8CCountry16GetCurCardTargetEv(struct CCountry *self);

int _ZN8CCountry15GetFirstCapitalEv(struct CCountry *self);

int _ZN8CCountry19GetHighestValueAreaEv(struct CCountry *self);

int _ZN8CCountry12GetIndustrysEv(struct CCountry *self);

float _ZN8CCountry18GetMinDstToAirportEi(struct CCountry *self, int AreaID);

int _ZN8CCountry13GetNumAirportEv(struct CCountry *self);

int _ZN8CCountry8GetTaxesEv(struct CCountry *self);

int _ZN8CCountry16GetWarMedalLevelE11WARMEDAL_ID(struct CCountry *self, enum WARMEDAL_ID);

bool _ZN8CCountry14IsActionFinishEv(struct CCountry *self);

bool _ZN8CCountry12IsCardUnlockEP7CardDef(struct CCountry *self, struct CardDef *);

bool _ZN8CCountry12IsConquestedEv(struct CCountry *self);

bool _ZN8CCountry16IsEnoughIndustryEP7CardDef(struct CCountry *self, struct CardDef *);

bool _ZN8CCountry13IsEnoughMoneyEP7CardDef(struct CCountry *self, struct CardDef *);

bool _ZN8CCountry13IsLocalPlayerEv(struct CCountry *self);

void _ZN8CCountry14NextCardTargetEv(struct CCountry *self);

void _ZN8CCountry10RemoveAreaEi(struct CCountry *self, int AreaID);

void _ZN8CCountry14SetCardTargetsEP7CardDef(struct CCountry *self, struct CardDef *);

void _ZN8CCountry12SetCommanderEPKc(struct CCountry *self, const char *name);

void
_ZN8CCountry16SetWarMedalLevelE11WARMEDAL_IDi(struct CCountry *self, enum WARMEDAL_ID, int level);

void _ZN8CCountry9TurnBeginEv(struct CCountry *self);

void _ZN8CCountry7TurnEndEv(struct CCountry *self);

void _ZN8CCountry6UpdateEf(struct CCountry *self, float time);

void _ZN8CCountry7UseCardEP7CardDefii(struct CCountry *self, struct CardDef *, int TargetAreaID,
                                      int ArmyIndex);

__END_DECLS

struct Belligerent {
    char id[8];
    char name[8];
    char commander[20];
    int alliance;
};

#ifdef __cplusplus

void GetBattleKeyName(int SeriesID, int LevelID, char *name);

#endif

__BEGIN_DECLS
void _Z16GetBattleKeyNameiiPc(int SeriesID, int LevelID, char *name);

__END_DECLS

struct DialogueDef {
    std::string Commander;
    int Index;
    int AtRound;
    bool Left;
};

struct SaveHeader;


struct CGameManager {
    std:: vector<CCountry *> ListCountry; //国家战败依然不会被释放
    std:: vector<CCountry *> DefeatCountry;//List of countries that can be defeated
    std:: vector<DialogueDef *> DialogueList;
    int CurrentCountryIndex;
    int CurrentDialogueIndex;
    //从0开始
    int CurrentTurnNumMinusOne;
    int RandomRewardMedal;
    enum GameMode {
        Campaign = 1,
        Conquest,
        MultiPlayer = 4,
        Tutorial
    } GameMode;
    int AlwaysZero;
    int MapID;
    char AreasEnable[32];
    char BattleFileName[32];
    char LoadFileName[16];
    char PlayerCountryName[8][4];
    char ConquestPlayerCountryID[8];
    bool IsNewGame;
    bool LocalHost;
    bool Result;
    bool ResultWin;
    bool SelectNextBattle;
    int CampaignSeriesID;
    int CampaignLevelID;
    int VictoryTurn;
    int GreatVictoryTurn;
    int CampaignRewardMedal;
    bool IsIntelligentAI;//AI增强
    unsigned int G_Seed = 1;
    int GameDifficulty = 1;
    bool IsPause;
    float  CircleProgress;
    bool IsReduce ; //负责绘制圈圈

#ifdef __cplusplus
    unsigned int GetRand();
    void Set_skipMode(bool isSkip);
    bool Get_skipMode();
    void Set_spectatorMode();
    bool Is_spectetorMode();
    void BattleVictory();
    void Set_gameGivenResult(bool isWin);
    bool CheckAndSetResult();

    void ClearBattle();

    void EndTurn();

    CCountry *FindCountry(const char *ID);

    void GameUpdate(float time);

    CCountry *GetCountryByIndex(int index);

    CCountry *GetCurCountry();

    DialogueDef *GetCurDialogue();

    bool GetCurDialogueString(char *result);

    DialogueDef GetDialogueByIndex(int index);

    CCountry *GetLocalPlayerCountry();

    CCountry *GetNewDefeatedCountry();

    int GetNumCountries();

    int GetNumDialogues();

    int GetNumVictoryStars();

    CCountry *GetPlayerCountry();

    const char *GetPlayerCountryName(int index);

    int GetPlayerNo(const char *);

    void GetSaveHeader(const char *FileName, SaveHeader &);

    void InitBattle();

    void InitCameraPos();

    bool IsLastBattle();

    bool IsManipulate();

    void LoadBattle(const char *FileName);

    void LoadGame(const char *FileName);

    void MovePlayerCountryToFront();

    void NewGame(int GameMode, int, int CampaignSeriesID, int LevelID);

    void Next();

    void NextDialogue();
//读取存档
    void RealLoadGame(const char *filename);

    void Release();

    void RetryGame();

    void SaveBattle(const char *FileName);

    void SaveGame(const char *FileName);

    void SetConquestPlayerCountryID(const char *ID);

    void SetPlayercountryname(int index, const char *name);

    void TurnBegin();

    void TurnEnd();

    operator CCountry *() { return this->GetCurCountry(); }
#ifdef extends_CGameManager
    extends_CGameManager
#endif
#endif
};

__BEGIN_DECLS
void _ZN12CGameManagerC1Ev(struct CGameManager *self);

void _ZN12CGameManagerD1Ev(struct CGameManager *self);

void _ZN12CGameManager13BattleVictoryEv(struct CGameManager *self);

bool _ZN12CGameManager17CheckAndSetResultEv(struct CGameManager *self);

void _ZN12CGameManager11ClearBattleEv(struct CGameManager *self);

void _ZN12CGameManager7EndTurnEv(struct CGameManager *self);

struct CCountry *_ZN12CGameManager11FindCountryEPKc(struct CGameManager *self, const char *ID);

void _ZN12CGameManager10GameUpdateEf(struct CGameManager *self, float time);

struct CCountry *_ZN12CGameManager17GetCountryByIndexEi(struct CGameManager *self, int index);

struct CCountry *_ZN12CGameManager13GetCurCountryEv(struct CGameManager *self);

struct DialogueDef *_ZN12CGameManager14GetCurDialogueEv(struct CGameManager *self);

bool _ZN12CGameManager20GetCurDialogueStringEPc(struct CGameManager *self, char *result);

struct DialogueDef _ZN12CGameManager18GetDialogueByIndexEi(struct CGameManager *self, int index);

struct CCountry *_ZN12CGameManager21GetLocalPlayerCountryEv(struct CGameManager *self);

struct CCountry *_ZN12CGameManager21GetNewDefeatedCountryEv(struct CGameManager *self);

int _ZN12CGameManager15GetNumCountriesEv(struct CGameManager *self);

int _ZN12CGameManager15GetNumDialoguesEv(struct CGameManager *self);

int _ZN12CGameManager18GetNumVictoryStarsEv(struct CGameManager *self);

struct CCountry *_ZN12CGameManager16GetPlayerCountryEv(struct CGameManager *self);

const char *_ZN12CGameManager20GetPlayerCountryNameEi(struct CGameManager *self, int index);

int _ZN12CGameManager11GetPlayerNoEPKc(struct CGameManager *self, const char *);

void
_ZN12CGameManager13GetSaveHeaderEPKcR10SaveHeader(struct CGameManager *self, const char *FileName,
                                                  struct SaveHeader *);

void _ZN12CGameManager10InitBattleEv(struct CGameManager *self);

void _ZN12CGameManager13InitCameraPosEv(struct CGameManager *self);

bool _ZN12CGameManager12IsLastBattleEv(struct CGameManager *self);

bool _ZN12CGameManager12IsManipulateEv(struct CGameManager *self);

void _ZN12CGameManager10LoadBattleEPKc(struct CGameManager *self, const char *FileName);

void _ZN12CGameManager8LoadGameEPKc(struct CGameManager *self, const char *FileName);

void _ZN12CGameManager24MovePlayerCountryToFrontEv(struct CGameManager *self);

void
_ZN12CGameManager7NewGameEiiii(struct CGameManager *self, int GameMode, int, int CampaignSeriesID,
                               int LevelID);

void _ZN12CGameManager4NextEv(struct CGameManager *self);

void _ZN12CGameManager12NextDialogueEv(struct CGameManager *self);

void _ZN12CGameManager12RealLoadGameEPKc(struct CGameManager *self, const char *filename);

void _ZN12CGameManager7ReleaseEv(struct CGameManager *self);

void _ZN12CGameManager9RetryGameEv(struct CGameManager *self);

void _ZN12CGameManager10SaveBattleEPKc(struct CGameManager *self, const char *FileName);

void _ZN12CGameManager8SaveGameEPKc(struct CGameManager *self, const char *FileName);

void _ZN12CGameManager26SetConquestPlayerCountryIDEPKc(struct CGameManager *self, const char *ID);

void _ZN12CGameManager20SetPlayercountrynameEiPKc(struct CGameManager *self, int index,
                                                  const char *name);

void _ZN12CGameManager9TurnBeginEv(struct CGameManager *self);

void _ZN12CGameManager7TurnEndEv(struct CGameManager *self);

extern struct CGameManager g_GameManager;
__END_DECLS

#ifdef __cplusplus

static inline bool operator==(CGameManager &manager, CCountry *country) {
    return manager.GetCurCountry() == country;
}

static inline bool operator==(CCountry *country, CGameManager &manager) {
    return manager == country;
}

#endif


struct SaveHeader {
    char VerificationCode[4];
    int FileVersion;
    enum CGameManager::GameMode GameMode;
    int AlwaysZero;
    int MapID;
    char AreasEnable[32];
    char PlayerCountryName[8][4];
    char BattleFileName[32];
    float CenterPos[2];
    float Scale;
    int CurrentCountryIndex;
    int CurrentDialogueIndex;
    int CountryNum;
    int AreaNum;
    int CurrentTurnNumMinusOne;
    int RandomRewardMedal;
    int Year;
    int Month;
    int Day;
    int Hour;
    int Second;
    int CampaignSeriesID;
    int CampaignLevelID;
    int VictoryTurn;
    int GreatVictoryTurn;
    bool isIntelligentAI;
    unsigned int g_Seed;
    int GameDifficulty;
    bool IsPause;
    bool Arr_eventTurnBegin_ofCommonEvents_isFinished[512];
};
struct ArmisticeStatus {
    char CountryName[16];
    int Round;
};
struct SaveCountryInfo {
    int Money;
    int Industry;
    int Tech;
    int TechTurn;
    bool AI;
    int Alliance;
    int DefeatType;
    int CardRound[28];
    char ID[16];
    char Name[16];
    unsigned char R;
    unsigned char G;
    unsigned char B;
    unsigned char A;
    float TaxRate;
    int DestroyCount[10];
    int WarMedal[6];
    char CommanderName[24];
    int CommanderTurn;
    bool CommanderAlive;
    bool BorrowedLoan;
    bool Defeated;
    int Oil;
    bool IsMaritime;
    ArmisticeStatus listArmisticeStatus[225];
    int listArmisticeStatusCount;
    int listArrDesignationsLand_Count;
    int listArrDesignationsSea_Count;
    int listArrDesignationsAir_Count;
    int listArrDesignationsTank_Count;
    bool arr_land[2048];
    bool arr_air[512];
    bool arr_sea[512];
    bool arr_tank[1024];
    int MaxArmyDesignationsLand;
    int MaxArmyDesignationsSea;
    int MaxArmyDesignationsAir;
    int MaxArmyDesignationsTank;
    int eventTurnBeginsNum;
    int eventOccupyAreasNum;
    int eventResolutionsNum;
    int SpecificMark;
    int OilConversionRate;
    int New_Card_CDRound_Current[128];
    int New_Card_CDRound_Each[128];
    bool arr_eventFinished_turnBegin[512];
    bool arr_eventFinished_moveArmyTo[512];
    bool arr_eventFinished_resolution[512];
    short PolicyType_military;//AI军事行动的策略类型
    short PolicyType_economy; //AI经济行动的策略类型
    bool IsNewActionOver;
//    SaveEventGeneral List_event_turnBegin [512];
//    SaveEventGeneral List_event_moveArmyTo [512];
//    SaveEventGeneral List_resolution [512];

};



struct SaveArmyInfo {
    int ID;
    int HP;
    int Movement;
    int Cards;
    int MaxHP;
    int Level;
    int Exp;
    int Morale;
    int MoraleUpTurn;
    float Direction;
    bool Active;
    unsigned int Seed;
    int Designation;
    bool IsA;
    int OldDesignation[3];
    int MaxOrganization;
    int Organization;

};

struct SaveAreaInfo {
    int ID;
    int Country;
    int ArmyCount;
    int ConstructionType;
    int ConstructionLevel;
    int InstallationType;
    SaveArmyInfo ArmyInfo[4];
    bool safe;
    int CityGrowthRate;
    int IndustryGrowthRate;
    int OilGrowthRate;
};

struct ecFile {
    int space[999];

#ifdef __cplusplus
    bool Open(char const*,char const*);

    void Close();
#ifdef __LP64__
    bool Read(void *, unsigned int);

    bool Write(void const *, unsigned int);
#else
    //第一个参数：存储本次所读取的内容。第二个参数：本次读取的字节长度。
    bool Read(void *, unsigned long);

    bool Write(void const *, unsigned long);
#endif


#ifdef extends_ecFile
    extends_ecFile
#endif
#endif
};
#endif //EASYTECH_CGAMEMANAGER_H