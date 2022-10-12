#ifndef EASYTECH_CMENUSTATE_H
#define EASYTECH_CMENUSTATE_H

#include "../cxx.h"
#include "CStateManager.h"

#define OTHER_MAIL 20
#define OTHER_HOME 27
#define FADE_IN_AD 100

__sub2_struct(CMenuState, CBaseState, IEventReceiver)

#ifdef __cplusplus
    bool OnEvent(const Event &event);
#endif
__end_struct


__BEGIN_DECLS
    bool _ZN10CMenuState7OnEventERK5Event(struct CMenuState *self, struct Event const *event);
__END_DECLS
#endif //EASYTECH_CMENUSTATE_H
