#include "nm_protocol.h"
#include <stdlib.h>


int recall(int sockfd,void* buff,int *len){
	int total = 0; /* how many bytes we've read */
	int bytesleft = *len; /* how many we have left to read */
	int n;
	while(total < *len) {
	   n = recv(sockfd, buff+total, bytesleft, 0);
	   if (n == -1) { break; }
	   total += n;
	   bytesleft -= n;
	}
	*len = total; /* return number actually read here */
	return n == -1 ? -1:0; /*-1 on failure, 0 on success */
}

int sendall(int sockfd,void* buff,int *len){
	int total = 0; /* how many bytes we've sent */
	int bytesleft = *len; /* how many we have left to send */
	int n;
	while(total < *len) {
	   n = send(sockfd, buff+total, bytesleft, 0);
	   if (n == -1) { break; }
	   total += n;
	   bytesleft -= n;
	}
	*len = total; /* return number actually sent here */
	return n == -1 ? -1:0; /*-1 on failure, 0 on success */
}

int main(int argc , char** argv) {
	const char hostname[] = "localhost";
	int port = 6444;
	struct addrinfo  hints;
	struct addrinfo * dest_addr , *rp;
	int sockfd;
	int i;
	client_msg c_msg;
	server_msg s_msg;
	after_move_msg am_msg;


	printf("%d\n", argc);
	if (argc < 4) {
		// Error.
		return -1;
	}

	// Initializing piles.
	s_msg.n_a = (int)strtol(argv[1], NULL, 10);
	s_msg.n_b = (int)strtol(argv[2], NULL, 10);
	s_msg.n_c = (int)strtol(argv[3], NULL, 10);
	s_msg.winner = 'n';

	printf("%d %d %d\n", s_msg.n_a, s_msg.n_b, s_msg.n_c);

	if (argc > 4) {
		port = (int)strtol(argv[4], NULL, 10);
	}

	// Initializing socket.
	sockfd = socket(PF_INET, SOCK_STREAM, 0);

	struct sockaddr_in my_addr,client_adrr;


	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons( port );

	  /* resolve hostname */
	struct hostent* host;
	if ((host = gethostbyname(hostname)) == NULL) {
		// Error.
		return -1;
	}
	memcpy(&my_addr.sin_addr, host->h_addr_list[0], host->h_length);

	bind(sockfd, &my_addr, sizeof(my_addr));

	listen(sockfd, 5);

	int sin_size = sizeof(struct sockaddr_in);
	int new_sock = accept(sockfd, (struct sockaddr*)&client_adrr, &sin_size);
	if (new_sock < 0) {
		// Error.
	}

	int result;

	// Message loop.
	while (1) {
		result = sendall(new_sock, &s_msg, sizeof(s_msg));
		if (result < 0) {
			// Error.
		}

		result = recall(new_sock, &c_msg, sizeof(c_msg));
		if (result < 0) {
			// Error.
		}

		// Do move.
		am_msg.legal = 'g';
		if ((c_msg.heap_name == 'a') && (c_msg.num_cubes_to_remove <= s_msg.n_a)) {
			s_msg.n_a = s_msg.n_a - c_msg.num_cubes_to_remove;
		} else if ((c_msg.heap_name == 'b') && (c_msg.num_cubes_to_remove <= s_msg.n_b)) {
			s_msg.n_b = s_msg.n_b - c_msg.num_cubes_to_remove;
		} else if ((c_msg.heap_name == 'c') && (c_msg.num_cubes_to_remove <= s_msg.n_c)) {
			s_msg.n_c = s_msg.n_c - c_msg.num_cubes_to_remove;
		} else {
			am_msg.legal = 'b';
		}


		result = sendall(new_sock, &am_msg, sizeof(am_msg));
		if (result < 0) {
			// Error.
		}
	}
}

