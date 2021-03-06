#include "computer.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sstream>
using namespace std;

Computer::Computer(string colour, Board *b, int level): colour{colour}, b{b}, level{level}, currR{-1}, currC{-1}, destR{-1}, destC{-1} {}

Computer::~Computer() {}

struct Coordinates{
	int row,col;
};

string Computer::getColour() { return colour; }
string Computer::getName(){ return "computer"; }

int Computer::getR(){ return currR; }
int Computer::getC(){ return currC; }
int Computer::getDestR(){ return destR; }
int Computer::getDestC(){ return destC; }

void Computer::nextMove(){
	if(level==1) randomMove();
	else if(level==2){ if(capturingMove() ==0) randomMove(); }
	else if(level==3){ if(capturingMove() == 0) avoidCapture(); }

	if(b->isPromo(currR, currC, destR, destC)){ pawnPromo(currR,currC); }
}

void Computer::pawnPromo(int r, int c){
	if(level==3){
		b->setup_add(getColour()=="black" ? 'q' : 'Q', r,c);
	}
	else{
		char letter;
		srand(time(NULL));
		int x;
		if(level==2){ x = rand() % 6; }
		if(level==1){ x = rand() % 4; }
		switch(x) {
			case 5: letter = 'q'; break;
			case 4: letter = 'q'; break;
			case 0: letter = 'q'; break;
			case 1: letter = 'b'; break;
			case 2: letter = 'r'; break;
			case 3: letter = 'n';
		}
		if(getColour()=="white") letter = letter - 'a' + 'A';
		b->setup_add(letter,r,c);
	}
}

void Computer::randomMove(){
	vector<Coordinates> coord1,coord2;
	for(int r=0; r<8; r++){
		for(int c=0; c<8; c++){
			if(b->isEmpty(r,c)) continue;
			if(b->checkState(r,c)->getColour() != getColour()) continue;
			for(int i=0; i<8; i++){
				for(int j=0; j<8; j++){
					if(!b->canMove(b->checkState(r,c), i, j, getColour())) continue;
					if(b->willBeChecked(r,c,i,j, getColour())) continue;
					coord1.push_back(Coordinates{r,c});
					coord2.push_back(Coordinates{i,j});
				}
			}
		}
	}
	srand (time(NULL));
	int x = rand() % coord1.size();
	currR = coord1[x].row;
	currC = coord1[x].col;
	destR = coord2[x].row;
	destC = coord2[x].col;
}

void Computer::avoidCapture(){
	// priority of pieces to avoid captures for:
	// pawn < knight < rook < bishop < queen < king
	// each corresponds to 1-6 according to the priority
	int r1,r2,c1,c2;
	int currPriority=0;
	string oppColour = getColour()=="black" ? "white" : "black";

	for(int r=0; r<8; r++){
		for(int c=0; c<8; c++){
			if(b->isEmpty(r,c)) continue;
			if(b->checkState(r,c)->getColour() == getColour()) continue;
			for(int i=0; i<8; i++){
				for(int j=0; j<8; j++){
					if(b->checkState(i,j) == nullptr) continue;
					if(!b->canMove(b->checkState(r,c), i, j, oppColour)) continue;					
					if(b->checkState(i,j)->getColour() != getColour()) continue;
					char letter = b->checkState(i,j)->getLetter();
					letter = letter < 'Z' ? letter + 'a' - 'A' : letter;
					int priority;
					switch(letter) {
						case 'k': priority = 6; break;
						case 'q': priority = 5; break;
						case 'b': priority = 4; break;
						case 'r': priority = 3; break;
						case 'n': priority = 2; break;
						case 'p': priority = 1;
					}
					if(priority<= currPriority) continue;
					bool movable=false;
					for(int x=0; x<8; x++){
						if(movable) break;
						for(int y=0; y<8; y++){
							if(b->willBeChecked(i,j,x,y, getColour())) continue;
							if(b->canMove(b->checkState(i,j),x,y, getColour())){
								r2=x;
								c2=y;
								movable = true;
								// stops the loop once it finds a move
								// which might not necessarily be the best move.
								break;
							}
						}
					}
					if(!movable) continue;
					currPriority = priority;
					r1=i;
					c1=j;
				}
			}
		}
	}
	if(currPriority==0){
		randomMove();
		return;
	}
	currR = r1;
	currC = c1;
	destR = r2;
	destC = c2;
}


int Computer::capturingMove(){
	string oppColour = getColour()=="black" ? "white" : "black";
	int currPriority=0;
	for(int r=0; r<8; r++){
		if(currPriority==6) break;
		for(int c=0; c<8; c++){
			if(currPriority==6) break;
			if(b->isEmpty(r,c)) continue;
			if(b->checkState(r,c)->getColour() != getColour()) continue;
			for(int i=0; i<8; i++){
				if(currPriority==6) break;
				for(int j=0; j<8; j++){
					if(b->checkState(i,j) == nullptr) continue;
					if(!b->canMove(b->checkState(r,c), i, j, getColour())) continue;
					if(b->willBeChecked(r,c,i,j, getColour())) continue;
					if(b->checkState(i,j)->getColour() != oppColour) continue;
					if(b->willBeChecked(r,c,i,j,oppColour)){
						currPriority = 6;
						break;
					}
					char letter = b->checkState(i,j)->getLetter();
					letter = letter < 'Z' ? letter + 'a' - 'A' : letter;
					int priority;
					switch(letter) {
						case 'q': priority = 5; break;
						case 'b': priority = 4; break;
						case 'r': priority = 3; break;
						case 'n': priority = 2; break;
						case 'p': priority = 1;
					}
					if(priority<= currPriority) continue;
					currPriority = priority;
					currR = r;
					currC = c;
					destR = i;
					destC = j;
				}
			}
		}
	}
	return currPriority;
}


