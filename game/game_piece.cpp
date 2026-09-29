#include "game_piece.hpp"

Game_Piece::Game_Piece() {
    piece_length = 0;
    piece_symbol = '\0';
}

Game_Piece::Game_Piece(int length, char symbol) {
    piece_length = length;
    piece_symbol = symbol;
}

int Game_Piece::Get_Piece_Length() {
    return piece_length;
}

char Game_Piece::Get_Piece_Symbol() {
    return piece_symbol;
}