—————————————————————————————————————————————————————telemetryc.c
 1 	#include "etcp.h"
 2 	int main( int argc, char **argv )
 3 	{
 4		SOCKET s;
 5		int rc;
 6		int i;
 7		int pkt[ 3 ];
 8		INIT();
 9		s = tcp_client( argv[ 1 ], argv[ 2 ] );
10		for ( i = 2;; i = 5 - i )
11		{
12			pkt[ 0 ] = htonl( i );
13			rc = send( s, ( char * )pkt, i * sizeof( int ), 0 );
14			if ( rc < 0 )
15				error( 1, errno, "îøèáêà âûçîâà send" );
16			sleep( 1 );
17		}
18 	}
—————————————————————————————————————————————————————telemetryc.c