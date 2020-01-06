———————————————————————————————————————————————————————readvrec.c
 1 int readvrec( SOCKET fd, char *bp, size_t len )
 2 {
 3		u_int32_t reclen;
 4		int rc;
 5		/* Прочитать длину записи. */
 6		rc = readn( fd, ( char * )&reclen, sizeof( u_int32_t ) );
 7		if ( rc != sizeof( u_int32_t ) )
 8			return rc < 0 ? -1 : 0;
 9		reclen = ntohl( reclen );
10		if ( reclen > len )
11		{
12			/*
13			 *  Не хватает места в буфере для размещения данных -
14			 *  отбросить их и вернуть код ошибки.
15			 */
16			while ( reclen > 0 )
17			{
18				rc = readn( fd, bp, len );
19				if ( rc != len )
20					return rc < 0 ? -1 : 0;
21				reclen -= len;
22				if ( reclen < len )
23					len = reclen;
24			}
25			set_errno( EMSGSIZE );
26			return -1;
27		}
28		/* Прочитать саму запись */
29		rc = readn( fd, bp, reclen );
30		if ( rc != reclen )
31			return rc < 0 ? -1 : 0;
32		return rc;
33 }
———————————————————————————————————————————————————————readvrec.c