#include"chessboard.h"
#include<iostream>
char board[8][8] = {
    {'.', '.', '.', '.', '.', '.', '.', '.'},
    {'.', '.', '.', '.', '.', '.', '.', '.'},
    {'.', '.', '.', '.', '.', '.', '.', '.'},
    {'.', '.', '.', '.', '.', '.', '.', '.'},
    {'.', '.', '.', '.', '.', '.', '.', '.'},
    {'.', '.', '.', '.', '.', '.', '.', '.'},
    {'.', '.', '.', '.', '.', '.', '.', '.'},
    {'.', '.', '.', '.', '.', '.', '.', '.'},
};

void Chessboard::setFEN(std::string fen)
{
    int row = 0;
    int col = 0;
    for (int i = 0; fen[i] != '\0'; i++)
    {

        if (fen[i] == ' ')
        {
            break;
        }

        if (isdigit(fen[i]))
        {
            int empty = fen[i] - '0';
            for (int k = 0; k < empty; k++)
            {
                board[row][col] = '.';
                col++;
            }
        }

        else if (fen[i] == '/')
        {
            row++;
            col = 0;
        }

        else
        {
            board[row][col] = fen[i];
            col++;
        }
    }
}


void Chessboard::renderBoard(SDL_Renderer *renderer, SDL_Texture * boardTexture)
{
    SDL_FRect srcRect = {0, 0, 1600, 1600};
    SDL_FRect destRect = {0, 0, 640, 640};

    SDL_RenderTexture(renderer, boardTexture, &srcRect, &destRect);
}

void Chessboard::setPieceAt(int x, int y, char p, int CELL_SIZE){

    // Optional: add bounds check to prevent crash
    if (x >= 0 && x < 8 && y >= 0 && y < 8) {
        board[y][x] = p;
        std::cout<<x<<" "<<y<<std::endl;
    }
}


void Chessboard::highLightSquare(int row, int col, int CELL_SIZE, SDL_Renderer * renderer){
    SDL_FRect  rect = {(row * (CELL_SIZE)), (col * (CELL_SIZE)), CELL_SIZE, CELL_SIZE};
    SDL_RenderFillRect(renderer, &rect);
}
