—————————————————————————————————————————————————————hb_client2.c
 1 	#include "etcp.h"
 2 	#include "heartbeat.h"
 3 	int main( int argc, char **argv )
 4 	{
 5		fd_set allfd;
 6		fd_set readfd;
 7		char msg[ 1024 ];
 8		struct timeval tv;
 9		struct sockaddr_in hblisten;
10		SOCKET sdata;
11		SOCKET shb;
12		SOCKET slisten;
13		int rc;
14		int hblistenlen = sizeof( hblisten );
15	int heartbeats = 0;
16	int maxfd1;
17	char hbmsg[ 1 ];
18	INIT();
19	slisten = tcp_server( NULL, "0" );
20	rc = getsockname( slisten, ( struct sockaddr * )&hblisten,
21		&hblistenlen );
22	if ( rc )
23		error( 1, errno, "îøèáêà âûçîâà getsockname" );
24	sdata = tcp_client( argv[ 1 ], argv[ 2 ] );
25	rc = send( sdata, ( char * )&hblisten.sin_port,
26		sizeof( hblisten.sin_port ), 0 );
27	if ( rc < 0 )
28		error( 1, errno, "îøèáêà ïðè ïîñûëêå íîìåðà ïîðòà" );
29	shb = accept( slisten, NULL, NULL );
30	if ( !isvalidsock( shb ) )
31		error( 1, errno, "îøèáêà âûçîâà accept" );
32	FD_ZERO( &allfd );
33	FD_SET( sdata, &allfd );
34	FD_SET( shb, &allfd );
35	maxfd1 = ( sdata > shb ? sdata: shb ) + 1;
36	tv.tv_sec = T1;
37	tv.tv_usec = 0;
—————————————————————————————————————————————————————hb_client2.c