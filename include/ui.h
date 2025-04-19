#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include<SDL3_mixer/SDL_mixer.h>
#include<SDL3/SDL_audio.h>
#include <iostream>
#include <unordered_map>
#include <string>
#include <filesystem>
#include <vector>
#include "imgui.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_sdlrenderer3.h"

#include"chessboard.h"

class UI{
    public:
    void init(SDL_Window * window, SDL_Renderer * renderer);
    void update();
    void dropDown(std::vector<std::string> & texture_of_piece, int & current_item_piece_theme, bool & piece_texture_changed, std::vector<std::string> & texture_of_board, int & current_item_board_theme, bool &board_texture_changed);
    void loadFEN(std::string & fen, Chessboard & board);
    void render();

    private:

};