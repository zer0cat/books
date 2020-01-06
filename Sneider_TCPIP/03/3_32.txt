———————————————————————————————————————————————————————————smbc.c
 1 #include "etcp.h"
 2 int main( int argc, char **argv )
 3 {
 4		char *bp;
 5		SOCKET s;
 6		INIT();
 7		s = tcp_client( argv[ 1 ], argv[ 2 ] );
 8		init_smb( FALSE );
 9		bp = smballoc();
10		while ( fgets( bp, SMBUFSZ, stdin ) != NULL  )
11		{
12			smbsend( s, bp );
13			bp = smballoc();
14	}
15	EXIT( 0 );
16 }
———————————————————————————————————————————————————————————smbc.c
