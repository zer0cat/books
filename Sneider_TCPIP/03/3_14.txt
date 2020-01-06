————————————————————————————————————————————————————————tselect.c
 1 #include "etcp.h"
 2 #define NTIMERS 25
 3 typedef struct tevent_t tevent_t;
 4 struct tevent_t
 5 {
 6		tevent_t *next;
 7		struct timeval tv;
 8		void ( *func )( void * );
 9		void *arg;
10		unsigned int id;
11 };
12 static tevent_t *active = NULL;	/* Àêòèâíûå òàéìåğû. */
13 static tevent_t *free_list = NULL;	/* Íåàêòèâíûå òàéìåğû. */
————————————————————————————————————————————————————————tselect.c
