————————————————————————————————————————————————————————tselect.c
 1 int tselect( int maxp1, fd_set *re, fd_set *we, fd_set *ee )
 2 {
 3		fd_set rmask;
 4		fd_set wmask;
 5		fd_set emask;
 6		struct timeval now;
 7		struct timeval tv;
 8		struct timeval *tvp;
 9		tevent_t *tp;
10		int n;
11		if ( re )
12			rmask = *re;
13		if ( we )
14			wmask = *we;
15		if ( ee )
16			emask = *ee;
17		for ( ;; )
18		{
19			if ( gettimeofday( &now, NULL ) < 0 )
20				error( 1, errno, "tselect: îøèáêà âûçîâà gettimeofday" );
21			while ( active && !timercmp( &now, &active->tv, < ) )
22			{
23				active->func( active->arg );
24				tp = active;
25				active = active->next;
26				tp->next = free_list;
27				free_list = tp;
28			}
29			if ( active )
30			{
31				tv.tv_sec = active->tv.tv_sec - now.tv_sec;;
32				tv.tv_usec = active->tv.tv_usec - now.tv_usec;
33				if ( tv.tv_usec < 0 )
34				{
35					tv.tv_usec += 1000000;
36					tv.tv_sec--;
37				}
38				tvp = &tv;
39			}
40			else if ( re == NULL && we == NULL && ee == NULL )
41				return 0;
42			else
43				tvp = NULL;
44			n = select( maxp1, re, we, ee, tvp );
45			if ( n < 0 )
46				return -1;
47			if ( n > 0 )
48				return n;
49			if ( re )
50				*re = rmask;
51			if ( we )
52				*we = wmask;
53			if ( ee )
54				*ee = emask;
55		}
56 }
————————————————————————————————————————————————————————tselect.c
