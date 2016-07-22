#include <string>
#include <sstream>
#include <vector>
#include <cstdlib>
using namespace std;

#include "controller.h"

Controller::Controller(): in{&cin}, board{this}, customized{false} {}
Controller::~Controller() {}

void Controller::notify(int r, int c, int destr, int destc, char piece) {
	if (currPlayer->getName() == "human") {
		if (!board.canMove(board.checkState(r,c), destr, destc, currPlayerColour) /*|| 
			board.willBeChecked(r,c,destr,destc,currPlayerColour)*/) throw iv;	
	}
	// Castling Move
	if (board.checkState(r,c)->getLetter() == 'k' || board.checkState(r,c)->getLetter() == 'K') {
		if (destr == r && abs(destc - c) == 2) {
			board.castling(r,c,destc);	
		} 
	}
	// EnPassant Move + Pawn Promotion
	else if (board.checkState(r,c)->getLetter() == 'p' || board.checkState(r,c)->getLetter() == 'P') {
		if (abs(destr -r) == 1 && abs(destc -c) == 1 &&
			board.isEmpty(destr,destc)) {
			if (currPlayerColour == "white") board.setup_delete(destr -1,destc);
			else board.setup_delete(destr +1, destc);
		} 
	}
	board.offEnPassant(currPlayerColour);
	board.checkState(r,c)->move(destr,destc); // Regular Move
	if (board.isPromo(destr,destc)) {
		if (piece == 'K' || piece == 'k') throw iv;
		board.setup_delete(destr,destc);
		board.setup_add(piece,destr,destc);
	}
	view->notify(&board);
}

void Controller::setNextPlayer() {
	if (currPlayerColour == "white") currPlayer = bp;
	else currPlayer = wp;
}

void Controller::calculateScore(string colour) {
	if (colour == "white") wp->addScore();
	else bp->addScore();
}

void Controller::rebuild() {
	board.init();
	view->notify(&board);
	currPlayer = wp;
}

void Controller::setup() {
	customized = true;
	iv.setupMessage();
	board.setup();
	while (1) {
		cout << "Setup: ";
		try {
			string cmd, colour;
			char p, r, c;
			*in >> cmd;
			if (cmd == "+") {
				*in >> p >> c >> r;
				if (!iv.isValid(r,c,p)) throw iv;
				board.setup_add(p, r-'0'-1, c-'a');	
			} else if (cmd == "-") {
				*in >> c >> r;
				if (!iv.isValid(r,c)) throw iv;
				board.setup_delete(r-'0'-1, c-'a');
			} else if (cmd == "=") {
				*in >> colour;
				if (colour == "white") currPlayerColour = "white";
				else if (colour == "black") currPlayerColour = "black";
				else throw iv;
			} else if (cmd == "done") {
				if (board.numKing("white") != 1 || board.numKing("black") != 1) iv.numKingMessage();
				else if (!board.isbadPawnPosition()) iv.badPawnPositionMessage();
				else break;
			} else throw iv;
		} catch (InputValidation e) {
			e.errorMessage();
		}
	}	
}

void Controller::init() {
	try {
		string w, b;
		*in >> w >> b;
		if (!iv.isPlayer(w,b)) throw iv;

		//Player init
		if (w == "human") wp = make_shared<Human>("white");
		else wp = make_shared<Computer>("white", stoi(w.substr(8,1)), &board);
		if (b == "human") bp = make_shared<Human>("black");
		else bp = make_shared<Computer>("black", stoi(b.substr(8,1)), &board);
		
		board.setPlayers(wp,bp);
		if (!customized) currPlayer = wp;
		if (customized) {
			if (currPlayerColour == "white" || currPlayerColour == "") currPlayer = wp;
			else currPlayer = bp;
		}

		//Board init
		if (!customized) board.init();

		//Display init
		view->notify(&board);
	} catch (InputValidation e) {
		throw e;
	}
}

void Controller::game() {
	try {
		init();
		iv.gameMessage(); //Start new game
		while (1) {
			currPlayerColour = currPlayer->getColour();
			cout << endl;
			view->print();
			iv.currPlayerMessage(currPlayerColour);
			try {
				string cmd;
				*in >> cmd;
				if (cmd == "move") {
					string move;
					getline(*in,move);
					vector<string> cord;
					if (move == "" && currPlayer->getName() == "computer") { // move for Computer
						currPlayer->nextMove();
						notify(currPlayer->getR(), currPlayer->getC(), currPlayer->getDestR(), currPlayer->getDestC());
					}
					else {
						istringstream iss{move};
						string tmp;
						while(iss >> tmp) cord.emplace_back(tmp);
						if (cord.size() != 3 && cord.size() != 2) throw iv;
						else if (cord.size() == 2) {
							if (!iv.isValid(cord[0][1],cord[0][0]) || !iv.isValid(cord[1][1],cord[1][0])) throw iv;
							int r,c,destr,destc;
							r = cord[0][1]-'0'-1;
							c = cord[0][0]-'a';	
							destr = cord[1][1]-'0'-1;
							destc = cord[1][0]-'a';
							notify(r,c,destr,destc);
						} else { // Pawn Promotion
							if(!iv.isValid(cord[0][1],cord[0][0],cord[2][0]) || !iv.isValid(cord[1][1],cord[1][0],cord[2][0])) throw iv;
							int r,c,destr,destc;
							char piece;
							r = cord[0][1]-'0'-1;
							c = cord[0][0]-'a';	
							destr = cord[1][1]-'0'-1;
							destc = cord[1][0]-'a';
							if (currPlayerColour == "white" && islower(cord[2][0])) piece = toupper(cord[2][0]);
							else if (currPlayerColour == "black" && isupper(cord[2][0])) piece = tolower(cord[2][0]);
							else piece = cord[2][0];
							notify(r,c,destr,destc,piece);
						}
					}
				} else if (cmd == "resign") {
					iv.resignMessage(currPlayerColour);
					if (currPlayerColour == "white") calculateScore("black");
					else calculateScore("white");
					throw 1;
				} else throw iv;

				// Display state of the game
				/*if (wp->isCheckmate()) { // checkmate
					iv.checkmateMessage(wp->getColour());
					break;
				}
				if (bp->isCheckmate()) {
					iv.checkmateMessage(bp->getColour());
					break;
				}
				if (wp->isCheck()) iv.checkMessage(wp->getColour()); //check
				if (bp->isCheck()) iv.checkMessage(bp->getColour());
				if (wp->isStalemate() && bp->isStalemate()) iv.stalemateMessage(); */

				setNextPlayer();
			} catch (int e) {
				string ans;
				iv.regameMessage();
				*in >> ans;
				if (ans == "Y" || ans == "y") {
					rebuild();
				} else {
					iv.printScore(wp->getScore(), bp->getScore());
					break;
				}
			} catch (InputValidation e) {
				e.errorMessage();
			}
		}
		if(customized) customized = false;
	} catch (InputValidation e) {
		throw e;
	}
}

void Controller::play() {
	in->exceptions(ios::failbit|ios::eofbit);
	iv.level0Message();
	string d;
	while (*in >> d) {
		try {
			if (d == "td") {
				view = make_shared<TextDisplay>();
				break;
			} else if (d == "gd") {
				shared_ptr<Xwindow> xw = make_shared<Xwindow>();
				view = make_shared<GraphicDisplay>(xw);
				break;
			} else throw iv;
			
		} catch (InputValidation e) {
			e.errorMessage();
		}
	}

	iv.menuMessage();
	string cmd;
	while (1) {
		cout << "Menu: ";
		try {
			*in >> cmd;
			if (cmd == "game") game();
			else if (cmd == "setup") setup();
			else if (cmd == "quit") break;
			else throw iv;
		} catch (InputValidation e) {
			e.errorMessage();
		} //input failure for menu
	}
}
