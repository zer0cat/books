——————————————————————————————————————————————————————————tcprw.c
 1 #include "etcp.h"
 2 int main( int argc, char **argv )
 3 {
 4		SOCKET s;
 5		int rc;
 6		int len;
 7		char buf[ 120 ];
 8		INIT();
 9		s = tcp_client( argv[ 1 ], argv[ 2 ] );
10		while ( fgets( buf, sizeof( buf ), stdin ) != NULL )
11		{
12			len = strlen( buf );
13			rc = send( s, buf, len, 0 );
14			if ( rc < 0 )
15				error( 1, errno, "îøèáêà âûçîâà send" );
16			rc = readline( s, buf, sizeof( buf ) );
17			if ( rc < 0 )
18				error( 1, errno, "îøèáêà âûçîâà readline" );
19			else if ( rc == 0 )
20				error( 1, 0, "ñåðâåð çàâåðøèë ðàáîòó\n" );
21			else
22				fputs( buf, stdout );
23		}
24		EXIT( 0 );
25 }
——————————————————————————————————————————————————————————tcprw.c