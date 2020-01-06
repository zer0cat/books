—————————————————————————————————————————————————————telemetrys.c
 1 	#include "etcp.h"
 2 	#define TWOINTS		( sizeof( int ) * 2 )
 3 	#define THREEINTS		( sizeof( int ) * 3 )
 4 	int main( int argc, char **argv )
 5 	{
 6		SOCKET s;
 7		SOCKET s1;
 8		int rc;
 9		int i = 1;
10		int pkt[ 3 ];
11		INIT();
12		s = tcp_server( NULL, argv[ 1 ] );
13		s1 = accept( s, NULL, NULL );
14		if ( !isvalidsock( s1 ) )
15			error( 1, errno, "ξψθακΰ βϋηξβΰ accept" );
16		for ( ;; )
17		{
18			rc = recv( s1, ( char * )pkt, sizeof( pkt ), 0 );
19			if ( rc != TWOINTS && rc != THREEINTS )
20				error( 1, 0, "recv βεπνσλΰ %d\n", rc );
21			printf( "Οΰκες %d ρξδεπζθς %d ηνΰχενθι β %d αΰιςΰυ\n",
22				i++, ntohl( pkt[ 0 ] ), rc );
23		}
24 }
—————————————————————————————————————————————————————telemetrys.c