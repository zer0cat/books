——————————————————————————————————————————————————————shutdownc.c
 1 	#include "etcp.h"
 2 	int main( int argc, char **argv )
 3 	{
 4		SOCKET s;
 5		fd_set readmask;
 6		fd_set allreads;
 7		int rc;
 8		int len;
 9		int c;
10		int closeit = FALSE;
11		int err = FALSE;
12		char lin[ 1024 ];
13		char lout[ 1024 ];
14		INIT();
15		opterr = FALSE;
16		while ( ( c = getopt( argc, argv, "c" ) ) != EOF )
17		{
18			switch( c )
19			{
20				case 'c' :
21					closeit = TRUE;
22					break;
23				case '?' :
24					err = TRUE;
25			}
26		}
27		if ( err || argc - optind != 2 )
28			error( 1, 0, "Ïîðÿäîê âûçîâà: %s [-c] õîñò ïîðò\n",
29				program_name );
30		s = tcp_client( argv[ optind ], argv[ optind + 1 ] );
31		FD_ZERO( &allreads );
32		FD_SET( 0, &allreads );
33		FD_SET( s, &allreads );
34		for ( ;; )
35		{
36			readmask = allreads;
37			rc = select( s + 1, &readmask, NULL, NULL, NULL );
38			if ( rc <= 0 )
39				error( 1, errno, "îøèáêà: select âåðíóë (%d)", rc );
40			if ( FD_ISSET( s, &readmask ) )
41			{
42				rc = recv( s, lin, sizeof( lin ) - 1, 0 );
43				if ( rc < 0 )
44					error( 1, errno, "îøèáêà âûçîâà recv" );
45				if ( rc == 0 )
46					error( 1, 0, "ñåðâåð îòñîåäèíèëñÿ\n" );
47				lin[ rc ] = '\0';
48				if ( fputs( lin, stdout ) == EOF )
49					error( 1, errno, "îøèáêà âûçîâà fputs" );
50			}
51			if ( FD_ISSET( 0, &readmask ) )
52			{
53				if ( fgets( lout, sizeof( lout ), stdin ) == NULL )
54				{
55					FD_CLR( 0, &allreads );
56					if ( closeit )
57					{
58						CLOSE( s );
59						sleep( 5 );
60						EXIT( 0 );
61					}
62					else if ( shutdown( s, 1 ) )
63						error( 1, errno, "îøèáêà âûçîâà shutdown" );
64				}
65				else
66				{
67					len = strlen( lout );
68					rc = send( s, lout, len, 0 );
69					if ( rc < 0 )
70						error( 1, errno, "îøèáêà âûçîâà send" );
71				}
72			}
73		}
74 }
——————————————————————————————————————————————————————shutdownc.c