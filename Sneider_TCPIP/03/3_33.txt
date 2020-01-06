———————————————————————————————————————————————————————————smbs.c
 1 #include "etcp.h"
 2 int main( int argc, char **argv )
 3 {
 4		char *bp;
 5		SOCKET s;
 6		SOCKET s1;
 7		INIT();
 8		init_smb( TRUE );
 9		s = tcp_server( NULL, argv[ 1 ] );
10		s1 = accept( s, NULL, NULL );
11		if ( !isvalidsock( s1 ) )
12			error( 1, errno, "îøèáêà âûçîâà accept" );
13		for ( ;; )
14		{
15			bp = smbrecv( s1 );
16			fputs( bp, stdout );
17			smbfree( bp );
18		}
19		EXIT( 0 );
20 }
———————————————————————————————————————————————————————————smbs.c