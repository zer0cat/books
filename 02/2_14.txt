————————————————————————————————————————————————————————————vrs.c
 1 #include "etcp.h"
 2 int main( int argc, char **argv )
 3 {
 4		struct sockaddr_in peer;
 5		SOCKET s;
 6		SOCKET s1;
 7		int peerlen = sizeof( peer );
 8		int n;
 9		char buf[ 10 ];
10		INIT();
11		if ( argc == 2 )
12			s = tcp_server( NULL, argv[ 1 ] );
13		else
14			s = tcp_server( argv[ 1 ], argv[ 2 ] );
15		s1 = accept( s, ( struct sockaddr * )&peer, &peerlen );
16		if ( !isvalidsock( s1 ) )
17			error( 1, errno, "îøèáêà âûçîâà accept" );
18		for ( ;; )
19		{
20			n = readvrec( s1, buf, sizeof( buf ) );
21			if ( n < 0 )
22				error( 0, errno, "readvrec âåðíóëà êîä îøèáêè" );
23			else if ( n == 0 )
24				error( 1, 0, "êëèåíò îòêëþ÷èëñÿ\n" );
25			else
26				write( 1, buf, n );
27		}
28		EXIT( 0 );		/* Ñþäà íå ïîïàäàåì. */
29 }
————————————————————————————————————————————————————————————vrs.c