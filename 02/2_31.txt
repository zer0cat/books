———————————————————————————————————————————————————————readline.c
 1 	int readline( SOCKET fd, char *bufptr, size_t len )
 2 	{
 3		char *bufx = bufptr;
 4		static char *bp;
 5		static int cnt = 0;
 6		static char b[ 1500 ];
 7		char c;
 8		while ( len –– > 0 )
 9		{
10			if ( cnt –– <= 0 )
11			{
12				cnt = recv( fd, b, sizeof( b ), 0 );
13				if ( cnt < 0 )
14					return -1;
15				if ( cnt == 0 )
16					return 0;
17				bp = b;
18			}
19			c = *bp++;
20			*bufptr++ = c;
21			if ( c == "\n" )
22			{
23				*bufptr = "\0";
24				return bufptr - bufx;
25			}
26		}
27		set_errno( EMSGSIZE );
28		return -1;
29 }
———————————————————————————————————————————————————————readline.c