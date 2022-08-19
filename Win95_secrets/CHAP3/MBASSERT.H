//==================================
// WIN32WLK - Matt Pietrek 1995
// FILE: MBASSERT.H
//==================================
#ifdef NDEBUG
#define MBassert(exp)((void)0)
#else
#ifdef __cplusplus
extern "C" {
#endif
void __cdecl _MBassert(void *, void *, unsigned);
#ifdef __cplusplus
}
#endif
#define MBassert(exp)(void)( (exp) || (_MBassert(#exp, __FILE__, __LINE__), 0) )
#endif  /* NDEBUG */

#define IsSelector( value ) ((((DWORD)value & 0x7) == 0x7) ? 1 : 0)
#define Is16BitGlobalHandle( value ) ((((DWORD)value & 0x6) == 0x6) ? 1 : 0)
#define IsDivisibleBy4( value ) ((((DWORD)value & 0x3) == 0) ? 1 : 0)
#define IsK32HeapHandle( value ) (   IsDivisibleBy4( value ) \
                                     && ((DWORD)value > 0x80000000) \
                                     && ((DWORD)value < 0xC0000000) )
#define IsRing0HeapHandle( value ) ( IsDivisibleBy4( value ) \
                                     && ((DWORD)value > 0xC0000000) )
#define IsHeapStart( value ) ( value && !( (DWORD)value & 0x00000FFF) )
