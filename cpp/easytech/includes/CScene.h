#ifndef EASYTECH_CSCENE_H
#define EASYTECH_CSCENE_H

#include <queue>
#include <stack>
#include "CGameManager.h"
#include "ecLibrary.h"

struct SaveArmyInfo;
struct CArea;
struct CArmy {
    enum Card {
        LightArtillery = 0,//轻炮
        EngineeringCorps  , //工程
        AntitankGun ,//反坦克
        Commander ,
        HeavyArtillery = 4 ,//重炮
        TankDestroyer ,//  突击炮(歼击车)
        TransportationTroops ,// 运输
        MoveArtillery ,//    自走炮
        SeaAntiaircraft,
        SeaArmor ,
        SeaGun ,
        Poisoning_1 ,
        Poisoning_2 ,
        Poisoning_3
    };
    struct ArmyDef *BasicAbilities;
    struct CCountry *Country;
    int Hp;
    int MaxHp;
    int Movement;
    int Cards;
    int Level;
    int Exp;
    //1等于士气下降，2等于士气上升
    int Morale;
    int MoraleUpTurn;
    float Direction;
    //是否为活动状态？ false不可活动
    bool Active;
    unsigned int Seed = 1;
    //军队番号
    int Designation;
    //是否是军级单位
    bool IsA;
    //如果是军机单位，这里保存其师级单位的番号 无则为0
    int OldDesignation[3];
    int MaxOrganization;
    int Organization;
#ifdef __cplusplus
    bool Is_collapsed();
    bool Is_eliteArmy();
 bool Has_engineeringCorps();
    void Init(ArmyDef *, CCountry *);
   int GetPoisoningState();
    void SaveArmy(SaveArmyInfo *);
   void SetArmyOrganization(ArmyDef* armyDef);
    void LoadArmy(const SaveArmyInfo *);
    void SetPoisoning(int state);
    void AddExp(int exp);

    bool Is_panzer();
    bool Is_infantry();
    bool Is_artillery();
    bool Is_tank();
    bool Is_airForce();
    bool Is_fightPlane();
    bool Is_boomPlane();
    bool IsNavy(); //原版函数
    int Get_armyValue();

    //决定单位的远程进攻方式
    bool Is_remoteAttack();
    bool Is_radiudAttack();

    //得到装备的战术卡的数量
    int Get_countOfCard();
    void AddStrength(int hp);
    void Restore_oganization(int o);
    //得到单位的远程攻击半径
    float GetAirstrikeRadius();
    bool Is_wounded();
    //得到这个军下属师级单位的数量
    int Get_count_oldDesignation();

    void Lost_card(int percentNum);
    void Breakthrough();

    int GetMaxStrength();

    int GetNumDices();

    int GetNumDicesIfLostStrength(int damage);



    bool LostStrength(int damage);

    void Poisoning();

    void ResetMaxStrength(bool KeepHpValue);

    void SetMorale(int morale);

    void TurnBegin();

    void TurnEnd();

    void Upgrade();
    void AddCard(Card CardID) {
        Cards |= 1 << CardID;
    }

    void RemoveCard(Card CardID) {
        Cards &= ~(1 << CardID);
    }
    bool HasCard(Card CardID) {
        return ((this->Cards >> CardID) & 1) != 0;
    }

#ifdef extends_CArmy
    extends_CArmy
#endif
#endif
};

__BEGIN_DECLS
void _ZN5CArmyC1Ev(struct CArmy *self);

void _ZN5CArmyD1Ev(struct CArmy *self);

void _ZN5CArmy4InitEP7ArmyDefP8CCountry(struct CArmy *self, struct ArmyDef *, struct CCountry *);

void _ZN5CArmy8SaveArmyEP12SaveArmyInfo(struct CArmy *self, struct SaveArmyInfo *);

void _ZN5CArmy8LoadArmyEPK12SaveArmyInfo(struct CArmy *self, const struct SaveArmyInfo *);

void _ZN5CArmy6AddExpEi(struct CArmy *self, int exp);

void _ZN5CArmy11AddStrengthEi(struct CArmy *self, int hp);

void _ZN5CArmy12BreakthroughEv(struct CArmy *self);

int _ZN5CArmy14GetMaxStrengthEv(struct CArmy *self);

int _ZN5CArmy11GetNumDicesEv(struct CArmy *self);

int _ZN5CArmy25GetNumDicesIfLostStrengthEi(struct CArmy *self, int damage);

bool _ZN5CArmy6IsNavyEv(struct CArmy *self);

bool _ZN5CArmy12LostStrengthEi(struct CArmy *self, int damage);

void _ZN5CArmy9PoisoningEv(struct CArmy *self);

void _ZN5CArmy16ResetMaxStrengthEb(struct CArmy *self, bool KeepHpValue);

void _ZN5CArmy9SetMoraleEi(struct CArmy *self, int morale);

void _ZN5CArmy9TurnBeginEv(struct CArmy *self);

void _ZN5CArmy7TurnEndEv(struct CArmy *self);

void _ZN5CArmy7UpgradeEv(struct CArmy *self);

__END_DECLS
struct AreaSurroundingInfoManager{
private:
    std::vector<CAreaSurroundingInfo*>* ListAreaSurroundingInfo;
    std::vector<CJointlyLinkWithAroundAreasInfo*>* ListAreaJointlyLinkInfo;
#ifdef __cplusplus
public:
    void InitListAreaSurroundingInfo();
    std::stack<int>* Get_shortestPath(int pathType, int startAreaID, int targetAreaID, int armyIndex);
    int
    Get_consume_betweenTwoAreas(int startAreaID, int targetAreaID, CArmy* theArmy, CArea* armyArea);

    int
    Get_extraDistance_basisOfPathType(CCountry* theCountry, int targetAreaID, int pathType, int armyAreaID, int armyIndex, int startAreaID);
    //得到指定地块的所有圈层信息
    CAreaSurroundingInfo* Get_theAreaSurroundingInfo(int theAreaID);
    CJointlyLinkWithAroundAreasInfo* Get_theAreaJointlyLinkWithAroundAreasInfo(int theAreaID);
    void Init();
#ifdef extends_CAreaSurroundingInfoManager
    extends_CAreaSurroundingInfoManager
#endif
#endif
};
 extern  AreaSurroundingInfoManager g_areaSurroundingInfoManager;
struct NewAiAction{

    void Try_TheArmy_toTargetArea_alongOptimalRoute(int startAreaID, CArmy *theArmy, std::stack<int>& OptimalRoute);
    int Get_areaID_existNearestArmy(int startAreaID, std::vector<CountryArmyDef::servicesType>* list_servicesType_toBePrioritySearch, int maxRingCount_toPrioritySearch);
    int Get_consume_aboutTheline(std::vector<int>& list_line, CArmy* army_preparing);
    int Get_areaID_ofHighestValueWeakTargetAround(std::vector<int>& list_area_existEnemy_inRange);
    int Get_areaID_ofNearestEnemy(int startAreaID, int maxRingCount);
    int Get_areaID_ofHighestMilitaryValueAround(int startAreaID, int maxRingCount);
    int Get_areaID_ofBestRemoteArmyAttackEnemy(int startAreaID, int maxRingCount);
    void Add_newArmy_toImportantTown(CCountry* theCountry);
};
struct CArea;
extern NewAiAction g_newAiAction;
struct Algorithm{
public:
    void
    QuickSort_value(int leftIndex, int rightIndex, std::vector<int>& list_value, std::vector<int>& list_areaID, bool isSmaliToLarge);
    void QuickSort_armyValue(int leftIndex, int rightIndex, bool  isSmallToLarge, std::vector<int>& list_areaID);
    void
    QuickSort_areaValue(int leftIndex, int rightIndex, bool isSmallToLarge, std::vector<CArea*>& list_area, CAreaSurroundingInfo& startInfo);
    void Dijkstra(int pathType, int startAreaID, int targetAreaID, int armyIndex, int maxRIngCount);
    //起点到这个下标地块的线路中，抵达这个下标地块的上一个地块是？
    static int Arr_lastAreaID_inShortestPath[];
    static int Arr_distance_ofStartAreaToTargetArea[]; //记录起点到这个下标地块id的距离。如果必定不能去，给它大于500的值
private:
    void Set_lastID_inShortestPath(int targetAreaID, CArmy* theArmy, int pathType, CArea* armyArea);
};
extern Algorithm g_algorithm;
enum AreaType {
    normal, capital, port, large_city, normal_city, forest, hilly, desert
};

struct AreaInfo {
    AreaType Type;
    int Tax;
    int ArmyPos[2];
    int ConstructionPos[2];
    int InstalltionPos[2];
};
struct SaveAreaInfo;
//音乐
struct CSoundRes {
    enum SND_EFFECT {
        aDraft_wav,//0
        aMove_wav,//1
        aOccupy_wav,//2
        aCannon_wav,//3
        aRocket_wav,//4
        aFire_wav,//5
        aExp_wav,//6
        aStrike_wav,//7
        aSelect_wav,//8
        aCancel_wav,//9
        aShip_wav,//10
        aCelebrate_wav,//11
        aAfter_war_wav,//12
        aBuff_wav,//13
        aBuild_wav,//14
        aCard_interface,//15
        aGas_wav,//16
        aLvup_wav,//17
        aMachine_gun_wa,//18
        aNaval_gun_wav,//19
        aPop_wav,//20
        aSupply_wav//21
    };

};
//处理声音播放 加速回合屏蔽声音播放
struct CCSoundBox {
    static CCSoundBox *GetInstance();
    void LoadSE(char const*);
    int PlaySE(char const*);

};
__BEGIN_DECLS
int _ZN10CCSoundBox6PlaySEEPKc(CCSoundBox *self, char const*);
__END_DECLS
__BEGIN_DECLS
void _ZN9CSoundRes10PlayCharSEE10SND_EFFECT(struct CSoundRes *self, CSoundRes::SND_EFFECT);

extern struct CSoundRes g_SoundRes;
__END_DECLS
struct CArea {
    int ID;
    AreaType Type;
    int Tax;
    int ArmyPos[2];
    int ConstructionPos[2];
    int InstalltionPos[2];
    bool Enable;
    bool Sea;
    enum ConstructionType {
        NoConstruction, city, industry, airport,oilwell
    } ConstructionType;
    int ConstructionLevel;
    enum InstallationType {
        NoInstallation, fort, entrenchment, antiaircraft, radar
    } InstallationType;
    struct CCountry *Country;
    struct CArmy *Army[4];
    int ArmyCount;
    struct CArmy *ArmyDrafting; //暂存新造的军队指针
    struct CArmy *ArmyMovingIn; //暂存移动的军队指针
    struct CArmy *ArmyMovingToFront;
    float DraftingArmyOffset; //军队掉下来的动作
    float MovingInArmyOffset[2];
    float MovingInArmySpeed[2];
    float MoveArmyToFrontAngle;
    bool Selected;
    bool MovingInArmyOccupy;
    bool MovingInArmyComplain;
    char *ComplainCommanderName;
    int TargetType; // 2是远程攻击箭头，3是移动箭头，4是近战攻击箭头
    float ArrowEndOffset[2];
    float ArrowEndOffsetMinus[2];
    float ArrowStartOffset[2];
    float ArrowStartOffsetMinus[2];
    int Oil;
    bool IsSafe;
    //地块经济自动增长率
    int CityGrowthRate;
    int IndustryGrowthRate;
    int OilGrowthRate;
#ifdef __cplusplus

bool Is_existTheServicesType(std::vector< CountryArmyDef::servicesType>&  list_armyType);
    int Get_valueMilitary(int ringNumber);
    bool Exist_eliteArmy();
    bool Retreat_missileArmy(CArmy* needCheckArmy);
    std::vector<CArea*>* Get_areas_inRange(int armyIndex);
    int Is_hasExport();
    int Get_oil();
    void Set_SafeOfArea(int ringCount, int enemyCount);
    void MoveArmyTo(CArea *targetArea, CArmy *actArmy, bool bottom);
    void Init(int ID, const AreaInfo &);
    void Adjust_armyOrder();
    bool Is_supplyPort();
    void SetAllArmyPoisoning(int state);
    void SaveAera(SaveAreaInfo *);
    void Army_followTheRouteToTargetArea(CArmy *theArmy, std::vector<int>& linePassed);
    int Get_cityGrowthRate();
    int Get_industryGrowthRate();
    bool Is_supplyPoint(CArmy* needSupplyArmy);
    bool Is_supplyCity();
    void Check_retreatableArmy(int ringCount);
    int Get_oilGrowthRate();
    bool Try_armyToSuppliedArea(CArmy *theArmy, std::vector<std::vector<int>*>& list_linePassed);
    int Check_moveBestStatusArmyToOtherArea(bool isAutomaticRemoval);
    bool Is_existMilitaryThreat(int ringCount, int EnemyCount, int alliance);
    int Get_consumedMovement();
    int Get_consumedMovement(CArmy* army);
    void Recover_armyState();
    //得到这个地块师级单位的总量
    int Get_countOfDesigantions();
    //撤退到某个地块
    void Retreat_frontArmy_toAdjacentArea(CArea* adjacentArea);
    std::vector<int>* Get_areas_existEnemy_inRange(int armyIndex);
    //新函数
    void ClearArmy(int count);
    void LoadArea(const SaveAreaInfo *);
    bool Is_existNoAlly(int ringCount, int armys_noAlly);
    bool Is_existTheCamp(int ringCount, int allanceOfTheCamp,int armyCount,int construction);
    void AddArmy(CArmy *, bool bottom);

    void AddArmyCard(int ArmyIndex, int CardID);

    void AddArmyStrength(int ArmyIndex, int Hp);

    void AllArmyPoisoning();

    bool CanConstruct(int construction_willBeBuilt);

    bool CheckEncirclement();

    void ClearAllArmy();

    void Construct(int ConstructionType);

    void DelArmyCard(int ArmyIndex, int CardID);

    void DestroyConstruction();

    struct CArmy *DraftArmy(int ArmyType);

    void Encirclement();

    CArmy *GetArmy(int ArmyIndex);

    int GetArmyIdx(CArmy *);

    int GetCityLevel();

    int GetIndustry();

    int GetIndustryLevel();

    int GetRealTax();

    bool HasArmyCard(int CardID);

    bool HasArmyCard(int ArmyIndex, int CardID);
//如果地块中有活跃军队，返回true
    bool IsActive();

    bool IsArmyActive(int ArmyIndex);

    bool LostArmyStrength(int ArmyIndex, int damage);

    void MoveArmyTo(CArea *);

    void MoveArmyToFront(CArmy *, bool instantly);

    void MoveArmyToFront(int ArmyIndex, bool instantly);

    void OccupyArea(CArea *);

    void ReduceConstructionLevel();
//这是移除军队（用于单位移动后重置这个地块的信息），军队死亡请配合delete
    void RemoveArmy(CArmy *);

    void Render();

    void RenderArmy(float, float, int, CArmy *);

    void RenderBuilding();
//恢复军队的最大血量
    void RevertArmyStrength(int ArmyIndex);

    void SetAllArmyMovement(int movement);

    void SetArmyActive(int ArmyIndex, bool active);

    void SetArmyDir(int ArmyIndex, float direction);

    void SetConstruction(int ConstructionType, int ConstructionLevel);
    //这个函数内部不会扣除移动力 注意：需要在出发军队的地块上调用RemoveArmy()移除该军队.该函数就是设置动画的。
    void SetMoveInArmy(CArea *Origin, CArmy *, bool WillOccupy, bool WillComplain,
                       const char *ComplainCommander);

    void TurnBegin();

    void TurnEnd();

    void Update(float time);

    void UpgradeArmy(int ArmyIndex);

#ifdef extends_CArea
    extends_CArea
#endif
#endif
};

__BEGIN_DECLS
void _ZN5CAreaC1Ev(struct CArea *self);

void _ZN5CAreaD1Ev(struct CArea *self);

void _ZN5CArea4InitEiRK8AreaInfo(struct CArea *self, int ID, const struct AreaInfo *);

void _ZN5CArea8SaveAeraEP12SaveAreaInfo(struct CArea *self, struct SaveAreaInfo *);

void _ZN5CArea8LoadAreaEPK12SaveAreaInfo(struct CArea *self, const struct SaveAreaInfo *);

void _ZN5CArea7AddArmyEP5CArmyb(struct CArea *self, struct CArmy *, bool bottom);

void _ZN5CArea11AddArmyCardEii(struct CArea *self, int ArmyIndex, int CardID);

void _ZN5CArea15AddArmyStrengthEii(struct CArea *self, int ArmyIndex, int Hp);

void _ZN5CArea16AllArmyPoisoningEv(struct CArea *self);

bool _ZN5CArea12CanConstructEi(struct CArea *self, int ConstructionType);

bool _ZN5CArea17CheckEncirclementEv(struct CArea *self);

void _ZN5CArea12ClearAllArmyEv(struct CArea *self);

void _ZN5CArea9ConstructEi(struct CArea *self, int ConstructionType);

void _ZN5CArea11DelArmyCardEii(struct CArea *self, int ArmyIndex, int CardID);

void _ZN5CArea19DestroyConstructionEv(struct CArea *self);

struct CArmy *_ZN5CArea9DraftArmyEi(struct CArea *self, int ArmyType);

void _ZN5CArea12EncirclementEv(struct CArea *self);

struct CArmy *_ZN5CArea7GetArmyEi(struct CArea *self, int ArmyIndex);

int _ZN5CArea10GetArmyIdxEP5CArmy(struct CArea *self, struct CArmy *);

int _ZN5CArea12GetCityLevelEv(struct CArea *self);

int _ZN5CArea11GetIndustryEv(struct CArea *self);

int _ZN5CArea16GetIndustryLevelEv(struct CArea *self);

int _ZN5CArea10GetRealTaxEv(struct CArea *self);

bool _ZN5CArea11HasArmyCardEi(struct CArea *self, int CardID);

bool _ZN5CArea11HasArmyCardEii(struct CArea *self, int ArmyIndex, int CardID);

bool _ZN5CArea8IsActiveEv(struct CArea *self);

bool _ZN5CArea12IsArmyActiveEi(struct CArea *self, int ArmyIndex);

bool _ZN5CArea16LostArmyStrengthEii(struct CArea *self, int ArmyIndex, int damage);

void _ZN5CArea10MoveArmyToEPS_(struct CArea *self, struct CArea *);

void _ZN5CArea15MoveArmyToFrontEP5CArmyb(struct CArea *self, struct CArmy *, bool instantly);

void _ZN5CArea15MoveArmyToFrontEib(struct CArea *self, int ArmyIndex, bool instantly);

void _ZN5CArea10OccupyAreaEPS_(struct CArea *self, struct CArea *);

void _ZN5CArea23ReduceConstructionLevelEv(struct CArea *self);

void _ZN5CArea10RemoveArmyEP5CArmy(struct CArea *self, struct CArmy *);

void _ZN5CArea6RenderEv(struct CArea *self);

void _ZN5CArea10RenderArmyEffiP5CArmy(struct CArea *self, float, float, int, struct CArmy *);

void _ZN5CArea14RenderBuildingEv(struct CArea *self);

void _ZN5CArea18RevertArmyStrengthEi(struct CArea *self, int ArmyIndex);

void _ZN5CArea18SetAllArmyMovementEi(struct CArea *self, int movement);

void _ZN5CArea13SetArmyActiveEib(struct CArea *self, int ArmyIndex, bool active);

void _ZN5CArea10SetArmyDirEif(struct CArea *self, int ArmyIndex, float direction);

void _ZN5CArea15SetConstructionEii(struct CArea *self, int ConstructionType, int ConstructionLevel);

void
_ZN5CArea13SetMoveInArmyEPS_P5CArmybbPKc(struct CArea *self, struct CArea *Origin, struct CArmy *,
                                         bool WillOccupy, bool WillComplain,
                                         const char *ComplainCommander);

void _ZN5CArea9TurnBeginEv(struct CArea *self);

void _ZN5CArea7TurnEndEv(struct CArea *self);

void _ZN5CArea6UpdateEf(struct CArea *self, float time);

void _ZN5CArea11UpgradeArmyEi(struct CArea *self, int ArmyIndex);

__END_DECLS
struct NewCountryAction{
private:
    CCountry* ActionCountry;
    std::vector<int> List_area_wounded;
    std::vector<int> List_area_frontLine;
    std::vector<int> List_area_existArmy;
    std::vector<int> List_area_existEliteArmy;
    std::vector<int> List_area_existAirForce; //如果id为-1，则表示该地块已被移除
    std::vector<int> List_area_opportunityForAttack;
    std::map<int,int> Map_area_existEliteArmy; //存储地块的精锐部队数
    int ChoosedAreaID;//当前选中的地块ID
    int ChoosedArmyIndex;//当前选中单位所在地块的下标
    static bool Arr_area_isOpportunityForAttack[]; //用于判断是否是有利战机地块
    static bool Arr_area_isFrontLine[];
    static bool Arr_area_existEliteArmy[];
public:
    NewCountryAction();
    void Start_countryAction(CCountry* actionCountry);
    void Collect_data();
    void Collect_area_wounded(CArea* collectedArea);
    void Collect_area_frontLine(CArea* collectArea);
    void Collect_arae_existArmy(CArea* collectArea);
    void Collect_area_existEliteArmy(CArea* collectArea);
    void Collect_area_existAirForce(CArea* collectArea);
    void Collect_area_opportunityForAttack(CArea* collectArea);
    void Init_Array();
    void EliteArmyTo_opportunityArea();
    void NormalArmyTo_opportunityArea();
    int Find_nearestAreaID_opportunityForAttack(CArea* startArea, CArmy* findingArmy, int ringCount);
    int Find_areaID_nearestArmy(CArea* startArea, int ringCount);
    void AirGroundOperation();
    void QuickSort_area_opportunityForAttack();
    void Adjust_armyToTopOfArea();
};
struct CMedal {
    float Pos[2];
    float Offset_y;
    float Alpha;
    bool AnimationCompleted;
#ifdef __cplusplus

    void Init(float x, float y);

    void Render();

    void Upate(float time);

#ifdef extends_CMedal
    extends_CMedal
#endif
#endif
};

__BEGIN_DECLS
void _ZN6CMedalC1Ev(struct CMedal *self);

void _ZN6CMedal4InitEff(struct CMedal *self, float x, float y);

void _ZN6CMedal6RenderEv(struct CMedal *self);

void _ZN6CMedal5UpateEf(struct CMedal *self, float time);

__END_DECLS

struct CAreaMark {
    int Size[2];
    short *Mark;
#ifdef __cplusplus

    void Init(int MapID);

    void Release();

    short GetMark(int x, int y);

#ifdef extends_CAreaMark
    extends_CAreaMark
#endif
#endif
};

__BEGIN_DECLS
void _ZN9CAreaMarkC1Ev(struct CAreaMark *self);

void _ZN9CAreaMarkD1Ev(struct CAreaMark *self);

void _ZN9CAreaMark4InitEi(struct CAreaMark *self, int MapID);

void _ZN9CAreaMark7ReleaseEv(struct CAreaMark *self);

short _ZN9CAreaMark7GetMarkEii(struct CAreaMark *self, int x, int y);

__END_DECLS

struct CCamera {
    float CenterPos[2];
    float Scale;
    float Speed[2];
    float TargetPos[2];
    bool IsMoving;
    bool AutoFixPos;
    float TopLeftPos[2];
    float MapSize[2];
    float ScreenHalfSize[2];
#ifdef __cplusplus

    void Apply();

    void Init(float x, float y, float w, float h);

    bool IsRectInCamera(float x, float y, float w, float h);

    bool IsRectInVisibleRegion(float x, float y, float w, float h);

    bool Move(float x, float y, bool border);

    void MoveTo(float x, float y, bool border);

    void SetAutoFixPos(bool enabled);

    bool SetPos(float x, float y, bool border);

    void SetPosAndScale(float x, float y, float scale);

    void SetSceneRect(float x, float y, float w, float h);

    void Upate(float time);

#ifdef extends_CCamera
    extends_CCamera
#endif
#endif
};

__BEGIN_DECLS
void _ZN7CCameraC1Ev(struct CCamera *self);

void _ZN7CCamera5ApplyEv(struct CCamera *self);

void _ZN7CCamera4InitEffff(struct CCamera *self, float x, float y, float w, float h);

bool _ZN7CCamera14IsRectInCameraEffff(struct CCamera *self, float x, float y, float w, float h);

bool
_ZN7CCamera21IsRectInVisibleRegionEffff(struct CCamera *self, float x, float y, float w, float h);

bool _ZN7CCamera4MoveEffb(struct CCamera *self, float x, float y, bool border);

void _ZN7CCamera6MoveToEffb(struct CCamera *self, float x, float y, bool border);

void _ZN7CCamera13SetAutoFixPosEb(struct CCamera *self, bool enabled);

bool _ZN7CCamera6SetPosEffb(struct CCamera *self, float x, float y, const bool border);

void _ZN7CCamera14SetPosAndScaleEfff(struct CCamera *self, float x, float y, float scale);

void _ZN7CCamera12SetSceneRectEffff(struct CCamera *self, float x, float y, float w, float h);

void _ZN7CCamera5UpateEf(struct CCamera *self, float time);

__END_DECLS

struct CBackground {
    int MapSize[2];
    int MapTopRightPos[2];
    int MapCount;
    struct ecTexture *BackgroundTexture[112];
    struct ecImage *BackgroundImage[112];
    struct ecTexture *BoxTexture[3];
    struct ecImage *BoxImage[3];
#ifdef __cplusplus

    void
    Init(int MapID, int TotalX, int TotalY, int TotalW, int TotalH, float EnableX, float EnableY,
         float EnableW, float EnableH);

    void Render(CCamera *camera);

    void RenderBox(CCamera *camera, float x, float y, float w, float h);

#ifdef extends_CBackground
    extends_CBackground
#endif
#endif
};

__BEGIN_DECLS
void _ZN11CBackgroundC1Ev(struct CBackground *self);

void _ZN11CBackgroundD1Ev(struct CBackground *self);

void _ZN11CBackground4InitEiiiiiffff(struct CBackground *self, int MapID, int TotalX, int TotalY,
                                     int TotalW, int TotalH, float EnableX, float EnableY,
                                     float EnableW, float EnableH);

void _ZN11CBackground6RenderEP7CCamera(struct CBackground *self, struct CCamera *camera);

void
_ZN11CBackground9RenderBoxEP7CCameraffff(struct CBackground *self, struct CCamera *camera, float x,
                                         float y, float w, float h);

__END_DECLS

struct CBomber {
    float PlanePos[2];
    struct ecImage *PlaneImages[2];
    struct ecImage *x10PecImage;
    int StartAreaID;
    int TargetAreaID;
    float FlySpeed;
    float PlaneXPosLimit;
    float TargetPos[2];
    float ArrivedTargetPos[2];
    float EffectTimer;
    float EffectSpeed;
    bool Bombing;
    bool BombOrAirborne;
    bool EffectComplete;
    bool PlaneArrived;
    bool IsAirborne;
    int ActionType;
    int AirStrikeFireCount;
#ifdef __cplusplus

    void Init();

    void Airborne(int TargetAreaID);

    void AircraftCarrierBomb(int StartAreaID, int TargetAreaID);

    void BombArea(int TargetAreaID, int ActionType);

    bool IsBombing();

    void Render();

    void Update(float time);

#ifdef extends_CBomber
    extends_CBomber
#endif
#endif
};

__BEGIN_DECLS
void _ZN7CBomberC1Ev(struct CBomber *self);

void _ZN7CBomberD1Ev(struct CBomber *self);

void _ZN7CBomber4InitEv(struct CBomber *self);

void _ZN7CBomber8AirborneEi(struct CBomber *self, int TargetAreaID);

void _ZN7CBomber19AircraftCarrierBombEii(struct CBomber *self, int StartAreaID, int TargetAreaID);

void _ZN7CBomber8BombAreaEii(struct CBomber *self, int TargetAreaID, int ActionType);

bool _ZN7CBomber9IsBombingEv(struct CBomber *self);

void _ZN7CBomber6RenderEv(struct CBomber *self);

void _ZN7CBomber6UpdateEf(struct CBomber *self, float time);

__END_DECLS

struct AreaData {
    int AreaPos[2];
    int AreaSize[2];
    int ArmyPos[2];
    int ConstructPos[2];
    int InstallationPos[2];
    enum AreaType {
        LAND, SEA
    } Type;
};
struct AreaNode {
    int AreaID;
    int SeachBy;
    int Layer;
};


struct CScene {
    std::vector<CArea *> AreaList;
    vector(CArea *) RenderAreaList;
    struct CArea *AreaSelected;
    float FlashTimer;
    float FlashSpeed;
    int FightFlashAreaID[2];
    AreaData *AreasData;
    struct ecTextureRes AreaTextureRes;
    struct ecImage **ZoneImage;
    int AreaCount;
    struct CAreaMark AreaMark;
    struct CBackground *CBackground;
    struct CCamera CCamera;
    struct CBomber *CBomber;
    std::list<CMedal*> MedalList;
    float VerticalArrowTimer;
    float VerticalArrowSpeed;
    float ArrowTimer;
    float ArrowSpeed;
    int *AdjoinRaw;
    int *Adjoin;
    float TopLeftPos[2];
    float Size[2];
    bool NewRoundFlashing;
    bool isIntelligentAI;
#ifdef __cplusplus
    bool Is_safedOpen(CArea* area);
    bool Is_passedArea(CArea* passedArea,CArmy* theArmy);
    int Get_passedArea_fromAdjcentAreas(CArea* startArea, CArmy* theArmy );
    bool Is_freeArea(CCountry* startCountry, CArea* targetArea);
    bool Is_ally(int startAreaID, int targetAreaID);
    bool Is_nonAlly(int startAreaID, int targetAreaID);
    bool Is_armisticeStatus(CCountry *theCountry, CCountry *targetCountry);
    bool Is_waring(int startAreaID,int targetAreaID);
    bool Is_inWarFog(CArea* targetArea,int allance);
    bool NewCheckMoveAble(int StartAreaID, int TargetAreaID, CArmy* attackArmy, int ArmyAreaID);
    bool NewCheckAttackAble(int StartAreaID, int TargetAreaID, int ArmyIndex,
                            int ArmyAreaID);

    void Init(const char *areasenable, int MapID);

    void Release();

    void AdjacentAreasEncirclement(int AreaID);

    void Airborne(int TargetAreaID);

    void AircraftCarrierBomb(int StartAreaID, int TargetAreaID);

    void AllAreasEncirclement();

    void AreaDisableOutofRect(int x, int y, int w, int h);

    void BombArea(int TargetAreaID, int ActionType);

    void CalSceneRect(float &x, float &y, float &w, float &h);

    bool CheckAdjacent(int AreaID_1, int AreaID_2);

    void CheckAdjacentData();

    bool CheckAttackable(int StartAreaID, int TargetAreaID, int ArmyIndex);

    bool CheckMoveable(int StartAreaID, int TargetAreaID, int ArmyIndex);

    void ClearAreas();

    void ClearTargets();

    void CreateArrow(int StartAreaID, int TargetAreaID);

    void CreateRenderAreaList();

    void DelAdjoin(int AreaID_1, int AreaID_2);

    void GainMedal(float x, float y);

    CArea *GetAdjacentArea(int AreaID, int index);

    int GetAdjacentAreaID(int AreaID, int index);

    CArea *GetArea(int AreaID);

    unsigned int GetNumAdjacentAreas(int AreaID);

    unsigned int GetNumAreas();

    CArea *GetSelectedArea();

    float GetTwoAreasDistance(int AreaID_1, int AreaID_2);

    int GetTwoAreasDistanceSquare(int AreaID_1, int AreaID_2);

    void InitAreaImage(int MapID);

    void InitAreas(int MapID, const char *areasenable);

    bool IsAdjoin(int StartAreaID, int TargetAreaID);

    bool IsBombing();

    bool IsMoving();

    bool IsRectInSecen(float x, float y, float w, float h);

    void LoadAdjoin(int MapID);

    void LoadAreaData(int MapID);

    void LoadAreaEnable(const char *areasenable);

    void LoadAreaTax(int MapID);

    void MakeAdjoin(int AreaID_1, int AreaID_2);

    bool Move(int x, int y);

    void MoveCameraBetweenArea(int AreaID_1, int AreaID_2);

    void MoveCameraCenterToArea(int AreaID);

    void MoveCameraToArea(int AreaID);

    bool MoveTo(int x, int y);

    void NewAdjoin();

    void Render();

    void RenderSeaAreas();

    void ResetTarget();

    void SaveAdjoin(int MapID);

    void SaveAreaData(int MapID);

    void SaveAreaTax(int MapID);

    CArea *ScreenToArea(float x, float y);

    int ScreenToAreaID(float x, float y);

    void ScreenToScene(float &x, float &y);

    void SelectArea(CArea *);

    void SelectArea(int AreaID);

    void SetAreaCountry(int AreaID, CCountry *);

    void SetCameraToArea(int AreaID);

    void SetSelAreaTargets(CArea *);

    void UnselectArea();

    void Update(float time);

    CArea *operator[](int AreaID) { return this->GetArea(AreaID); }

    operator unsigned int() { return this->GetNumAreas(); }
#ifdef extends_CScene
    extends_CScene
#endif
#endif
};

__BEGIN_DECLS
void _ZN6CSceneC1Ev(struct CScene *self);

void _ZN6CSceneD1Ev(struct CScene *self);

void _ZN6CScene4InitEPKci(struct CScene *self, const char *areasenable, int MapID);

void _ZN6CScene7ReleaseEv(struct CScene *self);

void _ZN6CScene25AdjacentAreasEncirclementEi(struct CScene *self, int AreaID);

void _ZN6CScene8AirborneEi(struct CScene *self, int TargetAreaID);

void _ZN6CScene19AircraftCarrierBombEii(struct CScene *self, int StartAreaID, int TargetAreaID);

void _ZN6CScene20AllAreasEncirclementEv(struct CScene *self);

void _ZN6CScene20AreaDisableOutofRectEiiii(struct CScene *self, int x, int y, int w, int h);

void _ZN6CScene8BombAreaEii(struct CScene *self, int TargetAreaID, int ActionType);

void
_ZN6CScene12CalSceneRectERfS0_S0_S0_(struct CScene *self, float *x, float *y, float *w, float *h);

bool _ZN6CScene13CheckAdjacentEii(struct CScene *self, int AreaID_1, int AreaID_2);

void _ZN6CScene17CheckAdjacentDataEv(struct CScene *self);

bool _ZN6CScene15CheckAttackableEiii(struct CScene *self, int StartAreaID, int TargetAreaID,
                                     int ArmyIndex);

bool _ZN6CScene13CheckMoveableEiii(struct CScene *self, int StartAreaID, int TargetAreaID,
                                   int ArmyIndex);

void _ZN6CScene10ClearAreasEv(struct CScene *self);

void _ZN6CScene12ClearTargetsEv(struct CScene *self);

void _ZN6CScene11CreateArrowEii(struct CScene *self, int StartAreaID, int TargetAreaID);

void _ZN6CScene20CreateRenderAreaListEv(struct CScene *self);

void _ZN6CScene9DelAdjoinEii(struct CScene *self, int AreaID_1, int AreaID_2);

void _ZN6CScene9GainMedalEff(struct CScene *self, float x, float y);

struct CArea *_ZN6CScene15GetAdjacentAreaEii(struct CScene *self, int AreaID, int index);

int _ZN6CScene17GetAdjacentAreaIDEii(struct CScene *self, int AreaID, int index);

struct CArea *_ZN6CScene7GetAreaEi(struct CScene *self, int AreaID);

unsigned int _ZN6CScene19GetNumAdjacentAreasEi(struct CScene *self, int AreaID);

unsigned int _ZN6CScene11GetNumAreasEv(struct CScene *self);

struct CArea *_ZN6CScene15GetSelectedAreaEv(struct CScene *self);

float _ZN6CScene19GetTwoAreasDistanceEii(struct CScene *self, int AreaID_1, int AreaID_2);

int _ZN6CScene25GetTwoAreasDistanceSquareEii(struct CScene *self, int AreaID_1, int AreaID_2);

void _ZN6CScene13InitAreaImageEi(struct CScene *self, int MapID);

void _ZN6CScene9InitAreasEiPKc(struct CScene *self, int MapID, const char *areasenable);

bool _ZN6CScene8IsAdjoinEii(struct CScene *self, int StartAreaID, int TargetAreaID);

bool _ZN6CScene9IsBombingEv(struct CScene *self);

bool _ZN6CScene8IsMovingEv(struct CScene *self);

bool _ZN6CScene13IsRectInSecenEffff(struct CScene *self, float x, float y, float w, float h);

void _ZN6CScene10LoadAdjoinEi(struct CScene *self, int MapID);

void _ZN6CScene12LoadAreaDataEi(struct CScene *self, int MapID);

void _ZN6CScene14LoadAreaEnableEPKc(struct CScene *self, const char *areasenable);

void _ZN6CScene11LoadAreaTaxEi(struct CScene *self, int MapID);

void _ZN6CScene10MakeAdjoinEii(struct CScene *self, int AreaID_1, int AreaID_2);

bool _ZN6CScene4MoveEii(struct CScene *self, int x, int y);

void _ZN6CScene21MoveCameraBetweenAreaEii(struct CScene *self, int AreaID_1, int AreaID_2);

void _ZN6CScene22MoveCameraCenterToAreaEi(struct CScene *self, int AreaID);

void _ZN6CScene16MoveCameraToAreaEi(struct CScene *self, int AreaID);

bool _ZN6CScene6MoveToEii(struct CScene *self, int x, int y);

void _ZN6CScene9NewAdjoinEv(struct CScene *self);

void _ZN6CScene6RenderEv(struct CScene *self);

void _ZN6CScene14RenderSeaAreasEv(struct CScene *self);

void _ZN6CScene11ResetTargetEv(struct CScene *self);

void _ZN6CScene10SaveAdjoinEi(struct CScene *self, int MapID);

void _ZN6CScene12SaveAreaDataEi(struct CScene *self, int MapID);

void _ZN6CScene11SaveAreaTaxEi(struct CScene *self, int MapID);

struct CArea *_ZN6CScene12ScreenToAreaEff(struct CScene *self, float x, float y);

int _ZN6CScene14ScreenToAreaIDEff(struct CScene *self, float x, float y);

void _ZN6CScene13ScreenToSceneERfS0_(struct CScene *self, float *x, float *y);

void _ZN6CScene10SelectAreaEP5CArea(struct CScene *self, struct CArea *);

void _ZN6CScene10SelectAreaEi(struct CScene *self, int AreaID);

void _ZN6CScene14SetAreaCountryEiP8CCountry(struct CScene *self, int AreaID, struct CCountry *);

void _ZN6CScene15SetCameraToAreaEi(struct CScene *self, int AreaID);

void _ZN6CScene17SetSelAreaTargetsEP5CArea(struct CScene *self, struct CArea *);

void _ZN6CScene12UnselectAreaEv(struct CScene *self);

void _ZN6CScene6UpdateEf(struct CScene *self, float time);

extern struct CScene g_Scene;
__END_DECLS

#endif //EASYTECH_CSCENE_H
