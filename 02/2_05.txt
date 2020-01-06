—————————————————————————————————————————————————————tcp_server.c
 1 SOCKET tcp_server( char *hname, char *sname )
 2 {
 3		struct sockaddr_in local;
 4		SOCKET s;
 5		const int on = 1;
 6		set_address( hname, sname, &local, "tcp" );
 7		s = socket( AF_INET, SOCK_STREAM, 0 );
 8		if ( !isvalidsock( s ) )
 9			error( 1, errno, "îøèáêà âûçîâà socket" );
10		if ( setsockopt( s, SOL_SOCKET, SO_REUSEADDR,
11			( char * )&on, sizeof( on ) ) )
12			error( 1, errno, "îøèáêà âûçîâà setsockopt" );
13		if ( bind( s, ( struct sockaddr * ) &local,
14			 sizeof( local ) ) )
15			error( 1, errno, "îøèáêà âûçîâà bind" );
16		if ( listen( s, NLISTEN ) )
17			error( 1, errno, "îøèáêà âûçîâà listen" );
18		return s;
19 }
—————————————————————————————————————————————————————tcp_server.c