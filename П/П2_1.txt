———————————————————————————————————————————————————————————skel.h
 1 #ifndef __SKEL_H__
 2 #define __SKEL_H__
 3 /* Âåðñèÿ Winsock. */
 4 #include <windows.h>
 5 #include <winsock2.h>
 6 struct timezone
 7 {
 8		long tz_minuteswest;
 9		long tz_dsttime;
10 };
11 typedef unsigned int u_int32_t;
12 #define EMSGSIZE				WSAEMSGSIZE
13 #define INIT()				init( argv );
14 #define EXIT(s)				do { WSACleanup(); exit( ( s ) ); } \
15									while ( 0 )
16 #define CLOSE(s)				if ( closesocket( s ) ) \
17									error( 1, errno, "îøèáêà âûçîâà close")
18 #define errno					( GetLastError() )
19 #define set_errno(e)		SetLastError( ( e ) )
20 #define isvalidsock(s)		( ( s ) != SOCKET_ERROR )
21 #define bzero(b,n)	memset	( ( b ), 0, ( n ) )
22 #define sleep(t)				Sleep( ( t ) * 1000 )
23 #define WINDOWS
24 #endif  /* __SKEL_H__ */
——————————————————————————————————————————————————————————————skel.h
