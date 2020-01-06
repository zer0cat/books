—————————————————————————————————————————————————————————daemon.c
 1 int daemon( int nocd, int noclose )
 2 {
 3	struct rlimit rlim;
 4	pid_t pid;
 5	int i;
 6	umask( 0 );		/* Очистить маску создания файлов. */
 7	/* Получить максимальное число открытых файлов. */
 8	if ( getrlimit( RLIMIT_NOFILE, &rlim ) < 0 )
 9		error( 1, errno, "getrlimit failed" );
10	/* Стать лидером сессии, потеряв при этом управляющий терминал... */
11	pid = fork();
12	if ( pid < 0 )
13		return -1;
14	if ( pid != 0 )
15		exit( 0 );
16	setsid();
17	/* ... и гарантировать, что больше его не будет. */
18	signal( SIGHUP, SIG_IGN );
19	pid = fork();
20	if ( pid < 0 )
21		return -1;
22	if ( pid != 0 )
23		exit( 0 );
24	/* Сделать текущим корневой каталог, если не требовалось обратное */
25	if ( !nocd )
26		chdir( "/" );
27	/*
28	 * Если нас не просили этого не делать, закрыть все файлы.
29	 * Затем перенаправить stdin, stdout и stderr
30	 * на /dev/null.
31	 */
32	if ( !noclose )
33	{
34 #if 0	/* Заменить на 1 для закрытия всех файлов. */
35	if ( rlim.rlim_max == RLIM_INFINITY )
36		rlim.rlim_max = 1024;
37	for ( i = 0; i < rlim.rlim_max; i++ )
38		close( i );
39 #endif
40	i = open( "/dev/null", O_RDWR );
41	if ( i < 0 )
42		return -1;
43	dup2( i, 0 );
44	dup2( i, 1 );
45	dup2( i, 2 );
46	if ( i > 2 )
47		close( i );
48 }
49 return 0;
50 }
—————————————————————————————————————————————————————————————————daemon.c
