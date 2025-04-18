#include "game.h"

#define CELL_SIZE 80

std::vector<std::string> texture_of_piece;
std::vector<std::string> texture_of_board;
std::vector<std::pair<int,int>> moves;


bool piece_texture_changed = false;
bool board_texture_changed = false;

int mX, mY;
int hoveringSquareX;
int hoveringSquareY;

void mousePrinting(SDL_Event e, SDL_Renderer *renderer);
bool isMouseMoved = false;
bool hasMovedAfterPickup = false;

// draggin

// picking up
struct PickedUpPiece
{
    char piece;
    float x, y;
    float offsetX, offsetY;
    int originalX, originalY;
};

bool isPickedUp = false;
PickedUpPiece pickedUppiece;

bool isLeftMouseButtonDown = false;
bool isPieceSelected = false;
int selectedX = -1, selectedY = -1;

bool mouseDown = false;
int mouseDownX = 0, mouseDownY = 0;
const int dragThreshold = 4; // Pixels to move before it's considered a drag

bool isInLegalMoves(std::vector<std::pair<int,int>> &moves, int x, int y);


Game::Game(int W_W, int W_H)
{
    WINDOW_H = W_H;
    WINDOW_W = W_W;
}

Game::~Game() {}

bool Game::init(const char *title, char *fenInput, int args)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) == 0)
    {
        return false;
    }

    if (Mix_Init(MIX_INIT_MP3 | MIX_INIT_OGG) == 0)
    {
        return false;
    }

    SDL_AudioSpec desired{};
    desired.freq = 48000;
    desired.format = SDL_AUDIO_F32;
    desired.channels = 2;

    if (Mix_OpenAudio(0, &desired) == 0)
    {
        SDL_Log(SDL_GetError());
        // return false;
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

    pieceTexture = piecesTextures["pixel"];
    boardTexture = boardTextures["8_bit"];

    if (!boardTexture || !pieceTexture)
    {
        SDL_Log("Failed to load texture!");
        return false;
    }

    placeSound = Mix_LoadMUS("Move.mp3");
    if (!placeSound)
    {
        SDL_Log("Failed to load music: %s", SDL_GetError());
        return -1;
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
        else if (e.type == SDL_EVENT_MOUSE_MOTION && isMouseMoved && e.button.button == SDL_BUTTON_LEFT)
        {
            // std::cout << "i am in" << std::endl;

            hoveringSquareX = e.motion.x / (CELL_SIZE);
            hoveringSquareY = e.motion.y / (CELL_SIZE);

            if (mouseDown && !isPickedUp)
            {
                int dx = std::abs(e.motion.x - mouseDownX);
                int dy = std::abs(e.motion.y - mouseDownY);

                if (dx > dragThreshold || dy > dragThreshold)
                {
                    int x = e.button.x / CELL_SIZE;
                    int y = e.button.y / CELL_SIZE;

                    if (board.hasPieceAt(x, y) && !isPickedUp && (e.button.x >= 0 && e.button.x <= 640 && e.button.y >= 0 && e.button.y <= 640))
                    {

                        float mouseX, mouseY;
                        SDL_GetMouseState(&mouseX, &mouseY);

                        pickedUppiece.x = mouseX;
                        pickedUppiece.y = mouseY;

                        pickedUppiece.offsetX = mouseX - (x * CELL_SIZE);
                        pickedUppiece.offsetY = mouseY - (y * CELL_SIZE);

                        pickedUppiece.piece = board.getPiecesAt(x, y);
                        // std::cout << pickedUppiece.piece << std::endl;

                        pickedUppiece.originalX = x;
                        pickedUppiece.originalY = y;

                        board.clearPieceAt(x, y);

                        isPickedUp = true;

                        // selection
                        isPieceSelected = true;
                        selectedX = x;
                        selectedY = y;

                        
                    }
                }
            }
        }
        else if (e.type == SDL_EVENT_MOUSE_MOTION)
        {
            if (isPickedUp)
            {
                pickedUppiece.x = e.motion.x;
                pickedUppiece.y = e.motion.y;
                hasMovedAfterPickup = true;
            }

            isMouseMoved = true;
        }
        else if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
        {
            mouseDown = true;
            mouseDownX = e.button.x;
            mouseDownY = e.button.y;

            int x = e.button.x / CELL_SIZE;
            int y = e.button.y / CELL_SIZE;

            moves = piece.legalMoves(x, y, board);

            std::cout<<x<<" ------> "<<y<<std::endl;

            // Handle selection and deselection logic
            if (board.hasPieceAt(x, y) && (e.button.x >= 0 && e.button.x <= 640 && e.button.y >= 0 && e.button.y <= 640))
            {
                if (isPieceSelected && selectedX == x && selectedY == y)
                {
                    // If the clicked piece is already selected, deselect it
                    isPieceSelected = false;
                    selectedX = -1;
                    selectedY = -1;
                    // std::cout << "Deselected piece at " << x << ", " << y << std::endl;
                }
                else
                {
                    // If a different piece is clicked, select it
                    isPieceSelected = true;
                    selectedX = x;
                    selectedY = y;
                    // std::cout << "Selected piece at " << x << ", " << y << std::endl;
                }
            }
            else
            {
                // If no piece is at the clicked location, deselect any selected piece
                isPieceSelected = false;
                selectedX = -1;
                selectedY = -1;
                // std::cout << "Deselected piece due to empty space at " << x << ", " << y << std::endl;
            }
        }

        else if (e.type == SDL_EVENT_MOUSE_BUTTON_UP && e.button.button == SDL_BUTTON_LEFT)
        {
            int x = e.button.x / CELL_SIZE;
            int y = e.button.y / CELL_SIZE;

            if (isPickedUp && (e.button.x < 0 || e.button.x > 640 || e.button.y < 0 || e.button.y > 640))
            {
                board.setPieceAt(pickedUppiece.originalX, pickedUppiece.originalY, pickedUppiece.piece, CELL_SIZE);
                isPickedUp = false;
                isPieceSelected = false;
                mouseDown = false;
                return;
            }
            else if (isPickedUp && isInLegalMoves(moves, x, y))
            {
                board.setPieceAt(x,y, pickedUppiece.piece, CELL_SIZE);
                Mix_PlayMusic(placeSound, 1);
                isPickedUp = false;
                isPieceSelected = false;
                mouseDown = false;
                return;
            }
            else if(isPickedUp && !isInLegalMoves(moves, x, y) ){ 
                board.setPieceAt(pickedUppiece.originalX, pickedUppiece.originalY, pickedUppiece.piece, CELL_SIZE);
                isPickedUp = false;
                isPieceSelected = false;
                mouseDown = false;
                return;
            }

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
    if (ImGui::BeginCombo("Choose Theme##Combo", texture_of_piece[current_item_piece_theme].c_str()))
    { // "##Combo" ensures a valid ID
        for (int i = 0; i < static_cast<int>(texture_of_piece.size()); ++i)
        {
            bool is_selected = (current_item_piece_theme == i);
            if (ImGui::Selectable(texture_of_piece[i].c_str(), is_selected))
                current_item_piece_theme = i;
            if (is_selected)
            {
                ImGui::SetItemDefaultFocus();
                piece_texture_changed = true;
            }
        }
        ImGui::EndCombo();
    }

    if (ImGui::BeginCombo("Choose Board Theme##Combo", texture_of_board[current_item_board_theme].c_str()))
    {
        for (int i = 0; i < static_cast<int>(texture_of_board.size()); ++i)
        {
            bool is_selected = (current_item_board_theme == i);
            if (ImGui::Selectable(texture_of_board[i].c_str(), is_selected))
                current_item_board_theme = i;
            if (is_selected)
            {
                ImGui::SetItemDefaultFocus();
                board_texture_changed = true;
            }
        }
        ImGui::EndCombo();
    }

    ImGui::End();

    static char buffer[128] = ""; // Buffer to store the input text
    ImGui::Begin("Load FEN");     // Start a window

    ImGui::InputText("FEN", buffer, sizeof(buffer));
    ImGui::SameLine();
    if (ImGui::Button("Load"))
    {
        fen = buffer;
        board.setFEN(fen);
    }
    if(ImGui::Button("RESET")){
        fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
        board.setFEN(fen);
    }
    ImGui::End();

    if (current_item_piece_theme < 0 || current_item_piece_theme >= texture_of_piece.size() && !piece_texture_changed)
    {
        pieceTexture = piecesTextures["pixel"];
    }
    else
    {
        pieceTexture = piecesTextures[texture_of_piece[current_item_piece_theme]];
    }

    if (current_item_board_theme < 0 || current_item_board_theme >= texture_of_board.size() && !board_texture_changed)
    {
        boardTexture = boardTextures["wood"];
    }
    else
    {
        boardTexture = boardTextures[texture_of_board[current_item_board_theme]];
    }

    if (isPickedUp)
    {
        float mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        pickedUppiece.x = mouseX;
        pickedUppiece.y = mouseY;
        // std::cout << board.getPiecesAt(selectedX, selectedY) << std::endl;
    }
}

void Game::render()
{
    ImGui::Render();
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 0);
    SDL_RenderClear(renderer);

    // all rendering is here

    board.renderBoard(renderer, boardTexture);

    if (isPieceSelected)
    {
        SDL_FRect highlight = {
            selectedX * CELL_SIZE + 1,
            selectedY * CELL_SIZE + 1,
            CELL_SIZE - 2,
            CELL_SIZE - 2};
            
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 100);
            SDL_RenderFillRect(renderer, &highlight);
            
        for(auto it: moves){
            board.highLightSquare(it.first, it.second, CELL_SIZE, renderer);
            std::cout<<it.first<<"--->"<<it.second<<std::endl;
        }
    }

    
    piece.renderPieces(renderer, board, pieceTexture, CELL_SIZE);
    if (isPickedUp)
    {
        float drawX = pickedUppiece.x - pickedUppiece.offsetX;
        float drawY = pickedUppiece.y - pickedUppiece.offsetY;
        piece.renderPieceAt(renderer, pickedUppiece.piece, drawY, drawX, pieceTexture, CELL_SIZE);
        
        
        SDL_SetRenderDrawColor(renderer, 255,255,255,100);
        for(int i= 0; i < 4; i++){
            
            SDL_FRect outline = {hoveringSquareX * CELL_SIZE + i, hoveringSquareY * CELL_SIZE + i, CELL_SIZE - 2 * i, CELL_SIZE - 2 * i};
            SDL_RenderRect(renderer, &outline);
        }
    }

    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
    SDL_RenderPresent(renderer);
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

                // std::cout << textureName << std::endl;

                SDL_Texture *tex = loadTexture((folder_path + "/" + filename).c_str());
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

void Game::cleanup()
{
    for (auto &[_, tex] : piecesTextures)
        SDL_DestroyTexture(tex);
    for (auto &[_, tex] : boardTextures)
        SDL_DestroyTexture(tex);

    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Game::renderPickedUPPiece()
{
    piece.renderPieceAt(renderer, pickedUppiece.piece, pickedUppiece.y, pickedUppiece.x, pieceTexture, CELL_SIZE);
}

bool isInLegalMoves(std::vector<std::pair<int,int>> & moves, int x, int y){

    for(auto it: moves){
        if(it.first == x && it.second == y){
            return true;
        }
    }

    return false;
}