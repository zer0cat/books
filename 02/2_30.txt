———————————————————————————————————————————————————————————keep.c
 1 	#include "etcp.h"
 2 	int main( int argc, char **argv )
 3 	{
 4		SOCKET s;
 5		SOCKET s1;
 6		int on = 1;
 7		int rc;
 8		char buf[ 128 ];
 9		INIT();
10		s = tcp_server( NULL, argv[ 1 ] );
11		s1 = accept( s, NULL, NULL );
12		if ( !isvalidsock( s1 ) )
13			error( 1, errno, "îøèáêà âûçîâà accept\n" );
14		if ( setsockopt( s1, SOL_SOCKET, SO_KEEPALIVE,
15			( char * )&on, sizeof( on ) ) )
16			error( 1, errno, "îøèáêà âûçîâà setsockopt" );
17		for ( ;; )
18		{
19			rc = readline( s1, buf, sizeof( buf ) );
20			if ( rc == 0 )
21				error( 1, 0, "äğóãîé êîíåö îòêëş÷èëñÿ\n" );
22			if ( rc < 0 )
23				error( 1, errno, "îøèáêà âûçîâà recv" );
24			write( 1, buf, rc );
25			}
26 	}
———————————————————————————————————————————————————————————keep.c