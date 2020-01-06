——————————————————————————————————————————————————————badserver.c
 1 #include "etcp.h"
 2 int main( int argc, char **argv )
 3 {
 4		struct sockaddr_in local;
 5		SOCKET s;
 6		SOCKET s1;
 7		int rc;
 8		char buf[ 1024 ];
 9		INIT();
10		s = socket( PF_INET, SOCK_STREAM, 0 );
11		if ( !isvalidsock( s ) )
12			error( 1, errno, "Не могу получить сокет" );
13		bzero( &local, sizeof( local ) );
14		local.sin_family = AF_INET;
15		local.sin_port = htons( 9000 );
16		local.sin_addr.s_addr = htonl( INADDR_ANY );
17		if ( bind( s, ( struct sockaddr * )&local,
18			sizeof( local ) ) < 0 )
19			error( 1, errno, "Не могу привязать сокет" );
20		if ( listen( s, NLISTEN ) < 0 )
21			error( 1, errno, "ошибка вызова listen" );
22		s1 = accept( s, NULL, NULL );
23		if ( !isvalidsock( s1 ) )
24			error( 1, errno, "ошибка вызова accept" );
25		for ( ;; )
26		{
27			rc = recv( s1, buf, sizeof( buf ), 0 );
28			if ( rc < 0 )
29				error( 1, errno, "ошибка вызова recv" );
30			if ( rc == 0 )
31				error( 1, 0, "Клиент отсоединился\n" );
32			rc = send( s1, buf, rc, 0 );
33			if ( rc < 0 )
34				error( 1, errno, "ошибка вызова send" );
35		}
36 }
——————————————————————————————————————————————————————badserver.c