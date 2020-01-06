———————————————————————————————————————————————————————————vrcv.c
 1 #include "etcp.h"
 2 #include <sys/uio.h>
 3 int main( int argc, char **argv )
 4 {
 5		SOCKET s;
 6		int n;
 7		char buf[ 128 ];
 8		struct iovec iov[ 2 ];
 9		INIT();
10		s = tcp_client( argv[ 1 ], argv[ 2 ] );
11		iov[ 0 ].iov_base = ( char * )&n;
12		iov[ 0 ].iov_len = sizeof( n );
13		iov[ 1 ].iov_base = buf;
14		while ( fgets( buf, sizeof( buf ), stdin ) != NULL )
15		{
16			iov[ 1 ].iov_len = strlen( buf );
17			n = htonl( iov[ 1 ].iov_len );
18			if ( writev( s, iov, 2 ) < 0 )
19				error( 1, errno, "îøèáêà âûçîâà writev" );
20		}
21		EXIT( 0 );
22 }
———————————————————————————————————————————————————————————vrcv.c
