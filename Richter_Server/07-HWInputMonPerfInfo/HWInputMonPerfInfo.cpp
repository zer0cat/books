/******************************************************************************
Module:  HWInputMonPerfInfo.cpp
Notices: Copyright (c) 2000 Jeffrey Richter
Description: DLL that exposes HWInputMon's Performance Information
******************************************************************************/


#include "..\CmnHdr.h"                 // See Appendix A.

#define PERFDATA_COLLECT_SUPPORTED

// NOTE: PERFDATA_COLLECT_SUPPORTED must be defined for this project
#if !defined(PERFDATA_COLLECT_SUPPORTED)
#error PERFDATA_COLLECT_SUPPORTED must be defined for this project
#endif

#define HWINPUTPERFDATAMAP_IMPL
#include "..\07-HWInputMon\HWInputPerfDataMap.h"


///////////////////////////////// End Of File /////////////////////////////////
