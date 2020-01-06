——————————————————————————————————————————————————————hb_client.c
 1 #include "etcp.h"
 2 #include "heartbeat.h"
 3 int main( int argc, char **argv )
 4 {
 5		fd_set allfd;
 6		fd_set readfd;
 7		msg_t msg;
 8		struct timeval tv;
 9		SOCKET s;
10		int rc;
11		int heartbeats = 0;
12		int cnt = sizeof( msg );
13		INIT();
14		s = tcp_client( argv[ 1 ], argv[ 2 ] );
15		FD_ZERO( &allfd );
16		FD_SET( s, &allfd );
17		tv.tv_sec = T1;
18		tv.tv_usec = 0;
19		for ( ;; )
20		{
21			readfd = allfd;
22			rc = select( s + 1, &readfd, NULL, NULL, &tv );
23			if ( rc < 0 )
24				error( 1, errno, "ошибка вызова select" );
25			if ( rc == 0 )	/* Произошел тайм-аут. */
26			{
27				if ( ++heartbeats > 3 )
28				error( 1, 0, "соединения нет\n" );
29			error( 0, 0, "посылаю пульс #%d\n", heartbeats );
30			msg.type = htonl( MSG_HEARTBEAT );
31			rc = send( s, ( char * )&msg, sizeof( msg ), 0 );
32			if ( rc < 0 )
33				error( 1, errno, "ошибка вызова send" );
34			tv.tv_sec = T2;
35			continue;
36		}
37		if ( !FD_ISSET( s, &readfd ) )
38			error( 1, 0, "select вернул некорректный сокет\n" );
39		rc = recv( s, ( char * )&msg + sizeof( msg ) - cnt,
40			cnt, 0 );
41		if ( rc == 0 )
42			error( 1, 0, "сервер закончил работу\n" );
43		if ( rc < 0 )
44			error( 1, errno, "ошибка вызова recv" );
45		heartbeats = 0;
46		tv.tv_sec = T1;
47		cnt -= rc;					/* Встроенный readn. */
48		if ( cnt > 0 )
49			continue;
50		cnt = sizeof( msg );
51		/* Обработка сообщения. */
52		}
53	}
——————————————————————————————————————————————————————hb_client.c
