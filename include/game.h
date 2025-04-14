#ifndef GAME_H
#define GAME_H

#include<SDL3/SDL.h>
#include<SDL3_image/SDL_image.h>
#include<iostream>
#include<unordered_map>
#include<string>

class Game{
    public:

    Game(int W_W, int W_H);
    ~Game();

    bool init(const char * title, char * fen, int args);
    void handleEvent();
    void update();
    void render();
    bool running(){
        return isRunning;
    }


    // HELPER FUNCTIONS
    SDL_Texture * loadTexture(const char * path);


    private:

    SDL_Window * window;
    SDL_Renderer * renderer;
    int WINDOW_W, WINDOW_H;
    std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    bool isRunning;

};


#endif