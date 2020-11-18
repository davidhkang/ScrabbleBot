#include "board.h"
#include "board_square.h"
#include "exceptions.h"
#include "formatting.h"
#include <iomanip>
#include <fstream>

using namespace std;


bool Board::Position::operator==(const Board::Position& other) const {
    return this->row == other.row && this->column == other.column;
}

bool Board::Position::operator!=(const Board::Position& other) const {
    return this->row != other.row || this->column != other.column;
}

Board::Position Board::Position::translate(Direction direction) const {
    return this->translate(direction, 1);
}

Board::Position Board::Position::translate(Direction direction, ssize_t distance) const {
    if (direction == Direction::DOWN) {
        return Board::Position(this->row + distance, this->column);
    } else {
        return Board::Position(this->row, this->column + distance);
    }
}

Board Board::read(const string& file_path) {
    ifstream file(file_path);
    if (!file) {
        throw FileException("cannot open board file!");
    }

    size_t rows;
    size_t columns;
    size_t starting_row;
    size_t starting_column;
    file >> rows >> columns >> starting_row >> starting_column;

    Board board(rows, columns, starting_row, starting_column);

    string row;
    for(int r=0; r<(int)rows; r++){
        file >> row;
        vector<BoardSquare> row_vector;
        for(int c=0; c<(int)columns; c++){      
            Position position(r,c);
            
            char spot = row[c];

            if(spot == '.'){
                row_vector.push_back(BoardSquare(1,1));
            }else if(spot == '2'){
                row_vector.push_back(BoardSquare(2,1));
            }else if(spot == '3'){
                row_vector.push_back(BoardSquare(3,1));
            }else if(spot == 'd'){
                row_vector.push_back(BoardSquare(1,2));
            }else if(spot == 't'){
                row_vector.push_back(BoardSquare(1,3));
            }
        }
        board.squares.push_back(row_vector);
    }

    return board;
}

size_t Board::get_move_index() const {
    return this->move_index;
}

PlaceResult Board::test_place(const Move& move) const {
    // check for whether there's at least one adjacent tile
    bool is_adjacent = false;
    // if starting tile hasn't been used, this must be the first move
    if(!in_bounds_and_has_tile(start)){
        // if this is the first word, it can't be adjacent to any tile so we let this pass
        is_adjacent = true;

        // checks that the first move touches the starting position
        bool isValid = false;
        size_t current_row = move.row;
        size_t current_column = move.column; 
        for(size_t i=0; i<move.tiles.size(); i++){
            if(current_row == start.row && current_column == start.column){ 
                isValid = true;
                break;
            }
            if(move.direction == Direction::ACROSS){
                current_column++;
            }else if(move.direction == Direction::DOWN){
                current_row++;
            }
        }
        if(!isValid){
            return PlaceResult("your first move must start at the board's starting tile.");
        }
    }



    // scanning the main word being placed
    string main_word = "";
    unsigned int main_word_points = 0;
    unsigned int extra_word_points = 0;
    unsigned short total_word_multiplier = 1;

    // check that the position for the first tile isn't taken already
    Position first(move.row, move.column);
    if(in_bounds_and_has_tile(first)){
        return PlaceResult("The spot for your first tile is already taken.");
    }else if(!is_in_bounds(first)){
        return PlaceResult("Your first tile is out of bounds.");
    }
    
    // vector containing all words found
    vector<string> words_found;

    Position current_position = first;
    size_t tiles_used = 0;
    
    // checks:
    // 1. there are tiles remaining to be placed
    // 2. current_position is still inbounds
    // 3. current_position already has a tile which should be added to the main word
    while(((tiles_used != move.tiles.size()) && is_in_bounds(current_position)) || in_bounds_and_has_tile(current_position)){
        
        // if a tile alredy exists at current_position
        if(in_bounds_and_has_tile(current_position)){
            is_adjacent = true;


            if(at(current_position).get_tile_kind().letter == '?'){
                main_word += at(current_position).get_tile_kind().assigned;
            }else{
                main_word += at(current_position).get_tile_kind().letter;
            }

            main_word_points += at(current_position).get_tile_kind().points;

            // increment in the correct direction
            if(move.direction == Direction::ACROSS) current_position.column++;
            else if(move.direction == Direction::DOWN) current_position.row++;
      
            continue; 
        }
        // if current tile is not filled 
        else{
            // check for blank tiles
            if(move.tiles[tiles_used].letter == '?'){
                main_word += move.tiles[tiles_used].assigned;
            }else{
                main_word += move.tiles[tiles_used].letter;
            }
            // add points
            main_word_points += move.tiles[tiles_used].points * at(current_position).letter_multiplier;
            total_word_multiplier *= at(current_position).word_multiplier;

            tiles_used++;
        }

        

        // at every tile you place, check up and down if you're going across, or left and right if you're going down
        Position temp_position = current_position;
        bool word_found = false;
        string extra_word = "";
        unsigned int temp_points = 0;
        unsigned int extra_word_multiplier = 1;

        // take care of blank tiles
        if(move.tiles[tiles_used - 1].letter == '?'){
            extra_word += move.tiles[tiles_used - 1].assigned;
        }else{
            extra_word += move.tiles[tiles_used - 1].letter;
        }
        temp_points += move.tiles[tiles_used - 1].points * at(current_position).letter_multiplier;
        extra_word_multiplier *= at(current_position).word_multiplier;

        // check up or left depending on the direction
        if(move.direction == Direction::ACROSS) temp_position.row--;
        if(move.direction == Direction::DOWN) temp_position.column--;

        // loop while there's a tile found at the current position
        while(in_bounds_and_has_tile(temp_position)){
            is_adjacent = true;
            word_found = true;

            // check for blank tiles and add the right letter 
            if(at(temp_position).get_tile_kind().letter == '?'){
                extra_word = at(temp_position).get_tile_kind().assigned + extra_word;
            }else{
                extra_word = at(temp_position).get_tile_kind().letter + extra_word;
            }
            temp_points += at(temp_position).get_tile_kind().points;

            // increment towards the correct direction
            if(move.direction == Direction::ACROSS) temp_position.row--;
            if(move.direction == Direction::DOWN) temp_position.column--;
        }

        // check down or right depending on the direction
        temp_position = current_position;
        if(move.direction == Direction::ACROSS) temp_position.row++;
        if(move.direction == Direction::DOWN) temp_position.column++;
        while(in_bounds_and_has_tile(temp_position)){
            is_adjacent = true;
            word_found = true;

            // check for blank tiles and add the right letter 
            if(at(temp_position).get_tile_kind().letter == '?'){
                extra_word = extra_word + at(temp_position).get_tile_kind().assigned;
            }else{
                extra_word = extra_word + at(temp_position).get_tile_kind().letter;
            }
            temp_points += at(temp_position).get_tile_kind().points;

            // increment towards the correct direction
            if(move.direction == Direction::ACROSS) temp_position.row++;
            if(move.direction == Direction::DOWN) temp_position.column++;
        }

        // if any extra words are found, add them to the words vector 
        if(word_found){
            words_found.push_back(extra_word);
            extra_word_points += temp_points * extra_word_multiplier;
        }

        // increment in the correct direction
        if(move.direction == Direction::ACROSS) current_position.column++;
        else if(move.direction == Direction::DOWN) current_position.row++;
    }

    if(tiles_used != move.tiles.size()){
        return PlaceResult("ran out of space to place tiles");
    }

    // check backwards starting from original position
    current_position = first;
    if(move.direction == Direction::ACROSS) current_position.column--;
    if(move.direction == Direction::DOWN) current_position.row--;
    while(in_bounds_and_has_tile(current_position)){
        is_adjacent = true;
        // checks for blank tiles and adds the letter in the square to the main word
        if(at(current_position).get_tile_kind().letter == '?'){
            main_word = at(current_position).get_tile_kind().assigned + main_word;
        }else{
            main_word = at(current_position).get_tile_kind().letter + main_word;
        }

        main_word_points += at(current_position).get_tile_kind().points;
        
        // increment in backwards (up or left) direction
        if(move.direction == Direction::ACROSS) current_position.column--;
        else if(move.direction == Direction::DOWN) current_position.row--;
    }

    if(!is_adjacent){
        return PlaceResult("Word isn't adjacent to any tile.");
    }

    // push main_word 
    

    words_found.push_back(main_word);

    // calculate total points
    int total_points = extra_word_points + main_word_points * total_word_multiplier;
    
    // SHIP IT
    return PlaceResult(words_found, total_points);
}

PlaceResult Board::place(const Move& move) {
    // test the move first
    PlaceResult test_result = test_place(move);
    if (test_result.valid){
        
        Position current(move.row, move.column);
        
        // places tiles
        for(size_t i=0; i<move.tiles.size(); i++){
            
            if(!at(current).has_tile()){
                at(current).set_tile_kind(move.tiles[i]);
            }
            // if there's already a tile there, make sure you're not incrementing the index i
            else{
                i--;
            }
            if(move.direction == Direction::ACROSS) current.column++;
            else if(move.direction == Direction::DOWN) current.row++;
        }
        return test_result;
    }else{
        return test_result;
    }
}



/* ---------------- HW 5 FUNCTIONS ----------------------------------*/


char Board::letter_at(Position p) const{
    return at(p).get_tile_kind().letter;
}

bool Board::is_anchor_spot(Position p) const{
    // starting point is a valid anchor
    if(p.row == start.row && p.column == start.column && !at(p).has_tile()) {
        return true;
    }

    // look up down left right
    if(is_in_bounds(p) && !at(p).has_tile()){
        Position temp = p;
        temp.column--;
        if(in_bounds_and_has_tile(temp)) return true;
        temp = p;
        temp.column++;
        if(in_bounds_and_has_tile(temp)) return true;
        temp = p;
        temp.row++;
        if(in_bounds_and_has_tile(temp)) return true;
        temp = p;
        temp.row--;
        if(in_bounds_and_has_tile(temp)) return true;
    }
    return false;
}

std::vector<Board::Anchor> Board::get_anchors() const{
    Position current(0,0);
    vector<Board::Anchor> result;
    // look through every tile
    for(int i=0; i<(int)rows; i++){
        current.column = 0;
        for(int i=0; i<(int)columns; i++){
            if(is_anchor_spot(current)){
                int limit = 0;
                Position temp(current.row, current.column);
                
                // check accross (left)
                temp.column--;
                while(is_in_bounds(temp) && !at(temp).has_tile() && !is_anchor_spot(temp)){
                    limit++;
                    temp.column--;
                }
                Anchor across(current, Direction::ACROSS, limit);
                result.push_back(across);
                
                // check down (up)
                limit = 0;
                temp = current;
                temp.row--;
                while(is_in_bounds(temp) && !at(temp).has_tile() && !is_anchor_spot(temp)){
                    limit++;
                    temp.row--;
                }
                Anchor down(current, Direction::DOWN, limit);
                result.push_back(down);
            }
            current.column++;
        }
        current.row++;
    }
    return result;
}



/* ------------------------------ GIVEN -----------------------------*/


// The rest of this file is provided for you. No need to make changes.

BoardSquare& Board::at(const Board::Position& position) {
    return this->squares.at(position.row).at(position.column);
}

const BoardSquare& Board::at(const Board::Position& position) const {
    return this->squares.at(position.row).at(position.column);
}

bool Board::is_in_bounds(const Board::Position& position) const {
    return position.row < this->rows && position.column < this->columns;
}

bool Board::in_bounds_and_has_tile(const Position& position) const{
    return is_in_bounds(position) && at(position).has_tile();
}




void Board::print(ostream& out) const {
    // Draw horizontal number labels
    for (size_t i = 0; i < BOARD_TOP_MARGIN - 2; ++i) {
        out << std::endl;
    }
    out << FG_COLOR_LABEL << repeat(SPACE, BOARD_LEFT_MARGIN);
    const size_t right_number_space = (SQUARE_OUTER_WIDTH - 3) / 2;
    const size_t left_number_space = (SQUARE_OUTER_WIDTH - 3) - right_number_space;
    for (size_t column = 0; column < this->columns; ++column) {
        out << repeat(SPACE, left_number_space) << std::setw(2) << column + 1 << repeat(SPACE, right_number_space);
    }
    out << std::endl;

    // Draw top line
    out << repeat(SPACE, BOARD_LEFT_MARGIN);
    print_horizontal(this->columns, L_TOP_LEFT, T_DOWN, L_TOP_RIGHT, out);
    out << endl;

    // Draw inner board
    for (size_t row = 0; row < this->rows; ++row) {
        if (row > 0) {
            out << repeat(SPACE, BOARD_LEFT_MARGIN);
            print_horizontal(this->columns, T_RIGHT, PLUS, T_LEFT, out);
            out << endl;
        }

        // Draw insides of squares
        for (size_t line = 0; line < SQUARE_INNER_HEIGHT; ++line) {
            out << FG_COLOR_LABEL << BG_COLOR_OUTSIDE_BOARD;

            // Output column number of left padding
            if (line == 1) {
                out << repeat(SPACE, BOARD_LEFT_MARGIN - 3);
                out << std::setw(2) << row + 1;
                out << SPACE;
            } else {
                out << repeat(SPACE, BOARD_LEFT_MARGIN);
            }

            // Iterate columns
            for (size_t column = 0; column < this->columns; ++column) {
                out << FG_COLOR_LINE << BG_COLOR_NORMAL_SQUARE << I_VERTICAL;
                const BoardSquare& square = this->squares.at(row).at(column);
                bool is_start = this->start.row == row && this->start.column == column;

                // Figure out background color
                if (square.word_multiplier == 2) {
                    out << BG_COLOR_WORD_MULTIPLIER_2X;
                } else if (square.word_multiplier == 3) {
                    out << BG_COLOR_WORD_MULTIPLIER_3X;
                } else if (square.letter_multiplier == 2) {
                    out << BG_COLOR_LETTER_MULTIPLIER_2X;
                } else if (square.letter_multiplier == 3) {
                    out << BG_COLOR_LETTER_MULTIPLIER_3X;
                } else if (is_start) {
                    out << BG_COLOR_START_SQUARE;
                }

                // Text
                if (line == 0 && is_start) {
                    out << "  \u2605  ";
                } else if (line == 0 && square.word_multiplier > 1) {
                    out << FG_COLOR_MULTIPLIER << repeat(SPACE, SQUARE_INNER_WIDTH - 2) << 'W' << std::setw(1) << square.word_multiplier;
                } else if (line == 0 && square.letter_multiplier > 1) {
                    out << FG_COLOR_MULTIPLIER << repeat(SPACE, SQUARE_INNER_WIDTH - 2) << 'L' << std::setw(1) << square.letter_multiplier;
                } else if (line == 1 && square.has_tile()) {
                    char l = square.get_tile_kind().letter == TileKind::BLANK_LETTER ? square.get_tile_kind().assigned : ' ';
                    out << repeat(SPACE, 2) << FG_COLOR_LETTER << square.get_tile_kind().letter << l << repeat(SPACE, 1);
                } else if (line == SQUARE_INNER_HEIGHT - 1 && square.has_tile()) {
                    out << repeat(SPACE, SQUARE_INNER_WIDTH - 1) << FG_COLOR_SCORE << square.get_points();
                } else {
                    out << repeat(SPACE, SQUARE_INNER_WIDTH);
                }
            }

            // Add vertical line
            out << FG_COLOR_LINE << BG_COLOR_NORMAL_SQUARE << I_VERTICAL << BG_COLOR_OUTSIDE_BOARD << std::endl;
        }
    }

    // Draw bottom line
    out << repeat(SPACE, BOARD_LEFT_MARGIN);
    print_horizontal(this->columns, L_BOTTOM_LEFT, T_UP, L_BOTTOM_RIGHT, out);
    out << endl << rang::style::reset << std::endl;
}
