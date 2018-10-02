#ifndef _PROG1_SERVER_H_
#define _PROG1_SERVER_H_

#include <stdlib.h>

#define EMPTY NULL

void playGame(int sd2);
void checkWord(char *cWord, char *word, char letterBuf, unsig8_t *guessRem);

#endif
