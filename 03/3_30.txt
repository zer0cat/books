————————————————————————————————————————————————————————————smb.c
 1 #include <sys/shm.h>
 2 #include <sys/sem.h>
 3 #define MUTEX_KEY	0x534d4253	/* SMBS */
 4 #define SM_KEY	0x534d424d	/* SMBM */
 5 #define lock_buf()	if ( semop( mutex, &lkbuf, 1 ) < 0 ) \
 6						error( 1, errno, "îøèáêà âûçîâà semop" )
 7 #define unlock_buf()	if ( semop( mutex, &unlkbuf, 1 ) < 0 ) \
 8						error( 1, errno, "îøèáêà âûçîâà semop" )
 9 int mutex;
10 struct sembuf lkbuf;
11 struct sembuf unlkbuf;
12 void init_smb( int init_freelist )
13 {
14		union semun arg;
15		int smid;
16		int i;
17		int rc;
18		lkbuf.sem_op = -1;
19		lkbuf.sem_flg = SEM_UNDO;
20		unlkbuf.sem_op = 1;
21		unlkbuf.sem_flg = SEM_UNDO;
22		mutex = semget( MUTEX_KEY, 1,
23			IPC_EXCL | IPC_CREAT | SEM_R | SEM_A );
24		if ( mutex >= 0 )
25		{
26			arg.val = 1;
27			rc = semctl( mutex, 0, SETVAL, arg );
28			if ( rc < 0 )
29				error( 1, errno, "semctl failed" );
30		}	
31		else if ( errno == EEXIST )
32		{
33			mutex = semget( MUTEX_KEY, 1, SEM_R | SEM_A );
34			if ( mutex < 0 )
35				error( 1, errno, "îøèáêà âûçîâà semctl" );
36		}
37		else
38			error( 1, errno, "îøèáêà âûçîâà semctl" );
39		smid = shmget( SM_KEY, NSMB * sizeof( smb_t ) + sizeof( int ),
40			SHM_R | SHM_W | IPC_CREAT );	
41		if ( smid < 0 )
42			error( 1, errno, "îøèáêà âûçîâà shmget" );
43		smbarray = ( smb_t * )shmat( smid, NULL, 0 );
44		if ( smbarray == ( void * )-1 )
45			error( 1, errno, "îøèáêà âûçîâà shmat" );
46		if ( init_freelist )
47		{
48			for ( i = 0; i < NSMB - 1; i++ )
49				smbarray[ i ].nexti = i + 1;
50			smbarray[ NSMB - 1 ].nexti = -1;
51			FREE_LIST = 0;
52		}
53 }
————————————————————————————————————————————————————————————smb.c