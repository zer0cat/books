———————————————————————————————————————————————————————readline.c
 1 	int readline( SOCKET fd, char *bufptr, size_t len )
 2 	{
 3	char *bufx = bufptr;
 4	static char *bp;
 5	static int cnt = 0;
 6	static char b[ 1500 ];
 7	char c;
 8	while ( --len > 0 )
 9	{
10		if ( --cnt <= 0 )
11		{
12			cnt = recv( fd, b, sizeof( b ), 0 );
13			if ( cnt < 0 )
14			{
15				if ( errno == EINTR )
16				{
17					len++;	/* Óìåíüøèì íà 1 â çàãîëîâêå while. */
18					continue;
19				}
20				return -1;
21			}
22			if ( cnt == 0 )
23				return 0;
24			bp = b;
25		}
26		c = *bp++;
27		*bufptr++ = c;
28		if ( c == "\n" )
29		{
30			*bufptr = "\0";
31			return bufptr - bufx;
32		}
33		}
34		set_errno( EMSGSIZE );
35		return -1;
36 }
———————————————————————————————————————————————————————readline.c