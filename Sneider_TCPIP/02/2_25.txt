————————————————————————-—————————————————-——————————————————-hb_server.c
 1 	#include "etcp.h"
 2 	#include "heartbeat.h"
 3 	int main( int argc, char **argv )
 4 	{
 5		  fd_set allfd;
 6		  fd_set readfd;
 7		  msg_t msg;
 8		  struct timeval tv;
 9		  SOCKET s;
10		  SOCKET s1;
11		  int rc;
12		  int missed_heartbeats = 0;
13		  int cnt = sizeof( msg );
14		  INIT();
15		  s = tcp_server( NULL, argv[ 1 ] );
16		  s1 = accept( s, NULL, NULL );
17		  if ( !isvalidsock( s1 ) )
18			 error( 1, errno, "ошибка вызова accept" );
19		  tv.tv_sec = T1 + T2;
20		  tv.tv_usec = 0;
21		  FD_ZERO( &allfd );
22		  FD_SET( s1, &allfd );
23		  for ( ;; )
24		  {
25		readfd = allfd;
26		rc = select( s1 + 1, &readfd, NULL, NULL, &tv );
27		if ( rc < 0 )
28			error( 1, errno, "ошибка вызова select" );
29		if ( rc == 0 )	/* Произошел тайм-аут. */
30		{
31			if ( ++missed_heartbeats > 3 )
32				error( 1, 0, "соединение умерло\n" );
33			error( 0, 0, "пропущен пульс #%d\n",
34				missed_heartbeats );
35			tv.tv_sec = T2;
36			continue;
37		}
38		if ( !FD_ISSET( s1, &readfd ) )
39			error( 1, 0, "select вернул некорректный сокет\n" );
40		rc = recv( s1, ( char * )&msg + sizeof( msg ) - cnt,
41			cnt, 0 );
42		if ( rc == 0 )
43			error( 1, 0, "клиент завершил работу\n" );
44		if ( rc < 0 )
45			error( 1, errno, "ошибка вызова recv" );
46		missed_heartbeats = 0;
47		tv.tv_sec = T1 + T2;
48		cnt -= rc;	/* Встроенный readn. */
49		if ( cnt > 0 )
50			continue;
51		cnt = sizeof( msg );
52		switch ( ntohl( msg.type ) )
53		{
54			case MSG_TYPE1 :
55				/* Обработать сообщение типа TYPE1. */
56				break;
57			case MSG_TYPE2 :
58				/* Обработать сообщение типа TYPE2. */
59				break;
60			case MSG_HEARTBEAT :
61				rc = send( s1, ( char * )&msg, sizeof( msg ), 0 );
62				if ( rc < 0 )
63					error( 1, errno, "ошибка вызова send" );
64				break;
65			default :
66				error( 1, 0, "неизвестный тип сообщения (%d)\n",
67					ntohl( msg.type ) );
68		}
69	}
70	EXIT( 0 );
71 	}
——————————————————————————————————————————————————————hb_server.c