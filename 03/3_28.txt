————————————————————————————————————————————————————————————smb.c
 1 #include "etcp.h"
 2 #define FREE_LIST		smbarray[ NSMB ].nexti
 3 typedef union
 4 {
 5		int nexti;
 6		char buf[ SMBUFSZ ];
 7 } smb_t;
 8 smb_t *smbarray;
 9 void *smballoc( void )
10 {
11		smb_t *bp;
12		lock_buf();
13		if ( FREE_LIST < 0 )
14		error( 1, 0, "áîëüøå íåò áóôåðîâ â ðàçäåëÿåìîé ïàìÿòè\n" );
15		bp = smbarray + FREE_LIST;
16		FREE_LIST  = bp->nexti;
17		unlock_buf();
18		return bp;
19 }
20 void smbfree( void *b )
21 {
22		smb_t *bp;
23		bp = b;
24		lock_buf();
25		bp->nexti = FREE_LIST;
26		FREE_LIST  = bp - smbarray;
27		unlock_buf();
28 }
————————————————————————————————————————————————————————————smb.c
