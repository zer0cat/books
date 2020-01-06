———————————————————————————————————————————————————————————icmp.c
 1 #include <sys/types.h>
 2 #include <netinet/in_systm.h>
 3 #include <netinet/in.h>
 4 #include <netinet/ip.h>
 5 #include <netinet/ip_icmp.h>
 6 #include <netinet/udp.h>
 7 #include "etcp.h"
 8 int main( int argc, char **argv )
 9 {
10	SOCKET s;
11	struct protoent *pp;
12	int rc;
13	char icmpdg[ 1024 ];
14	INIT();
15	pp = getprotobyname( "icmp" );
16	if ( pp == NULL )
17		error( 1, errno, "îøèáêà âûçîâà getprotobyname" );
18	s = socket( AF_INET, SOCK_RAW, pp->p_proto );
19	if ( !isvalidsock( s ) )
20		error( 1, errno, "îøèáêà âûçîâà socket" );
21	for ( ;; )
22	{
23		rc = recvfrom( s, icmpdg, sizeof( icmpdg ), 0,
24			NULL, NULL );
25		if ( rc < 0 )
26			error( 1, errno, "îøèáêà âûçîâà recvfrom" );
27		print_dg( icmpdg, rc );
28	}
29 }
———————————————————————————————————————————————————————————icmp.c