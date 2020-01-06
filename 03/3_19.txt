——————————————————————————————————————————————————————————xout3.c
 1 int main( int argc, char **argv )
 2 {
 3		fd_set allreads;
 4		fd_set readmask;
 5		msgrec_t *mp;
 6		int rc;
 7		int mid;
 8		int cnt = 0;
 9		u_int32_t msgid = 0;
10		char ack[ ACKSZ ];
11		INIT();
12		s = tcp_client( argv[ 1 ], argv[ 2 ] );
13		FD_ZERO( &allreads );
14		FD_SET( s, &allreads );
15		FD_SET( 0, &allreads );
16		for ( mp = mr; mp < mr + MRSZ; mp++ )
17			mp->pkt.len = -1;
18		for ( ;; )
19		{
20			readmask = allreads;
21			rc = tselect( s + 1, &readmask, NULL, NULL );
22			if ( rc < 0 )
23				error( 1, errno, "îøèáêà âûçîâà tselect" );
24			if ( rc == 0 )
25				error( 1, 0, "tselect ñêàçàëà, ÷òî íåò ñîáûòèé\n" );
26			if ( FD_ISSET( s, &readmask ) )
27			{
28				rc = recv( s, ack + cnt, ACKSZ - cnt, 0 );
29				if ( rc == 0 )
30					error( 1, 0, "ñåðâåð îòñîåäèíèëñÿ\n" );
31				else if ( rc < 0 )
32					error( 1, errno, "îøèáêà âûçîâà recv" );
33				if ( ( cnt += rc ) < ACKSZ ) /* Öåëîå ñîîáùåíèå? */
34					continue;		/* Íåò, åùå ïîäîæäåì. */
35				cnt = 0;		/* Â ñëåäóþùèé ðàç íîâîå ñîîáùåíèå. */
36				if ( ack[ 0 ] != ACK )
37				{
38					error( 0, 0, "ïðåäóïðåæäåíèå: íåâåðíîå ïîäòâåðæäåíèå\n" );
39					continue;
40				}
41				memcpy( &mid, ack + 1, sizeof( u_int32_t ) );
42				mp = findmsgrec( mid );
43				if ( mp != NULL )
44				{
45				untimeout( mp->id );	/* Îòìåíèòü òàéìåð. */
46				freemsgrec( mp ); /* Óäàëèòü ñîõðàíåííîå ñîîáùåíèå. */
47				}
48			}
49			if ( FD_ISSET( 0, &readmask ) )
50			{
51				mp = getfreerec();
52				rc = read( 0, mp->pkt.buf, sizeof( mp->pkt.buf ) );
53				if ( rc < 0 )
54					error( 1, errno, "îøèáêà âûçîâà read" );
55				mp->pkt.buf[ rc ] = '\0';
56				mp->pkt.cookie = msgid++;
57				mp->pkt.len = htonl( sizeof( u_int32_t ) + rc );
58				if ( send( s, &mp->pkt,
59					 2 * sizeof( u_int32_t ) + rc, 0 ) < 0 )
60					error( 1, errno, "îøèáêà âûçîâà send" );
61				mp->id = timeout( ( tofunc_t )lost_ACK, mp, T1 );
62			}
63		}
64 }
——————————————————————————————————————————————————————————xout3.c
