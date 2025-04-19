#include "ui.h"

void UI::init(SDL_Window *window, SDL_Renderer *renderer)
{
    // imgui setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);
}

void UI::update()
{
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}

void UI::dropDown(std::vector<std::string> & texture_of_piece, int & current_item_piece_theme, std::vector<std::string> & texture_of_board, int & current_item_board_theme)
{
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
            }
        }
        ImGui::EndCombo();
    }

    ImGui::End();

}


void UI::loadFEN(std::string & fen, Chessboard & board){
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
}