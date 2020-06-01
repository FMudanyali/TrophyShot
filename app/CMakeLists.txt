cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{DOLCESDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{DOLCESDK}/share/dolce.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define DOLCESDK to point to your SDK path!")
  endif()
endif()

project(trophyshot_app)
include("$ENV{DOLCESDK}/share/dolce.cmake" REQUIRED)

dolce_gen_libs(SceShellSvc_stubs
  SceShellSvc.yml
  LIB SceShellSvc_stub_weak)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -Wall -O3 -std=gnu99")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -nostdlib")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fno-rtti -fno-exceptions")

link_directories(
	${CMAKE_CURRENT_BINARY_DIR}
)

add_executable(${PROJECT_NAME}
	trophy_app.c
)

target_link_libraries(${PROJECT_NAME}
	SceLibKernel_stub
	SceKernelModulemgr_stub
	SceKernelThreadMgr_stub
	SceSysmem_stub
	SceScreenShot_stub_weak
	SceShellSvc_stub_weak
	SceSysmodule_stub
	SceShellSvc_stub
	ScePaf_stub_weak
	SceRtc_stub
	gcc
	k
	taihen_stub
	taihenModuleUtils_stub
)
dolce_create_self(${PROJECT_NAME}.suprx ${PROJECT_NAME}
  UNSAFE
  CONFIG ${CMAKE_SOURCE_DIR}/TrophyShot.yml
)

