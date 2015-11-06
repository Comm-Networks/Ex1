#include "nm_protocol.h"
#include <stdlib.h>


int recall(int sockfd,void* buff,unsigned int *len){
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

int sendall(int sockfd,void* buff,unsigned int *len){
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

	int sockfd;
	unsigned int size;

	client_msg c_msg;
	server_msg s_msg;
	after_move_msg am_msg;


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
	my_addr.sin_port = htons(port);

	  /* resolve hostname */
	struct hostent* host;
	if ((host = gethostbyname(hostname)) == NULL) {
		// Error.
		return -1;
	}
	memcpy(&my_addr.sin_addr, host->h_addr_list[0], host->h_length);

	size = sizeof(my_addr);
	bind(sockfd, (const struct sockaddr*)&my_addr, size);

	listen(sockfd, 5);

	size = sizeof(struct sockaddr_in);
	int new_sock = accept(sockfd, (struct sockaddr*)&client_adrr, &size);
	if (new_sock < 0) {
		// Error.
	}

	int result;

	// Message loop.
	while (1) {
		size = sizeof(s_msg);
		result = sendall(new_sock, &s_msg, &size);
		if (result < 0) {
			// Error.
		}

		// Ending loop if we had a winner in previous iteration.
		if (s_msg.winner != 'n') {
			return 0;
		}

		size = sizeof(c_msg);
		result = recall(new_sock, &c_msg, &size);
		if (result < 0) {
			// Error.
		}

		// Do move.
		am_msg.legal = 'g';
		if (c_msg.num_cubes_to_remove > 0) {
			if ((c_msg.heap_name == 'a') && (c_msg.num_cubes_to_remove <= s_msg.n_a)) {
				s_msg.n_a = s_msg.n_a - c_msg.num_cubes_to_remove;
			} else if ((c_msg.heap_name == 'b') && (c_msg.num_cubes_to_remove <= s_msg.n_b)) {
				s_msg.n_b = s_msg.n_b - c_msg.num_cubes_to_remove;
			} else if ((c_msg.heap_name == 'c') && (c_msg.num_cubes_to_remove <= s_msg.n_c)) {
				s_msg.n_c = s_msg.n_c - c_msg.num_cubes_to_remove;
			} else {
				// Illegal move (trying to get more cubes than available).
				am_msg.legal = 'b';
			}
		} else {
			// Illegal move (number of cubes to remove not positive).
			am_msg.legal = 'b';
		}

		size = sizeof(am_msg);
		result = sendall(new_sock, &am_msg, &size);
		if (result < 0) {
			// Error.
		}

		if (am_msg.legal == 'g') {
			// Client made a legal move, check if he won.
			if ((s_msg.n_a == 0) && (s_msg.n_b == 0) && (s_msg.n_c == 0)) {
				// Client won.
				s_msg.winner = 'c'; // Letting the client know.

			} else {
				// Making server move.
				if ((s_msg.n_a >= s_msg.n_b) && (s_msg.n_a >= s_msg.n_c)) {
					s_msg.n_a = s_msg.n_a - 1;

				} else if ((s_msg.n_b >= s_msg.n_c)) {
					s_msg.n_b = s_msg.n_b - 1;

				} else {
					s_msg.n_c = s_msg.n_c - 1;
				}

				// Checking if server won.
				if ((s_msg.n_a == 0) && (s_msg.n_b == 0) && (s_msg.n_c == 0)) {
					// Server won.
					s_msg.winner = 's'; // Letting the client know.
				}
			}
		}
	}
}

