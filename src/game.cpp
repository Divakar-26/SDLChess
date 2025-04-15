#include "game.h"

#define CELL_SIZE 80

std::vector<std::string> texture_of_piece;
std::vector<std::string> texture_of_board;

int mX, mY;

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
        else if(e.type == SDL_EVENT_MOUSE_MOTION && isMouseMoved && e.button.button == SDL_BUTTON_LEFT){
            std::cout<<"i am in"<<std::endl;
            if(mouseDown && !isPickedUp){
                int dx = std::abs(e.motion.x - mouseDownX);
                int dy = std::abs(e.motion.y - mouseDownY);

                if(dx > dragThreshold || dy > dragThreshold){
                    int x = e.button.x / CELL_SIZE;
                    int y = e.button.y / CELL_SIZE;
        
                    if (board.hasPieceAt(y, x) && !isPickedUp)
                    {
        
                        float mouseX, mouseY;
                        SDL_GetMouseState(&mouseX, &mouseY);
        
                        pickedUppiece.x = mouseX;
                        pickedUppiece.y = mouseY;
        
                        pickedUppiece.offsetX = mouseX - (x * CELL_SIZE);
                        pickedUppiece.offsetY = mouseY - (y * CELL_SIZE);
        
                        pickedUppiece.piece = board.getPiecesAt(y, x);
                        std::cout << pickedUppiece.piece << std::endl;
        
                        pickedUppiece.originalX = mouseX;
                        pickedUppiece.originalY = mouseY;
        
                        board.clearPieceAt(y, x);
        
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

            mousePrinting(e, renderer);
            isMouseMoved = true;
        }
        else if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN )
        {
            mouseDown = true;
            mouseDownX = e.button.x;
            mouseDownY = e.button.y;

            int x = e.button.x / CELL_SIZE;
            int y = e.button.y / CELL_SIZE;

            // Handle selection and deselection logic
            if (board.hasPieceAt(y, x))
            {
                if (isPieceSelected && selectedX == x && selectedY == y)
                {
                    // If the clicked piece is already selected, deselect it
                    isPieceSelected = false;
                    selectedX = -1;
                    selectedY = -1;
                    std::cout << "Deselected piece at " << x << ", " << y << std::endl;
                }
                else
                {
                    // If a different piece is clicked, select it
                    isPieceSelected = true;
                    selectedX = x;
                    selectedY = y;
                    std::cout << "Selected piece at " << x << ", " << y << std::endl;
                }
            }
            else
            {
                // If no piece is at the clicked location, deselect any selected piece
                isPieceSelected = false;
                selectedX = -1;
                selectedY = -1;
                std::cout << "Deselected piece due to empty space at " << x << ", " << y << std::endl;
            }
        }
        else if (e.type == SDL_EVENT_MOUSE_BUTTON_UP && e.button.button == SDL_BUTTON_LEFT)
        {
            if (isPickedUp)
            {
                std::cout << pickedUppiece.originalX << " " << pickedUppiece.originalY << std::endl;
                board.setPieceAt(e.button.x, e.button.y, pickedUppiece.piece, CELL_SIZE);

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
    if (ImGui::BeginCombo("Choose Theme##Combo", texture_of_piece[current_item].c_str()))
    { // "##Combo" ensures a valid ID
        for (int i = 0; i < static_cast<int>(texture_of_piece.size()); ++i)
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

    static char buffer[128] = ""; // Buffer to store the input text
    ImGui::Begin("Load FEN");     // Start a window
    ImGui::InputText("FEN", buffer, sizeof(buffer));
    ImGui::SameLine();
    if (ImGui::Button("Load"))
    {
        fen = buffer;
        board.setFEN(fen);
    }
    ImGui::End();

    // if (buffer[0] != '\0')
    // {
    //     fen = buffer;
    //     board.setFEN(fen);
    // }

    if (current_item >= 0 && current_item < static_cast<int>(texture_of_piece.size()))
    {
        pieceTexture = piecesTextures[texture_of_piece[current_item]];
    }

    if (isPickedUp)
    {
        float mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        pickedUppiece.x = mouseX;
        pickedUppiece.y = mouseY;
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
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 100); // Yellow with alpha
        SDL_RenderFillRect(renderer, &highlight);
    }

    if (isMouseMoved)
    {
        SDL_FRect grid = {(float)(mX * CELL_SIZE), (float)(mY * CELL_SIZE), CELL_SIZE, CELL_SIZE};
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 0);
        SDL_RenderFillRect(renderer, &grid);
    }

    piece.renderPieces(renderer, board, pieceTexture, CELL_SIZE);

    if (isPickedUp)
    {
        float drawX = pickedUppiece.x - pickedUppiece.offsetX;
        float drawY = pickedUppiece.y - pickedUppiece.offsetY;

        piece.renderPieceAt(renderer, pickedUppiece.piece, drawY, drawX, pieceTexture, CELL_SIZE);
    }

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
