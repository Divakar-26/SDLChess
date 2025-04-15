#include "game.h"

#define CELL_SIZE 80

std::vector<std::string> texture_of_piece;
std::vector<std::string> texture_of_board;

int mX, mY;

void mousePrinting(SDL_Event e, SDL_Renderer *renderer);
bool isMouseMoved = false;


Game::Game(int W_W, int W_H)
{
    WINDOW_H = W_H;
    WINDOW_W = W_W;
}


Game::~Game() {}

bool Game::init(const char *title, char *fenInput, int args)
{
    if (SDL_Init(SDL_INIT_VIDEO) == 0)
    {
        return false;
    }

    window = SDL_CreateWindow(title, WINDOW_W, WINDOW_H, SDL_WINDOW_RESIZABLE);
    if (!window)
    {
        return false;
    }

    renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer)
    {
        return false;
    }

    // imgui setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    if (args == 1)
    {
        this->fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    }
    else
        this->fen = fenInput;

    // FEN Parsing  
    board.setFEN(fen);


    loadTexturesFromFolder("assets/piecesTexture", piecesTextures, texture_of_piece);
    loadTexturesFromFolder("assets/boardTexture", boardTextures, texture_of_board);


    pieceTexture = piecesTextures["cburnett"];
    boardTexture = boardTextures["board"];

    if (!boardTexture || !pieceTexture)
    {
        SDL_Log("Failed to load texture!");
        return false;
    }

    isRunning = true;
    return true;
}

void Game::handleEvent()
{
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        ImGui_ImplSDL3_ProcessEvent(&e);
        if (e.type == SDL_EVENT_QUIT)
        {
            isRunning = false;
        }
        else if (e.type == SDL_EVENT_MOUSE_MOTION)
        {
            mousePrinting(e, renderer);
            isMouseMoved = true;
        }
    }
}

void Game::update()
{
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    // UI with dropdown
    ImGui::Begin("THEMES");
    if (ImGui::BeginCombo("Choose Theme##Combo", texture_of_piece[current_item].c_str()))
    { // "##Combo" ensures a valid ID
        for (int i = 0; i < texture_of_piece.size(); ++i)
        {
            bool is_selected = (current_item == i);
            if (ImGui::Selectable(texture_of_piece[i].c_str(), is_selected))
                current_item = i;
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    ImGui::End();

    
    char buffer[128] = ""; // Buffer to store the input text
    ImGui::Begin("Load FEN"); // Start a window
    if (ImGui::InputText("FEN", buffer, sizeof(buffer)))
    {
    } // Create the text box
    ImGui::End();

    if (buffer[0] != '\0')
    {
        fen = buffer;
        board.setFEN(fen);
    }

    if (current_item >= 0 && current_item < texture_of_piece.size())
    {
        pieceTexture = piecesTextures[texture_of_piece[current_item]];
    }
}

void Game::render()
{
    ImGui::Render();
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 0);
    SDL_RenderClear(renderer);

    // all rendering is here

    board.renderBoard(renderer, boardTexture);

    if (isMouseMoved)
    {
        SDL_FRect grid = {(float)(mX * CELL_SIZE), (float)(mY * CELL_SIZE), CELL_SIZE, CELL_SIZE};
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 0);
        SDL_RenderFillRect(renderer, &grid);
    }
    
    piece.renderPieces(renderer, board, pieceTexture, CELL_SIZE);


    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
    SDL_RenderPresent(renderer);
}


void mousePrinting(SDL_Event e, SDL_Renderer *renderer)
{
    float mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    mX = (int)(mouseX / CELL_SIZE);
    mY = (int)(mouseY / CELL_SIZE);
}



SDL_Texture *Game::loadTexture(const char *path)
{
    SDL_Surface *tempSurface = IMG_Load(path);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, tempSurface);
    SDL_DestroySurface(tempSurface);
    return texture;
}

bool Game::loadTexturesFromFolder(
    const std::string &folder_path,
    std::unordered_map<std::string, SDL_Texture *> &targetMap,
    std::vector<std::string> &textureNames)
{
    try
    {
        for (const auto &entry : fs::directory_iterator(folder_path))
        {
            if (entry.is_regular_file())
            {
                std::string filename = entry.path().filename().string();
                std::string textureName = filename.substr(0, filename.find_last_of('.'));

                SDL_Texture *tex = loadTexture((folder_path + "/" + filename ).c_str());
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
