/****************************************************************************
 * Copyright (C) 2015
 * by Dimok
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 ***************************************************************************/
#ifndef __PROC_UI_FUNCTIONS_H_
#define __PROC_UI_FUNCTIONS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "os_types.h"

extern u32 proc_ui_handle;

typedef u32 (*ProcUICallback)(void*);
typedef void (*ProcUISaveCallback)(void);

typedef enum ProcUIStatus
{
   //! The application is in the foreground. All resources may be used.
   PROCUI_STATUS_IN_FOREGROUND,
   //! The application is in the background, only limited resources are usable.
   PROCUI_STATUS_IN_BACKGROUND,
   //! The application must release the foregound - see ProcUIDrawDoneRelease()
   PROCUI_STATUS_RELEASE_FOREGROUND,
   //! The application must release all resources (including ProcUI) and quit
   PROCUI_STATUS_EXITING,
} ProcUIStatus;

void InitProcUIFunctionPointers(void);
void InitAcquireProcUI(void);

extern void (*ProcUIInit)(ProcUISaveCallback saveCallback);
extern void (*ProcUIDrawDoneRelease)();
extern void (*ProcUIShutdown)();

extern ProcUIStatus (*ProcUIProcessMessages)(BOOL block);

extern u32 (*ProcUIInForeground)(void);
extern void (*ProcUIRegisterCallback)(u32 type,ProcUICallback callback,void* param, u32 unkwn);

#ifdef __cplusplus
}
#endif

#endif // __PROC_UI_FUNCTIONS_H_
