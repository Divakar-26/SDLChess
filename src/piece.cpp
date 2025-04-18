#include "piece.h"
#include <iostream>

const int NORTH = -8;
const int SOUTH = 8;
const int WEST = -1;
const int EAST = 1;

const int NORTH_EAST = -7;
const int NORTH_WEST = -9;
const int SOUTH_EAST = 9;
const int SOUTH_WEST = 7;

const int ROOK_DIRS[4] = {-8, 8, -1, 1};   // up, down, left, right
const int BISHOP_DIRS[4] = {-9, -7, 7, 9}; // diagonals
const int QUEEN_DIRS[8] = {-8, 8, -1, 1, -9, -7, 7, 9};
const int KNIGHT_OFFSETS[8] = {-17, -15, -10, -6, 6, 10, 15, 17};
const int KING_OFFSETS[8] = {-9, -8, -7, -1, 1, 7, 8, 9};

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

void Pieces::renderPieces(SDL_Renderer *renderer, Chessboard board, SDL_Texture *pieceTexture, int CELL_SIZE)
{
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if (board.getPiecesAt(i, j) == '.')
            {
                continue;
            }
            renderPiece(renderer, board.getPiecesAt(i, j), j, i, pieceTexture, CELL_SIZE);
        }
    }
}

void Pieces::renderPiece(SDL_Renderer *renderer, char piece, int x, int y, SDL_Texture *pieceTexture, int CELL_SIZE)
{
    SDL_FRect srcRect = {pieces[piece].texX, pieces[piece].texY, 200, 200};
    SDL_FRect destRect = {(float)(y * CELL_SIZE), (float)(x * CELL_SIZE), (float)CELL_SIZE, (float)CELL_SIZE};
    SDL_RenderTexture(renderer, pieceTexture, &srcRect, &destRect);
}

void Pieces::renderPieceAt(SDL_Renderer *renderer, char piece, int x, int y, SDL_Texture *pieceTexture, int CELL_SIZE)
{

    SDL_FRect srcRect = {pieces[piece].texX, pieces[piece].texY, 200, 200};
    SDL_FRect destRect = {(float)(y), (float)(x), (float)CELL_SIZE, (float)CELL_SIZE};
    SDL_RenderTexture(renderer, pieceTexture, &srcRect, &destRect);
}

std::vector<std::pair<int, int>> Pieces::legalMoves(int row, int col, Chessboard &board)
{

    std::vector<std::pair<int, int>> moves;

    if (board.isSlidingPiece(row, col))
    {
        moves = getSlidingLegalMove(row, col, board);
    }
    else if (board.getPiecesAt(row, col) == 'n' || board.getPiecesAt(row, col) == 'N')
    {
        moves = getKnightLegalMove(row, col, board);
    }
    else if (board.getPiecesAt(row, col) == 'p' || board.getPiecesAt(row, col) == 'P')
    {
        moves = getPawnLegalMove(row, col, board);
    }
    else if (board.getPiecesAt(row, col) == 'k' || board.getPiecesAt(row, col) == 'K')
    {
        moves = getKingLegalMove(row, col, board);
    }


    return moves;
}

std::vector<std::pair<int, int>> Pieces::getSlidingLegalMove(int row, int col, Chessboard &board)
{

    std::vector<std::pair<int, int>> moves;

    int index = to64(row, col);
    char c = board.getPiecesAt(row, col);

    const int *direction = nullptr;
    int loopCounter;

    if (c == 'r' || c == 'R')
    {
        direction = ROOK_DIRS;
        loopCounter = 4;
    }
    else if (c == 'q' || c == 'Q')
    {
        direction = QUEEN_DIRS;
        loopCounter = 8;
    }
    else if (c == 'b' || c == 'B')
    {
        direction = BISHOP_DIRS;
        loopCounter = 4;
    }
    else
        return moves;

    for (int i = 0; i < loopCounter; i++)
    {
        int newIndex = index;

        while (true)
        {

            newIndex += direction[i];

            if (newIndex < 0 || newIndex >= 64)
                break;

            int prevRow = rowFrom64(newIndex - direction[i]);
            int currRow = rowFrom64(newIndex);
            int prevCol = colFrom64(newIndex - direction[i]);
            int currCol = colFrom64(newIndex);

            if (std::abs(currRow - prevRow) > 1 || std::abs(currCol - prevCol) > 1)
                break;

            if (board.isOwnPiece(currRow, currCol, c))
            {
                break;
            }

            moves.emplace_back(currRow, currCol);

            if (board.isEnemyPiece(currRow, currCol, c))
            {
                break;
            }
        }
    }
    return moves;
}

std::vector<std::pair<int, int>> Pieces::
    getKnightLegalMove(int row, int col, Chessboard &board)
{
    std::vector<std::pair<int, int>> moves;

    char c = board.getPiecesAt(row, col);
    if (c != 'n' && c != 'N')
        return moves;

    int index = to64(row, col);

    for (int i = 0; i < 8; i++)
    {
        int newIndex = index + KNIGHT_OFFSETS[i];
        if (newIndex < 0 || newIndex >= 64)
            continue;

        int newRow = rowFrom64(newIndex);
        int newCol = colFrom64(newIndex);

        int dr = std::abs(newRow - row);
        int dc = std::abs(newCol - col);
        if (!((dr == 2 && dc == 1) || (dr == 1 && dc == 2)))
            continue;

        if (board.isOwnPiece(newRow, newCol, c))
            continue;

        moves.emplace_back(newRow, newCol);
    }

    return moves;
}

std::vector<std::pair<int, int>> Pieces::
    getPawnLegalMove(int row, int col, Chessboard &board)
{
    std::vector<std::pair<int, int>> moves;

    char c = board.getPiecesAt(row, col);
    int index = to64(row, col);

    bool isWhite = std::isupper(c);
    int direction = isWhite ? -8 : 8; // White moves up (negative), Black moves down (positive)
    int startRow = isWhite ? 6 : 1;
    int doubleMoveOffset = direction * 2;

    int oneStep = index + direction;
    if (oneStep >= 0 && oneStep < 64)
    {
        int newRow = rowFrom64(oneStep);
        int newCol = colFrom64(oneStep);
        if (board.getPiecesAt(newRow, newCol) == '.')
        {
            moves.emplace_back(newRow, newCol);

            // Double move from starting row (only if the pawn hasn't moved yet)
            if (col == startRow)
            {
                int twoStep = index + doubleMoveOffset;
                if (twoStep >= 0 && twoStep < 64)
                {
                    int r2 = rowFrom64(twoStep);
                    int c2 = colFrom64(twoStep);
                    if (board.getPiecesAt(r2, c2) == '.')
                    {
                        moves.emplace_back(r2, c2);
                    }
                }
            }
        }
    }

    const int captureOffsets[2] = {direction - 1, direction + 1}; // Left and right diagonal captures
    for (int i = 0; i < 2; i++)
    {
        int captureIndex = index + captureOffsets[i];
        if (captureIndex < 0 || captureIndex >= 64)
            continue;

        int newRow = rowFrom64(captureIndex);
        int newCol = colFrom64(captureIndex);

        // Prevent out-of-bounds captures
        if (std::abs(newCol - col) != 1)
            continue;

        char target = board.getPiecesAt(newRow, newCol);
        if (target != '.' && board.isEnemyPiece(newRow, newCol, c))
        {
            moves.emplace_back(newRow, newCol);
        }
    }

    return moves;
}

std::vector<std::pair<int, int>> Pieces::
    getKingLegalMove(int row, int col, Chessboard &board)
{
    std::vector<std::pair<int, int>> moves;

    char c = board.getPiecesAt(row, col);
    int index = to64(row, col);

    if(c != 'k' && c != 'K'){
        return moves;
    }

    for(int i = 0;i < 8; i++){
        int nexIndex = index + KING_OFFSETS[i];

        if (nexIndex < 0 || nexIndex >= 64)
            continue;

        int newRow = rowFrom64(nexIndex);
        int newCol = colFrom64(nexIndex);

        if (std::abs(newRow - row) > 1 || std::abs(newCol - col) > 1)
        continue;

        char target = board.getPiecesAt(newRow, newCol);

        if (target != '.' && std::islower(target) == std::islower(c))
            continue;


        moves.emplace_back(rowFrom64(nexIndex), colFrom64(nexIndex));
    }

    return moves;
}
