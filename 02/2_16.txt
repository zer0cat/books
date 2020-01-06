——————————————————————————————————————————————————————udpsource.c
 1 #include "etcp.h"
 2 int main( int argc, char **argv )
 3 {
 4		struct sockaddr_in peer;
 5		SOCKET s;
 6		int rc;
 7		int datagrams;
 8		int dgramsz = 1440;
 9		char buf[ 1440 ];
10		INIT();
11		datagrams = atoi( argv[ 2 ] );
12		if ( argc > 3 )
13			dgramsz = atoi( argv[ 3 ] );
14		s = udp_client( argv[ 1 ], "9000", &peer );
15		while ( datagrams-- > 0 )
16		{
17			rc = sendto( s, buf, dgramsz, 0,
18				( struct sockaddr * )&peer, sizeof( peer ) );
19			if ( rc <= 0 )
20				error( 0, errno, "îøèáêà âûçîâà sendto" );
21		}
22		sendto( s, "", 0, 0,
23			( struct sockaddr * )&peer, sizeof( peer ) );
24		EXIT( 0 );
25 }
——————————————————————————————————————————————————————udpsource.c