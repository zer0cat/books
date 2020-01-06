——————————————————————————————————————————————————————heartbeat.h
 1 #ifndef __HEARTBEAT_H__
 2 #define __HEARTBEAT_H__
 3 #define MSG_TYPE1		1	/* Сообщение прикладного уровня. */
 4 #define MSG_TYPE2		2	/* Еще одно. */
 5 #define MSG_HEARTBEAT	3	/* Сообщение-пульс. */
 6 typedef struct				/* Структура сообщения. */
 7 {
 8	u_int32_t type;				/* MSG_TYPE1, ... */
 9	char data[ 2000 ];
10 } msg_t;
11 #define T1			60	/* Время простоя перед отправкой пульса. */
12 #define T2			10	/* Время ожидания ответа. */
13 #endif  /* __HEARTBEAT_H__ */
——————————————————————————————————————————————————————————heartbeat.h