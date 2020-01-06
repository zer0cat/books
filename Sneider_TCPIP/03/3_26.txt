—————————————————————————————————————————————————————connectto1.c
 1 int isconnected( SOCKET s, fd_set *rd, fd_set *wr, fd_set *ex )
 2 {
 3		int err;
 4		int len = sizeof( err );
 5		errno = 0;			/* Οπεδοξλΰγΰεμ, χςξ ξψθακθ νες. */
 6		if ( !FD_ISSET( s, rd ) && !FD_ISSET( s, wr ) )
 7			return 0;
 8		if ( getsockopt( s, SOL_SOCKET, SO_ERROR, &err, &len ) < 0 )
 9			return 0;
10		errno = err;		/* Ερλθ μϋ νε ρξεδθνθλθρό. */
11		return err == 0;
12 }
—————————————————————————————————————————————————————connectto1.c
