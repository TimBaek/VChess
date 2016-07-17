#include "piece.h"
#include "knight.h"
using namespace std;

Knight::Knight(Board *bd, int c, int r, string co):
  Piece{bd, r, c, co} {
}

Knight::~Knight() { }

void Knight::move(int dr, int dc) {
  notifyBoard(dr, dc);
  row = dr;
  col = dc;
}

char Knight::getLetter() {
  if (colour == "white") {
    return 'N';
  } else {
   return 'n';
  }
}
