————————————————————————————————————————————————————————tcpecho.c
 1 #include "etcp.h"
 2 int main( int argc, char **argv )
 3 {
 4		SOCKET s;
 5		SOCKET s1;
 6		char buf[ 1024 ];
 7		int rc;
 8		int nap = 0;
 9		INIT();
10		if ( argc == 3 )
11			nap = atoi( argv[ 2 ] );
12		s = tcp_server( NULL, argv[ 1 ] );
13		s1 = accept( s, NULL, NULL );
14		if ( !isvalidsock( s1 ) )
15			error( 1, errno, "ошибка вызова accept" );
16		signal( SIGPIPE, SIG_IGN ); /* Игнорировать сигнал SIGPIPE. */
17		for ( ;; )
18		{
19			rc = recv( s1, buf, sizeof( buf ), 0 );
20			if ( rc == 0 )
21				error( 1, 0, "клиент отсоединился\n" );
22			if ( rc < 0 )
23				error( 1, errno, "ошибка вызова recv" );
24			if ( nap )
25				sleep( nap );
26			rc = send( s1, buf, rc, 0 );
27			if ( rc < 0 )
28				error( 1, errno, "ошибка вызова send" );
29		}
30 }
————————————————————————————————————————————————————————tcpecho.c