——————————————————————————————————————————————————————————count.c
 1 #include "etcp.h"
 2 int main( int argc, char **argv )
 3 		{
 4		SOCKET s;
 5		SOCKET s1;
 6		int rc;
 7		int len;
 8		int counter = 1;
 9		char buf[ 120 ];
10		INIT();
11		s = tcp_server( NULL, argv[ 1 ] );
12		s1 = accept( s, NULL, NULL );
13		if ( !isvalidsock( s1 ) )
14			error( 1, errno, "îøèáêà âûçîâà accept" );
15		while ( ( rc = readline( s1, buf, sizeof( buf ) ) ) > 0 )
16		{
17			sleep( 5 );
18			len = sprintf( buf, "ïîëó÷åíî ñîîáùåíèå %d\n", counter++ );
19			rc = send( s1, buf, len, 0 );
20			if ( rc < 0 )
21				error( 1, errno, "îøèáêà âûçîâà send" );
22		}
23		EXIT( 0 );
24 }
——————————————————————————————————————————————————————————count.c