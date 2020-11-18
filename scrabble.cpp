#include "scrabble.h"
#include "formatting.h"
#include <iostream>
#include <iomanip>
#include <map>

using namespace std;


// Given to you. this does not need to be changed
Scrabble::Scrabble(const ScrabbleConfig& config)
    : hand_size(config.hand_size)
    , minimum_word_length(config.minimum_word_length)
    , tile_bag(TileBag::read(config.tile_bag_file_path, config.seed))
    , board(Board::read(config.board_file_path))
    , dictionary(Dictionary::read(config.dictionary_file_path)) {}


void Scrabble::add_players(){
    int num_players;
    cout << "How many players? ";
    cin >> num_players;
    for(int i=0; i<num_players; i++){
        string name;
        char is_comp;
        cout << "Name of Player " << i+1 << "?" << endl;
        
        // take care of the whitespace  
        cin >> ws;
        getline(cin, name);

        // check if its a computer
        cout << "Is this player a computer? (y/n)" << endl;
        cin >> is_comp;

        // new player
        shared_ptr<Player> new_player; 
        num_human_players = 0;
        if(is_comp == 'y'){
            new_player = make_shared<ComputerPlayer>(name, hand_size);
        }else{
            new_player = make_shared<HumanPlayer>(name, hand_size);
            num_human_players ++;
        }
        players.push_back(new_player);
    }
}

// Game Loop should cycle through players and get and execute that players move
// until the game is over.
void Scrabble::game_loop() {
    // TODO: implement this.

    // give out first hands
    for(int i=0; i<(int)players.size(); i++){
        vector<TileKind> first_hand = tile_bag.remove_random_tiles(hand_size);
        players[i]->add_tiles(first_hand);
    }

    size_t pass_count = 0;
    bool tiles_left = true;
    
    // as long as tiles are left in the bag 
    while(pass_count != num_human_players && tiles_left == true){
        // reset pass_count;
        pass_count = 0;

        // for every player 
        for(int i=0; i<(int)players.size(); i++){
            
            board.print(cout);
            
            // print player points
            for(int i=0; i<(int)players.size(); i++){
                cout << "Player " << i+1 << " has " << players[i]->get_points() << " points." << endl;
            }

            // get move for each player
            Move player_move = players[i]->get_move(board, dictionary);
            
            if(player_move.kind == MoveKind::PASS){
                // if pass, just increment the count to make sure not everyone passes
                if(players[i]->is_human()) pass_count++;
            }else if(player_move.kind == MoveKind::EXCHANGE){

                int num_removed = player_move.tiles.size();

                // if tiles are being removed at all
                if(num_removed > 0){
                    // remove tiles from hand
                    players[i]->remove_tiles(player_move.tiles);
                    
                    // add tiles to bag
                    for(int i=0; i<(int)player_move.tiles.size(); i++){
                        tile_bag.add_tile(player_move.tiles[i]);
                    }
                    
                    // get random tiles from bag
                    vector<TileKind> random_tiles = tile_bag.remove_random_tiles(num_removed);

                    // add tiles to hand
                    players[i]->add_tiles(random_tiles);

                    // print tiles picked up
                    cout << "You picked up the tiles: ";
                    for(int i=0; i<(int)random_tiles.size(); i++){
                        cout << random_tiles[i].letter << " "<< endl;
                    }
                }
            }else if(player_move.kind == MoveKind::PLACE){
                PlaceResult move_result = board.place(player_move);
                
                if(move_result.valid){
                    int tiles_removed = player_move.tiles.size();

                    // apply empty hand bonus +50
                    if(player_move.tiles.size() == hand_size){
                        move_result.points += EMPTY_HAND_BONUS;
                    }
                    players[i]->add_points(move_result.points);
                    players[i]->remove_tiles(player_move.tiles);

                    // check if tiles have run out of this hand and the bag
                    if(players[i]->count_tiles() == 0 && tile_bag.count_tiles() == 0){
                        tiles_left = false; 
                        cout << "No more tiles left!" << endl;
                        break;
                    }

                    cout << "You placed the words:" << endl;
                    for(int i=0; i<(int)move_result.words.size(); i++){
                        cout << move_result.words[i] << endl;
                    }

                    cout << "You gained " << SCORE_COLOR << move_result.points << rang::style::reset << " points!" << endl;

                    // get random tiles from bag
                    vector<TileKind> random_tiles = tile_bag.remove_random_tiles(tiles_removed);

                    // add tiles to hand
                    players[i]->add_tiles(random_tiles);
                    cout << "You picked up the tiles: ";
                    for(int i=0; i<(int)random_tiles.size(); i++){
                        cout << random_tiles[i].letter << " ";
                    }

                    cout << "The board now looks like:";
                    board.print(cout);
                }
            }

            cout << "Your current score: " << SCORE_COLOR << players[i]->get_points() << rang::style::reset << endl;
            cout << endl << "Press [enter] to continue.";
            cin.ignore(256, '\n');
        }
    }  
}

// Performs final score subtraction. Players lose points for each tile in their
// hand. The player who cleared their hand receives all the points lost by the
// other players.
void Scrabble::final_subtraction(vector<shared_ptr<Player>> & plrs) {
    int index_of_empty;
    size_t left_over_value = 0;
    bool player_with_none = false;

    for(int i=0; i<(int)plrs.size(); i++){
        // keep track of index with no tiles to later give points to
        if(plrs[i]->count_tiles() == 0){
            player_with_none = true;
            index_of_empty = i;
        }else{
            // subtract hand value from points
            if(plrs[i]->get_hand_value() > plrs[i]->get_points()){
                plrs[i]->subtract_points(plrs[i]->get_points());
            }else{
                plrs[i]->subtract_points(plrs[i]->get_hand_value());
                left_over_value += plrs[i]->get_hand_value();
            }
        }
    }
    // if there was a player with an empty hand
    if(player_with_none){
        plrs[index_of_empty]->add_points(left_over_value);
    }

}

// You should not need to change this function.
void Scrabble::print_result() {
    // Determine highest score
    size_t max_points = 0;
    for (auto player : this->players) {
        if (player->get_points() > max_points) {
            max_points = player->get_points();
        }
    }

    // Determine the winner(s) indexes
    vector<shared_ptr<Player>> winners;
    for (auto player : this->players) {
        if (player->get_points() >= max_points) {
            winners.push_back(player);
        }
    }

    cout << (winners.size() == 1 ? "Winner:" : "Winners: ");
    for (auto player : winners) {
        cout << SPACE << PLAYER_NAME_COLOR << player->get_name();
    }
    cout << rang::style::reset << endl;

    // now print score table
    cout << "Scores: " << endl;
    cout << "---------------------------------" << endl;

    // Justify all integers printed to have the same amount of character as the high score, left-padding with spaces
    cout << setw(static_cast<uint32_t>(floor(log10(max_points) + 1)));

    for (auto player : this->players) {
        cout << SCORE_COLOR << player->get_points() << rang::style::reset << " | " << PLAYER_NAME_COLOR << player->get_name() << rang::style::reset << endl;
    }
}

// You should not need to change this.
void Scrabble::main() {
    add_players();
    game_loop();
    final_subtraction(this->players);
    print_result();
}