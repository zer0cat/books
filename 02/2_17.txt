————————————————————————————————————————————————————————udpsink.c
 1 #include "etcp.h"
 2 int main( int argc, char **argv )
 3 {
 4		SOCKET s;
 5		int rc;
 6		int datagrams = 0;
 7		int rcvbufsz = 5000 * 1440;
 8		char buf[ 1440 ];
 9		INIT();
10		s = udp_server( NULL, "9000" );
11		setsockopt( s, SOL_SOCKET, SO_RCVBUF,
12			( char * )&rcvbufsz, sizeof( int ) );
13		for( ;; )
14		{
15			rc = recv( s, buf, sizeof( buf ), 0 );
16			if ( rc <= 0 )
17				break;
18			datagrams++;
19		}
20		error( 0, 0, "ïîëó÷åíî äàòàãðàìì: %d \n", datagrams );
21		EXIT( 0 );
22 }
————————————————————————————————————————————————————————udpsink.c