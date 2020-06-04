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
int sceAppMgrIsGameProgram(int);

int hook[2];
static tai_hook_ref_t screenshot_hook;
static tai_hook_ref_t screen_disable_hook;
const static int done_magic = DONE_MAGIC;
static int frames = 0;

void sceScreenShotDisable_patched() {
	printf("INFO TrophyShot: Prevented disabling screenshot. \n");
}

int sceDisplaySetFrameBuf_patched(const SceDisplayFrameBuf* pParam, SceDisplaySetBufSync sync) {

	int read_data, screenshot_ret;

	if (frames == 5) {

		frames = 0;
		sceShellUtilTextClipboardRead(&read_data, sizeof(int), NULL);

		if (read_data == TROPHY_MAGIC) {

			printf("SHOOTING\n");

			SceScreenShotCaptureFileInfo captureFileInfo;
			sceClibMemset(&captureFileInfo, 0, sizeof(captureFileInfo));
			screenshot_ret = sceScreenShotCapture(1, &captureFileInfo, NULL, NULL);

			printf("INFO TrophyShot: sceScreenShotCapture() : 0x%08x   path='%s'\n", screenshot_ret, captureFileInfo.path);

			sceShellUtilTextClipboardWrite(&done_magic, sizeof(int));
		}
	}

	frames++;

	return TAI_CONTINUE(int, screenshot_hook, pParam, sync);
}

void _start() __attribute__((weak, alias("module_start")));
int module_start(SceSize args, void *argp) {
	printf("TrophyShot APP PART 2.0 SALVO FIRE\n");

	int ret = sceAppMgrIsGameProgram(-2);
	printf("APP TYPE: %d\n", ret);

	if (ret == 1) {
		sceSysmoduleLoadModule(SCE_SYSMODULE_SCREEN_SHOT);
		hook[1] = taiHookFunctionExport(
			&screen_disable_hook,
			"SceScreenShot",
			0xF26FC97D, //SceScreenShot
			0x50AE9FF9, //SceScreenShotDisable
			sceScreenShotDisable_patched);

		hook[0] = taiHookFunctionImport(
			&screenshot_hook,
			TAI_MAIN_MODULE,
			TAI_ANY_LIBRARY, //SceDisplayUser
			0x7A410B64, //sceDisplaySetFrameBuf
			sceDisplaySetFrameBuf_patched);

		printf("hooks: 0x%x 0x%x\n", hook[0], hook[1]);
	}

	return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize args, void *argp) {
	if (hook[0] >= 0) taiHookRelease(hook[0], screenshot_hook);
	if (hook[1] >= 0) taiHookRelease(hook[1], screen_disable_hook);
	return SCE_KERNEL_STOP_SUCCESS;
}