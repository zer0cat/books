—————————————————————————————————————————————————————————tcpmux.c
 1 static void parsetab( void )
 2 {
 3		FILE *fp;
 4		servtab_t *stp = service_table;
 5		char *cp;
 6		int i;
 7		int lineno;
 8		char line[ MAXLINE ];
 9		fp = fopen( CONFIG, "r" );
10		if ( fp == NULL )
11			error( 1, errno, "не могу открыть %s", CONFIG );
12		lineno = 0;
13		while ( fgets( line, sizeof( line ), fp ) != NULL )
14		{	
15			lineno++;
16			if ( line[ strlen( line ) - 1 ] != '\n' )
17				error( 1, 0, "строка %d слишком длинная\n", lineno );
18			if ( stp >= service_table + NSERVTAB )
19				error( 1, 0, "слишком много строк в tcpmux.conf\n" );
20			cp = strchr( line, '#' );
21			if ( cp != NULL )
22				*cp = '\0';
23			cp = strtok( line, " \t\n" );
24			if ( cp == NULL )
25				continue;
26			if ( *cp == '+' )
27			{
28				stp->flag = TRUE;
29				cp++;
30				if ( *cp == '\0' || strchr( " \t\n", *cp ) != NULL )
31					error( 1, 0, "строка %d: пробел после ‘+‘\n",
32						lineno );
33			}
34			stp->service = strdup( cp );
35			if ( stp->service == NULL )
36				error( 1, 0, "не хватило памяти\n" );
37			cp = strtok( NULL, " \t\n" );
38			if ( cp == NULL )
39				error( 1, 0, "строка %d: не задан путь (%s)\n",
40					lineno, stp->service );
41			stp->path = strdup( cp );
42			if ( stp->path == NULL )
43				error( 1, 0, "не хватило памяти\n" );
44			for ( i = 0; i < MAXARGS; i++ )
45			{
46				cp = strtok( NULL, " \t\n" );
47				if ( cp == NULL )
48					break;
49				stp->args[ i ] = strdup( cp );
50				if ( stp->args[ i ] == NULL )
51					error( 1, 0, "не хватило памяти\n" );
52			}
53			if ( i >= MAXARGS && strtok( NULL, " \t\n" ) != NULL )
54			error( 1, 0, "строка %d: слишком много аргументов (%s)\n",
55				lineno, stp->service );
56			stp->args[ i ] = NULL;
57			stp++;
58		}
59		stp->service = NULL;
60		fclose ( fp );
61 }
—————————————————————————————————————————————————————————tcpmux.c