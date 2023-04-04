// TODOs
// Join threads after game end
// Complete the game ruleset

// gcc server.c -o SERVER
// gcc -Wall -lpthread -o SERVER_PTHREAD.exe SERVER_PTHREAD.c
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <wait.h>
#include <pthread.h>
#include "board.h"

#define BUFFER_SIZE 512
#define PORT 1234
#define MAX_GAMES 10

struct client
{
  int clientFileDescr;
  struct sockaddr_in clientAddr;
};

struct players
{
  struct client *white, *black;
};

void clearBuffer(char buffer[BUFFER_SIZE])
{
  // Cleaning the buffer
  int i;
  for (i = 0; i < BUFFER_SIZE; ++i)
    buffer[i] = '\0';
}

const int readFromClient(struct client *c, char buffer[BUFFER_SIZE])
{
  clearBuffer(buffer);

  int i = 0;
  do
  {
    i = read(c->clientFileDescr, buffer + i, BUFFER_SIZE);

    // Client disconnected
    if (i == 0)
      return -1;

  } while (buffer[strlen(buffer)] != '\0');
}

const int writeToClient(struct client *c, char message[])
{
  int i = 0, j = 0;
  for (j = 0; j < strlen(message); j += i)
  {
    i = write(c->clientFileDescr, message + j, strlen(message) - j);
    // Client disconnected
    if (i == -1)
      return -1;
  }
}

void matchmake(struct players *players)
{
  // Awaiting first player WANNA_PLAY message
  char buffer[BUFFER_SIZE];

  while (true)
  {
    readFromClient(players->white, buffer);
    if (strcmp(buffer, "WANNA_PLAY") == 0)
      break;
    else
      writeToClient(players->white, "ERROR");
  }

  // Awaiting second player player WANNA_PLAY message
  while (true)
  {
    readFromClient(players->black, buffer);
    if (strcmp(buffer, "WANNA_PLAY") == 0)
      break;
    else
      writeToClient(players->black, "ERROR");
  }

  writeToClient(players->white, "GAME_START_WHITE");
  writeToClient(players->black, "GAME_START_BLACK");
}

const int updateGameStatus(struct players *players, struct board *board) // Returns 1 if game has ended
{
  char buffer[BUFFER_SIZE], move[BUFFER_SIZE];
  char boardState[BOARD_SIZE];
  clearBuffer(buffer);

  boardStateToArray(board, boardState);
  printf("Board state: %s\n", strcat(strcpy(buffer, "BOARD:"), boardState));

  if (board->playerToMove == whiteToMove)
  {
    writeToClient(players->white, "YOUR_TURN");
    writeToClient(players->black, "OPPONENTS_TURN");

    readFromClient(players->white, buffer); // take in positions from and to
    printf("White player: %s\n", buffer);
  }
  else if (board->playerToMove == blackToMove)
  {
    writeToClient(players->white, "OPPONENTS_TURN");
    writeToClient(players->black, "YOUR_TURN");

    readFromClient(players->black, buffer); // take in positions from and to
    printf("Black player: %s\n", buffer);
  }

  // Buffer now has a move proposition from player -> check if it's legal
  //...
  bool legalMove = false;
  char moveType;
  while (!legalMove)
  {
    char squareFrom[3] = {'\0', '\0', '\0'};
    char squareTo[3] = {'\0', '\0', '\0'};
    int i, j = 0;

    for (i = 0; buffer[i] != ' '; ++i)
      squareFrom[j++] = buffer[i];

    moveType = buffer[++i];
    i += 2;

    for (j = 0; buffer[i] != '\0'; ++i)
      squareTo[j++] = buffer[i];

    int cellFrom = atoi(squareFrom);
    int cellTo = atoi(squareTo);

    if (!isLegalMove(board, cellFrom, moveType, cellTo))
    {
      if (board->playerToMove == whiteToMove)
      {
        writeToClient(players->white, "NOT_LEGAL");
        clearBuffer(buffer);
        readFromClient(players->white, buffer);
      }
      else
      {
        writeToClient(players->black, "NOT_LEGAL");
        clearBuffer(buffer);
        readFromClient(players->black, buffer);
      }
      printf("Illegal move: %s\n", buffer);
    }
    else
      legalMove = true;
  }

  // TODO
  // wykonaj ruch na planszy serwera
  //...

  if (board->playerToMove == whiteToMove)
  {
    writeToClient(players->white, "LEGAL");
    writeToClient(players->black, buffer);
  }
  else
  {
    writeToClient(players->white, buffer);
    writeToClient(players->black, "LEGAL");
  }

  if (moveType != 'x')
    board->playerToMove = !board->playerToMove;

  /*if (isLegalMove("FILL IN")) {
    move("FILL IN");   //Handle attacking
    playerTurn = !playerTurn;
  } else {
    if (playerTurn == 0)
      writeToClient(players->white, "ILLEGAL_MOVE");
    else
      writeToClient(players->black, "ILLEGAL_MOVE")*/

  if (gameFinished(board)) // Game has finished
  {
    // TODO game finished
    return 1;
  }

  return 0;
}

// Game thread function
void *cthread(void *arg)
{
  struct players *players = (struct players *)arg;
  struct client *whitePlayer = players->white;
  struct client *blackPlayer = players->black;

  matchmake(players);

  printf("%s\n", "Game created!");

  struct board _board;
  _board.playerToMove = whiteToMove;
  struct board *board = &_board;
  startingPosition(board);

  int gameStatus = 0;
  while (gameStatus == 0)
  {
    gameStatus = updateGameStatus(players, board);
  }

  printf("%s\n", "Game ended!");

  // Cleaning up...
  close(whitePlayer->clientFileDescr);
  close(blackPlayer->clientFileDescr);
  return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
  pthread_t tid;
  socklen_t slt;
  int sfd, on = 1;
  struct sockaddr_in saddr;

  saddr.sin_family = AF_INET;
  saddr.sin_addr.s_addr = INADDR_ANY;
  saddr.sin_port = htons(PORT);
  sfd = socket(AF_INET, SOCK_STREAM, 0);
  setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
  bind(sfd, (struct sockaddr *)&saddr, sizeof(saddr));
  listen(sfd, 10);

  while (true)
  {
    struct client *whitePlayer = malloc(sizeof(struct client));
    struct client *blackPlayer = malloc(sizeof(struct client));
    slt = sizeof(whitePlayer->clientAddr);

    whitePlayer->clientFileDescr = accept(sfd, (struct sockaddr *)&whitePlayer->clientAddr, &slt);
    printf("New connection: %s\n", inet_ntoa((struct in_addr)whitePlayer->clientAddr.sin_addr));

    blackPlayer->clientFileDescr = accept(sfd, (struct sockaddr *)&blackPlayer->clientAddr, &slt);
    printf("New connection: %s\n", inet_ntoa((struct in_addr)blackPlayer->clientAddr.sin_addr));

    struct players arg = {whitePlayer, blackPlayer};
    pthread_create(&tid, NULL, cthread, &arg);
    pthread_detach(tid);
  }
  close(sfd);
  return EXIT_SUCCESS;
}