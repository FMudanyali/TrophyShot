#include <taihen.h>
#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/sysmodule.h>
#include <psp2/kernel/clib.h>
#include <psp2/screenshot.h>
#include <dolcesdk.h>
#include <stdio.h>
#include <string.h>

#define printf sceClibPrintf
#define TROPHY_MAGIC 12345678
#define DONE_MAGIC 87654321

int sceShellUtilTextClipboardRead(void* data, SceSize size, SceSize *textlen);
int sceShellUtilTextClipboardWrite(const void* data, SceSize size);

int hook[1];
static tai_hook_ref_t shell_hook;
const static int trophy_magic = TROPHY_MAGIC;

int sub_81229FAC_patched(int arg1, int arg2) {
	int waiter = 0;
	printf("I'M HOOKINGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG\n");
	int ret = TAI_CONTINUE(int, shell_hook, arg1, arg2);

	sceKernelDelayThread(1300000);
	printf("INFO TrophyShot: Trophy unlocked.\n");
	sceShellUtilTextClipboardWrite(&trophy_magic, sizeof(int));

	while (waiter != DONE_MAGIC) {
		sceShellUtilTextClipboardRead(&waiter, sizeof(int), NULL);
		sceKernelDelayThread(1000);
	}

	return ret;
}

void _start() __attribute__((weak, alias("module_start")));
int module_start(SceSize args, void *argp) {
	printf("TrophyShot 2.0 SALVO FIRE\n");

	tai_module_info_t tai_info;
	tai_info.size = sizeof(tai_module_info_t);
	int ret = taiGetModuleInfo("SceShell", &tai_info);
	if (ret < 0) return SCE_KERNEL_START_FAILED;

	int offset;
	switch (tai_info.module_nid) { // trophy notification
		case 0x0552F692: // retail 3.60 SceShell
		case 0x532155E5: // retail 3.61 SceShell
			offset = 0x229fac;
			break;
		case 0x5549BF1F: // retail 3.65 SceShell
		case 0x34B4D82E: // retail 3.67 SceShell
		case 0x12DAC0F3: // retail 3.68 SceShell
		case 0x0703C828: // retail 3.69 SceShell
		case 0x2053B5A5: // retail 3.70 SceShell
		case 0xF476E785: // retail 3.71 SceShell
		case 0x939FFBE9: // retail 3.72 SceShell
		case 0x734D476A: // retail 3.73 SceShell
		    offset = 0x22a048;
            break;
		default:
		return SCE_KERNEL_START_SUCCESS;
	}
	hook[0] = taiHookFunctionOffset(
		&shell_hook,
		tai_info.modid,
		0,
		offset,
		1,
		sub_81229FAC_patched);

	printf("hook: 0x%x\n", hook[0]);
	return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize args, void *argp) {
	if (hook[0] >= 0) taiHookRelease(hook[0], shell_hook);
	return SCE_KERNEL_STOP_SUCCESS;
}