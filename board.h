//TODO
//Complete the ruleset

#ifndef BOARDH
#define BOARDH

#include <stdbool.h>
#define BOARD_SIZE 33

char emptySpace = 0;

const char whitePiece = 'W';
const char blackPiece = 'B';

const char whiteKing = 'G';
const char blackKing = 'D';

enum playerToMove
{
  whiteToMove,
  blackToMove
};

struct board
{
  enum playerToMove playerToMove;
  char board[BOARD_SIZE];
};

void boardStateToArray(struct board *board, char buffer[BOARD_SIZE])
{
  int i;
  for (i = 0; i < BOARD_SIZE; ++i)
    buffer[i] = board->board[i];

  buffer[BOARD_SIZE] = '\0';
}

char simulate2dArray(char squarePos)
{
  // Instead of FEN positions, returns positions on a would-be 2D array that includes immovable positions
  if ((squarePos / 4) % 2)
    squarePos = squarePos * 2;
  else
    squarePos = squarePos * 2 + 1;

  return squarePos;
}

void startingPosition(struct board *board)
{
  board->playerToMove = whiteToMove;
  int i;

  // Placing white pieces
  for (i = 1; i <= 12; ++i)
    board->board[i] = whitePiece;

  // Placing black pieces
  for (i = 21; i <= 32; ++i)
    board->board[i] = blackPiece;
}

char isIllegalLeft(char squareFrom)
{
  if ((squareFrom % 16) == 8)
    return true;

  return false;
}

char isIllegalRight(char squareFrom)
{
  if ((squareFrom % 16) == 7)
    return true;

  return false;
}

char isIllegalLeftDouble(char squareFrom)
{
  if ((squareFrom % 16) == 1)
    return true;

  return false;
}

char isIllegalRightDouble(char squareFrom)
{
  if ((squareFrom % 16) == 14)
    return true;

  return false;
}

const bool isLegalMove(struct board *board, int squareFrom, char moveType, int squareTo)
{
  return true;

  char pieceFrom = board->board[squareFrom];
  char pieceTo = board->board[squareTo];
  char pieceBetween = board->board[(squareFrom + squareTo) / 2];

  // Handle movement beyond range of play space
  if (squareFrom < 1 || squareFrom > BOARD_SIZE)
    return false;
  if (squareTo < 1 || squareTo > BOARD_SIZE)
    return false;

  // FEN decryption
  squareFrom = squareFrom - 1;
  squareTo = squareTo - 1;

  // Simulate 2D array for given piece positions
  squareFrom = simulate2dArray(squareFrom);
  squareTo = simulate2dArray(squareTo);

  if (board->playerToMove == whiteToMove)
  {
    // Check if white is trying to move NOT white pieces
    if (!(pieceFrom == whitePiece || pieceFrom == whiteKing))
      return false;

    // If the piece is moved towards white spawn, and it's not a king, move is illegal
    if (pieceFrom <= pieceTo && pieceFrom != whiteKing)
      return false;

    if (moveType == '-')
    {
      // Allow to move diagonally down left by one space
      if ((squareFrom - squareTo) == 7 && !isIllegalLeft(squareFrom))
        return true;

      // Allow to move diagonally down right by one space
      if ((squareFrom - squareTo) == 9 && !isIllegalRight(squareFrom))
        return true;

      // Allow the king to move diagonally up left by one space
      if ((squareFrom - squareTo) == -9 && !isIllegalLeft(squareFrom) && pieceFrom == whiteKing)
        return true;

      // Allow the king to move diagonally up right by one space
      if ((squareFrom - squareTo) == -7 && !isIllegalRight(squareFrom) && pieceFrom == whiteKing)
        return true;
    }
    else if (moveType == 'x')
    {
      // Allow jump by 2 squares diagonally down left, if there's a black piece between position and destination
      if ((squareFrom - squareTo) == 14 && !isIllegalLeft(squareFrom) && !isIllegalLeftDouble(squareFrom) && (pieceBetween == blackPiece || pieceBetween == blackKing))
        return true;

      // Allow jump by 2 squares diagonally down right, if there's a black piece between position and destination
      if ((squareFrom - squareTo) == 18 && !isIllegalRight(squareFrom) && !isIllegalRightDouble(squareFrom) && (pieceBetween == blackPiece || pieceBetween == blackKing))
        return true;

      // Allow king jump by 2 squares diagonally up left, if there's a black piece between position and destination
      if ((squareFrom - squareTo) == -18 && !isIllegalLeft(squareFrom) && !isIllegalLeftDouble(squareFrom) && (pieceBetween == blackPiece || pieceBetween == blackKing) && pieceFrom == whiteKing)
        return true;

      // Allow king to jump by 2 squares diagonally up right, if there's a black piece between position and destination
      if ((squareFrom - squareTo) == -14 && !isIllegalRight(squareFrom) && !isIllegalRightDouble(squareFrom) && (pieceBetween == blackPiece || pieceBetween == blackKing) && pieceFrom == whiteKing)
        return true;
    }

    return false;
  }

  if (board->playerToMove == blackToMove)
  {
    // Check if black is trying to move NOT black pieces
    if (!(pieceFrom == blackPiece || pieceFrom == blackKing))
      return false;

    // Check if destination is not empty
    if (pieceTo != emptySpace)
      return false;

    // If the piece is moved towards black spawn, and it's not a king, move is illegal
    if (pieceFrom >= pieceTo && pieceFrom != blackKing)
      return false;

    if (moveType == '-')
    {

      return true;
    }
    else if (moveType == 'x')
    {

      return true;
    }

    printf("Unknown playerToMove in thread");
  }
}

void move(struct board *board, int squareFrom, int squareTo)
{
}

bool gameFinished(struct board *board)
{
  int i = 1, w = 0, b = 0;

  for (; i <= BOARD_SIZE; i++)
  {
    if (board->board[i] == whitePiece || board->board[i] == whiteKing)
    {
      w++;
    }
    else if (board->board[i] == blackPiece || board->board[i] == blackKing)
    {
      b++;
    }
  }

  if (w == 0 || b == 0)
    return true;

  return false;
}

#endif