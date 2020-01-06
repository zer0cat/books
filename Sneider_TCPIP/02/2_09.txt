———————————————————————————————————————————————————————udp_server.c
 1 SOCKET udp_server( char *hname, char *sname )
 2 {
 3	SOCKET s;
 4	struct sockaddr_in local;
 5	set_address( hname, sname, &local, "udp" );
 6	s = socket( AF_INET, SOCK_DGRAM, 0 );
 7	if ( !isvalidsock( s ) )
 8		error( 1, errno, "îøèáêà âûçîâà socket" );
 9	if ( bind( s, ( struct sockaddr * ) &local,
10		 sizeof( local ) ) )
11		error( 1, errno, "îøèáêà âûçîâà bind" );
12	return s;
13 }
———————————————————————————————————————————————————————udp_server.c