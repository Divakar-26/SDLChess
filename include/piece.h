#ifndef PIECE_H
#define PIECE_H

#include<SDL3/SDL.h>
#include<unordered_map>
#include<string>
#include"chessboard.h"
#include<utility>
#include<vector>


// utils
inline int to64(int row, int col) { return row * 8 + col; }
inline int rowFrom64(int idx) { return idx / 8; }
inline int colFrom64(int idx) { return idx % 8; }

inline bool onBoard(int square)
{
    return square >= 0 && square < 64;
}

inline bool sameRow(int a, int b) {
    return rowFrom64(a) == rowFrom64(b);
}



struct Piece
{
    float texX, texY;
    std::string name;
    int worth;
    char color;
};

extern Piece q, k, r, n, b, p;
extern Piece Q, K, R, N, B, P;

class Pieces{

    public:
    void renderPieces(SDL_Renderer *renderer, Chessboard board, SDL_Texture * pieceTexture, int CELL_SIZE);
    void renderPiece(SDL_Renderer *renderer, char piece, int x, int y, SDL_Texture * pieceTexture, int CELL_SIZE);
    void renderPieceAt(SDL_Renderer *renderer, char piece, int x, int y, SDL_Texture * pieceTexture, int CELL_SIZE);
    std::vector<std::pair<int,int>> legalMoves(int row, int col, Chessboard & board);
    std::vector<std::pair<int,int>> getSlidingLegalMove(int row, int col, Chessboard & board);


    private:
    std::unordered_map<char, Piece> pieces = {
        {'q', q},
        {'k', k},
        {'r', r},
        {'n', n},
        {'b', b},
        {'p', p},
    
        {'Q', Q},
        {'K', K},
        {'R', R},
        {'N', N},
        {'B', B},
        {'P', P}};
};



#endif