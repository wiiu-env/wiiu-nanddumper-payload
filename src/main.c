#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <malloc.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include "dynamic_libs/os_functions.h"
#include "dynamic_libs/fs_functions.h"
#include "dynamic_libs/gx2_functions.h"
#include "dynamic_libs/sys_functions.h"
#include "dynamic_libs/vpad_functions.h"
#include "dynamic_libs/socket_functions.h"
#include "dynamic_libs/proc_ui_functions.h"
#include "fs/fs_utils.h"
#include "fs/sd_fat_devoptab.h"
#include "system/memory.h"
#include "utils/logger.h"
#include "utils/utils.h"
#include "common/common.h"
#include "menu.h"
#include "main.h"
#include "ios_exploit.h"

static int exitToHBLOnLaunch = 0;

bool CheckRunning() {
    switch (ProcUIProcessMessages(true)) {
        case PROCUI_STATUS_EXITING: {
            return false;
        }
        case PROCUI_STATUS_RELEASE_FOREGROUND: {
            ProcUIDrawDoneRelease();
            break;
        }
        case PROCUI_STATUS_IN_FOREGROUND: {
            break;
        }
        case PROCUI_STATUS_IN_BACKGROUND:
        default:
            break;
    }
    return true;
}

int Menu_Main(int argc, char ** argv){
    //!---------INIT---------
    InitOSFunctionPointers();
    InitSysFunctionPointers();
    InitFSFunctionPointers();
    InitSocketFunctionPointers();
    InitVPadFunctionPointers();
    InitProcUIFunctionPointers();

    cfw_config_t config;
    default_config(&config);

    //int launch = 1;
    ShowMenu(&config);

    ExecuteIOSExploit(&config);

    if (
        OSGetTitleID() == 0x000500101004A200L || // mii maker eur
        OSGetTitleID() == 0x000500101004A100L || // mii maker usa
        OSGetTitleID() == 0x000500101004A000L) {   // mii maker jpn

        // restart mii maker.
        OSForceFullRelaunch();
        SYSLaunchMenu();
          return ( (int (*)(int, char **))(*(unsigned int*)0x1005E040) )(argc, argv);
    }
    
    ProcUIInit(OSSavesDone_ReadyToRelease);
    
    OSForceFullRelaunch();
    SYSLaunchMenu();

    while (CheckRunning()) {
        log_printf_("check");
        // wait.
        OSSleepTicks(MILLISECS_TO_TICKS(100));
    }
    ProcUIShutdown();
    log_printf_("exit");

    return 0;
}
