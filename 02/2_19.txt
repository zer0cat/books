————————————————————————————————————————————————————————tcpsink.c
 1 int main( int argc, char **argv )
 2 {
 3		struct sockaddr_in local;
 4		struct sockaddr_in peer;
 5		int peerlen;
 6		SOCKET s1;
 7		SOCKET s;
 8		int c;
 9		int rcvbufsz = 32 * 1024;
10		const int on = 1;
11		INIT();
12		opterr = 0;
13		while ( ( c = getopt( argc, argv, "b:" ) ) != EOF )
14		{
15			switch ( c )
16			{
17				case "b" :
18					rcvbufsz = atoi( optarg );
19					break;
20				case "?" :
21					error( 1, 0, "íåäîïóñòèìàÿ îïöèÿ: %c\n", c );
22			}
23		}
24		set_address( NULL, "9000", &local, "tcp" );
25		s = socket( AF_INET, SOCK_STREAM, 0 );
26		if ( !isvalidsock( s ) )
27			error( 1, errno, "îøèáêà âûçîâà socket" );
28		if ( setsockopt( s, SOL_SOCKET, SO_REUSEADDR,
29			( char * )&on, sizeof( on ) ) )
30			error( 1, errno, "îøèáêà âûçîâà setsockopt SO_REUSEADDR" );
31		if ( setsockopt( s, SOL_SOCKET, SO_RCVBUF,
32			( char * )&rcvbufsz, sizeof( rcvbufsz ) ) )
33			error( 1, errno, "îøèáêà âûçîâà setsockopt SO_RCVBUF" );
34		if ( bind( s, ( struct sockaddr * ) &local,
35			 sizeof( local ) ) )
36			error( 1, errno, "îøèáêà âûçîâà bind" );
37		listen( s, 5 );
38		do
39		{
40			peerlen = sizeof( peer );
41			s1 = accept( s, ( struct sockaddr * )&peer, &peerlen );
42			if ( !isvalidsock( s1 ) )
43				error( 1, errno, "îøèáêà âûçîâà accept" );
44			server( s1, rcvbufsz );
45			CLOSE( s1 );
46		} while ( 0 );
47		EXIT( 0 );
48 }
————————————————————————————————————————————————————————tcpsink.c