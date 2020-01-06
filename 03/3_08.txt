—————————————————————————————————————————————————————————tcpmux.c
 1 static void start_server( int s )
 2 {
 3		char line[ MAXLINE ];
 4		servtab_t *stp;
 5		int rc;
 6		static char err1[] = "-не могу прочесть имя сервиса \r\n";
 7		static char err2[] = "-неизвестный сервис\r\n";
 8		static char err3[] = "-не могу запустить сервис\r\n";
 9		static char ok[] = "+OK\r\n";
10		rc = fork();
11		if ( rc < 0 )		/* Ошибка вызова fork. */
12		{
13			write( s, err3, sizeof( err3 ) - 1 );
14			return;
15		}	
16		if ( rc != 0 )		/* Родитель. */
17			return;
18		/* Процесс-потомок. */
19		CLOSE( ls );		/* Закрыть прослушивающий сокет. */
20		alarm( 10 );
21		rc = readcrlf( s, line, sizeof( line ) );
22		alarm( 0 );
23		if ( rc <= 0 )
24		{
25			write( s, err1, sizeof( err1 ) - 1 );
26			EXIT( 1 );
27		}
28		for ( stp = service_table; stp->service; stp++ )
29			if ( strcasecmp( line, stp->service ) == 0 )
30				break;
31		if ( !stp->service )
32		{
33			write( s, err2, sizeof( err2 ) - 1 );
34			EXIT( 1 );
35		}
36		if ( stp->flag )
37			if ( write( s, ok, sizeof( ok ) - 1 ) < 0 )
38				EXIT( 1 );
39		dup2( s, 0 );
40		dup2( s, 1 );
41		dup2( s, 2 );
42		CLOSE( s );
43		execv( stp->path, stp->args );
44		write( 1, err3, sizeof( err3 ) - 1 );
45		EXIT( 1 );
46 }
—————————————————————————————————————————————————————————tcpmux.c
