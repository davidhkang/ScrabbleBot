#include <sstream>
#include <stdexcept>
#include <vector>
#include <map>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include "place_result.h"
#include "move.h"
#include "exceptions.h"
#include "human_player.h"
#include "tile_kind.h"
#include "formatting.h"
#include "rang.h"

using namespace std;


// This method is fully implemented.
inline string& to_upper(string& str) {
    transform(str.begin(), str.end(), str.begin(), ::toupper);
    return str;
}

Move HumanPlayer::get_move(const Board& board, const Dictionary& dictionary) const {
	// TODO: begin your implementation here.

    print_hand(cout);

    string input;
    cout << "Type in a move." << endl;
    getline(cin, input);

    try{
        Move potential_move = parse_move(input);

        // if the move is PASS/EXCHANGE, nothing to do
        if(potential_move.kind == MoveKind::PASS 
        || potential_move.kind == MoveKind::EXCHANGE){
            return potential_move;
        }

        // adjust to 0 indexing
        potential_move.row--;
        potential_move.column--;

        // test_place
        PlaceResult test_result = board.test_place(potential_move);

        if (!test_result.valid){
            // if test returns invalid
            cout << test_result.error << endl;
            throw invalid_argument("Tiles don't fit on the board!");
        }else{
            // check if words exist in the dictionary
            for(int i=0; i<(int)test_result.words.size(); i++){
                if(!dictionary.is_word(test_result.words[i])){
                    throw invalid_argument("Word(s) created doesn't exist in the dictionary!");
                }
            }
        }
        // if words make it through the for loop, the words exist in the dictionary so the move must be valid
        return potential_move; 
    }
    catch(out_of_range e){
        cout << "Some tile(s) don't exist in the hand!" << endl;
        return get_move(board, dictionary); 
    }
    catch(invalid_argument e){
        cout << e.what() << endl;
        return get_move(board, dictionary);
    }
}

vector<TileKind> HumanPlayer::parse_tiles(string& letters) const{
    // TODO: begin implementation here.
    vector<TileKind> output;
    map<char, size_t> repeats;
    for(size_t i=0; i<letters.size(); i++){
        // check that the letters given actually exist in the hand of the player
        try{
            // lookup_tile will throw an exception if it isn't found in the hand
            // do blank tile check
            map<char, size_t>::iterator it = repeats.find(letters[i]);

            // if tiles doesn't exist in the map yet
            if(it == repeats.end()){
                TileKind tile = tiles.lookup_tile(letters[i]);
                // insert tile into map
                repeats[letters[i]] = 1;
                // push tile onto hand 
                output.push_back(tile);
            }
            // if tile exists in the map 
            else{
                // if the # of this letter in the hand >= the number of times letter was in the string already + 1, then it's valid and the count in the map can be incremented
                if(tiles.count_tiles(tiles.lookup_tile(letters[i])) >= it->second+1){
                    it->second++; 
                    TileKind tile = tiles.lookup_tile(letters[i]);
                    output.push_back(tile);
                }
                // if not, it means that the count in the map exceeds the # of the letter that exists in the hand itself
                else{
                    throw out_of_range("not enough tiles");
                }
            }
            
        }catch(out_of_range e){
            // put it back into the hand
            throw e;
        }
    }
    return output;
}

Move HumanPlayer::parse_move(string& move_string) const {
    
    stringstream ss(move_string);

    // get the kind
    string kind;
    ss >> kind;

    // depending on the kind
    if(to_upper(kind) == "PASS"){
        Move move;
        return move;
    }
    else if(to_upper(kind) == "EXCHANGE"){
        string input;
        ss >> input;
        vector<TileKind> tiles = parse_tiles(input);
        
        // MoveKind is automatically set to EXCHANGE if just the tiles are passed to the constructor 
        Move move(tiles);

        return move;
    }
    else if(to_upper(kind) == "PLACE"){
        char dir;
        size_t r, c;
        string input;

        ss >> dir >> r >> c >> input;

        if(ss.fail()){
            throw invalid_argument("wrong input format");
        } 


        char assigned_letter;
        bool blank_letter_found = false;
        int erase_index = -1;
        // take care of blank tiles 
        for(int i=0; i<(int)input.size(); i++){
            if(input[i-1] == '?'){
                blank_letter_found = true;
                assigned_letter = input[i];
                erase_index = i;
            }
        }
        if(blank_letter_found){
            input.erase(input.begin() + erase_index);
        }

        // parse tiles
        vector<TileKind> tiles = parse_tiles(input);

        // take care of blank tiles
        for(size_t i=0; i<tiles.size(); i++){
            if(tiles[i].letter == '?'){
                tiles[i].assigned = assigned_letter;
            }
        }

        // set direction depending on '-' vs '|'
        Direction direction;
        if(dir == '-'){
            direction = Direction::ACROSS;
        }else if(dir == '|'){
            direction = Direction::DOWN;
        }else{
            throw invalid_argument("direction not given"); 
        }

        Move move(tiles, r, c, direction);
        return move;
    }else{
        throw invalid_argument("invalid move type"); 
    }
}








// This function is fully implemented.
void HumanPlayer::print_hand(ostream& out) const {
	const size_t tile_count = tiles.count_tiles();
	const size_t empty_tile_count = this->get_hand_size() - tile_count;
	const size_t empty_tile_width = empty_tile_count * (SQUARE_OUTER_WIDTH - 1);

	for(size_t i = 0; i < HAND_TOP_MARGIN - 2; ++i) {
		out << endl;
	}

	out << repeat(SPACE, HAND_LEFT_MARGIN) << FG_COLOR_HEADING << "Your Hand: " << endl << endl;

    // Draw top line
    out << repeat(SPACE, HAND_LEFT_MARGIN) << FG_COLOR_LINE << BG_COLOR_NORMAL_SQUARE;
    print_horizontal(tile_count, L_TOP_LEFT, T_DOWN, L_TOP_RIGHT, out);
    out << repeat(SPACE, empty_tile_width) << BG_COLOR_OUTSIDE_BOARD << endl;

    // Draw middle 3 lines
    for (size_t line = 0; line < SQUARE_INNER_HEIGHT; ++line) {
        out << FG_COLOR_LABEL << BG_COLOR_OUTSIDE_BOARD << repeat(SPACE, HAND_LEFT_MARGIN);
        for (auto it = tiles.cbegin(); it != tiles.cend(); ++it) {
            out << FG_COLOR_LINE << BG_COLOR_NORMAL_SQUARE << I_VERTICAL << BG_COLOR_PLAYER_HAND;

            // Print letter
            if (line == 1) {
                out << repeat(SPACE, 2) << FG_COLOR_LETTER << (char)toupper(it->letter) << repeat(SPACE, 2);

            // Print score in bottom right
            } else if (line == SQUARE_INNER_HEIGHT - 1) {
                out << FG_COLOR_SCORE << repeat(SPACE, SQUARE_INNER_WIDTH - 2) << setw(2) << it->points;
            } else {
                out << repeat(SPACE, SQUARE_INNER_WIDTH);
            }
        }
        if (tiles.count_tiles() > 0) {
            out << FG_COLOR_LINE << BG_COLOR_NORMAL_SQUARE << I_VERTICAL;
            out << repeat(SPACE, empty_tile_width) << BG_COLOR_OUTSIDE_BOARD << endl;
        }
    }

    // Draw bottom line
    out << repeat(SPACE, HAND_LEFT_MARGIN) << FG_COLOR_LINE << BG_COLOR_NORMAL_SQUARE;
    print_horizontal(tile_count, L_BOTTOM_LEFT, T_UP, L_BOTTOM_RIGHT, out);
    out << repeat(SPACE, empty_tile_width) << rang::style::reset << endl;
}
