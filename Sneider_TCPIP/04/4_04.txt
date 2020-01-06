———————————————————————————————————————————————————————————icmp.c
 1 static void print_dg( char *dg, int len )
 2 {
 3		struct ip *ip;
 4		struct icmp *icmp;
 5		struct hostent *hp;
 6		char *hname;
 7		int hl;
 8		static char *redirect_code[] =
 9		{
10			"сеть", "хост",
11			"тип сервиса и сеть", "тип сервиса и хост"
12		};
13		static char *timexceed_code[] =
14		{
15			"транзите", "сборке"
16		};
17		static char *param_code[] =
18		{
19			"Плохой IP-заголовок", "Нет обязательной опции"
20		};
21		ip = ( struct ip * )dg;
22		if ( ip->ip_v != 4 )
23		{
24			error( 0, 0, "IP-датаграмма не версии 4\n" );
25			return;
26		}
27		hl = ip->ip_hl << 2;	/* Длина IP-заголовка в байтах. */
28		if ( len < hl + ICMP_MINLEN )
29		{
30			error( 0, 0, "short datagram (%d bytes) from %s\n",
31				len, inet_ntoa( ip->ip_src ) );
32			return;
33		}
34		hp = gethostbyaddr( ( char * )&ip->ip_src, 4, AF_INET );
35		if ( hp == NULL )
36			hname = "";
37		else
38			hname = hp->h_name;
39		icmp = ( struct icmp * )( dg + hl );  /* ICMP-пакет. */
40		printf( "ICMP %s (%d) от %s (%s)\n",
41			get_type( icmp->icmp_type ),
42			icmp->icmp_type, hname, inet_ntoa( ip->ip_src ) );
43		if ( icmp->icmp_type == ICMP_UNREACH )
44			print_unreachable( icmp );
45		else if ( icmp->icmp_type == ICMP_REDIRECT )
46			printf( "\tПеренаправление на %s\n", icmp->icmp_code <= 3 ?
47				redirect_code[ icmp->icmp_code ] : "Некорректный код" );
48		else if ( icmp->icmp_type == ICMP_TIMXCEED )
49			printf( "\tTTL == 0 при %s\n", icmp->icmp_code <= 1 ?
50			timexceed_code[ icmp->icmp_code ] : "Некорректный код" );
51		else if ( icmp->icmp_type == ICMP_PARAMPROB )
52			printf( "\t%s\n", icmp->icmp_code <= 1 ?
53				param_code[ icmp->icmp_code ] : "Некорректный код" );
54 }
———————————————————————————————————————————————————————————icmp.c