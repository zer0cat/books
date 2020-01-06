——————————————————————————————————————————————————————badclient.c
 1 #include "etcp.h"
 2 int main( int argc, char **argv )
 3 {
 4		SOCKET s;
 5		fd_set readmask;
 6		fd_set allreads;
 7		int rc;
 8		int len;
 9		char lin[ 1024 ];
10		char lout[ 1024 ];
11		INIT();
12		s = tcp_client( argv[ optind ], argv[ optind + 1 ] );
13		FD_ZERO( &allreads );
14		FD_SET( 0, &allreads );
15		FD_SET( s, &allreads );
16		for ( ;; )
17		{
18			readmask = allreads;
19			rc = select( s + 1, &readmask, NULL, NULL, NULL );
20			if ( rc <= 0 )
21				error( 1, errno, "select âåðíóëà (%d)", rc );
22			if ( FD_ISSET( s, &readmask ) )
23			{
24				rc = recv( s, lin, sizeof( lin ) - 1, 0 );
25				if ( rc < 0 )
26					error( 1, errno, "îøèáêà âûçîâà recv" );
27				if ( rc == 0 )
28					error( 1, 0, "ñåðâåð îòñîåäèíèëñÿ\n" );
29				lin[ rc ] = '\0';
30				if ( fputs( lin, stdout ) )
31					error( 1, errno, "îøèáêà âûçîâà fputs" );
32			}
33			if ( FD_ISSET( 0, &readmask ) )
34		{
35				if ( fgets( lout, sizeof( lout ), stdin ) == NULL )
36				{
37					if ( shutdown( s, 1 ) )
38						error( 1, errno, "îøèáêà âûçîâà shutdown" );
39				}
40				else
41				{
42					len = strlen( lout );
43					rc = send( s, lout, len, 0 );
44					if ( rc < 0 )
45						error( 1, errno, "îøèáêà âûçîâà send" );
46					}
47			}
48		}
49 }
——————————————————————————————————————————————————————badclient.c