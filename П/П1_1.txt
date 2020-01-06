———————————————————————————————————————————————————————————etcp.h
 1 #ifndef __ETCP_H__
 2 #define __ETCP_H__
 3 /* Включаем стандартные заголовки. */
 4 #include <errno.h>
 5 #include <stdlib.h>
 6 #include <unistd.h>
 7 #include <stdio.h>
 8 #include <stdarg.h>
 9 #include <string.h>
10 #include <netdb.h>
11 #include <signal.h>
12 #include <fcntl.h>
13 #include <sys/socket.h>
14 #include <sys/wait.h>
15 #include <sys/time.h>
16 #include <sys/resource.h>
17 #include <sys/stat.h>
18 #include <netinet/in.h>
19 #include <arpa/inet.h>
20 #include "skel.h"
21 #define TRUE		1
22 #define FALSE	0
23 	#define NLISTEN	5	/* Максимальное число ожидающих соединений. */
24 #define NSMB		5	/* Число буферов в разделяемой памяти. */
25 #define SMBUFSZ	256	/* Размер буфера в разделяемой памяти. */
26 extern char *program_name;	/* Для сообщений об ошибках. */
27 #ifdef __SVR4
28 #define bzero(b,n) memset( ( b ), 0, ( n ) )
29 #endif
30 typedef void ( *tofunc_t )( void * );
31 void error( int, int, char*, ... );
32 int readn( SOCKET, char *, size_t );
33 int readvrec( SOCKET, char *, size_t );
34 int readcrlf( SOCKET, char *, size_t );
35 int readline( SOCKET, char *, size_t );
36 int tcp_server( char *, char * );
37 int tcp_client( char *, char * );
38 int udp_server( char *, char * );
39 int udp_client( char *, char *, struct sockaddr_in * );
40 int tselect( int, fd_set *, fd_set *, fd_set *);
41 unsigned int timeout( tofunc_t, void *, int );
42 void untimeout( unsigned int );
43 void init_smb( int );
44 void *smballoc( void );
45 void smbfree( void * );
46 void smbsend( SOCKET, void * );
47 void *smbrecv( SOCKET );
48 void set_address( char *, char *, struct sockaddr_in *, char * );
49 #endif  /* __ETCP_H__ */
———————————————————————————————————————————————————————————etcp.h