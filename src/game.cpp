#include "game.h"
#include <math.h>

bool isWhiteKingMoved = false;
bool isBlackKingMoved = false;
bool isWhiteARookMoved = false;
bool isWhiteHRookMoved = false;
bool isBlackARookMoved = false;
bool isBlackHRookMoved = false;

int enPassantTarget = -1; // -1 means no en passant target, otherwise store as 0-63 index
int enPassantPawnX = -1;  // Row of the pawn that can be captured en passant
int enPassantPawnY = -1;  // Column of the pawn that can be captured en passant

struct Animation
{
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

void changeState(int x, int y, char c);
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

    textureManager.init(pieceTexture, boardTexture, piecesTextures, boardTextures, texture_of_piece, texture_of_board, renderer);

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

                    std::cout<<"HELLO"<<std::endl;

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
            if (isPieceSelected)
            {
                if (isInLegalMoves(moves, x, y))
                {

                    char movingPiece = board.getPiecesAt(selectedX, selectedY);

                    if ((movingPiece == 'p' || movingPiece == 'P') && abs(selectedY - y) == 2)
                    {
                        // Set en passant target to the square behind the pawn (in x coordinate)
                        enPassantTarget = to64(selectedX, (selectedY + y) / 2);
                        enPassantPawnX = x; // store column
                        enPassantPawnY = y; // store row
                        std::cout << "En passant at " << x << " " << y << std::endl;
                    }
                    else
                    {
                        enPassantTarget = -1;
                    }

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

                    changeState(x, y, board.getPiecesAt(selectedX, selectedY));

                    if (board.getPiecesAt(selectedX, selectedY) == 'K')
                    {
                        if (x == 6 && y == 7)
                        {
                            board.setPieceAt(5, 7, 'R', CELL_SIZE);
                            board.clearPieceAt(7, 7);
                        }
                        else if (x == 2 && y == 7)
                        {
                            board.setPieceAt(3, 7, 'R', CELL_SIZE);
                            board.clearPieceAt(0, 7);
                        }
                    }
                    if (board.getPiecesAt(selectedX, selectedY) == 'k')
                    {
                        if (x == 6 && y == 0)
                        {
                            board.setPieceAt(5, 0, 'r', CELL_SIZE);
                            board.clearPieceAt(7, 0);
                        }
                        else if (x == 2 && y == 0)
                        {
                            board.setPieceAt(3, 0, 'r', CELL_SIZE);
                            board.clearPieceAt(0, 0);
                        }
                    }

                    board.clearPieceAt(x, y);
                    board.clearPieceAt(selectedX, selectedY);
                    animFinalTargetX = x;
                    animFinalTargetY = y;

                    whiteTurn = !whiteTurn;

                    std::cout << "Piece moved - " << board.getPiecesAt(x, y) << std::endl;
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

                if (board.getPiecesAt(x, y) == 'p' || board.getPiecesAt(x, y) == 'P')
                {
                    moves = getPawnMovesWithEnPassant(x, y, board);
                }
                else
                {
                    moves = piece.legalMoves(x, y, board);
                }

                moves = filtermoves(moves, x, y, board.getPiecesAt(x, y));

                if (board.getPiecesAt(x, y) == 'K' || board.getPiecesAt(x, y) == 'k')
                {
                    (canCastle(x, y, board.getPiecesAt(x, y), board, moves));
                }

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

                if (pickedUppiece.piece == 'K')
                {
                    if (x == 6 && y == 7)
                    {
                        board.setPieceAt(6, 7, 'K', CELL_SIZE);
                        board.setPieceAt(5, 7, 'R', CELL_SIZE);
                        board.clearPieceAt(7, 7);
                    }
                    else if (x == 2 && y == 7)
                    {
                        board.setPieceAt(2, 7, 'K', CELL_SIZE);
                        board.setPieceAt(3, 7, 'R', CELL_SIZE);
                        board.clearPieceAt(0, 7);
                    }
                }
                if (pickedUppiece.piece == 'k')
                {
                    if (x == 6 && y == 0)
                    {
                        board.setPieceAt(6, 0, 'k', CELL_SIZE);
                        board.setPieceAt(5, 0, 'r', CELL_SIZE);
                        board.clearPieceAt(7, 0);
                    }
                    else if (x == 2 && y == 0)
                    {
                        board.setPieceAt(2, 0, 'k', CELL_SIZE);
                        board.setPieceAt(3, 0, 'r', CELL_SIZE);
                        board.clearPieceAt(0, 0);
                    }
                }

                board.setPieceAt(x, y, pickedUppiece.piece, CELL_SIZE);

                if (isInCheck(!whiteTurn, board))
                {
                    std::cout << (whiteTurn ? "Black" : "White") << " is in check!" << std::endl;
                }

                changeState(x, y, board.getPiecesAt(x, y));

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
    ui.dropDown(texture_of_piece, current_item_piece_theme, texture_of_board, current_item_board_theme);
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

        for (const auto &[x, y] : moves)
        {
            board.highLightSquare(x, y, CELL_SIZE, renderer);
            std::cout << x << "--->" << y << std::endl;
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

    if (currentAnim.active)
    {
        float dx = currentAnim.toX - currentAnim.currentX;
        float dy = currentAnim.toY - currentAnim.currentY;
        float dist = sqrt(dx * dx + dy * dy);

        if (dist < currentAnim.speed)
        {
            // Snap to final position
            currentAnim.currentX = currentAnim.toX;
            currentAnim.currentY = currentAnim.toY;
            currentAnim.active = false;

            // Check for en passant capture
            if ((currentAnim.piece == 'P' || currentAnim.piece == 'p') &&
                currentAnim.fromX != animFinalTargetX &&                      // Diagonal move
                board.getPiecesAt(animFinalTargetX, animFinalTargetY) == '.') // Landing on empty square
            {
                // Verify this is an en passant capture

                std::cout << xFrom64(enPassantTarget) << " -> " << currentAnim.fromY << " -> " << animFinalTargetX << std::endl;
                int capturedPawnX = animFinalTargetX;
                int capturedPawnY = currentAnim.fromY;
                board.clearPieceAt(capturedPawnX, capturedPawnY);

                // if (enPassantTarget != -1 &&
                //     animFinalTargetX == xFrom64(enPassantTarget) &&
                //     ((currentAnim.piece == 'P' && currentAnim.fromY == 3 && animFinalTargetY == 2) ||  // White capturing
                //     (currentAnim.piece == 'p' && currentAnim.fromY == 4 && animFinalTargetY == 5)))    // Black capturing
                //     {
                //         // Remove the captured pawn (same file as destination, original rank)
                //     std::cout<<"en paddant"<<std::endl;
                //     std::cout << "En passant captured pawn at " << capturedPawnX << "," << capturedPawnY << std::endl;
                // }
            }

            // Place piece at final destination
            board.setPieceAt(animFinalTargetX, animFinalTargetY, currentAnim.piece, CELL_SIZE);
            board.clearPieceAt(currentAnim.fromX, currentAnim.fromY);
        }
        else
        {
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

void Game::cleanup()
{
    for (auto &[_, tex] : piecesTextures)
        SDL_DestroyTexture(tex);
    for (auto &[_, tex] : boardTextures)
        SDL_DestroyTexture(tex);

    if (placeSound)
    {
        Mix_FreeMusic(placeSound);
        placeSound = nullptr;
    }

    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    Mix_CloseAudio();
    Mix_Quit();
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

bool Game::isInCheck(bool isWhite, Chessboard &customBoard)
{
    int kingRow = -1;
    int kingCol = -1;
    char kingChar = isWhite ? 'K' : 'k';
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if (customBoard.getPiecesAt(j, i) == kingChar)
            {
                kingRow = j;
                kingCol = i;
            }
        }
    }
    // std::cout << kingRow << " " << kingCol << std::endl;
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {

            char pieceChar = customBoard.getPiecesAt(j, i);

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

void changeState(int x, int y, char c)
{
    switch (c)
    {
    case 'k':
        isBlackKingMoved = true;
        break;
    case 'K':
        isWhiteKingMoved = true;
        break;
    case 'r':
        if (x == 0)
        {
            isBlackARookMoved = true;
        }
        if (x == 7)
        {
            isBlackHRookMoved = true;
        }
        break;
    case 'R':
        if (x == 0)
        {
            isWhiteARookMoved = true;
        }
        if (x == 7)
        {
            isWhiteHRookMoved = true;
        }
        break;
    default:
        break;
    }
}

bool Game::canCastle(int x, int y, char p, Chessboard &board, std::vector<std::pair<int, int>> &moves)
{
    if (isInCheck(whiteTurn, board))
    {
        return false;
    }

    if (p == 'K') // White king
    {
        // Kingside
        if (!isWhiteKingMoved && !isWhiteHRookMoved &&
            board.getPiecesAt(5, 7) == '.' &&
            board.getPiecesAt(6, 7) == '.')
        {
            moves.emplace_back(6, 7); // King lands on g1
        }

        // Queenside
        if (!isWhiteKingMoved && !isWhiteARookMoved &&
            board.getPiecesAt(1, 7) == '.' && // optional b1
            board.getPiecesAt(2, 7) == '.' &&
            board.getPiecesAt(3, 7) == '.')
        {
            moves.emplace_back(2, 7); // King lands on c1
        }
    }
    else if (p == 'k') // Black king
    {
        // Kingside
        if (!isBlackKingMoved && !isBlackHRookMoved &&
            board.getPiecesAt(5, 0) == '.' &&
            board.getPiecesAt(6, 0) == '.')
        {
            moves.emplace_back(6, 0); // King lands on g8
        }

        // Queenside
        if (!isBlackKingMoved && !isBlackARookMoved &&
            board.getPiecesAt(1, 0) == '.' && // optional b8
            board.getPiecesAt(2, 0) == '.' &&
            board.getPiecesAt(3, 0) == '.')
        {
            moves.emplace_back(2, 0); // King lands on c8
        }
    }

    std::vector<std::pair<int, int>> enemyMoves;
    return true;
}

std::vector<std::pair<int, int>> Game::filtermoves(
    std::vector<std::pair<int, int>> &rawMoves,
    int fromX, int fromY, char movingPiece)
{
    std::vector<std::pair<int, int>> filteredMoves;
    bool isWhite = isupper(movingPiece);
    char kingChar = isWhite ? 'K' : 'k';

    // Find current king position (before move)
    int kingX = -1, kingY = -1;
    for (int y = 0; y < 8 && kingX == -1; y++)
    {
        for (int x = 0; x < 8 && kingX == -1; x++)
        {
            if (board.getPiecesAt(x, y) == kingChar)
            {
                kingX = x;
                kingY = y;
            }
        }
    }

    // Check if we're currently in check
    bool inCheck = isInCheck(isWhite, board);

    // Find all checking pieces if in check
    std::vector<std::pair<int, int>> checkingPieces;
    if (inCheck)
    {
        for (int y = 0; y < 8; y++)
        {
            for (int x = 0; x < 8; x++)
            {
                char attacker = board.getPiecesAt(x, y);
                if (attacker != '.' &&
                    ((isWhite && islower(attacker)) || (!isWhite && isupper(attacker))))
                {
                    auto attackerMoves = piece.legalMoves(x, y, board);
                    for (auto [ax, ay] : attackerMoves)
                    {
                        if (ax == kingX && ay == kingY)
                        {
                            checkingPieces.emplace_back(x, y);
                            break;
                        }
                    }
                }
            }
        }
    }

    // Handle special case when in check
    if (inCheck)
    {
        // In case of double check, only king moves are allowed
        if (checkingPieces.size() > 1)
        {
            if (movingPiece != kingChar)
            {
                return {};
            }
        }

        for (auto [toX, toY] : rawMoves)
        {
            Chessboard tempBoard = board;
            tempBoard.clearPieceAt(fromX, fromY);
            tempBoard.setPieceAt(toX, toY, movingPiece, CELL_SIZE);

            // For king moves, just check if new position is safe
            if (movingPiece == kingChar)
            {
                bool safe = true;
                for (int y = 0; y < 8 && safe; y++)
                {
                    for (int x = 0; x < 8 && safe; x++)
                    {
                        char attacker = tempBoard.getPiecesAt(x, y);
                        if (attacker != '.' &&
                            ((isWhite && islower(attacker)) || (!isWhite && isupper(attacker))))
                        {
                            auto attackerMoves = piece.legalMoves(x, y, tempBoard);
                            for (auto [ax, ay] : attackerMoves)
                            {
                                if (ax == toX && ay == toY)
                                {
                                    safe = false;
                                    break;
                                }
                            }
                        }
                    }
                }
                if (safe)
                {
                    filteredMoves.push_back({toX, toY});
                }
            }
            else
            {
                // For non-king pieces, must either:
                // 1. Capture the checking piece, or
                // 2. Block the check
                bool validMove = false;
                for (auto [cx, cy] : checkingPieces)
                {
                    // Capture the attacker
                    if (toX == cx && toY == cy)
                    {
                        validMove = true;
                        break;
                    }

                    // Block the attack
                    if (isCheckBlockable(kingX, kingY, cx, cy, board.getPiecesAt(cx, cy)))
                    {
                        int dx = (kingX > cx) ? 1 : (kingX < cx) ? -1
                                                                 : 0;
                        int dy = (kingY > cy) ? 1 : (kingY < cy) ? -1
                                                                 : 0;

                        int x = cx + dx;
                        int y = cy + dy;
                        while (x != kingX || y != kingY)
                        {
                            if (toX == x && toY == y)
                            {
                                validMove = true;
                                break;
                            }
                            x += dx;
                            y += dy;
                        }
                        if (validMove)
                            break;
                    }
                }

                if (validMove)
                {
                    // Verify this move doesn't expose king to new threats
                    bool kingStillSafe = true;
                    for (int y = 0; y < 8 && kingStillSafe; y++)
                    {
                        for (int x = 0; x < 8 && kingStillSafe; x++)
                        {
                            char attacker = tempBoard.getPiecesAt(x, y);
                            if (attacker != '.' &&
                                ((isWhite && islower(attacker)) || (!isWhite && isupper(attacker))))
                            {
                                auto attackerMoves = piece.legalMoves(x, y, tempBoard);
                                for (auto [ax, ay] : attackerMoves)
                                {
                                    if (ax == kingX && ay == kingY)
                                    {
                                        kingStillSafe = false;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    if (kingStillSafe)
                    {
                        filteredMoves.push_back({toX, toY});
                    }
                }
            }
        }

        return filteredMoves;
    }

    // Normal move filtering when not in check
    for (auto [toX, toY] : rawMoves)
    {
        Chessboard tempBoard = board;
        tempBoard.clearPieceAt(fromX, fromY);
        tempBoard.setPieceAt(toX, toY, movingPiece, CELL_SIZE);

        // Find the king's new position (might be same or different if moving king)
        int newKingX = (movingPiece == kingChar) ? toX : kingX;
        int newKingY = (movingPiece == kingChar) ? toY : kingY;

        // Check if king is under attack after this move
        bool kingInCheck = false;
        for (int y = 0; y < 8 && !kingInCheck; y++)
        {
            for (int x = 0; x < 8 && !kingInCheck; x++)
            {
                char attacker = tempBoard.getPiecesAt(x, y);
                if (attacker != '.' &&
                    ((isWhite && islower(attacker)) || (!isWhite && isupper(attacker))))
                {
                    auto attackerMoves = piece.legalMoves(x, y, tempBoard);
                    for (auto [ax, ay] : attackerMoves)
                    {
                        if (ax == newKingX && ay == newKingY)
                        {
                            kingInCheck = true;
                            break;
                        }
                    }
                }
            }
        }

        if (!kingInCheck)
        {
            filteredMoves.push_back({toX, toY});
        }
    }

    return filteredMoves;
}

bool Game::isCheckBlockable(int kingX, int kingY, int checkerX, int checkerY, char checkerPiece)
{
    // Only sliding pieces can be blocked
    if (tolower(checkerPiece) != 'q' &&
        tolower(checkerPiece) != 'r' &&
        tolower(checkerPiece) != 'b')
    {
        return false;
    }

    // Get path between checker and king
    std::vector<std::pair<int, int>> path;
    int dx = (kingX > checkerX) ? 1 : (kingX < checkerX) ? -1
                                                         : 0;
    int dy = (kingY > checkerY) ? 1 : (kingY < checkerY) ? -1
                                                         : 0;

    int x = checkerX + dx;
    int y = checkerY + dy;
    while (x != kingX || y != kingY)
    {
        path.emplace_back(x, y);
        x += dx;
        y += dy;
    }

    return !path.empty();
}

std::vector<std::pair<int, int>> Game::getPawnMovesWithEnPassant(int x, int y, Chessboard &board)
{
    auto moves = piece.legalMoves(x, y, board);
    char c = board.getPiecesAt(x, y);
    bool isWhite = isupper(c);

    // For white pawns (moving up, decreasing y)
    if (isWhite && y == 3)
    { // 4th rank (0-indexed)
        // Left en passant (x-1)
        if (x > 0 && enPassantTarget == to64(x - 1, 2))
        {
            moves.emplace_back(x - 1, 2);
        }
        // Right en passant (x+1)
        if (x < 7 && enPassantTarget == to64(x + 1, 2))
        {
            moves.emplace_back(x + 1, 2);
        }
    }
    // For black pawns (moving down, increasing y)
    else if (!isWhite && y == 4)
    { // 5th rank (0-indexed)
        // Left en passant (x-1)
        if (x > 0 && enPassantTarget == to64(x - 1, 5))
        {
            moves.emplace_back(x - 1, 5);
        }
        // Right en passant (x+1)
        if (x < 7 && enPassantTarget == to64(x + 1, 5))
        {
            moves.emplace_back(x + 1, 5);
        }
    }

    return moves;
}