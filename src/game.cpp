#include "game.h"

#define CELL_SIZE 80

struct Piece
{
    float texX, texY;
    std::string name;
    int worth;
};

Piece q = {0, 0, "Black Queen"};
Piece k = {200, 0, "Black King"};
Piece r = {400, 0, "Black Rook"};
Piece n = {600, 0, "Black Knight"};
Piece b = {800, 0, "Black Bishop"};
Piece p = {1000, 0, "Black Pawn"};

Piece Q = {0, 200, "White Queen"};
Piece K = {200, 200, "White King"};
Piece R = {400, 200, "White Rook"};
Piece N = {600, 200, "White Knight"};
Piece B = {800, 200, "White Bishop"};
Piece P = {1000, 200, "White Pawn"};

std::vector<std::string> items;

std::unordered_map<char, Piece> pieces = {
    {'q', q},
    {'k', k},
    {'r', r},
    {'n', n},
    {'b', b},
    {'p', p},

    {'Q', Q},
    {'K', K},
    {'R', R},
    {'N', N},
    {'B', B},
    {'P', P}};

SDL_Texture *pieceTexture;
SDL_Texture *boardTexture;
int mX, mY;
void renderBoard(SDL_Renderer *renderer);
void mousePrinting(SDL_Event e, SDL_Renderer *renderer);
bool isMouseMoved = false;
void renderPieces(SDL_Renderer *renderer);
void renderPiece(SDL_Renderer *renderer, char piece, int x, int y);
void setFEN(std::string fen);

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
    setFEN(fen);

    std::string folder_path = "assets"; // Replace with your folder path
    std::vector<std::string> filenames;
    int i = 0;
    try
    {
        for (const auto &entry : fs::directory_iterator(folder_path))
        {
            if (entry.is_regular_file())
            {
                filenames.push_back(entry.path().filename().string());
            }
        }
        for (const auto &name : filenames)
        {
            std::string textureFileName = name.substr(0, name.find_last_of('.'));
            items.push_back(textureFileName);
            SDL_Texture *tex = loadTexture(("assets/" + name).c_str());
            if (tex)
            {
                textures.insert({textureFileName, tex});
            }
            else
            {
                SDL_Log("Failed to load texture: %s", name.c_str());
            }
        }
    }
    catch (const fs::filesystem_error &e)
    {
    }

    pieceTexture = textures["cburnett"];
    boardTexture = textures["board"];

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
        else if (e.type == SDL_EVENT_KEY_DOWN)
        {
            switch (e.key.key)
            {
            case SDLK_E:
                pieceTexture = textures["rhosgfx"];
                break;

            default:
                break;
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
    if (ImGui::BeginCombo("Choose Theme##Combo", items[current_item].c_str()))
    { // "##Combo" ensures a valid ID
        for (int i = 0; i < items.size(); ++i)
        {
            bool is_selected = (current_item == i);
            if (ImGui::Selectable(items[i].c_str(), is_selected))
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
        setFEN(fen);
    }

    if (current_item >= 0 && current_item < items.size())
    {
        pieceTexture = textures[items[current_item]];
    }
}

void Game::render()
{
    ImGui::Render();
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 0);
    SDL_RenderClear(renderer);

    // all rendering is here

    renderBoard(renderer);
    if (isMouseMoved)
    {
        SDL_FRect grid = {(float)(mX * CELL_SIZE), (float)(mY * CELL_SIZE), CELL_SIZE, CELL_SIZE};
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 0);
        SDL_RenderFillRect(renderer, &grid);
    }

    renderPieces(renderer);
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);

    SDL_RenderPresent(renderer);
}

void renderBoard(SDL_Renderer *renderer)
{
    SDL_FRect srcRect = {0, 0, 512, 512};
    SDL_FRect destRect = {0, 0, 640, 640};

    SDL_RenderTexture(renderer, boardTexture, &srcRect, &destRect);
}
void mousePrinting(SDL_Event e, SDL_Renderer *renderer)
{
    float mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    mX = (int)(mouseX / CELL_SIZE);
    mY = (int)(mouseY / CELL_SIZE);
}

void renderPieces(SDL_Renderer *renderer)
{
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if (board[i][j] == '.')
            {
                continue;
            }
            renderPiece(renderer, board[i][j], i, j);
        }
    }
}

void renderPiece(SDL_Renderer *renderer, char piece, int x, int y)
{

    SDL_FRect srcRect = {pieces[piece].texX, pieces[piece].texY, 200, 200};
    SDL_FRect destRect = {(float)(y * CELL_SIZE), (float)(x * CELL_SIZE), CELL_SIZE, CELL_SIZE};
    SDL_RenderTexture(renderer, pieceTexture, &srcRect, &destRect);
}

SDL_Texture *Game::loadTexture(const char *path)
{
    SDL_Surface *tempSurface = IMG_Load(path);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, tempSurface);
    SDL_DestroySurface(tempSurface);
    return texture;
}

void setFEN(std::string fen)
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