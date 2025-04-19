#include "texture.h"

SDL_Texture *TextureManager::loadTexture(SDL_Renderer *renderer, const char *path)
{
    SDL_Surface *tempSurface = IMG_Load(path);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, tempSurface);
    SDL_DestroySurface(tempSurface);
    return texture;
}

bool TextureManager::loadTexturesFromFolder(const std::string &folder_path,
                                            std::unordered_map<std::string, SDL_Texture *> &targetMap,
                                            std::vector<std::string> &textureNames, SDL_Renderer * renderer)
{
    try
    {
        for (const auto &entry : fs::directory_iterator(folder_path))
        {
            if (entry.is_regular_file())
            {
                std::string filename = entry.path().filename().string();
                std::string textureName = filename.substr(0, filename.find_last_of('.'));

                // std::cout << textureName << std::endl;

                SDL_Texture *tex = loadTexture(renderer, (folder_path + "/" + filename).c_str());
                if (tex)
                {
                    textureNames.push_back(textureName);
                    targetMap[textureName] = tex;
                }
                else
                {
                    SDL_Log("Failed to load texture: %s", filename.c_str());
                }
            }
        }
    }
    catch (const fs::filesystem_error &e)
    {
        SDL_Log("Filesystem error: %s", e.what());
        return false;
    }
    return true;
}

bool TextureManager::init(SDL_Texture *pieceTexture, SDL_Texture *boardTexture, std::unordered_map<std::string, SDL_Texture *> & piecesTextures, std::unordered_map<std::string, SDL_Texture *> & boardTextures, std::vector<std::string> & texture_of_piece, std::vector<std::string> & texture_of_board, SDL_Renderer * renderer){

    loadTexturesFromFolder("assets/piecesTexture", piecesTextures, texture_of_piece, renderer);
    loadTexturesFromFolder("assets/boardTexture", boardTextures, texture_of_board,renderer);

    pieceTexture = piecesTextures["pixel"];
    boardTexture = boardTextures["8_bit"];

    if (!boardTexture || !pieceTexture)
    {
        SDL_Log("Failed to load texture!");
        return false;
    }

    return false;
}

