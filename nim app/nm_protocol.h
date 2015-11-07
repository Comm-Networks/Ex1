#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

//message from the server - start of a turn
#pragma pack(push, 1)
	typedef struct {
		short n_a; /* heap A size */
		short n_b; /* heap B size */
		short n_c; /* heap C size */
		char  winner; /* s-server/ c-client/ n- no winner yet */
	}server_msg;
#pragma pack(pop)

#pragma pack(push, 1)
	typedef struct  {
		short num_cubes_to_remove;
		char  heap_name; /* the heap we remove cubes from*/
	}client_msg;
#pragma pack(pop)

#pragma pack(push, 1)
	typedef struct  {
		char legal ; // g - good b- illegal
	}after_move_msg;
#pragma pack(pop)

#pragma pack(push,1)
	typedef struct {
		char close ;
	}close_msg;
#pragma pack(pop)
