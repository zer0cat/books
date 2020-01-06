——————————————————————————————————————————————————————————vrcvw.c
 1 #include "etcp.h"
 2 int main( int argc, char **argv )
 3 {
 4		SOCKET s;
 5		int n;
 6		char buf[ 128 ];
 7		WSABUF wbuf[ 2 ];
 8		DWORD sent;
 9		INIT();
10		s = tcp_client( argv[ 1 ], argv[ 2 ] );
11		wbuf[ 0 ].buf = ( char * )&n;
12		wbuf[ 0 ].len = sizeof( n );
13		wbuf[ 1 ].buf = buf;
14		while ( fgets( buf, sizeof( buf ), stdin ) != NULL )
15		{
16			wbuf[ 1 ].len = strlen( buf );
17			n = htonl( wbuf[ 1 ].len );
18			if ( WSASend( s, wbuf, 2, &sent, 0, NULL, NULL ) < 0 )
19				error( 1, errno, "îøèáêà âûçîâà WSASend" );
20		}
21		EXIT( 0 );
22 }
——————————————————————————————————————————————————————————vrcvw.c
