—————————————————————————————————————————————————————————extsys.c
 1 #include "etcp.h"
 2 #define COOKIESZ	4	/* Так установлено клиентом. */
 3 int main( int argc, char **argv )
 4 {
 5		SOCKET s;
 6		SOCKET s1;
 7		int rc;
 8		char buf[ 128 ];
 9		INIT();
10		s = tcp_server( NULL, argv[ 1 ] );
11		s1 = accept( s, NULL, NULL );
12		if ( !isvalidsock( s1 ) )
13			error( 1, errno, "ошибка вызова accept" );
14		srand( 127 );
15		for ( ;; )
16		{
17			rc = readvrec( s1, buf, sizeof( buf ) );
18			if ( rc == 0 )
19				error( 1, 0, "клиент отсоединился\n" );
20			if ( rc < 0 )
21				error( 1, errno, "ошибка вызова recv" );
22			if ( rand() % 100 < 33 )
23				continue;
24			write( 1, buf + COOKIESZ, rc - COOKIESZ );
25			memmove( buf + 1, buf, COOKIESZ );
26			buf[ 0 ] = '\006';
27			if ( send( s1, buf, 1 + COOKIESZ, 0 ) < 0 )
28				error( 1, errno, "ошибка вызова send" );
29		}
30 }
——————————————————————————————————————————————————————————extsys.c