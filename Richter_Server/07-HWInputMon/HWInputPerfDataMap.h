/******************************************************************************
Module:  HWInputPerfDataMap.h
Notices: Copyright (c) 2000 Jeffrey Richter
Description: Definition of performance objects and counters
******************************************************************************/


#ifdef HWINPUTPERFDATAMAP_IMPL
#define PERFDATA_IMPL
#endif
#include "PerfData.h"


///////////////////////////////////////////////////////////////////////////////


PERFDATA_DEFINE_OBJECT(PERFOBJ_HWINPUT,           100);
PERFDATA_DEFINE_COUNTER(HWINPUT_KEYS,             101);
PERFDATA_DEFINE_COUNTER(HWINPUT_KEYSPERSEC,       102);
PERFDATA_DEFINE_COUNTER(HWINPUT_MOUSEMOVES,       103);
PERFDATA_DEFINE_COUNTER(HWINPUT_MOUSEMOVESPERSEC, 104);

PERFDATA_DEFINE_OBJECT(PERFOBJ_MOUSECLCKS,        200);
PERFDATA_DEFINE_COUNTER(MOUSECLCKS_CLICKS,        201);
PERFDATA_DEFINE_COUNTER(MOUSECLCKS_CLICKSPERSEC,  202);


///////////////////////////////////////////////////////////////////////////////


#ifdef HWINPUTPERFDATAMAP_IMPL


///////////////////////////////////////////////////////////////////////////////


PERFDATA_MAP_BEGIN()
   PERFDATA_MAP_OBJ(PERFOBJ_HWINPUT, TEXT("Hardware Input"), 
      TEXT("The Hardware Input object type includes those counters ")
      TEXT("that apply to keystrokes and mouse moves."), 
      PERF_DETAIL_NOVICE, HWINPUT_KEYS, PERF_NO_INSTANCES, 0)

   PERFDATA_MAP_CTR(HWINPUT_KEYS,  TEXT("Keystrokes"),  
      TEXT("The number of down and up keystrokes"),  
      PERF_DETAIL_NOVICE, 0, PERF_COUNTER_RAWCOUNT)
   PERFDATA_MAP_CTR(HWINPUT_KEYSPERSEC,  TEXT("Keystrokes/sec"),  
      TEXT("The number of down and up keystrokes per second"),  
      PERF_DETAIL_NOVICE, 0, PERF_COUNTER_COUNTER)
   PERFDATA_MAP_CTR(HWINPUT_MOUSEMOVES,  TEXT("Mouse moves"),  
      TEXT("The number of mouse moves"),  
      PERF_DETAIL_NOVICE, 0, PERF_COUNTER_RAWCOUNT)
   PERFDATA_MAP_CTR(HWINPUT_MOUSEMOVESPERSEC,  TEXT("Mouse moves/sec"),  
      TEXT("The number of mouse moves per second"),  
      PERF_DETAIL_NOVICE, 0, PERF_COUNTER_COUNTER)


   PERFDATA_MAP_OBJ(PERFOBJ_MOUSECLCKS, TEXT("Mouse Clicks"), 
      TEXT("The Mouse Clicks object type includes those counters ")
      TEXT("that apply to mouse button clicks."), 
      PERF_DETAIL_NOVICE, MOUSECLCKS_CLICKS, 4, 10)

   PERFDATA_MAP_CTR(MOUSECLCKS_CLICKS,  TEXT("Clicks"),  
      TEXT("The number of down clicks"),  
      PERF_DETAIL_NOVICE, 0, PERF_COUNTER_RAWCOUNT)
   PERFDATA_MAP_CTR(MOUSECLCKS_CLICKSPERSEC,  TEXT("Clicks/sec"),  
      TEXT("The number of down clicks per second"),  
      PERF_DETAIL_NOVICE, 0, PERF_COUNTER_COUNTER)
PERFDATA_MAP_END("HWInputMon")


///////////////////////////////////////////////////////////////////////////////


#endif   // HWINPUTPERFDATAMAP_IMPL


///////////////////////////////// End Of File /////////////////////////////////
