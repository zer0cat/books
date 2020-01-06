——————————————————————————————————————————————————————————xout3.c
 1 msgrec_t *getfreerec( void )
 2 {
 3		msgrec_t *mp;
 4		for ( mp = mr; mp < mr + MRSZ; mp++ )
 5			if ( mp->pkt.len == -1 )	/* Запись свободна? */
 6				return mp;
 7		error( 1, 0, "getfreerec: исчерпан пул записей сообщений\n" );
 8		return NULL;	/* "Во избежание предупреждений компилятора. */
 9 }
10 		msgrec_t *findmsgrec( u_int32_t mid )
11 	{
12		msgrec_t *mp;
13		for ( mp = mr; mp < mr + MRSZ; mp++ )
14			if ( mp->pkt.len != -1 && mp->pkt.cookie == mid )
15				return mp;
16		error( 0, 0, 
		"findmsgrec: нет сообщения, соответствующего ACK %d\n", mid );
17		return NULL;
18 }
19 	void freemsgrec( msgrec_t *mp )
20 {
21		if ( mp->pkt.len == -1 )
22		error( 1, 0, "freemsgrec: запись сообщения уже освобождена\n" );
23		mp->pkt.len = -1;
24 }
25	 static void drop( msgrec_t *mp )
26 {
27		error( 0, 0, "Сообщение отбрасывается:   %s", mp->pkt.buf );
28		freemsgrec( mp );
29 }
30 static void lost_ACK( msgrec_t *mp )
31 {
32		error( 0, 0, "Повтор сообщения:   %s", mp->pkt.buf );
33		if ( send( s, &mp->pkt,
34			 sizeof( u_int32_t ) + ntohl( mp->pkt.len ), 0 ) < 0 )
35			error( 1, errno, "потерян ACK: ошибка вызова send" );
36		mp->id = timeout( ( tofunc_t )drop, mp, T2 );
37 }
——————————————————————————————————————————————————————————xout3.c