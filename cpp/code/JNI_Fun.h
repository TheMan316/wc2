#ifndef WC2_JNI_FUN_H
#define WC2_JNI_FUN_H


#include <jni.h>
#include "CLi.h"
class JNI_Fun {
    //事件窗口的定义
public:

    static void Show_resolutionForm();
 ;
};

extern JavaVM *g_vm;
#endif //WC2_JNI_FUN_H
