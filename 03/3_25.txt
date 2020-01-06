—————————————————————————————————————————————————————connectto1.c
 1 int main( int argc, char **argv )
 2 {
 3		fd_set rdevents;
 4		fd_set wrevents;
 5		fd_set exevents;
 6		struct sockaddr_in peer;
 7		struct timeval tv;
 8		SOCKET s;
 9		int flags;
10		int rc;
11		INIT();
12		set_address( argv[ 1 ], argv[ 2 ], &peer, "tcp" );
13		s = socket( AF_INET, SOCK_STREAM, 0 );
14		if ( !isvalidsock( s ) )
15			error( 1, errno, "ошибка вызова socket " );
16		if( ( flags = fcntl( s, F_GETFL, 0 ) ) < 0 )
17			error( 1, errno, "ошибка вызова fcntl (F_GETFL)" );
18		if ( fcntl( s, F_SETFL, flags | O_NONBLOCK ) < 0 )
19			error( 1, errno, "ошибка вызова fcntl (F_SETFL)" );
20		if ( ( rc = connect( s, ( struct sockaddr * )&peer,
21			 sizeof( peer ) ) ) && errno != EINPROGRESS )
22			error( 1, errno, "ошибка вызова connect" );
23		if ( rc == 0 )	/* Уже соединен? */
24		{
25			if ( fcntl( s, F_SETFL, flags ) < 0 )
26			error( 1, errno, "ошибка вызова fcntl (восстановление флагов)" );
27			client( s, &peer );
28			EXIT( 0 );
29		}
30		FD_ZERO( &rdevents );
31		FD_SET( s, &rdevents );
32		wrevents = rdevents;
33		exevents = rdevents;
34		tv.tv_sec = 5;
35		tv.tv_usec = 0;
36		rc = select( s + 1, &rdevents, &wrevents, &exevents, &tv );
37		if ( rc < 0 )
38			error( 1, errno, "ошибка вызова select" );
39		else if ( rc == 0 )
40			error( 1, 0, "истек тайм-аут connect\n" );
41		else if ( isconnected( s, &rdevents, &wrevents, &exevents ) )
42		{
43			if ( fcntl( s, F_SETFL, flags ) < 0 )
44			error( 1, errno, "ошибка вызова fcntl (восстановление флагов)" );
45			client( s, &peer );
46		}
47		else
48			error( 1, errno, "ошибка вызова connect" );
49		EXIT( 0 );
50 }
—————————————————————————————————————————————————————connectto1.c