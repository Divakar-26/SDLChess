#ifndef CHESSBOARD_H
#define CHESSBOARD_H

#include<SDL3/SDL.h>
#include<string>

class Chessboard{
    public:
    void setFEN(std::string fen);
    std::string getFEN();

    void renderBoard(SDL_Renderer *renderer, SDL_Texture * boardTexture);

    char getPiecesAt(int row, int col){
        return board[row][col];
    }

    bool hasPieceAt(int row, int col){
        if(board[row][col] != '.'){
            return true;
        }
        return false;
    }

    void clearPieceAt(int row, int col){
        board[row][col] = '.';
    }

    void setPieceAt(float x, float y, char p, int CELL_SIZE);

    private:
        char board[8][8];

};

#endif
