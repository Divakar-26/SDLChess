#ifndef GAME_H
#define GAME_H

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3/SDL_audio.h>
#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>

#include "chessboard.h"
#include "piece.h"
#include"ui.h"
#include"texture.h"

class Game
{
public:
    Game(int W_W, int W_H);
    ~Game();

    bool init(const char *title, char *fen, int args);
    void handleEvent();
    void update();
    void render();
    void cleanup();
    bool running()
    {
        return isRunning;
    }

    // HELPER FUNCTIONS
    bool loadTexturesFromFolder(
        const std::string &folder_path,
        std::unordered_map<std::string, SDL_Texture *> &targetMap,
        std::vector<std::string> &textureNames);

    void renderPickedUPPiece();

    bool isInCheck(bool isWhite, Chessboard & customBoard);
    std::vector<std::pair<int, int>> filtermoves(std::vector<std::pair<int, int>>& rawMoves, int fromX, int fromY, char piece);
    bool canCastle(int x, int y, char p, Chessboard &board, std::vector<std::pair<int, int>> &moves);
    bool isCheckBlockable(int kingX, int kingY, int checkerX, int checkerY, char checkerPiece);

    std::vector<std::pair<int, int>> getPawnMovesWithEnPassant(int row, int col, Chessboard &board);

    int to64(int x, int y) {  // x is column, y is row
        return y * 8 + x;
    }
    
    int xFrom64(int index) {  // returns column
        return index % 8;
    }
    
    int yFrom64(int index) {  // returns row
        return index / 8;
    }
private:
    SDL_Window *window;
    SDL_Renderer *renderer;
    int WINDOW_W, WINDOW_H;
    std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    bool isRunning;

    SDL_Texture *pieceTexture;
    SDL_Texture *boardTexture;

    std::unordered_map<std::string, SDL_Texture *> piecesTextures;
    std::unordered_map<std::string, SDL_Texture *> boardTextures;

    int current_item_piece_theme = 15;
    int current_item_board_theme = 7;

    Chessboard board;
    Pieces piece;
    UI ui;
    TextureManager textureManager;

    bool isWhiteTurn;  // Tracks whose turn it is


    Mix_Music* placeSound;
};

#endif