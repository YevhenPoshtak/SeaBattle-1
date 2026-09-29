#ifndef GAME_PIECE_HPP
#define GAME_PIECE_HPP

class Game_Piece {
    int piece_length;
    char piece_symbol;
public:
    Game_Piece();

    Game_Piece(int length, char symbol);

    int Get_Piece_Length();

    char Get_Piece_Symbol();
};

#endif 