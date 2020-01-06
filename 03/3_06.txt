———————————————————————————————————————————————————————udpecho2.c
 1 #include "etcp.h"
 2 int main( int argc, char **argv )
 3 {
 4		struct sockaddr_in peer;
 5		int s;
 6		int rc;
 7		int len;
 8		int pidsz;
 9		char buf[ 120 ];
10		pidsz = sprintf( buf, "%d: ", getpid() );
11		len = sizeof( peer );
12		rc = recvfrom( 0, buf + pidsz, sizeof( buf ) - pidsz,
13			0, ( struct sockaddr * )&peer, &len );
14		if ( rc < 0 )
15			exit( 1 );
16		s = socket( AF_INET, SOCK_DGRAM, 0 );
17		if ( s < 0 )
18			exit( 1 );
19		if ( connect( s, ( struct sockaddr * )&peer, len ) < 0 )
20			exit( 1 );
21		if ( fork() != 0 )	/* Îøèáêà èëè ðîäèòåëüñêèé ïðîöåññ? */
22			exit( 0 );
23		/* Ïîðîæäåííûé ïðîöåññ. */
24		while ( strncmp( buf + pidsz, "done", 4 ) != 0 )
25		{
26			if ( write( s, buf, rc + pidsz ) < 0 )
27				break;
28			pidsz = sprintf( buf, "%d: ", getpid() );
29			alarm( 30 );
30			rc = read( s, buf + pidsz, sizeof( buf ) - pidsz );
31			alarm( 0 );
32			if ( rc < 0 )
33				break;
34		}	
35		exit( 0 );
36 }
———————————————————————————————————————————————————————udpecho2.c