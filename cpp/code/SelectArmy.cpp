//Allow to check army stacks of other country
//allow to use card on select army side bar

#define extends_CGameRes \
void RenderUIArmy(const char *CountryName, float x, float y, int ArmyType, int HP, int MaxHP, int Movement, int Cards, int Level, int CommonType, bool AI);

#include <cmath>
#include <cstddef>
#include <easytech.h>
#include "CStateManager/CGameState.h"
#include "CGameRes.h"
#include "CLi.h"

//control the showing of the side bar
void CGameState::TouchEnd(float x, float y, int index) {
    //check touching state
    int FirstTouchIndex = this->FirstTouchIndex;
    if (index == this->SecondTouchIndex) {
        this->SecondTouch = false;
        this->SecondTouchIndex = 0;
    }
    if (index == this->FirstTouchIndex) {
        this->FirstTouch = false;
        this->FirstTouchIndex = 0;
    }
    //当玩家的手操作时
    if (g_GameManager.IsManipulate()) {
        CountryAction CountryAction = {CountryAction::NoAction, NoCard, 0, 0, 0};
        this->ShouldCameraAutoFixPos = false;
        if (index != FirstTouchIndex)
            return;
        this->CTouchInertia.TouchEnd(x, y, index);
        if (fabs(this->FirstTouchStartPos[0] - x) >= 15.0)
            return;
        if (fabs(this->FirstTouchStartPos[1] - y) >= 15.0)
            return;
        g_Scene.NewRoundFlashing = false;
        CArea *TouchArea = g_Scene.ScreenToArea(x, y);
        if (TouchArea == NULL || !TouchArea->Enable)
            return;
        //use card
        CardDef *Card = this->BuyCardGUI->GetSelCard();
        if (Card != NULL && this->BuyCardGUI->CardCanTarget && !this->BuyingCard) {
            if (g_GameManager.GetPlayerCountry()->CheckCardTargetArea(Card, TouchArea->ID)) {
                CountryAction.ActionType = CountryAction::UseCardAction;
                CountryAction.CardID = Card->ID;
                CountryAction.TargetAreaID = TouchArea->ID;
                CountryAction.ArmyIndex = 0;
                g_GameManager.GetPlayerCountry()->Action(CountryAction);
                this->BuyingCard = true;
                this->TaxGUI->SetArea(TouchArea->ID);
            }
            return;
        }
        //select or unselect area
        CArea *SelectedArea = g_Scene.GetSelectedArea();
        if (SelectedArea) {
            if (SelectedArea == TouchArea) {
                this->UnselectArea();
                return;
            }
            //manipulating army
            if (SelectedArea->IsActive() &&
                (SelectedArea->Country == g_GameManager)) {
                if (SelectedArea->ArmyDrafting || SelectedArea->ArmyMovingIn ||
                    SelectedArea->ArmyMovingToFront)
                    return;
                if (g_Scene.CheckMoveable(SelectedArea->ID, TouchArea->ID, 0)) {
                    CountryAction.ActionType = CountryAction::ArmyMoveAction;
                    CountryAction.StartAreaID = SelectedArea->ID;
                    CountryAction.ArmyIndex = 0;
                    CountryAction.TargetAreaID = TouchArea->ID;
                    g_GameManager.GetPlayerCountry()->Action(CountryAction);
                } else if (g_Scene.CheckAttackable(SelectedArea->ID, TouchArea->ID, 0)) {
                    this->AttackBoxGUI->SetAttack(SelectedArea->ID, TouchArea->ID);
                    this->AttackBoxGUI->Show();
                }
                this->UnselectArea();
                return;
            }
            this->UnselectArea();
        }
        this->SelectArea(TouchArea->ID);
    }
}


static char TempStr[32];
static char TempStr1[32];
ecText *ArmyMovementText;
ecText *ArmyDesignation;

//新建国需要添加到这个国家列表中，然后在army_hd.xml中，要把小国旗加上。
const char *arr_country[] = {"gb", "ru", "am", "fr", "ja", "it", "de", "ca", "fl", "ro", "bg", "be",
                             "gr", "cn", "tw", "in", "au", "pl", "hu", "yu", "ch", "se", "pt", "dk",
                             "no", "nl", "es", "mx", "nk", "rk", "tr", "ne", "bb"};
//剧本加载时被调用
void CGameRes::Load() {
    const char *medal_list[] = {"gb", "de", "am", "fr", "it", "ru", "ja", "cn", "tw",
                                "common_1", "common_2", "common_3", "zhukov", "manstein", "eisenhower"};
    const char *alliance_list[] = {"blue", "green", "red", "gray"};
    const char *installtion_list[] = {"fortress", "wire", "aagun", "radar"};

    if (true) {
        TextureRes_army.LoadRes("army_hd.xml", true);
        TextureRes_cardtex.LoadRes("cardtex_hd.xml", true);
    } else {
        TextureRes_army.LoadRes("army.xml", false);
        TextureRes_cardtex.LoadRes("cardtex.xml", false);
    }

    for (const char *country_name : arr_country) {
        map_countryArmyImages[country_name].push_back(TextureRes_army.GetImageByFormat("InfantryD1905_%s.png", country_name));          // InfantryD1905
        map_countryArmyImages[country_name].push_back(TextureRes_army.GetImageByFormat("panzer_%s.png", country_name));           // Panzer
        map_countryArmyImages[country_name].push_back(TextureRes_army.GetImageByFormat("cannon_%s.png", country_name));           // Artillery
        map_countryArmyImages[country_name].push_back(TextureRes_army.GetImageByFormat("rocketlauncher_%s.png", country_name));   // Rocket
        map_countryArmyImages[country_name].push_back(TextureRes_army.GetImageByFormat("tank_%s.png", country_name));             // Tank
        map_countryArmyImages[country_name].push_back(TextureRes_army.GetImageByFormat("heavytank_%s.png", country_name));        // HeavyTank
        map_countryArmyImages[country_name].push_back(TextureRes_army.GetImageByFormat("destroyer.png"));                         // Destroyer
        map_countryArmyImages[country_name].push_back(TextureRes_army.GetImageByFormat("cruiser.png"));                           // Cruiser
        map_countryArmyImages[country_name].push_back(TextureRes_army.GetImageByFormat("battleship.png"));                        // Battleship
        map_countryArmyImages[country_name].push_back(TextureRes_army.GetImageByFormat("aircraftcarrier.png"));                   // AircraftCarrier 9
        map_countryArmyImages[country_name].push_back(TextureRes_army.GetImageByFormat("transportship.png"));                     // AntitankGun
        map_countryArmyImages[country_name].push_back(TextureRes_army.GetImageByFormat("fighter.png"));                           // AirStrike
        map_countryArmyImages[country_name].push_back(TextureRes_army.GetImageByFormat("bomber.png"));                            // Bomber
        map_countryArmyImages[country_name].push_back(TextureRes_army.GetImageByFormat("bomber.png"));                            // Airborne
        map_countryArmyImages[country_name].push_back(TextureRes_army.GetImageByFormat("bomber.png"));                            // NuclearBomb
        map_countryArmyImages[country_name].push_back(TextureRes_army.GetImageByFormat("submarine_%s.png", country_name));                         // Submarine 15
        map_countryArmyImages[country_name].push_back(TextureRes_army.GetImageByFormat("intercontinentalMissile_%s.png", country_name));  // 16
        map_countryArmyImages[country_name].push_back(TextureRes_army.GetImageByFormat("InfantryA1905_%s.png", country_name));    //  17


        Image_flag[country_name] = TextureRes_army.GetImageByFormat("flag_%s.png", country_name);                      // Flag
    }

    for (const char *medal_name : medal_list) {
        ImageCountryMedal[medal_name] = TextureRes_army.GetImageByFormat("medal_%s.png", medal_name);
    }

    for (int i = 0; i < 10; i++) {
        Image_newCardmark[i] = TextureRes_army.GetImageByFormat("cardmark_%d.png", i + 1);
    }


    for (int i = 0; i < 3; ++i) {
        Image_buildmark_factory[i] = TextureRes_army.GetImageByFormat("buildmark_factory_%d.png", i + 1);
    }
    for (int i = 0; i < 4; i++) {
        Image_unitbase_blue[i] = TextureRes_army.GetImageByFormat("unitbase_blue_%d.png", i + 1);
        Image_unitbase_green[i] = TextureRes_army.GetImageByFormat("unitbase_green_%d.png", i + 1);
        Image_unitbase_red[i] = TextureRes_army.GetImageByFormat("unitbase_red_%d.png", i + 1);
        Image_unitbase_grey[i] = TextureRes_army.GetImageByFormat("unitbase_gray_%d.png", i + 1);
        Image_unitlevelmark[i] = TextureRes_army.GetImageByFormat("unitlevelmark_%d.png", i + 1);
        Image_movementmark[i] = TextureRes_army.GetImageByFormat(i > 0 ? "hpbar_movementmark_%d.png" : "hpbar_movementmark_e.png", i);
        Image_buildmark_city[i] = TextureRes_army.GetImageByFormat("buildmark_city_%d.png", i + 1);
        Image_maek_carriers_color[i] = TextureRes_army.GetImageByFormat("mark_carriers_%s.png", alliance_list[i]);
        Image_buildmark_installtion[i] = TextureRes_army.GetImageByFormat("buildmark_%s.png", installtion_list[i]);
    }

    for (int i = 0; i < 5; i++) {
        Image_commander_level[i] = TextureRes_army.GetImageByFormat("commander_level_%d_s.png", i + 1);
        Image_card_research[i] = TextureRes_cardtex.GetImageByFormat(i < 4 ? "card_d_research_%d.png" : "card_d_research_max.png", i + 2);
    }

    for (int i = 0; i < 6; i++) {
        Image_mark_carriers[i] = TextureRes_army.GetImageByFormat("mark_carriers_%d.png", i + 1);
    }

    Image_medal = TextureRes_army.GetImageByFormat("medal.png");
    Image_medal_light = TextureRes_army.GetImageByFormat("medal_light.png");
    Image_hpbar = TextureRes_army.GetImageByFormat("hpbar.png");
    Image_hpbar_fill = TextureRes_army.GetImageByFormat("hpbar_fill.png");
    Image_hpbar_fill->SetTextureRect(Image_hpbar_fill->TextureRect.x + 1.0f, Image_hpbar_fill->TextureRect.y, 1.0f, Image_hpbar_fill->TextureRect.h);
    Image_arrow_green = TextureRes_army.GetImageByFormat("arrow_green.png");
    Image_arrow_yellow = TextureRes_army.GetImageByFormat("arrow_yellow.png");
    Image_arrowshadow = TextureRes_army.GetImageByFormat("arrowshadow.png");
    Image_arrow_red = TextureRes_army.GetImageByFormat("arrow_red.png");
    Image_arrow_blue = TextureRes_army.GetImageByFormat("arrow_blue.png");
    Image_buildmark_airport = TextureRes_army.GetImageByFormat("buildmark_airport.png");
    Image_buildmark_port = TextureRes_army.GetImageByFormat("buildmark_port.png");
    Image_card_shadow = TextureRes_cardtex.GetImageByFormat("card_shadow.png");
    Image_card_common = TextureRes_cardtex.GetImageByFormat("card_common.png");

//    Image_OrganizationFill = TextureRes_army.GetImageByFormat("organizationFill.png");
//    Image_circle = TextureRes_army.GetImageByFormat("circle.png");
//    Image_circle->SetTextureRect(Image_circle->TextureRect.x + 1.0f, Image_circle->TextureRect.y, 1.0f, Image_circle->TextureRect.h);
//
//    Image_OrganizationFill->SetTextureRect(Image_OrganizationFill->TextureRect.x + 1.0f, Image_OrganizationFill->TextureRect.y, 1.0f, Image_OrganizationFill->TextureRect.h);

    ecGraphics *graphics = ecGraphics::Instance();
    if (graphics->WidthMode == 3) {
        TextureRes_battlebg.LoadRes("battlebg1_iPad.xml", false);
        TextureRes_battlebg.LoadRes("battlebg2_iPad.xml", false);
        TextureRes_battlebg.LoadRes("battlebg3_iPad.xml", false);
    } else if (graphics->DisplaySize[0] <= 480.0) {
        TextureRes_battlebg.LoadRes("battlebg1.xml", false);
        TextureRes_battlebg.LoadRes("battlebg2.xml", false);
    } else if (graphics->DisplaySize[0] <= 568.0) {
        TextureRes_battlebg.LoadRes("battlebg1_568h.xml", false);
        TextureRes_battlebg.LoadRes("battlebg2_568h.xml", false);
    } else {
        TextureRes_battlebg.LoadRes("battlebg1_640hd.xml", true);
        TextureRes_battlebg.LoadRes("battlebg2_640hd.xml", true);
    }

    if (graphics->WidthMode == 3 || false) {
        TextureRes_flag.LoadRes("flag.xml", false);
    } else {
        TextureRes_flag.LoadRes("flag_hd.xml", true);
    }

    ecEffectResManager::Instance()->LoadTextureRes("eff.xml");
    ecEffectResManager::Instance()->LoadEffectRes("effect_gunfire.xml");
    ecEffectResManager::Instance()->LoadEffectRes("effect_mgunfire.xml");
    ecEffectResManager::Instance()->LoadEffectRes("effect_tankfire.xml");
    ecEffectResManager::Instance()->LoadEffectRes("effect_fortfire.xml");
    ecEffectResManager::Instance()->LoadEffectRes("effect_shipfire.xml");
    ecEffectResManager::Instance()->LoadEffectRes("effect_exp.xml");
    ecEffectResManager::Instance()->LoadEffectRes("effect_airstrike.xml");
    ecEffectResManager::Instance()->LoadEffectRes("effect_parachute.xml");
    ecEffectResManager::Instance()->LoadEffectRes("effect_nuclearbomb.xml");
    ecEffectResManager::Instance()->LoadEffectRes("effect_strike1.xml");
    ecEffectResManager::Instance()->LoadEffectRes("effect_strike2.xml");
    ecEffectResManager::Instance()->LoadEffectRes("effect_strike3.xml");
    ecEffectResManager::Instance()->LoadEffectRes("effect_strike4.xml");
    ecEffectResManager::Instance()->LoadEffectRes("effect_strike5.xml");
    ArmyMovementText = new ecText();
    ArmyMovementText->Init(&g_Num3);
    ArmyMovementText->SetColor(0xFFFFDF5F);
    ArmyDesignation = new ecText();
    ArmyDesignation->Init(&g_Num3);
    ArmyDesignation->SetColor(0xFF00FF00);
    Image_buildmark_oilwell = new ecImage();
    Image_buildmark_oilwell->Init(TextureRes_army.GetImage("buildmark_oilwell.png"));

    Image_OrganizationFill =  new ecImage();
    Image_OrganizationFill->Init(TextureRes_army.GetImage("organizationFill.png"));

    Image_circle = new ecImage();
    Image_circle->Init(TextureRes_army.GetImage("circle.png"));
}

def_easytech(_ZN8CGameRes7ReleaseEv)
//剧本中点击菜单退出按钮被调用，用于释放资源
void CGameRes::Release() {
    easytech(_ZN8CGameRes7ReleaseEv)(this);
    for (const std::string& country_name : arr_country) {
        std::vector<ecImage *> armyImages_country = map_countryArmyImages[country_name];
        for (ecImage *armyImage : armyImages_country)
            delete armyImage;
        std::vector<ecImage *>().swap(armyImages_country);
    }
    std::map(std::string, std::vector<ecImage *>)().swap(map_countryArmyImages);
    delete ArmyMovementText;
    delete ArmyDesignation;
    delete Image_buildmark_oilwell;
    delete Image_OrganizationFill;
    delete Image_circle;
    for (int i = 0; i < 10; ++i) {
        delete Image_newCardmark[i];
    }
}

static unsigned long HpColor(int HP, int MaxHP) {
    int r, g, b;
    if (HP * 2 <= MaxHP) {
        r = 0;
        g = 255 - 255 * (MaxHP - 2 * HP) / MaxHP;
        b = 255;
    } else {
        g = 255;
        b = 255 - 255 * (2 * HP - MaxHP) / MaxHP;
        r = 128 - (b >> 1);
    }
    return b + (g << 8) + (r << 16) - 0x1000000ul;
}
static unsigned long OrganizationColor(int organization, int collapse) {
    int r, g, b;
    if (organization < collapse) {
        r = 255;
        g = 150;
        b = 0;
    } else {
        g = 250;
        b = 243;
        r = 128;
    }
    return b + (g << 8) + (r << 16) - 0x1000000ul;
}
void CGameRes::RenderArmyOrganization(int x, int y, int organization, int collapse, int armyCount){
    Image_OrganizationFill->SetColor(OrganizationColor(organization, collapse), -1);
    float _x = 13.0f;
    float _y = 10.0f;
        _y += (armyCount -1) *2.0f;
    Image_OrganizationFill->RenderEx(x - _x, y - _y, 0.0f, (float)organization/6, 1.0);
}

//绘制军队番号 新函数
void CGameRes::RenderArmyDesignation(int designation,float  x, float y,int armyCount) {

    sprintf(TempStr1, "%d", designation);
        ArmyDesignation->SetText(TempStr1);
        float temY = 0.0f;
    if (armyCount == 2){
        temY=1.5f;
    }else if (armyCount == 3){
        temY=1.0f;
    }else if(armyCount == 4){
        temY=3.5f;
    }
    if (designation<10){
        ArmyDesignation->DrawText(x - 24.0f, y - 71.0f - temY, 1);
    }
    else if (designation>=10 && designation < 100){
        ArmyDesignation->DrawText(x - 20.0f, y - 71.0f - temY, 1);
    }else if(designation>=100 && designation < 1000){
        ArmyDesignation->DrawText(x - 15.0f, y - 71.0f - temY, 1);
    }else{
        ArmyDesignation->DrawText(x - 10.0f, y - 71.0f - temY, 1);
    }
}

void CGameRes::RenderArmyInfo(int ArmyCount, float x, float y, int HP, int MaxHP, int Movement, int Cards, int Level) {
    const float OffsetY[] = {10.0, 13.0, 15.0, 17.0, 10.0, 13.0, 15.0, 17.0, 10.0};
    y -= OffsetY[ArmyCount - 1];
    Image_hpbar->Render(x - 30.0f, y - 12.0f);
    Image_hpbar_fill->SetColor(HpColor(HP, MaxHP), -1);
    Image_hpbar_fill->RenderEx(x - 11.0f, y - 4.0f, 0.0f, (float)((float)HP * 33.0) / (float)MaxHP, 1.0);
    sprintf(TempStr, "%d", Movement);
    if (Movement > 0) {
        ArmyMovementText->SetText(TempStr);
        ArmyMovementText->DrawText(x - 16.0f, y - 9.0f, 1);
    } else {
        Image_movementmark[0]->Render(x - 23.0f, y - 7.0f);
    }
    if ((Level > 0 && Level <= 4) && (Cards & (1 << CArmy::Commander)) == 0)
        Image_unitlevelmark[Level - 1]->Render(x + 8.0f, y - 8.0f);
    if (Cards != 0) {
        float tempY = y - 20.0;
        float tempX = x + 20.0;
        int cardCount = 0;
        int p = 0;
        for (int i = 0; i < 10; i++) {
            //绕过指挥官
            if (i == 3)
                continue;
            if ( i == 8)
                tempY = y- 20.0;
            if (i >= 8 && (Cards >> i) & 1){
                //如果卡牌数量已经等于3，则下一张卡往右
                if (cardCount == 3){
                    tempX += 12.0;
                    tempY = y - 20.0;
                }
                this->Image_newCardmark[p]->Render(tempX, tempY);
                tempY -= 15.0;
                cardCount++;
            }else{
                if ((Cards >> i) & 1) {
                    if (cardCount == 3){
                        tempX += 12.0;
                        tempY = y - 20.0;
                    }
                    this->Image_newCardmark[p]->Render(tempX  , tempY);
                    tempY -= 15.0;
                    cardCount++;
                }
            }
            p++;
        }
    }
}
/**
     *=======================================================<p>
     * 【功能描述】：<p>
     * 绘制右边单位选择界面的军队图片<p>
     * 【参数说明】：<p>
     * 【返回值说明】：<p>
     * void <p>
     *=======================================================<p>
     * 【创建人】： limlimg <p>
     * 【修改人】： 李德邻 <p>
     * 【创建时间】： ？？？<p>
     * 【修改时间】： ？？？
     */
void CGameRes::RenderUIArmy(const char *CountryName, float x, float y, int ArmyType, int HP, int MaxHP,
                       int Movement, int Cards, int Level, int CommonType, bool AI) {
    ecImage *ArmyImage = this->GetArmyImage(CountryName, ArmyType, false);
    if (ArmyImage != NULL) {
        ArmyImage->SetColor(0xFFFFFFFF, -1);
        ArmyImage->RenderEx(x, y, 0.0, -1.0, 1.0);
    }
    this->Image_hpbar->Render(x - 30.0, y - 12.0);
    this->Image_hpbar_fill->SetColor(HpColor(HP, MaxHP), -1);
    this->Image_hpbar_fill->RenderEx(x - 11.0, y - 4.0, 0.0, (float) HP * 33.0 / (float) MaxHP,
                                     1.0);

    sprintf(TempStr, "%d", Movement);
    if (Movement > 0) {
        ArmyMovementText->SetText(TempStr);
        ArmyMovementText->DrawText(x - 16.0f, y - 9.0f, 1);
    } else {
        this->Image_movementmark[0]->Render(x - 23.0f, y - 7.0f);
    }
    if (Cards & (1 << CArmy::Commander)) {
        if (AI)
            this->RenderAICommanderMedal(1, x, y + 10.0, CountryName, CommonType);
        else
            this->Image_commander_level[Level / 3]->Render(x + 8.0, y - 8.0);
    } else if (Level > 0 && Level <= 4)
        this->Image_unitlevelmark[Level - 1]->Render(x + 8.0, y - 8.0);
    if (Cards != 0) {
//        for (int i = 0; i < 10; i++) {
//            if ((Cards >> i) & 1) {
//                this->Image_newCardmark[i]->Render(x + 16.0, y - 20.0);
//                y -= 15.0;
//            }
//        }
        int p = 0;
        float tempX = x ;
        float tempY = y - 20.0;
        int cardCount = 0;
        for (int i = 0; i < 10; i++) {
            //绕过指挥官
            if (i == 3)
                continue;
            if ( i == 8)
                tempY = y- 20.0;

            if (i >= 8 && (Cards >> i) & 1){
                //如果卡牌数量已经等于3，则下一张卡往右
                if (cardCount == 3){
                    tempY = y - 20.0;
                }

                this->Image_newCardmark[p]->Render(tempX, tempY);
                tempY -= 15.0;
                cardCount++;
            }else{
                if ((Cards >> i) & 1) {
                    if (cardCount == 3){
                        tempX += 16.0;
                        tempY = y - 20.0;
                    }

                    this->Image_newCardmark[p]->Render(tempX  , tempY);
                    tempY -= 15.0;
                    cardCount++;
                }
            }
            p++;
        }
    }
}

void CGameRes::RenderUIAttackArmy(const char *CountryName, float x, float y, int ArmyType, int HP, int MaxHP, int Movement, int Cards, int Level, int CommonType, bool AI) {
    ecImage *ArmyImage = this->GetArmyImage(CountryName, ArmyType, false);
    if (ArmyImage != NULL) {
        ArmyImage->SetColor(0xFFFFFFFF, -1);
        ArmyImage->RenderEx(x, y, 0.0, 1.0, 1.0);
    }
    this->Image_hpbar->Render(x - 30.0, y - 12.0);
    this->Image_hpbar_fill->SetColor(HpColor(HP, MaxHP), -1);
    this->Image_hpbar_fill->RenderEx(x - 11.0, y - 4.0, 0.0, (float) HP * 33.0 / (float) MaxHP, 1.0);
    sprintf(TempStr, "%d", Movement);
    if (Movement > 0) {
        ArmyMovementText->SetText(TempStr);
        ArmyMovementText->DrawText(x - 16.0f, y - 9.0f, 1);
    } else {
        this->Image_movementmark[0]->Render(x - 23.0f, y - 7.0f);
    }
    if (Cards & (1 << CArmy::Commander)) {
        if (AI)
            this->RenderAICommanderMedal(1, x, y + 10.0, CountryName, CommonType);
        else
            this->Image_commander_level[Level / 3]->Render(x + 8.0, y - 8.0);
    } else if (Level > 0 && Level <= 4)
        this->Image_unitlevelmark[Level - 1]->Render(x + 8.0, y - 8.0);
    if (Cards != 0) {
//        for (int i = 0; i < 10; i++) {
//            if ((Cards >> i) & 1) {
//                this->Image_newCardmark[i]->Render(x + 36.0, y - 20.0);
//                y -= 15.0;
//            }
//        }
        int p = 0;
        float tempX = x + 20.0 ;
        float tempY = y - 20.0;
        int cardCount = 0;
        for (int i = 0; i < 10; i++) {
            //绕过指挥官
            if (i == 3)
                continue;
            if ( i == 8)
                tempY = y- 20.0;

            if (i >= 8 && (Cards >> i) & 1){
                //如果卡牌数量已经等于3，则下一张卡往右
                if (cardCount == 3){
                    tempX += 16.0;
                    tempY = y - 20.0;
                }

                this->Image_newCardmark[p]->Render(tempX, tempY);
                tempY -= 15.0;
                cardCount++;
            }else{
                if ((Cards >> i) & 1) {
                    if (cardCount == 3){
                        tempX += 16.0;
                        tempY = y - 20.0;
                    }

                    this->Image_newCardmark[p]->Render(tempX  , tempY);
                    tempY -= 15.0;
                    cardCount++;
                }
            }
            p++;
        }
    }
}

void CGameRes::RenderUIDefendArmy(const char *CountryName, float x, float y, int ArmyType, int HP, int MaxHP, int Movement, int Cards, int Level, int CommonType, bool AI) {
    ecImage *ArmyImage = this->GetArmyImage(CountryName, ArmyType, false);
    if (ArmyImage != NULL) {
        ArmyImage->SetColor(0xFFFFFFFF, -1);
        ArmyImage->RenderEx(x, y, 0.0, -1.0, 1.0);
    }
    this->Image_hpbar->Render(x - 30.0, y - 12.0);
    this->Image_hpbar_fill->SetColor(HpColor(HP, MaxHP), -1);
    this->Image_hpbar_fill->RenderEx(x - 11.0, y - 4.0, 0.0, (float) HP * 33.0 / (float) MaxHP, 1.0);
    sprintf(TempStr, "%d", Movement);

    if (Movement > 0) {
        ArmyMovementText->SetText(TempStr);
        ArmyMovementText->DrawText(x - 16.0f, y - 9.0f, 1);
    } else {
        this->Image_movementmark[0]->Render(x - 23.0f, y - 7.0f);
    }
    if (Cards & (1 << CArmy::Commander)) {
        if (AI)
            this->RenderAICommanderMedal(1, x, y + 10.0, CountryName, CommonType);
        else
            this->Image_commander_level[Level / 3]->Render(x + 8.0, y - 8.0);
    } else if (Level > 0 && Level <= 4)
        this->Image_unitlevelmark[Level - 1]->Render(x + 8.0, y - 8.0);
    if (Cards != 0) {
        for (int i = 0; i < 10; i++) {
            if ((Cards >> i) & 1) {
                this->Image_newCardmark[i]->Render(x - 50.0, y - 20.0);
                y -= 15.0;
            }
        }
    }
}
//disallow to sort other countries' army
//allow to use card on select army side bar
bool GUISelArmy::OnEvent(const Event &event) {
    if (event.type == Event::Touch) {
        if (event.info.touch.type == Event::info::Touch::begin &&
            this->CheckInRect(event.info.touch.pos[0], event.info.touch.pos[1]))
            return true;
    } else if (event.type == Event::GUI) {
        if (event.info.GUI.type == Event::info::GUI::Button &&
            g_Scene[this->AreaID]->Country == g_GameManager) {
            int i;
            for (i = 0; i < 4; i++) {
                if (event.info.GUI.ptr == this->ArmyItem[i]) {
                    CGameState *game = static_cast<CGameState *>(CStateManager::Instance()->GetStatePtr(
                            Game));
                    if ((game->BuyCardGUI->GetSelCard() != NULL) &&
                        game->BuyCardGUI->CardCanTarget && game->BuyCardGUI->CardTargetArmy)
                        this->TargetArmy(i);
                    else
                        this->MoveArmyToFront(i);
                    break;
                }
            }
        }
    }
    return this->GUIElement::OnEvent(event);
}





void GUIArmyItem::OnRender() {
    GUIRect rect;
    this->GetAbsRect(rect);
    if (this->Army == NULL)
        return;
    g_GameRes.RenderUIArmy(this->Army->Country->Name, rect.Pos[0] + rect.Size[0] * 0.5,
                           rect.Pos[1] + rect.Size[1], this->Army->BasicAbilities->ID,
                           this->Army->Hp, this->Army->GetMaxStrength(), this->Army->Movement,
                           this->Army->Cards, (this->Army->Cards & (1 << CArmy::Commander))
                                              ? this->Army->Country->GetCommanderLevel()
                                              : this->Army->Level, this->Army->Country->Alliance,
                           this->Army->Country->AI);
}
