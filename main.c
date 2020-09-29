#include <taihen.h>
#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/clib.h>
#include <psp2/kernel/iofilemgr.h> 

#define printf sceClibPrintf

int hook[1];
static tai_hook_ref_t shell_hook;
static uint8_t platinum_only = 1;

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
	switch(arg2 - arg1){
		case 0xDC4:
			printf("Regular trophy unlocked.\n");

			if(platinum_only){
				sceKernelDelayThread(3200000);
			}else{
				sceKernelDelayThread(1800000);
				shellShot();
			}
			break;

		case 0x930:
			printf("Platinum trophy unlocked.\n");

			sceKernelDelayThread(1800000);
			shellShot();
			break;

		default:
			break;
	}
	return ret;
}

void _start() __attribute__((weak, alias("module_start")));
int module_start(SceSize args, void *argp) {
	SceUID fd = sceIoOpen("ux0:/data/TrophyShot/platinum.txt", SCE_O_RDONLY, 0777);
	if(fd < 0){
		platinum_only = 0;
	}
	sceIoClose(fd);

	tai_module_info_t tai_info;
	int offset_shellshot, offset_dialog;
	tai_info.size = sizeof(tai_module_info_t);
	int ret = taiGetModuleInfo("SceShell", &tai_info);
	if (ret < 0) return SCE_KERNEL_START_SUCCESS;

	switch(tai_info.module_nid){
		case 0x0552F692: // 3.60 retail
			offset_shellshot = 0x14a928;
			offset_dialog = 0x229fac;
			break;
		case 0x6CB01295: // 3.60 PDEL
		case 0xEAB89D5C: // 3.60 PTEL
		case 0x5549BF1F: // 3.65 retail
			offset_shellshot = 0x14a980;
			offset_dialog = 0x22a048;
			break;
	  	case 0x34B4D82E: // 3.67 retail
	  	case 0x12DAC0F3: // 3.68 retail
	  	case 0x0703C828: // 3.69 retail
	  	case 0x2053B5A5: // 3.70 retail
	  	case 0xF476E785: // 3.71 retail
	  	case 0x939FFBE9: // 3.72 retail
	  	case 0x734D476A: // 3.73 retail
		case 0xE6A02F2B: // 3.65 PDEL
		case 0x587F9CED: // 3.65 PTEL
			offset_shellshot = 0x142db4;
			offset_dialog = 0x22247c;
			break;
		default:
			return SCE_KERNEL_START_SUCCESS;
	}
	//function found by teakhanirons
	module_get_offset(
		tai_info.modid,
		0,
		offset_shellshot | 1,
		&shellShot
	);
	//function found by Graphene
	hook[0] = taiHookFunctionOffset(
		&shell_hook,
		tai_info.modid,
		0,
		offset_dialog,
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