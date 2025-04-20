#include"game.h"
#include<iostream>
Game game(640 + 350,640);

int main(int argc, char* argv[]){

    if(!game.init("myWIndow", argv[1],argc)){
        return -1;
    }

    while(game.running()){
        game.handleEvent();
        game.update();
        game.render();

    }

    game.cleanup();
    return 0;
}