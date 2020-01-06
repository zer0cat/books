————————————————————————————————————————————————————————tcpsink.c
 1 static void server( SOCKET s, int rcvbufsz )
 2 {
 3		char *buf;
 4		int rc;
 5		int bytes = 0;
 6		if ( ( buf = malloc( rcvbufsz ) ) == NULL )
 7			error( 1, 0, "îøèáêà âûçîâà malloc\n" );
 8		for ( ;; )
 9		{
10			rc = recv( s, buf, rcvbufsz, 0 );
11			if ( rc <= 0 )
12				break;
13		bytes += rc;
14	}
15	error( 0, 0, "ïîëó÷åíî áàéò: %d\n", bytes );
16 }
————————————————————————————————————————————————————————tcpsink.c