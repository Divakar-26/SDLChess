#ifndef TEXTURE_H
#define TEXTURE_H

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>
#include <unordered_map>
#include <string>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

class TextureManager
{
public:
    SDL_Texture *loadTexture(SDL_Renderer *renderer, const char *path);
    bool loadTexturesFromFolder(const std::string &folder_path,
                                std::unordered_map<std::string, SDL_Texture *> &targetMap,
                                std::vector<std::string> &textureNames, SDL_Renderer *renderer);
    bool init(SDL_Texture *pieceTexture, SDL_Texture *boardTexture, std::unordered_map<std::string, SDL_Texture *> & piecesTextures, std::unordered_map<std::string, SDL_Texture *> & boardTextures, std::vector<std::string> & texture_of_piece, std::vector<std::string>& texture_of_board, SDL_Renderer * renderer);
    void setPieceTexture(const char * string);

private:
};

#endif