#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>


#define QLEN 6 /* size of request queue */

// Find all occurances of guessed letter in cWord and return new cWord
char* checkWord(char *cWord, char *word, char letterBuf, uint8_t *guessRem,
		uint8_t *unguessedLet) {

	// Create new ptr and copy address to manipulate word
	char *newCWord = strdup(cWord);
	cWord = newCWord;
	int correctGuess = 0;
	int size = strlen(word);
	// Replace "_" in cWord where letter is correct
	for(int i = 0; i < size; i++){
		if(word[i] == letterBuf) {
			*newCWord = letterBuf;
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

void playGame(int sd2, char **argv) {
	char buf[1000]; /* buffer for string the server sends */
	char letterBuf[1]; /* buffer for users guess */
	char* word = argv[2];
	int wordLen = strlen(word);
	uint8_t unguessedLet = wordLen;
	uint8_t guessBuf[] = {wordLen}; /* buffer for remaining guesses */

	char cWord[wordLen+1];
	for(int i = 0; i<wordLen; i++) {
		cWord[i] = '_';
	}
	cWord[wordLen] = '\0';

	char* newCWord = strdup(cWord);

	int gameOver = 0;
	int usedLetters[26] = {0};

	while(!gameOver) {
		//Send number of guesses remaining to client
		send(sd2, guessBuf, 1, 0);
		// Send unguessed string to client
		sprintf(buf,"%s",newCWord);
		send(sd2, buf, strlen(buf),0);
		int n = recv(sd2, letterBuf, 1, 0);
		if(n == 0) {
		    free(newCWord);
			return;
		}

		//checks if users guess is a valid letter
		if(letterBuf[0] >= 97 && letterBuf[0] <= 123) {
			//checks if letter has already been guessed
			if(usedLetters[letterBuf[0]-97] == 0) {
				usedLetters[letterBuf[0]-97] = 1;
				newCWord = checkWord(newCWord, word, letterBuf[0], guessBuf, &unguessedLet);
			} else {
				guessBuf[0]--;
			}
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
	sprintf(buf,"%s \n",newCWord);
	send(sd2, buf, strlen(buf),0);
	free(newCWord);
}

int main(int argc, char **argv) {
	struct protoent *ptrp; /* pointer to a protocol table entry */
	struct sockaddr_in sad; /* structure to hold server's address */
	struct sockaddr_in cad; /* structure to hold client's address */
	int sd, sd2; /* socket descriptors */
	int port; /* protocol port number */
	int alen; /* length of address */
	int optval = 1; /* boolean value when we set socket option */

	if( argc != 3 ) {
		fprintf(stderr,"Error: Wrong number of arguments\n");
		fprintf(stderr,"usage:\n");
		fprintf(stderr,"./server server_port\n");
		exit(EXIT_FAILURE);
	}

	memset((char *)&sad,0,sizeof(sad)); /* clear sockaddr structure */

	//Set socket family to AF_INET
	sad.sin_family = AF_INET;

	//Set local IP address to listen to all IP addresses this server can assume. You can do it by using INADDR_ANY
	sad.sin_addr.s_addr = INADDR_ANY;

	port = atoi(argv[1]);
	if (port > 0) { /* test for legal value */
		sad.sin_port = htons((u_short)port);
	} else {
		fprintf(stderr,"Error: bad port number %s\n",argv[1]);
		exit(EXIT_FAILURE);
	}

	/* Map TCP transport protocol name to protocol number */
	if ( ((long int)(ptrp = getprotobyname("tcp"))) == 0) {
		fprintf(stderr, "Error: Cannot map \"tcp\" to protocol number");
		exit(EXIT_FAILURE);
	}

	/* Create a socket with AF_INET as domain, protocol type as SOCK_STREAM, and protocol as ptrp->p_proto. This call returns a socket descriptor named sd. */
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

	/*  Bind a local address to the socket. For this, you need to pass correct parameters to the bind function. */
	if (bind(sd, (struct sockaddr*) &sad, sizeof(sad)) < 0) {
		fprintf(stderr,"Error: Bind failed\n");
		exit(EXIT_FAILURE);
	}

	/* Specify size of request queue.*/
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
			playGame(sd2, argv);
			close(sd2);
			return 0;
		}
		close(sd2);
	}
}
