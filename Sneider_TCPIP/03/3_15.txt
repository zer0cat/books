————————————————————————————————————————————————————————tselect.c
 1 static tevent_t *allocate_timer( void )
 2 {
 3		tevent_t *tp;
 4	if ( free_list == NULL ) /* нужен новый блок таймеров? */
 5	{
 6		free_list = malloc( NTIMERS * sizeof( tevent_t ) );
 7		if ( free_list == NULL )
 8			error( 1, 0, "не удалось получить таймеры\n" );
 9		for ( tp = free_list;
10			  tp < free_list + NTIMERS - 1; tp++ )
11			tp->next = tp + 1;
12		tp->next = NULL;
13	}
14	tp = free_list;	/* Выделить первый. */
15	free_list = tp->next;	/* Убрать его из списка. */
16	return tp;
17 	}
18 unsigned int timeout( void ( *func )( void * ), void *arg, int ms )
19 	{
20	tevent_t *tp;
21	tevent_t *tcur;
22	tevent_t **tprev;
23	static unsigned int id = 1;	/* Идентификатор таймера. */
24	tp = allocate_timer();
25	tp->func = func;
26	tp->arg = arg;
27	if ( gettimeofday( &tp->tv, NULL ) < 0 )
28		error( 1, errno, "timeout: ошибка вызова gettimeofday" );
29	tp->tv.tv_usec += ms * 1000;
30	if ( tp->tv.tv_usec > 1000000 )
31	{
32		tp->tv.tv_sec += tp->tv.tv_usec / 1000000;
33		tp->tv.tv_usec %= 1000000;
34	}
35	for ( tprev = &active, tcur = active;
36		  tcur && !timercmp( &tp->tv, &tcur->tv, < ); /* XXX */
37		  tprev = &tcur->next, tcur = tcur->next )
38	{ ; }
39	*tprev = tp;
40	tp->next = tcur;
41	tp->id = id++;	/* Присвоить значение идентификатору таймера. */
42	return tp->id;
43 }
————————————————————————————————————————————————————————tselect.c
