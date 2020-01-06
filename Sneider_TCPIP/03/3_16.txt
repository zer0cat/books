————————————————————————————————————————————————————————tselect.c
 1 void untimeout( unsigned int id )
 2 {
 3		tevent_t **tprev;
 4		tevent_t *tcur;
 5		for ( tprev = &active, tcur = active;
 6			  tcur && id != tcur->id;
 7			  tprev = &tcur->next, tcur = tcur->next )
 8		{ ; }
 9		if ( tcur == NULL )
10		{
11			error( 0, 0,
12			"ïðè âûçîâå untimeout óêàçàí íåñóùåñòâóþùèé òàéìåð (%d)\n", id );
13			return;
14		}
15		*tprev = tcur->next;
16		tcur->next = free_list;
17		free_list = tcur;
18 }
————————————————————————————————————————————————————————tselect.c
