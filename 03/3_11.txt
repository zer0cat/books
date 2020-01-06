—————————————————————————————————————————————————————————tcpmux.c
 1 void reaper( int sig )
 2 {
 3		int waitstatus;
 4		while ( waitpid( -1, &waitstatus, WNOHANG ) > 0 ) {;}
 5 }
—————————————————————————————————————————————————————————tcpmux.c