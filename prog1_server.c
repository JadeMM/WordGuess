#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "prog1_server.h"

#define QLEN 6 /* size of request queue */

int main(int argc, char **argv) {
	struct protoent *ptrp; /* pointer to a protocol table entry */
	struct sockaddr_in sad; /* structure to hold server's address */
	struct sockaddr_in cad; /* structure to hold client's address */
	int sd, sd2; /* socket descriptors */
	int port; /* protocol port number */
	int alen; /* length of address */
	int optval = 1; /* boolean value when we set socket option */

	memset((char *)&sad,0,sizeof(sad)); /* clear sockaddr structure */

	//TODO: Set socket family to AF_INET
	sad.sin_family = AF_INET;

	//TODO: Set local IP address to listen to all IP addresses this server can assume. You can do it by using INADDR_ANY
	sad.sin_addr.s_addr = INADDR_ANY;

	port = atoi(argv[1]);
	if (port > 0) /* test for legal value */
		sad.sin_port = htons((u_short)port);
	else {
		fprintf(stderr,"Error: bad port number %s\n",argv[1]);
		exit(EXIT_FAILURE);
	}

	/* Map TCP transport protocol name to protocol number */
	if ( ((long int)(ptrp = getprotobyname("tcp"))) == 0) {
		fprintf(stderr, "Error: Cannot map \"tcp\" to protocol number");
		exit(EXIT_FAILURE);
	}

	/* TODO: Create a socket with AF_INET as domain, protocol type as SOCK_STREAM, and protocol as ptrp->p_proto. This call returns a socket descriptor named sd. */

	sd = socket(AF_INET, SOCK_STREAM, ptrp->p_proto);
	if (sd < 0) {
		fprintf(stderr, "Error: Socket creation failed\n");
		exit(EXIT_FAILURE);
	}

	/* Allow reuse of port - avoid "Bind failed" issues */
	if( setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0 ) {
		fprintf(stderr, "Error Setting socket option failed\n");
		exit(EXIT_FAILURE);
	}

	/* TODO: Bind a local address to the socket. For this, you need to pass correct parameters to the bind function. */
	if (bind(sd, (struct sockaddr*) &sad, sizeof(sad)) < 0) {
		fprintf(stderr,"Error: Bind failed\n");
		exit(EXIT_FAILURE);
	}

	/* TODO: Specify size of request queue.*/
	if (listen(sd, QLEN) < 0) {
		fprintf(stderr,"Error: Listen failed\n");
		exit(EXIT_FAILURE);
	}

	/* Main server loop - accept and handle requests */
	while (1) {
		alen = sizeof(cad);
		if ((sd2 = accept(sd, (struct sockaddr *)&cad, &alen)) < 0) {
			fprintf(stderr, "Error: Accept failed\n");
			exit(EXIT_FAILURE);
		}
		int childDesc = fork();
		if(childDesc == 0) {
			playGame(sd2);
			close(sd2);
			return 0;
		}
		/*sprintf(buf,"This server has been contacted %d time%s\n",visits,visits==1?".":"s.");
		send(sd2, buf, strlen(buf),0);*/
		close(sd2);
	}
}

	void playGame(int sd2) {
		char buf[1000]; /* buffer for string the server sends */
		uint8_t guessBuf[] = {6}; /* buffer for remaining guesses */
		char letterBuf[1]; /* buffer for users guess */
		char* word = "banana";
		int wordLen = strlen(word);
		uint8_t unguessedLet = 6;
		char *cWord = "______";
		int gameOver = 0;
		int usedLetters[26] = {0};

		while(!gameOver) {
			//Send number of guesses remaining to client
			send(sd2, guessBuf, 1, 0);
			// Send unguessed string to client
			sprintf(buf,"%s",cWord);
			send(sd2, buf, strlen(buf),0);
			int n = recv(sd2, letterBuf, 1, 0);
			printf("Got %d bytes\n", n);
			if(n == 0) {
				return;
			}

			//checks if letter has already been guessed
			if(usedLetters[letterBuf[0]-97] == 0) {
				usedLetters[letterBuf[0]-97] = 1;
				cWord = checkWord(cWord, word, letterBuf, guessBuf, &unguessedLet);
			} else {
				guessBuf[0]--;
			}

			if(guessBuf[0] == 0 || unguessedLet == 0){
				gameOver = 1;
			}
		}
		if(guessBuf[0] != 0){
			// Set guessBuff to 255 to signify win
			guessBuf[0] = 255;
		}
		send(sd2, guessBuf, 1, 0);
		// Send board one last time
		sprintf(buf,"%s \n",cWord);
		send(sd2, buf, strlen(buf),0);
	}

	// Find all occurances of guessed letter in cWord and return new cWord
	char* checkWord(char *cWord, char *word, char *letterBuf, uint8_t *guessRem,
		 	uint8_t *unguessedLet) {

		// Create new ptr and copy address to manipulate word
		char *newCWord = strdup(cWord);
		cWord = newCWord;
		int correctGuess = 0;
		int size = strlen(word);
		// Replace "_" in cWord where letter is correct
		for(int i = 0; i < size; i++){
			if(word[i] == letterBuf[0]) {
				*newCWord = letterBuf[0];
				(*unguessedLet)--;
				correctGuess = 1;
			}
			newCWord++;
		}

		if(!correctGuess) {
			(*guessRem)--;
		}
		return cWord;
	}
