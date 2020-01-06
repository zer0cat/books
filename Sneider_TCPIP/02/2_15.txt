————————————————————————————————————————————————————————————vrc.c
 1 #include "etcp.h"
 2 int main( int argc, char **argv )
 3 {
 4		SOCKET s;
 5		int n;
 6		struct
 7		{	
 8			u_int32_t reclen;
 9		char buf[ 128 ];
10		} packet;
11		INIT();
12		s = tcp_client( argv[ 1 ], argv[ 2 ] );
13		while ( fgets( packet.buf, sizeof( packet.buf ), stdin )
14			!= NULL )
15		{
16			n = strlen( packet.buf );
17			packet.reclen = htonl( n );
18			if ( send( s, ( char * )&packet,
19				 n + sizeof( packet.reclen ), 0 ) < 0 )
20				error( 1, errno, "îøèáêà âûçîâà send" );
21		}
22		EXIT( 0 );
23 }
————————————————————————————————————————————————————————————vrc.c