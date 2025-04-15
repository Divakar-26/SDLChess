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
    SDL_FRect srcRect = {0, 0, 512, 512};
    SDL_FRect destRect = {0, 0, 640, 640};

    SDL_RenderTexture(renderer, boardTexture, &srcRect, &destRect);
}

void Chessboard::setPieceAt(float x, float y, char p, int CELL_SIZE){
    int xIndex = static_cast<int>(x / CELL_SIZE);
    int yIndex = static_cast<int>(y / CELL_SIZE);

    // Optional: add bounds check to prevent crash
    if (xIndex >= 0 && xIndex < 8 && yIndex >= 0 && yIndex < 8) {
        board[yIndex][xIndex] = p;
        std::cout<<xIndex<<" "<<yIndex<<std::endl;
    }
}
