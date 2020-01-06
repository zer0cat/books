———————————————————————————————————————————————————————udpecho1.c
 1 #include "etcp.h"
 2 int main( int argc, char **argv )
 3 {
 4		struct sockaddr_in peer;
 5		int rc;
 6		int len;
 7		int pidsz;
 8		char buf[ 120 ];
 9		pidsz = sprintf( buf, "%d: ", getpid() );
10		len = sizeof( peer );
11		rc = recvfrom( 0, buf + pidsz, sizeof( buf ) - pidsz, 0,
12			( struct sockaddr * )&peer, &len );
13		if ( rc <= 0 )
14			exit( 1 );
15		sendto( 1, buf, rc + pidsz, 0,
16			( struct sockaddr * )&peer, len );
17		exit( 0 );
18 }
—————————————————————————————————————————————————————————————udpecho1.c