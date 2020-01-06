———————————————————————————————————————————————————————————wincompat.c
 1 #include <sys/timeb.h>
 2 #include "etcp.h"
 3 #include <winsock2.h>
 4 #define MINBSDSOCKERR			( WSAEWOULDBLOCK )
 5 #define MAXBSDSOCKERR			( MINBSDSOCKERR + \
 6									( sizeof( bsdsocketerrs ) / \
 7									sizeof( bsdsocketerrs[ 0 ] ) ) )
 8 extern int sys_nerr;
 9 extern char *sys_errlist[];
10 extern char *program_name;
11 static char *bsdsocketerrs[] =
12 {
13	"Resource temporarily unavailable",	/* Ресурс временно недоступен. */
14	"Operation now in progress",			/* Операция начала выполняться. */
15	"Operation already in progress",	/* Операция уже выполняется. */
16	"Socket operation on non-socket",		/* Операция сокета не над сокетом. */
17	"Destination address required",		/* Нужен адрес назначения. */
18	"Message too long",						/* Слишком длинное сообщение. */
19	"Protocol wrong type for socket",		/* Неверный тип протокола для сокета. */
20	"Bad protocol option",					/* Некорректная опция протокола. */
21	"Protocol not supported",				/* Протокол не поддерживается. */
22	"Socket type not supported",			/* Тип сокета не поддерживается. */
23	"Operation not supported",				/* Операция не поддерживается. */
24	"Protocol family not supported",	/* Семейство протоколов не */
											/* поддерживается. */
25	"Address family not supported by protocol family", /* Адресное семейство */
								/* не поддерживается семейством протоколов*/	26	"Address already in use",				/* Адрес уже используется. */
27	"Can’t assign requested address",	/* Не могу выделить затребованный */
											/* адрес. */
28	"Network is down",						/* Сеть не работает. */
29	"Network is unreachable",				/* Сеть недоступна. */
30	"Network dropped connection on reset", /* Сеть сбросила соединение */
											/* при перезагрузке. */
31	"Software caused connection abort", 	/* Программный разрыв соединения. */
32	"Connection reset by peer",			/* Соединение сброшено другой */
											/* стороной. */
33	"No buffer space available",			/* Нет буферов. */
34	"Socket is already connected",			/* Сокет уже соединен. */
35	"Socket is not connected",				/* Сокет не соединен. */
36	"Cannot send after socket shutdown",	/* Не могу послать данные после */
											/* размыкания. */
37	"Too many references: can’t splice", /* Слишком много ссылок. */
38	"Connection timed out",					/* Таймаут на соединении. */
39	"Connection refused",					/* В соединении отказано. */
40	"Too many levels of symbolic links",	/* Слишком много уровней */
											/* символических ссылок. */
41	"File name too long",					/* Слишком длинное имя файла. */
42	"Host is down",							/* Хост не работает. */
43	"No route to host"						/* Нет маршрута к хосту. */
44 };
45 void init( char **argv )
46 {
47		WSADATA wsadata;
48		( program_name = strrchr( argv[ 0 ], '\\' ) ) ?
49			program_name++ : ( program_name = argv[ 0 ] );
50		WSAStartup( MAKEWORD( 2, 2 ), &wsadata );
51	}
52 /* inet_aton - версия inet_aton для SVr4 и Windows. */
53 int inet_aton( char *cp, struct in_addr *pin )
54 {
55    int rc;
56		rc = inet_addr( cp );
57		if ( rc == -1 && strcmp( cp, "255.255.255.255" ) )
58			return 0;
59		pin->s_addr = rc;
60		return 1;
61 }
62 /* gettimeofday - для tselect. */
63 int gettimeofday( struct timeval *tvp, struct timezone *tzp )
64 {
65		struct _timeb tb;
66		_ftime( &tb );
67		if ( tvp )
68		{
69			tvp->tv_sec = tb.time;
70			tvp->tv_usec = tb.millitm * 1000;
71		}
72		if ( tzp )
73		{
74			tzp->tz_minuteswest = tb.timezone;
75			tzp->tz_dsttime = tb.dstflag;
76		}
77 }
78 /* strerror - версия, включающая коды ошибок Winsock. */
79 char *strerror( int err )
80 {
81		if ( err >= 0 && err < sys_nerr )
82			return sys_errlist[ err ];
83		else if ( err >= MINBSDSOCKERR && err < MAXBSDSOCKERR )
84			return bsdsocketerrs[ err - MINBSDSOCKERR ];
85		else if ( err == WSASYSNOTREADY )
86			return "Network subsystem is unusable";
						/* С0етевая подсистема неработоспособна. */
87		else if ( err == WSAVERNOTSUPPORTED )
88			return "This version of Winsock not supported";
						/* Эта версия Winsock не поддерживается. */
89		else if ( err == WSANOTINITIALISED )
90			return "Winsock not initialized";
						/* Winsock не инициализирована. */
91		else
92			return "Unknown error";
						/* Неизвестная ошибка. */
93 }
——————————————————————————————————————————————————————————————wincompat.c