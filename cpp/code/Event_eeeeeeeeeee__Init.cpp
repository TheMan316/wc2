//
// Created by KK on 2022/10/6.
//
#include "event.h"
void Init_events_common_conquer1() {

}
void Init_events_de_conquer1() {
    CCountry *country = g_GameManager.FindCountry("de");
    //如果找到这个国家，设置它的事件
    if (country != nullptr) {
        EventTurnBegin *event_0001 = new EventTurnBegin();
        event_0001->Set_condition(2, 3);
        event_0001->Set_countryText(1, CommonEvent::turnBegin, "de", "de");
        country->List_event_turnBegin.push_back(event_0001);

    }
}

