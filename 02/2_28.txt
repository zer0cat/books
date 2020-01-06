—————————————————————————————————————————————————————hb_server2.c
 1 	#include "etcp.h"
 2 	#include "heartbeat.h"
 3 	int main( int argc, char **argv )
 4 	{
 5		fd_set allfd;
 6		fd_set readfd;
 7		char msg[ 1024 ];
 8		struct sockaddr_in peer;
 9		struct timeval tv;
10		SOCKET s;
11		SOCKET sdata;
12		SOCKET shb;
13		int rc;
14		int maxfd1;
15		int missed_heartbeats = 0;
16		int peerlen = sizeof( peer );
17		char hbmsg[ 1 ];
18		INIT();
19		s = tcp_server( NULL, argv[ 1 ] );
20		sdata = accept( s, ( struct sockaddr * )&peer,
21			&peerlen );
22		if ( !isvalidsock( sdata ) )
23			error( 1, errno, "accept failed" );
24		rc = readn( sdata, ( char * )&peer.sin_port,
25			sizeof( peer.sin_port ) );
26		if ( rc < 0 )
27			error( 1, errno, "îøèáêà ïðè ÷òåíèè íîìåðà ïîðòà" );
28		shb = socket( PF_INET, SOCK_STREAM, 0 );
29		if ( !isvalidsock( shb ) )
30			error( 1, errno, "îøèáêà ïðè ñîçäàíèè ñîêåòà shb" );
31		rc = connect( shb, ( struct sockaddr * )&peer, peerlen );
32		if ( rc )
33			error( 1, errno, "îøèáêà âûçîâà connect äëÿ ñîêåòà shb" );
34		tv.tv_sec = T1 + T2;
35		tv.tv_usec = 0;
36		FD_ZERO( &allfd );
37		FD_SET( sdata, &allfd );
38		FD_SET( shb, &allfd );
39		maxfd1 = ( sdata > shb ? sdata : shb ) + 1;
——————————————————————————————————————————————————————hb_server2.c