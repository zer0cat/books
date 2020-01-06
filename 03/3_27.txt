—————————————————————————————————————————————————————connectto1.c
 1 int isconnected( SOCKET s, fd_set *rd, fd_set *wr, fd_set *ex )
 2 {
 3		WSASetLastError( 0 );
 4		if ( !FD_ISSET( s, rd ) && !FD_ISSET( s, wr ) )
 5			return 0;
 6		if ( FD_ISSET( s, ex ) )
 7			return 0;
 8		return 1;
 9 }
—————————————————————————————————————————————————————connectto1.c