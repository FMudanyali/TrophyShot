#include <taihen.h>
#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/clib.h>
#include <psp2/sysmodule.h>

#define printf sceClibPrintf

int hook[1];
static tai_hook_ref_t shell_hook;

int (*shellShot)(void);

// Taken from sysident - taihen_min.c https://github.com/cuevavirus/sysident
int module_get_offset(SceUID modid, int segidx, uint32_t offset, void *stub_out){
	int res = 0;
	SceKernelModuleInfo info;

	if(segidx > 3) return -1;
	if(stub_out == NULL) return -2;

	res = sceKernelGetModuleInfo(modid, &info);
	if(res < 0) return res;
	if(offset > info.segments[segidx].memsz) return -3;

	*(uint32_t *)stub_out = (uint32_t)(info.segments[segidx].vaddr + offset);
	return 0;
}

int sub_81229FAC_patched(int arg1, int arg2){
	int ret = TAI_CONTINUE(int, shell_hook, arg1, arg2);
	sceKernelDelayThread(1800000);
	printf("INFO TrophyShot: Trophy unlocked.\n");
	shellShot();
    return ret;
}

void _start() __attribute__((weak, alias("module_start")));
int module_start(SceSize args, void *argp) {
    tai_module_info_t tai_info;
	tai_info.size = sizeof(tai_module_info_t);
	int ret = taiGetModuleInfo("SceShell", &tai_info);
	if (ret < 0) return SCE_KERNEL_START_SUCCESS;
	//function found by teakhanirons
	module_get_offset(
		tai_info.modid,
		0,
		0x14a928 | 1,
		&shellShot
	);
	//function found by Graphene
    hook[0] = taiHookFunctionOffset(
		&shell_hook,
		tai_info.modid,
		0,
		0x229fac,
		1,
		sub_81229FAC_patched
	);

    printf("INFO sub_81229FAC_patched: 0x%x\n", hook[0]);
	return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize args, void *argp) {
	if (hook[0] >= 0) taiHookRelease(hook[0], shell_hook);
	return SCE_KERNEL_STOP_SUCCESS;
}