——————————————————————————————————————————————————————————xout1.c
 1 #include "etcp.h"
 2 int main( int argc, char **argv )
 3 {
 4		fd_set allreads;
 5		fd_set readmask;
 6		SOCKET s;
 7		int rc;
 8		char buf[ 128 ];
 9		INIT();
10		s = tcp_client( argv[ 1 ], argv[ 2 ] );
11		FD_ZERO( &allreads );
12		FD_SET( s, &allreads );
13		FD_SET( 0, &allreads );
14		for ( ;; )
15		{
16			readmask = allreads;
17			rc = select( s + 1, &readmask, NULL, NULL, NULL );
18			if ( rc <= 0 )
19				error( 1, rc ? errno : 0, "select âåðíóë %d", rc );
20			if ( FD_ISSET( 0, &readmask ) )
21			{
22				rc = read( 0, buf, sizeof( buf ) - 1 );
23				if ( rc < 0 )
24					error( 1, errno, "îøèáêà âûçîâà read" );
25				if ( send( s, buf, rc, 0 ) < 0 )
26					error( 1, errno, "îøèáêà âûçîâà send" );
27			}
28			if ( FD_ISSET( s, &readmask ) )
29			{
30				rc = recv( s, buf, sizeof( buf ) - 1, 0 );
31				if ( rc == 0 )
32					error( 1, 0, "ñåðâåð îòñîåäèíèëñÿ\n" );
33				else if ( rc < 0 )
34					error( 1, errno, "îøèáêà âûçîâà recv" );
35				else
36				{
37					buf[ rc ] = '\0';
38					error( 1, 0, "íåîæèäàííûé âõîä [%s]\n", buf );
39				}
40			}
41		}
42 }
——————————————————————————————————————————————————————————xout1.c