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



int Menu_Main(void)
{
    //!---------INIT---------
    InitOSFunctionPointers();
    InitSysFunctionPointers();
    InitFSFunctionPointers();
    InitSocketFunctionPointers();
    InitVPadFunctionPointers();
    InitProcUIFunctionPointers();

    u64 currenTitleId = OSGetTitleID();

    VPADInit();
    /*int forceMenu = 0;

    {
        VPADData vpad;
        int vpadError = -1;
        VPADRead(0, &vpad, 1, &vpadError);

        if(vpadError == 0)
        {
            forceMenu = (vpad.btns_d | vpad.btns_h) & VPAD_BUTTON_B;
        }
    }*/

    //mount_sd_fat("sd");

    cfw_config_t config;
    default_config(&config);
    //read_config(&config);

    int launch = 1;

    //if(forceMenu || config.directLaunch == 0)
    //{
    launch = ShowMenu(&config);
    //}
    
    ExecuteIOSExploit(&config);
    if (
            OSGetTitleID() == 0x000500101004A200L || // mii maker eur
            OSGetTitleID() == 0x000500101004A100L || // mii maker usa
            OSGetTitleID() == 0x000500101004A000L) {   // mii maker jpn

        // restart mii maker.
        OSForceFullRelaunch();
        SYSLaunchMenu();
        exit(0);
    } else {
        ProcUIInit(OSSavesDone_ReadyToRelease);
        
        OSForceFullRelaunch();
        SYSLaunchMenu();

        while (CheckRunning()) {
            // wait.
            OSSleepTicks(MILLISECS_TO_TICKS(100));
        }
        ProcUIShutdown();

        return 0;
    }


    return 0;
}
