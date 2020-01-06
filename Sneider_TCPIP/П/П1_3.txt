—————————————————————————————————————————————————————————signal.c
 /* signal - íàäåæíàÿ âåğñèÿ äëÿ SVR4 è íåêîòîğûõ äğóãèõ ñèñòåì. */
 1 typedef void sighndlr_t( int );
 2 sighndlr_t *signal( int sig, sighndlr_t *hndlr )
 3 {
 4		struct sigaction act;
 5		struct sigaction xact;
 6		act.sa_handler = hndlr;
 7		act.sa_flags = 0;
 8		sigemptyset( &act.sa_mask );
 9		if ( sigaction( sig, &act, &xact ) < 0 )
10			return SIG_ERR;
11		return xact.sa_handler;
12 }
—————————————————————————————————————————————————————————signal.c
