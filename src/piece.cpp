#include"piece.h"

const int NORTH = -8;
const int SOUTH = 8;
const int WEST = -1;
const int EAST = 1;

const int NORTH_EAST = -7;
const int NORTH_WEST = -9;
const int SOUTH_EAST = 9;
const int SOUTH_WEST = 7;

const int ROOK_DIRS[4] = {-8, 8, -1, 1};  // up, down, left, right
const int BISHOP_DIRS[4] = {-9, -7, 7, 9};  // diagonals
const int QUEEN_DIRS[8] = {-8, 8, -1, 1, -9, -7, 7, 9};
const int KNIGHT_OFFSETS[8] = {-17, -15, -10, -6, 6, 10, 15, 17};



Piece q = {0, 0, "Black Queen", 'b'};
Piece k = {200, 0, "Black King", 'b'};
Piece r = {400, 0, "Black Rook", 'b'};
Piece n = {600, 0, "Black Knight", 'b'};
Piece b = {800, 0, "Black Bishop", 'b'};
Piece p = {1000, 0, "Black Pawn", 'b'};

Piece Q = {0, 200, "White Queen", 'w'};
Piece K = {200, 200, "White King", 'w'};
Piece R = {400, 200, "White Rook", 'w'};
Piece N = {600, 200, "White Knight", 'w'};
Piece B = {800, 200, "White Bishop", 'w'};
Piece P = {1000, 200, "White Pawn", 'w'};


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
            renderPiece(renderer, board.getPiecesAt(i,j), j, i, pieceTexture, CELL_SIZE);
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

std::vector<std::pair<int,int>> Pieces::legalMoves(int row, int col, Chessboard & board){

    std::vector<std::pair<int, int>> moves;

    
    if(board.isSlidingPiece(row, col)){
        moves = getSlidingLegalMove(row, col, board);
    }

    return moves;
}


std::vector<std::pair<int,int>> Pieces::getSlidingLegalMove(int row, int col, Chessboard & board){

    std::vector<std::pair<int, int>> moves;

    int index = to64(row, col);
    char c = board.getPiecesAt(row, col);

    const int * direction = nullptr;
    int loopCounter;


    if(c == 'r' || c == 'R'){
        direction = ROOK_DIRS;
        loopCounter = 4;
    }
    else if(c == 'q' || c == 'Q'){
        direction = QUEEN_DIRS;
        loopCounter = 8;
    }
    else if(c == 'b' || c == 'B'){
        direction = BISHOP_DIRS;
        loopCounter = 4;
    }
    else return moves;
    

    for(int i= 0; i < loopCounter; i++){
        int newIndex = index;

        while(true){

            newIndex += direction[i];

            if (newIndex < 0 || newIndex >= 64) break;

            int prevRow = rowFrom64(newIndex - direction[i]);
            int currRow = rowFrom64(newIndex);
            int prevCol = colFrom64(newIndex - direction[i]);
            int currCol = colFrom64(newIndex);

            if (std::abs(currRow - prevRow) > 1 || std::abs(currCol - prevCol) > 1) break;
            

            if(board.isOwnPiece(currRow, currCol, c)){
                break;
            }

            moves.emplace_back(currRow, currCol);

            if(board.isEnemyPiece(currRow, currCol, c)){
                break;
            }

        }
    }
    return moves;
}
