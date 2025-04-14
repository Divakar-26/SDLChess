#include "game.h"

struct Piece{
    float texX, texY;
    std::string name;
    int worth;
};

Piece q = {0,0,"Black Queen"};
Piece k = {60,0,"Black King"};
Piece r = {120,0,"Black Rook"};
Piece n = {180,0,"Black Knight"};
Piece b = {240,0,"Black Bishop"};
Piece p = {300,0,"Black Pawn"};

Piece Q = {0,60,"White Queen"};
Piece K = {60,60,"White King"};
Piece R = {120,60,"White Rook"};
Piece N = {180,60,"White Knight"};
Piece B = {240,60,"White Bishop"};
Piece P = {300,60,"White Pawn"};

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
    {'P', P}
};

SDL_Texture *pieceTexture;
SDL_Texture *boardTexture;
int mX, mY;
void renderBoard(SDL_Renderer *renderer);
void mousePrinting(SDL_Event e, SDL_Renderer *renderer);
bool isMouseMoved = false;
void renderPieces(SDL_Renderer * renderer);
void renderPiece(SDL_Renderer * renderer, char piece, int x, int y);



char board[8][8] = {
    {'.', '.', '.', '.', '.', '.','.','.'},
    {'.', '.', '.', '.', '.', '.','.','.'},
    {'.', '.', '.', '.', '.', '.','.','.'},
    {'.', '.', '.', '.', '.', '.','.','.'},
    {'.', '.', '.', '.', '.', '.','.','.'},
    {'.', '.', '.', '.', '.', '.','.','.'},
    {'.', '.', '.', '.', '.', '.','.','.'},
    {'.', '.', '.', '.', '.', '.','.','.'},
};

Game::Game(int W_W, int W_H)
{
    WINDOW_H = W_H;
    WINDOW_W = W_W;
}

Game::~Game() {}

bool Game::init(const char *title, char * fenInput, int args)
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

    boardTexture = loadTexture("assets/board.png");
    pieceTexture = loadTexture("assets/pieces.png");

    if (!boardTexture || !pieceTexture) {
        SDL_Log("Failed to load texture!");
        return false;
    }
    

    if(args == 1){
        this->fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    }
    else this->fen = fenInput;



    //FEN Parsing
    int row = 0;
    int col = 0;
    for(int i = 0; fen[i] != '\0'; i++){

        if(fen[i] == ' '){
            break;
        }

        if(isdigit(fen[i])){
            int empty = fen[i] - '.';
            for(int k = 0; k < empty; k++){
                board[row][col] = '.';
                col++;
            }
        }

        else if(fen[i] == '/'){
            row++;
            col=0;
        }

        else{
            board[row][col] = fen[i];
            col++;
        }   
    }


    isRunning = true;
    return true;
}

void Game::handleEvent()
{

    SDL_Event e;
    while (SDL_PollEvent(&e))
    {

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

}

void Game::render()
{
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 0);
    SDL_RenderClear(renderer);

    // all rendering is here

    
    renderBoard(renderer);
    if(isMouseMoved){
        SDL_FRect grid = {(float)(mX * 64), (float)(mY * 64), 64, 64};
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 0);
        SDL_RenderFillRect(renderer, &grid);
    }


    renderPieces(renderer);

    SDL_RenderPresent(renderer);
}

void renderBoard(SDL_Renderer *renderer)
{
    SDL_FRect srcRect = {0, 0, 512, 512};
    SDL_FRect destRect = {0, 0, 512, 512};

    SDL_RenderTexture(renderer, boardTexture, &srcRect, &destRect);
}
void mousePrinting(SDL_Event e, SDL_Renderer *renderer)
{
    float mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    mX = (int)(mouseX / 64);
    mY = (int)(mouseY / 64);
}

void renderPieces(SDL_Renderer * renderer){
    for(int i= 0; i < 8; i++){
        for(int j = 0; j < 8; j++){
            if(board[i][j] == '.'){
                continue;
            }
            renderPiece(renderer, board[i][j], i, j);
        }
    }
}

void renderPiece(SDL_Renderer * renderer, char piece, int x, int y){

    SDL_FRect srcRect = {pieces[piece].texX, pieces[piece].texY, 60,60};
    SDL_FRect destRect = {(float)(y * 64), (float)(x * 64), 64,64};
    SDL_RenderTexture(renderer, pieceTexture,&srcRect, &destRect);
}


SDL_Texture *Game::loadTexture(const char *path)
{
    SDL_Surface *tempSurface = IMG_Load(path);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, tempSurface);
    SDL_DestroySurface(tempSurface);
    return texture;
}