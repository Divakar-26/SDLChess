#ifndef CHESSBOARD_H
#define CHESSBOARD_H

#include <SDL3/SDL.h>
#include <string>

class Chessboard
{
public:
    void setFEN(std::string fen);
    std::string getFEN();

    void renderBoard(SDL_Renderer *renderer, SDL_Texture *boardTexture);

    char getPiecesAt(int row, int col)
    {
        return board[col][row];
    }

    bool hasPieceAt(int row, int col)
    {
        if (board[col][row] != '.')
        {
            return true;
        }
        return false;
    }

    void clearPieceAt(int row, int col)
    {
        board[col][row] = '.';
    }

    void setPieceAt(int x, int y, char p, int CELL_SIZE);

    void highLightSquare(int row, int col, int CELL_SIZE, SDL_Renderer *renderer);

    bool isOwnPiece(int row, int col, char c){
        char cp = board[col][row];

        if(cp == '.'){
            return false;
        }

        return std::isupper(cp) == std::isupper(cp);
    }
    bool isEnemyPiece(int row, int col, char c){

        char cp = board[col][row];

        if(cp == '.'){
            return false;
        }

        return std::islower(cp) == std::islower(cp);
    }


    bool isSlidingPiece(int row, int col){
        
        char cp = board[col][row];

        if(cp == 'b' || cp == 'B' || cp == 'q' || cp == 'Q' ||cp == 'r' || cp == 'R' ){
            return true;
        }

        return false;
    }
    // utility


    

private:
    char board[8][8];
};

#endif
