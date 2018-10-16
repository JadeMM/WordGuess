#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


/**********************************************************************
*   Socket creation code taken from demo_client.c
***********************************************************************/

int main( int argc, char **argv) {
	struct hostent *ptrh; /* pointer to a host table entry */
	struct protoent *ptrp; /* pointer to a protocol table entry */
	struct sockaddr_in sad; /* structure to hold an IP address */
	int sd; /* socket descriptor */
	int port; /* protocol port number */
	char *host; /* pointer to host name */
	char buf[1000] = {0}; /* buffer for data from the server */

	memset((char *)&sad,0,sizeof(sad)); /* clear sockaddr structure */
	sad.sin_family = AF_INET; /* set family to Internet */

	if( argc != 3 ) {
		fprintf(stderr,"Error: Wrong number of arguments\n");
		fprintf(stderr,"usage:\n");
		fprintf(stderr,"./client server_address server_port\n");
		exit(EXIT_FAILURE);
	}

	port = atoi(argv[2]); /* convert to binary */
	if (port > 0) { /* test for legal value */
		sad.sin_port = htons((u_short)port);
	} else {
		fprintf(stderr,"Error: bad port number %s\n",argv[2]);
		exit(EXIT_FAILURE);
	}

	host = argv[1]; /* if host argument specified */

	/* Convert host name to equivalent IP address and copy to sad. */
	ptrh = gethostbyname(host);
	if ( ptrh == NULL ) {
		fprintf(stderr,"Error: Invalid host: %s\n", host);
		exit(EXIT_FAILURE);
	}

	memcpy(&sad.sin_addr, ptrh->h_addr, ptrh->h_length);

	/* Map TCP transport protocol name to protocol number. */
	if ( ((long int)(ptrp = getprotobyname("tcp"))) == 0) {
		fprintf(stderr, "Error: Cannot map \"tcp\" to protocol number");
		exit(EXIT_FAILURE);
	}

	/* Create a socket. */
	sd = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
	if (sd < 0) {
		fprintf(stderr, "Error: Socket creation failed\n");
		exit(EXIT_FAILURE);
	}

	/* Connect the socket to the specified server. You have to pass correct parameters to the connect function.*/
	if (connect(sd, (struct sockaddr*) &sad, sizeof(sad)) < 0) {
		fprintf(stderr,"connect failed\n");
		exit(EXIT_FAILURE);
	}

	/* Repeatedly read data from socket and write to user's screen. */
	uint8_t guessesRem[1];
	char guess[1];
	recv(sd, guessesRem, 1, MSG_WAITALL);
	int lengthWord = guessesRem[0];

	// Keep getting data from server to play guessing game
	while (guessesRem[0] != 0 && guessesRem[0] != 255) {
		recv(sd, buf, lengthWord, MSG_WAITALL);
		printf("Board: %s (%u guesses left)\n", buf, guessesRem[0]);
		printf("Enter guess: ");
		// Get letter from user
		scanf("%s", guess);

		send(sd, guess, 1, 0);
		recv(sd, guessesRem, 1, MSG_WAITALL);
		printf("\n");
	}
	// Get final board status
	recv(sd, buf, lengthWord, MSG_WAITALL);
	printf("Board: %s\n", buf);

	if(guessesRem[0] == 0){
		printf("You lost\n");
	}else{
		printf("You win\n");
	}
	close(sd);

	exit(EXIT_SUCCESS);
}
