——————————————————————————————————————————————————————————xout2.c
 1 #include "etcp.h"
 2 #define ACK		0x6	/* Символ подтверждения ACK. */
 3 int main( int argc, char **argv )
 4 {
 5		fd_set allreads;
 6		fd_set readmask;
 7		fd_set sockonly;
 8		struct timeval tv;
 9		struct timeval *tvp = NULL;
10		SOCKET s;
11		int rc;
12		char buf[ 128 ];
13		const static struct timeval T0 = { 2, 0 };
14		INIT();
15		s = tcp_client( argv[ 1 ], argv[ 2 ] );
16		FD_ZERO( &allreads );
17		FD_SET( s, &allreads );
18		sockonly = allreads;
19		FD_SET( 0, &allreads );
20		readmask = allreads;
21		for ( ;; )
22		{
23			rc = select( s + 1, &readmask, NULL, NULL, tvp );
24			if ( rc < 0 )
25				error( 1, errno, "ошибка вызова select" );
26			if ( rc == 0 )
27				error( 1, 0, "тайм-аут при приеме сообщения\n" );
28			if ( FD_ISSET( s, &readmask ) )
29			{
30				rc = recv( s, buf, sizeof( buf ), 0 );
31				if ( rc == 0 )
32					error( 1, 0, "сервер отсоединился\n" );
33				else if ( rc < 0 )
34					error( 1, errno, "ошибка вызова recv" );
35				else if ( rc != 1 || buf[ 0 ] != ACK )
36					error( 1, 0, "неожиданный вход [%c]\n", buf[ 0 ] );
37				tvp = NULL;	/* Отключить таймер */
38				readmask = allreads; /* и продолжить чтение из stdin. */
39			}
40			if ( FD_ISSET( 0, &readmask ) )
41			{
42			rc = read( 0, buf, sizeof( buf ) );
43			if ( rc < 0 )
44				error( 1, errno, "ошибка вызова read" );
45			if ( send( s, buf, rc, 0 ) < 0 )
46				error( 1, errno, "ошибка вызова send" );
47			tv = T0;					/* Переустановить таймер. */
48			tvp = &tv;				/* Взвести таймер */
49			readmask = sockonly; /* и прекратить чтение из stdin. */
50		}
51	  }
52 }
—————————————————————————————————————————————————————————-xout2.c
