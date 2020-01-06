—————————————————————————————————————————————————————————tcpmux.c
 1 #include "etcp.h"
 2 #define MAXARGS	10		/* Максимальное число аргументов сервера. */
 3 #define MAXLINE	256		/* Максимальная длина строки в tcpmux.conf. */
 4 #define NSERVTAB	10		/* Число элементов в таблице service_table. */
 5 #define CONFIG	"tcpmux.conf"
 6 typedef struct
 7 {
 8		int flag;
 9		char *service;
10		char *path;
11		char *args[ MAXARGS + 1 ];
12		 } servtab_t;
13 		int ls;				/* Прослушиваемый сокет. */
14 		servtab_t service_table[ NSERVTAB + 1 ];
15 		int main( int argc, char **argv )
16 		{
17		struct sockaddr_in peer;
18		int s;
19		int peerlen;
20		/* Инициализировать и запустить сервер tcpmux. */
21		INIT();
22		parsetab();
23		switch ( argc )
24		{
25			case 1:		/* Все по умолчанию. */
26				ls = tcp_server( NULL, "tcpmux" );
27				break;
28			case 2:		/* Задан интерфейс и номер порта. */
29				ls = tcp_server( argv[ 1 ], "tcpmux" );
30				break;
31			case 3:		/* Заданы все параметры. */
32				ls = tcp_server( argv[ 1 ], argv[ 2 ] );
33				break;
34			default:
35				error( 1, 0, "Вызов: %s [ интерфейс [ порт ] ]\n",
36					program_name );
37		}
38		daemon( 0, 0 );
39		signal( SIGCHLD, reaper );
40		/* Принять соединения с портом tcpmux. */
41		for ( ;; )
42		{
43			peerlen = sizeof( peer );
44			s = accept( ls, ( struct sockaddr * )&peer, &peerlen );
45			if ( s < 0 )
46				continue;
47			start_server( s );
48			CLOSE( s );
49		}
50 }
—————————————————————————————————————————————————————————tcpmux.c