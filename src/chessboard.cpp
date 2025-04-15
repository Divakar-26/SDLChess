#include"chessboard.h"

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