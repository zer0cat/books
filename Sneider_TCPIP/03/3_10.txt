———————————————————————————————————————————————————————readcrlf.c
 1 int readcrlf( SOCKET s, char *buf, size_t len )
 2 {
 3		char *bufx = buf;
 4		int rc;
 5		char c;
 6		char lastc = 0;
 7		while ( len > 0 )
 8		{
 9			if ( ( rc = recv( s, &c, 1, 0 ) ) != 1 )
10			{
11				/*
12				 *  Если нас прервали, повторим,
13				 *  иначе вернем EOF или код ошибки.
14				 */
15				if ( rc < 0 && errno == EINTR )
16					continue;
17				return rc;
18			}
19			if ( c == '\n' )
20			{
21				if ( lastc == '\r' )
22					buf--;
23				*buf = '\0';				/* Не включать <CR><LF>. */
24				return buf - bufx;
25			}
26			*buf++ = c;
27			lastc = c;
28			len--;
29		}
30		set_errno( EMSGSIZE );
31		return -1;
32 }
———————————————————————————————————————————————————————readcrlf.c