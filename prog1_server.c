#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

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

		if(int childDesc = fork() == 0) {
			playGame(sd2);
		}

		/*sprintf(buf,"This server has been contacted %d time%s\n",visits,visits==1?".":"s.");
		send(sd2, buf, strlen(buf),0);
		close(sd2);*/
	}
}

	void playGame(int sd2) {
		char buf[1000]; /* buffer for string the server sends */
		char letterBuf[1]; /* buffer for users guess */
		char* word = "banana";
		int wordLen = strlen(word);
		unsig8_t guessRem = 6;
		char* cWord = "______";
		boolean gameOver = false;

		while(!gameOver) {
			sprintf(buf,"Board: %s (%d guesses left)\n",cWord, wordLen)
			send(sd2, buf, strlen(buf),0);
			
			recv(sd, letterBuf, 1, MSG_WAITALL);
			checkWord(&cWord, &word, letterBuf, &guessRem);
		}
	}

	checkWord(char *cWord, char *word, char letterBuf, unsig8_t *guessRem) {
		boolean correctGuess = false;
		int inc = 0;
		while(cWord[inc]) {
			if( word[inc] == letterBuf) {
				cWord[inc] = letterBuf;
				correctGuess = true;
			}
			inc++:
		}
		
		if(!correctGuess) {
			*guessRem--;
		}
	}













