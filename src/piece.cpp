#include"piece.h"


Piece q = {0, 0, "Black Queen"};
Piece k = {200, 0, "Black King"};
Piece r = {400, 0, "Black Rook"};
Piece n = {600, 0, "Black Knight"};
Piece b = {800, 0, "Black Bishop"};
Piece p = {1000, 0, "Black Pawn"};

Piece Q = {0, 200, "White Queen"};
Piece K = {200, 200, "White King"};
Piece R = {400, 200, "White Rook"};
Piece N = {600, 200, "White Knight"};
Piece B = {800, 200, "White Bishop"};
Piece P = {1000, 200, "White Pawn"};


void Pieces::renderPieces(SDL_Renderer *renderer, Chessboard board, SDL_Texture * pieceTexture, int CELL_SIZE)
{
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if (board.getPiecesAt(i,j) == '.')
            {
                continue;
            }
            renderPiece(renderer, board.getPiecesAt(i,j), i, j, pieceTexture, CELL_SIZE);
        }
    }
}



void Pieces::renderPiece(SDL_Renderer *renderer, char piece, int x, int y, SDL_Texture * pieceTexture, int CELL_SIZE)
{

    SDL_FRect srcRect = {pieces[piece].texX, pieces[piece].texY, 200, 200};
    SDL_FRect destRect = {(float)(y * CELL_SIZE), (float)(x * CELL_SIZE), (float)CELL_SIZE, (float)CELL_SIZE};
    SDL_RenderTexture(renderer, pieceTexture, &srcRect, &destRect);
}

void Pieces::renderPieceAt(SDL_Renderer *renderer, char piece, int x, int y, SDL_Texture * pieceTexture, int CELL_SIZE)
{

    SDL_FRect srcRect = {pieces[piece].texX, pieces[piece].texY, 200, 200};
    SDL_FRect destRect = {(float)(y), (float)(x), (float)CELL_SIZE, (float)CELL_SIZE};
    SDL_RenderTexture(renderer, pieceTexture, &srcRect, &destRect);
}
