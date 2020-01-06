——————————————————————————————————————————————————————tcpsource.c
 1 int main( int argc, char **argv )
 2 {
 3		struct sockaddr_in peer;
 4		char *buf;
 5		SOCKET s;
 6		int c;
 7		int blks = 5000;
 8		int sndbufsz = 32 * 1024;
 9		int sndsz = 1440;	/* MSS для Ethernet по умолчанию. */
10		INIT();
11		opterr = 0;
12		while ( ( c = getopt( argc, argv, "s:b:c:" ) ) != EOF )
13		{
14			switch ( c )
15			{
16				case "s" :
17					sndsz = atoi( optarg );
18					break;
19				case "b" :
20					sndbufsz = atoi( optarg );
21					break;
22				case "c" :
23					blks = atoi( optarg );
24					break;
25				case "?" :
26					error( 1, 0, "некорректный параметр: %c\n", c );
27			}
28		}
28		if ( argc <= optind )
30			error( 1, 0, "не задано имя хоста\n" );
31		if ( ( buf = malloc( sndsz ) ) == NULL )
32			error( 1, 0, "ошибка вызова malloc\n" );
33		set_address( argv[ optind ], "9000", &peer, "tcp" );
34		s = socket( AF_INET, SOCK_STREAM, 0 );
35		if ( !isvalidsock( s ) )
36			error( 1, errno, "ошибка вызова socket" );
37		if ( setsockopt( s, SOL_SOCKET, SO_SNDBUF,
38			( char * )&sndbufsz, sizeof( sndbufsz ) ) )
39			error( 1, errno, "ошибка вызова setsockopt с опцией 					SO_SNDBUF" );
40		if ( connect( s, ( struct sockaddr * )&peer,
41			 sizeof( peer ) ) )
42			error( 1, errno, "ошибка вызова connect" );
43		while( blks–– > 0 )
44			send( s, buf, sndsz, 0 );
45		EXIT( 0 );
46 }
——————————————————————————————————————————————————————tcpsource.c