——————————————————————————————————————————————————————————readn.c
 1 int readn( SOCKET fd, char *bp, size_t len)
 2 {
 3		int cnt;
 4		int rc;
 5		cnt = len;
 6		while ( cnt > 0 )
 7		{
 8			rc = recv( fd, bp, cnt, 0 );
 9			if ( rc < 0 )			/* Îøèáêà ÷òåíèÿ? */
10			{
11				if ( errno == EINTR )	/* Âûçîâ ïðåðâàí? */
12					continue;			/* Ïîâòîðèòü ÷òåíèå. */
13				return -1;			/* Âåðíóòü êîä îøèáêè. */
14			}
15			if ( rc == 0 )			/* Êîíåö ôàéëà? */
16				return len - cnt;	/* Âåðíóòü íåïîëíûé ñ÷åò÷èê. */
17			bp += rc;
18			cnt -= rc;
19		}
20		return len;
21 }
——————————————————————————————————————————————————————————readn.c