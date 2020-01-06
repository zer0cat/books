——————————————————————————————————————————————————————udpclient.c
 1 #include "etcp.h"
 2 int main( int argc, char **argv )
 3 {
 4		struct sockaddr_in peer;
 5		SOCKET s;
 6		int rc = 0;
 7		int len;
 8		char buf[ 120 ];
 9		INIT();
10		s = udp_client( argv[ 1 ], argv[ 2 ], &peer );
11		while ( fgets( buf, sizeof( buf ), stdin ) != NULL )
12		{
13			rc = sendto( s, buf, strlen( buf ), 0,
14				( struct sockaddr * )&peer, sizeof( peer ) );
15			if ( rc < 0 )
16				error( 1, errno, "îøèáêà âûçîâà sendto" );
17			len = sizeof( peer );
18			rc = recvfrom( s, buf, sizeof( buf ) - 1, 0,
19				( struct sockaddr * )&peer, &len );
20			if ( rc < 0 )
21				error( 1, errno, "îøèáêà âûçîâà recvfrom" );
22			buf[ rc ] = '\0';
23			fputs( buf, stdout );
24		}
25		EXIT( 0 );
26 }
——————————————————————————————————————————————————————udpclient.c