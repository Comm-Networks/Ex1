#include "nm_protocol.h"

#define INPUT_SIZE 2

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

/*
* Recieving an input as 2 word string array
*/
char** getInput() {
	char * input[INPUT_SIZE];
	int i = 0, j = 1, k = 0;
	char* word = calloc(1, sizeof(char));
	assert(word!=NULL);
	char c = '\0';
	while (c != '\n' || k>1)
	{
		c = getchar();
		if (c == ' ' && word != '\0') {
			word[i] = '\0';
			input[k++] = word;
			word = calloc(1, sizeof(char));
			assert(word!=NULL);
			j = 1, i = 0;
			continue;

		}
		else if (c == ' ') {
			continue;
		}
		word = (char*)realloc(word, (++j)*sizeof(char));
		assert(word!=NULL);
		word[i] = c;
		i++;
	}
	if (k<1) {
		word[i - 1] = '\0';
		input[k] = word;
	}
	return input;

}

int main(int argc , char** argv){
	char * hostname = "localhost";
	char* port="6444";
	struct addrinfo  hints;
	struct addrinfo * dest_addr , *rp;
	int sockfd;
	int i;
	client_msg c_msg;
	server_msg s_msg;
	after_move_msg am_msg;
	if (argc==3) {
		hostname=argv[1];
		port=argv[2];
	}
	else if (argc==2) {
		hostname=argv[1];
	}

	// Obtain address(es) matching host/port
	memset(&hints,0,sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;
	hints.ai_protocol = 0;
	int status =getaddrinfo(hostname,port,&hints,&dest_addr);
	if (status!=0){
		 printf("getaddrinfo error: %s\n", strerror(status));
		 exit(1);
	}

	// loop through all the results and connect to the first we can
	for (rp=dest_addr ; rp!=NULL ; rp=rp->ai_next){
		if (rp->ai_family!=AF_INET) {
			continue;
		}
		//opening a new socket
		sockfd = socket(hints.ai_family,hints.ai_socktype,hints.ai_protocol);
		if (sockfd==-1) {
			continue;
		}
		if (connect(sockfd,dest_addr->ai_addr,dest_addr->ai_addrlen)!=-1){
			break ; //successfuly connected
		}
		close(sockfd);
	}
	// No address succeeded
	 if (rp == NULL) {
	        fprintf(stderr, "Client:failed to connect\n");
	        close(sockfd);
	        exit(1);
	    }
	freeaddrinfo(dest_addr);

	for (;;) {

		//getting heap info and winner info from server
		if (recvall(sockfd,(void *) s_msg,sizeof(server_msg))<0){
			fprintf(stderr, "Client:failed to read from server\n");
			close(sockfd);
			exit(1);
		}
		printf("Heap A: %d\nHeap B: %d\nHeap C: %d\n",s_msg.n_a,s_msg.n_b,s_msg.n_c);

		if (s_msg.winner!='n') {
			char * winner = s_msg.winner=='c' ? "You" : "Server" ;
			printf("%s win!\n",winner);
			break;
		}
		else {
			printf("Your turn:");
			char * input[INPUT_SIZE]=getInput();
			if (input[0]=="Q"){
				free(input[0]);
				free(input[1]);
				break;
			}
			//sending the move to the server
			c_msg.heap_name=*input[0];
			c_msg.num_cubes_to_remove=(short) strtol(input[1],NULL,2);
			if (sendall(sockfd,(void *)c_msg,sizeof(c_msg))<0){
				fprintf(stderr, "Client:failed to write to server\n");
				free(input[0]);
				free(input[1]);
				break;
			}
			//reciving from server a message if that was a legal move or not
			if (recvall(sockfd,(void *) am_msg,sizeof(am_msg))<0){
				fprintf(stderr, "Client:failed to read from server\n");
				free(input[0]);
				free(input[1]);
				break;
			}
			char * msg = am_msg.legal=='b' ? "Illegal move\n" : "Move accepted\n";
			printf("%s",msg);
			free(input[0]);
			free(input[1]);
		}
	}
	close(sockfd);
	return 0;
}
