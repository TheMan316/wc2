#ifndef EASYTECH_COBJECTDEF_H
#define EASYTECH_COBJECTDEF_H

#include "cxx.h"
#include "cxxmap.h"
#include "cxxstring.h"

enum CARD_ID {
    NoCard,
    InfantryCard = 0,
    ArmourCard,
    ArtilleryCard,
    RocketCard,
    TankCard,
    MediumTankCard,
    DestroyerCard,
    CruiserCard,
    BattleShipCard,
    AircraftCarrierCard,//9
    AirStrikeCard,
    BomberCard,
    AirborneForceCard,
    NuclearBombCard,
    CityCard,
    IndustryCard,
    AirportCard,
    LandFortCard,
    EntrenchmentCard,
    AntiaircraftCard,
    RadarCard,
    ResearchCard,
    AGRCard,//22 反坦克
    LARCard,   //轻炮
    ECRCard,//工程
    CommanderCard,
    SupplyLineCard,
    AceForcesCard,//27
    HeavyTankCard,
    BiochemicalForceCard,
    SubmarineCard,//30
    ExpendArmyCard, //31
    SplitCard ,//32
    AirDefenseCard, //33
    HARCard,//34 重炮团
    TDRCard, //反坦克突击炮
    TTRCard,//后勤团
    MARCard//37 自走炮团
};

//Game data definition
struct CardDef {
    string *Name;
    string *Image;
    string *Intro;
    enum CARD_ID ID;
    enum CardType {
        Army, Navy, AirForce, Development, Strategy
    };
    enum CardType Type;
    int Price;
    int Industry;
    int Round;
    int Tech;

};

struct ArmyDef {
    string Name;
    //添加新单位 记得去Is_xxx中增加
    //记得在 Get_armyValue中添加
    enum ArmyType {
        InfantryD1905,//0
        Panzer,
        Artillery,
        Rocket,
        Tank,
        HeavyTank,
        Destroyer,
        Cruiser,
        Battleship,
        AircraftCarrier,//9
        Carrier,//新增 10
        AirStrike,//新增
        Bomber,//新增
        Airborne,//新增
        NuclearBomb,//新增  14
        Submarine = 28,
        IntercontinentalMissile = 29,
        InfantryA1905 = 30,
        HeavyTank_1918 = 31,
        Fight_1914 = 32,
        Boom_1914 = 33
    } ID;
    int Hp;
    int Movement;
    int MinAttack;
    int MaxAttack;
    int MinPiercing; //穿甲攻击
    int MaxPiercing;
    int Armor; //装甲硬度
    int MaxOrganization; //最大组织度
    int Collapse;//崩溃的组织度阈值
    int MinBlow;//最小组织度击损
    int MaxBlow; //最大组织度击损
    int RecoverOrganization; //每回合恢复的组织度
    int Miss;//闪避值

};
struct CountryArmyDef{
    enum servicesType {
        //添加新成员记得修改 Is_existTheServicesType函数
        Infantry,//0
        Panzer,
        Artillery,
        Tank,
        Navy,
        AirForce
    } ;
    char CountryName[16];
    std::map<int , ArmyDef*> mapArmydef;
};
struct CommanderDef {
    std::string name;
    string country;
    int rank;
    int medal[6];
};

struct UnitMotions;
struct UnitPositions;
struct GeneralPhoto;
struct BattleDef;
struct ConquestDef;

struct CObjectDef {
    struct ArmyDefList {
        struct ArmyDef *Def[15];
    };
    map(const string, ArmyDefList*) mapArmyDefList;
    struct CardDef CardDef28[28];
    map(const string, UnitMotions*) mapUnitMotions;
    map(const string, UnitPositions*) mapUnitPos;
    map(const string, CommanderDef*) mapCommanderDef;
    map(const string, GeneralPhoto*) mapGeneralPhotos;
    map(const string, BattleDef*) mapBattleDef;
    map(const string, ConquestDef*) mapConquestList;
#ifdef __cplusplus

    static CObjectDef *Instance();

    void Init();

    void Release();

    void Destroy();

    ArmyDef *GetArmyDef(int ArmyType, const char *CountryName);

    BattleDef *GetBattleDef(const char *key);

    CardDef *GetCardDef(CARD_ID);

    int GetCardTargetType(CardDef *);

    CommanderDef *GetCommanderDef(const char *name);

    ConquestDef *GetConquestDef(const char *name);

    GeneralPhoto *GetGeneralPhoto(const char *name);

    UnitMotions *GetUnitMotions(const char *, const char *);

    UnitPositions *GetUnitPositions(const char *);
//游戏启动时被调用
    void LoadArmyDef();

    void LoadBattleList();
//游戏启动时被调用
    void LoadCardDef();

    void LoadCommanderDef();

    void LoadConquestList();

    void LoadGeneralPhotos();

    void LoadUnitMotions();

    void LoadUnitPositons();

    void ReleaseArmyDef();

    void ReleaseBattleList();

    void ReleaseCommanderDef();

    void ReleaseConquestList();

    void ReleaseGeneralPhotos();

    void ReleaseUnitMotions();

    void ReleaseUnitPositions();

#ifdef extends_CObjectDef
    extends_CObjectDef
#endif
#endif
};

__BEGIN_DECLS
struct CObjectDef *_ZN10CObjectDef8InstanceEv();

void _ZN10CObjectDefC1Ev(struct CObjectDef *self);

void _ZN10CObjectDefD1Ev(struct CObjectDef *self);

void _ZN10CObjectDef4InitEv(struct CObjectDef *self);

void _ZN10CObjectDef7ReleaseEv(struct CObjectDef *self);

void _ZN10CObjectDef7DestroyEv(struct CObjectDef *self);

struct ArmyDef*  _ZN10CObjectDef10GetArmyDefEiPKc(struct CObjectDef *self, int ArmyType, const char *CountryName);

struct BattleDef *_ZN10CObjectDef12GetBattleDefEPKc(struct CObjectDef *self, const char *key);

struct CardDef *_ZN10CObjectDef10GetCardDefE7CARD_ID(struct CObjectDef *self, enum CARD_ID);

int _ZN10CObjectDef17GetCardTargetTypeEP7CardDef(struct CObjectDef *self, struct CardDef *);

struct CommanderDef *
_ZN10CObjectDef15GetCommanderDefEPKc(struct CObjectDef *self, const char *name);

struct ConquestDef *_ZN10CObjectDef14GetConquestDefEPKc(struct CObjectDef *self, const char *name);

struct GeneralPhoto *
_ZN10CObjectDef15GetGeneralPhotoEPKc(struct CObjectDef *self, const char *name);

struct UnitMotions *
_ZN10CObjectDef14GetUnitMotionsEPKcS1_(struct CObjectDef *self, const char *, const char *);

struct UnitPositions *_ZN10CObjectDef16GetUnitPositionsEPKc(struct CObjectDef *self, const char *);

void _ZN10CObjectDef11LoadArmyDefEv(struct CObjectDef *self);

void _ZN10CObjectDef14LoadBattleListEv(struct CObjectDef *self);

void _ZN10CObjectDef11LoadCardDefEv(struct CObjectDef *self);

void _ZN10CObjectDef16LoadCommanderDefEv(struct CObjectDef *self);

void _ZN10CObjectDef16LoadConquestListEv(struct CObjectDef *self);

void _ZN10CObjectDef17LoadGeneralPhotosEv(struct CObjectDef *self);

void _ZN10CObjectDef15LoadUnitMotionsEv(struct CObjectDef *self);

void _ZN10CObjectDef16LoadUnitPositonsEv(struct CObjectDef *self);

void _ZN10CObjectDef14ReleaseArmyDefEv(struct CObjectDef *self);

void _ZN10CObjectDef17ReleaseBattleListEv(struct CObjectDef *self);

void _ZN10CObjectDef19ReleaseCommanderDefEv(struct CObjectDef *self);

void _ZN10CObjectDef19ReleaseConquestListEv(struct CObjectDef *self);

void _ZN10CObjectDef20ReleaseGeneralPhotosEv(struct CObjectDef *self);

void _ZN10CObjectDef18ReleaseUnitMotionsEv(struct CObjectDef *self);

void _ZN10CObjectDef20ReleaseUnitPositionsEv(struct CObjectDef *self);

extern struct CObjectDef *_ZN10CObjectDef10m_InstanceE;
__END_DECLS

#endif //EASYTECH_COBJECTDEF_H
