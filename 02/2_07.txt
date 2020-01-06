—————————————————————————————————————————————————————tcp_client.c
 1 SOCKET tcp_client( char *hname, char *sname )
 2 {
 3		struct sockaddr_in peer;
 4		SOCKET s;
 5		set_address( hname, sname, &peer, "tcp" );
 6		s = socket( AF_INET, SOCK_STREAM, 0 );
 7		if ( !isvalidsock( s ) )
 8			error( 1, errno, "îøèáêà âûçîâà socket" );
 9		if ( connect( s, ( struct sockaddr * )&peer,
10			 sizeof( peer ) ) )
11			error( 1, errno, "îøèáêà âûçîâà connect" );
12		return s;
13 }
—————————————————————————————————————————————————————tcp_client.c