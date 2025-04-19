#include "game.h"
#include<math.h>
struct Animation {
    char piece;
    int fromX, fromY;
    float toX, toY;
    float currentX, currentY;
    float speed = 10.0f;
    bool active = false;
};
Animation currentAnim;
int animFinalTargetX = 0;
int animFinalTargetY = 0;


#define CELL_SIZE 80
bool whiteTurn = true; // true = white's turn, false = black's

std::vector<std::string> texture_of_piece;
std::vector<std::string> texture_of_board;
std::vector<std::pair<int, int>> moves;

bool piece_texture_changed = false;
bool board_texture_changed = false;

int mX, mY;
int hoveringSquareX;
int hoveringSquareY;

bool isMouseMoved = false;
bool hasMovedAfterPickup = false;

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

bool isInLegalMoves(std::vector<std::pair<int, int>> &moves, int x, int y);

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

    ui.init(window, renderer);

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

            // std::cout << x << " ------> " << y << std::endl;
            if(isPieceSelected){
                if(isInLegalMoves(moves, x, y)){

                    currentAnim = {
                        .piece = board.getPiecesAt(selectedX, selectedY),
                        .fromX = selectedX,
                        .fromY = selectedY,
                        .toX = (float)x * CELL_SIZE,
                        .toY = (float)y * CELL_SIZE,
                        .currentX = (float)selectedX * CELL_SIZE,
                        .currentY = (float)selectedY * CELL_SIZE,
                        .speed = 2.3f,
                        .active = true,
                    };


                    board.setPieceAt(x, y , '.', CELL_SIZE);
                    // board.clearPieceAt(selectedX, selectedY);

                    animFinalTargetX = x;
                    animFinalTargetY = y;


                    whiteTurn = !whiteTurn;

                    std::cout<<"Piece moved - "<<board.getPiecesAt(x,y)<<std::endl;
                    Mix_PlayMusic(placeSound, 1);

                }
            }
            // Handle selection and deselection logic
            if (board.hasPieceAt(x, y) && (e.button.x >= 0 && e.button.x <= 640 && e.button.y >= 0 && e.button.y <= 640))
            {

                char piecea = board.getPiecesAt(x, y);
                // Block pickup if it’s the wrong turn
                if ((whiteTurn && islower(piecea)) || (!whiteTurn && isupper(piecea)))
                {
                    moves.clear();
                    isPieceSelected = false;
                    selectedX = -1;
                    selectedY = -1;
                    return;
                }
                moves = piece.legalMoves(x, y, board);

                if (isPieceSelected && selectedX == x && selectedY == y)
                {
                    // If the clicked piece is already selected, deselect it
                    isPieceSelected = false;
                    selectedX = -1;
                    selectedY = -1;
                }
                else
                {
                    // If a different piece is clicked, select it
                    isPieceSelected = true;
                    selectedX = x;
                    selectedY = y;
                }
            }
            else
            {
                // If no piece is at the clicked location, deselect any selected piece
                isPieceSelected = false;
                selectedX = -1;
                selectedY = -1;
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
                if ((whiteTurn && islower(pickedUppiece.piece)) || (!whiteTurn && isupper(pickedUppiece.piece)))
                {
                    // Invalid move
                    board.setPieceAt(pickedUppiece.originalX, pickedUppiece.originalY, pickedUppiece.piece, CELL_SIZE);
                    isPickedUp = false;
                    isPieceSelected = false;
                    mouseDown = false;
                    return;
                }

                board.setPieceAt(x, y, pickedUppiece.piece, CELL_SIZE);
                if (isInCheck(!whiteTurn))
                {
                    std::cout << (whiteTurn ? "Black" : "White") << " is in check!" << std::endl;
                }

                whiteTurn = !whiteTurn;

                Mix_PlayMusic(placeSound, 1);
                isPickedUp = false;
                isPieceSelected = false;
                mouseDown = false;
                return;
            }
            else if (isPickedUp && !isInLegalMoves(moves, x, y))
            {
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
    ui.update();
    // UI with dropdown
    ui.dropDown(texture_of_piece, current_item_piece_theme, piece_texture_changed, texture_of_board, current_item_board_theme, board_texture_changed);
    // UI of FEN LOADER
    ui.loadFEN(fen, board);

    if (current_item_piece_theme < 0 || current_item_piece_theme >= static_cast<int>(texture_of_piece.size()))
    {
        pieceTexture = piecesTextures["pixel"];
    }
    else
    {
        pieceTexture = piecesTextures[texture_of_piece[current_item_piece_theme]];
    }

    if (current_item_board_theme < 0 || current_item_board_theme >= static_cast<int>(texture_of_board.size()))
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
            (float)(selectedX * CELL_SIZE + 1),
            (float)(selectedY * CELL_SIZE + 1),
            (float)(CELL_SIZE - 2),
            (float)(CELL_SIZE - 2)};

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 100);
        SDL_RenderFillRect(renderer, &highlight);

        for (auto it : moves)
        {
            board.highLightSquare(it.first, it.second, CELL_SIZE, renderer);
            std::cout << it.first << "--->" << it.second << std::endl;
        }
    }

    piece.renderPieces(renderer, board, pieceTexture, CELL_SIZE);
    if (isPickedUp)
    {
        float drawX = pickedUppiece.x - pickedUppiece.offsetX;
        float drawY = pickedUppiece.y - pickedUppiece.offsetY;
        piece.renderPieceAt(renderer, pickedUppiece.piece, drawY, drawX, pieceTexture, CELL_SIZE);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 100);
        for (int i = 0; i < 4; i++)
        {

            SDL_FRect outline = {(float)(hoveringSquareX * CELL_SIZE + i), (float)(hoveringSquareY * CELL_SIZE + i), (float)(CELL_SIZE - 2 * i), (float)(CELL_SIZE - 2 * i)};
            SDL_RenderRect(renderer, &outline);
        }
    }

    if (currentAnim.active) {
        float dx = currentAnim.toX - currentAnim.currentX;
        float dy = currentAnim.toY - currentAnim.currentY;
        float dist = sqrt(dx * dx + dy * dy);
    
        if (dist < currentAnim.speed) {
            // Snap to final position
            currentAnim.currentX = currentAnim.toX;
            currentAnim.currentY = currentAnim.toY;
            currentAnim.active = false;
    
            // Place piece at final destination
            board.setPieceAt(animFinalTargetX, animFinalTargetY, currentAnim.piece, CELL_SIZE);
            board.clearPieceAt(currentAnim.fromX, currentAnim.fromY);
        } else {
            // Move a bit closer
            currentAnim.currentX += currentAnim.speed * (dx / dist);
            currentAnim.currentY += currentAnim.speed * (dy / dist);
        }
    
        // Draw the animated piece
        piece.renderPieceAt(renderer, currentAnim.piece, currentAnim.currentY, currentAnim.currentX, pieceTexture, CELL_SIZE);
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

bool isInLegalMoves(std::vector<std::pair<int, int>> &moves, int x, int y)
{

    for (auto it : moves)
    {
        if (it.first == x && it.second == y)
        {
            return true;
        }
    }
    return false;
}

bool Game::isInCheck(bool isWhite)
{
    int kingRow = -1;
    int kingCol = -1;
    char kingChar = isWhite ? 'K' : 'k';
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if (board.getPiecesAt(j, i) == kingChar)
            {
                kingRow = j;
                kingCol = i;
            }
        }
    }

    std::cout << kingRow << " " << kingCol << std::endl;

    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {

            char pieceChar = board.getPiecesAt(j, i);

            if (pieceChar == '.' || (isWhite && isupper(pieceChar)) || (!isWhite && islower(pieceChar)))
            {
                continue;
            }

            auto pieceMoves = piece.legalMoves(j, i, board);

            if (isInLegalMoves(pieceMoves, kingRow, kingCol))
            {
                return true;
            }
        }
    }

    return false;
}
