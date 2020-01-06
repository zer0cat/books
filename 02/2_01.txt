———————————————————————————————————————————————————————————skel.h
 1 #ifndef __SKEL_H__
 2 #define __SKEL_H__
 3 /* âåðñèÿ äëÿ UNIX */
 4 #define INIT()				( program_name = \
 5									strrchr( argv[ 0 ], '/' ) ) ? \
 6									program_name++ : \
 7									( program_name = argv[ 0 ] )
 8 #define EXIT(s)				exit( s )
 9 #define CLOSE(s)			if ( close( s ) ) error( 1, errno, \
10									"îøèáêà close " )
11 #define set_errno(e)		errno = ( e )
12 #define isvalidsock(s)	( ( s ) >= 0 )
13 typedef int SOCKET;
14 #endif  /* __SKEL_H__ */
———————————————————————————————————————————————————————————skel.h