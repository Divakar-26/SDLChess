#ifndef GAME_H
#define GAME_H

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>
#include <unordered_map>
#include <string>
#include <filesystem>
#include <vector>
#include "imgui.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_sdlrenderer3.h"

#include "chessboard.h"
#include "piece.h"

namespace fs = std::filesystem;

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
    SDL_Texture *loadTexture(const char *path);
    bool loadTexturesFromFolder(
        const std::string &folder_path,
        std::unordered_map<std::string, SDL_Texture *> &targetMap,
        std::vector<std::string> &textureNames);

    void renderPickedUPPiece();

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
    int current_item = 0;

    Chessboard board;
    Pieces piece;
};

#endif