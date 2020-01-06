————————————————————————————————————————————————————————simplec.c
1 #include <sys/types.h>
2 #include <sys/socket.h>
3 #include <netinet/in.h>
4 #include <arpa/inet.h>
5 #include <stdio.h>
6 int main( void )
7 {
8	struct sockaddr_in peer;
9	int s;
10	int rc;
11	char buf[ 1 ];
12	peer.sin_family = AF_INET;
13	peer.sin_port = htons( 7500 );
14	peer.sin_addr.s_addr = inet_addr( "127.0.0.1" );
15	s = socket( AF_INET, SOCK_STREAM, 0 );
16	if ( s < 0 )
17	{
18		perror( "îøèáêà âûçîâà socket" );
19		exit( 1 );
20	}
21	rc = connect( s, ( struct sockaddr * )&peer, sizeof( peer ) );
22	if ( rc )
23	{
24		perror( "îøèáêà âûçîâà connect" );
25		exit( 1 );
26	}
27	rc = send( s, "1", 1, 0 );
28	if ( rc <= 0 )
29	{
30		perror( "îøèáêà âûçîâà send" );
31		exit( 1 );
32	}
33	rc = recv( s, buf, 1, 0 );
34	if ( rc <= 0 )
35		perror( "îøèáêà âûçîâà recv" );
36	else
37		printf( "%c\n", buf[ 0 ] );
38	exit( 0 );
39 }
————————————————————————————————————————————————————————simplec.c