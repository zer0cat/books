————————————————————————————————————————————————————————simples.c
1 #include <sys/types.h>
2 #include <sys/socket.h>
3 #include <netinet/in.h>
4 #include <stdio.h>
5 int main( void )
6 {
7	struct sockaddr_in local;
8	int s;
9	int s1;
10	int rc;
11	char buf[ 1 ];
12	local.sin_family = AF_INET;
13	local.sin_port = htons( 7500 );
14	local.sin_addr.s_addr = htonl( INADDR_ANY );
15	s = socket( AF_INET, SOCK_STREAM, 0 );
16	if ( s < 0 )
17	{
18		perror( "îøèáêà âûçîâà socket" );
19		exit( 1 );
20	}
21	rc = bind( s, ( struct sockaddr * )&local, sizeof( local ) );
22	if ( rc < 0 )
23	{
24		perror( "îøèáêà âûçîâà bind" );
25		exit( 1 );
26	}
27	rc = listen( s, 5 );
28	if ( rc )
29	{
30		perror( "îøèáêà âûçîâà listen" );
31		exit( 1 );
32	}
33	s1 = accept( s, NULL, NULL );
34	if ( s1 < 0 )
35	{
36		perror( "îøèáêà âûçîâà accept" );
37		exit( 1 );
38	}
39	rc = recv( s1, buf, 1, 0 );
40	if ( rc <= 0 )
41	{
42		perror( "îøèáêà âûçîâà recv" );
43		exit( 1 );
44	}
45	printf( "%c\n", buf[ 0 ] );
46	rc = send( s1, "2", 1, 0 );
47	if ( rc <= 0 )
48		perror( "îøèáêà âûçîâà send" );
49	exit( 0 );
50 }
————————————————————————————————————————————————————————simples.c