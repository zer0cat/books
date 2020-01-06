———————————————————————————————————————————————————————udp_client.c
 1 SOCKET udp_client( char *hname, char *sname,
 2		struct sockaddr_in *sap )
 3 {
 4		SOCKET s;
 5		set_address( hname, sname, sap, "udp" );
 6		s = socket( AF_INET, SOCK_DGRAM, 0 );
 7		if ( !isvalidsock( s ) )
 8			error( 1, errno, "îøèáêà âûçîâà socket" );
 9		return s;
10 }
—————————————————————————————————————————————————————udp_client.c