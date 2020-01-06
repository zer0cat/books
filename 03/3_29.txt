————————————————————————————————————————————————————————————smb.c
 1 void smbsend( SOCKET s, void *b )
 2 {
 3		int index;
 4		index = ( smb_t * )b - smbarray;
 5		if ( send( s, ( char * )&index, sizeof( index ), 0 ) < 0 )
 6			error( 1, errno, "smbsend: îøèáêà âûçîâà send" );
 7 }
 8 void *smbrecv( SOCKET s )
 9 {
10		int index;
11		int rc;
12		rc = readn( s, ( char * )&index, sizeof( index ) );
13		if ( rc == 0 )
14			error( 1, 0, "smbrecv: äðóãîé êîíåö îòñîåäèíèëñÿ\n" );
15		else if ( rc != sizeof( index ) )
16			error( 1, errno, "smbrecv: îøèáêà âûçîâà readn" );
17		return smbarray + index;
18 }
—————————————————————————————————————————————————————————————smb.c
