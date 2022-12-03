//
// Created by KK on 2022/10/7.
//
#include "event.h"
void Init_resolutions_de_conquest1() {
    CCountry *country = g_GameManager.FindCountry("de");
    //如果找到这个国家，设置它的事件
    if (country != nullptr) {
        ResolutionDe *resolutionDe_0001 = new ResolutionDe();
        resolutionDe_0001->Set_condition(0, 3);
        resolutionDe_0001->Set_countryText(1, CommonEvent::resolution, "de", "de");
        resolutionDe_0001->Set_btnText(1,"hehe");
        resolutionDe_0001->Set_resolutionText("笑而不语","这是内容","没结果","注意!");
        country->List_resolution.push_back(resolutionDe_0001);

        ResolutionDe *resolutionDe_0002 = new ResolutionDe();
        resolutionDe_0002->Set_condition(0, 3);
        resolutionDe_0002->Set_countryText(2, CommonEvent::resolution, "de", "de");
        resolutionDe_0002->Set_btnText(2,"haaaaaehe","heeh");
        resolutionDe_0002->Set_resolutionText("2222","22222","没结果","注意!");
        country->List_resolution.push_back(resolutionDe_0002);

        ResolutionDe *resolutionDe_0003 = new ResolutionDe();
        resolutionDe_0003->Set_condition(0, 3);
        resolutionDe_0003->Set_countryText(3, CommonEvent::resolution, "de", "de");
        resolutionDe_0003->Set_btnText(3,"haaaaaehe","heeh","3333333");
        resolutionDe_0003->Set_resolutionText("2222","22222","没结果","注意!");
        country->List_resolution.push_back(resolutionDe_0003);

        ResolutionDe *resolutionDe_0004 = new ResolutionDe();
        resolutionDe_0004->Set_condition(0, 3);
        resolutionDe_0004->Set_countryText(4, CommonEvent::resolution, "de", "de");
        resolutionDe_0004->Set_btnText(1,"haaaaaehe","heeh");
        resolutionDe_0004->Set_resolutionText("2222","22222","没结果","注意!");
        country->List_resolution.push_back(resolutionDe_0004);
    }
}
//所有决议通过这个唯一的接口触发
void ResolutionDe::Trigger_resolution(int eventNum, int btnNum, CCountry* theCounry) {
    this->theCountry = theCounry;
    //每个case表示一个决议号:
    //如果要在case的作用域内写新的变量,那么就要给它加大括号
    switch (eventNum) {
        case 1:
            NothingHappened();
            break;
        case 2:
            Trigger_0002_2btn_resolution(btnNum);
            break;
        case 3:
            Trigger_0002_2btn_resolution(btnNum);
            break;
        case 4:
            Trigger_0002_2btn_resolution(btnNum);
            break;
        default:
            break;
    }
}

void ResolutionDe::Trigger_0002_2btn_resolution(int btnNUm) {
    if (btnNUm == 1){
        g_commonEvent.Add_resources_toTheCountry(this->theCountry, 100, 100, 100);
    }
    else
        NothingHappened();
}
