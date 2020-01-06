————————————————————————————————————————————————————————————smb.c
 1 #define FILENAME	"./smbfile"
 2 #define lock_buf()	if ( WaitForSingleObject( mutex, INFINITE )\
 3					 != WAIT_OBJECT_0 ) \
 4					error( 1, errno, "îøèáêà âûçîâà lock_buf " )
 5 #define unlock_buf()	if ( !ReleaseMutex( mutex ) )\
 6					error( 1, errno, "îøèáêà âûçîâà unlock_buf" )
 7 HANDLE mutex;
 8 void init_smb( int init_freelist )
 9 {
10		HANDLE hfile;
11		HANDLE hmap;
12		int i;
13		mutex = CreateMutex( NULL, FALSE, "smbmutex" );
14		if ( mutex == NULL )
15			error( 1, errno, "îøèáêà âûçîâà CreateMutex" );
16		hfile = CreateFile( FILENAME,
17			GENERIC_READ | GENERIC_WRITE,
18			FILE_SHARE_READ | FILE_SHARE_WRITE,
19			NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
20		if ( hfile == INVALID_HANDLE_VALUE )
21			error( 1, errno, "îøèáêà âûçîâà CreateFile" );
22		hmap = CreateFileMapping( hfile, NULL, PAGE_READWRITE,
23			0, NSMB * sizeof( smb_t ) + sizeof( int ), "smbarray" );
24		smbarray = MapViewOfFile( hmap, FILE_MAP_WRITE, 0, 0, 0 );
25		if ( smbarray == NULL )
26			error( 1, errno, "îøèáêà âûçîâà MapViewOfFile" );
27	
28		if ( init_freelist )
29		{
30			for ( i = 0; i < NSMB - 1; i++ )
31				smbarray[ i ].nexti = i + 1;
32			smbarray[ NSMB - 1 ].nexti = -1;
33			FREE_LIST = 0;
34		}
35 }
————————————————————————————————————————————————————————————smb.c