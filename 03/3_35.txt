—————————————————————————————————————————————————————————endian.c
 1 #include <stdio.h>
 2 #include <sys/types.h>
 3 #include "etcp.h"
 4 int main( void )
 5 {
 6		u_int32_t x = 0x12345678;	/* 305419896 */
 7		unsigned char *xp = ( char * )&x;
 9		printf( "%0x %0x %0x %0x\n",
10			xp[ 0 ], xp[ 1 ], xp[ 2 ], xp[ 3 ] );
11		exit( 0 );
12 }
—————————————————————————————————————————————————————————endian.c
