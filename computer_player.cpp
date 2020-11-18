
#include <memory>

#include "computer_player.h"
#include <string>
#include <vector>
#include <map>
#include <iostream>

using namespace std;

void ComputerPlayer::left_part(Board::Position anchor_pos, 
                               std::string partial_word,
                               Move partial_move, 
                               std::shared_ptr<Dictionary::TrieNode> node, 
                               size_t limit,
                               TileCollection& remaining_tiles,
                               std::vector<Move>& legal_moves,
                               const Board& board) const {
  // HW5: IMPLEMENT THIS

  // check if current string isn't valid prefix 
  if(node->nexts.empty()) return;
  // call extend right here
  extend_right(anchor_pos, partial_word, partial_move, node, remaining_tiles, legal_moves, board);
  
  if(limit > 0){
    map<char, shared_ptr<Dictionary::TrieNode>>::iterator it;
    // for each possible next letter 
    for(it = node->nexts.begin(); it != node->nexts.end(); it++){
      try{
        // throws exception if tile doesn't exist in tile bag
        TileKind current_tile = remaining_tiles.lookup_tile(it->first);
        
        // update partial_move
        partial_move.tiles.push_back(current_tile);
        if(partial_move.direction == Direction::ACROSS) 
          partial_move.column--;
        if(partial_move.direction == Direction::DOWN) 
          partial_move.row--;

        // remove it from the tile bag
        remaining_tiles.remove_tile(current_tile);
        
        left_part(anchor_pos, it->first + partial_word, partial_move, it->second, limit-1, remaining_tiles, legal_moves, board);
        
        // revert partial_move
        partial_move.tiles.pop_back();
        if(partial_move.direction == Direction::ACROSS) partial_move.column++;
        if(partial_move.direction == Direction::DOWN) partial_move.row++;

        // add it back if it returns 
        remaining_tiles.add_tile(current_tile);
      }
      catch(out_of_range &e){
        // if an exception is caught, it means that tile doesn't exist in the bag anymore
      }
      try{
        // throws exception if tile doesn't exist in tile bag
        TileKind blank_tile = remaining_tiles.lookup_tile('?');
        blank_tile.assigned = it->first;

        // update partial_move
        partial_move.tiles.push_back(blank_tile);
        if(partial_move.direction == Direction::ACROSS) 
          partial_move.column--;
        if(partial_move.direction == Direction::DOWN) 
          partial_move.row--;

        // remove it from the tile bag
        remaining_tiles.remove_tile(blank_tile);
        

        left_part(anchor_pos, it->first + partial_word, partial_move, it->second, limit-1, remaining_tiles, legal_moves, board);

        // revert partial_move
        partial_move.tiles.pop_back();
        if(partial_move.direction == Direction::ACROSS) partial_move.column++;
        if(partial_move.direction == Direction::DOWN) partial_move.row++;

        // add it back if it returns 
        remaining_tiles.add_tile(blank_tile);
      }
      catch(out_of_range &e){
        // if an exception is caught, it means that tile doesn't exist in the bag anymore
      }
    }
  }
}

void ComputerPlayer::extend_right(Board::Position square,
                                  std::string partial_word,
                                  Move partial_move, 
                                  std::shared_ptr<Dictionary::TrieNode> node,
                                  TileCollection& remaining_tiles,
                                  std::vector<Move>& legal_moves,
                                  const Board& board) const {
    // HW5: IMPLEMENT THIS
  // check that the tile is vacant
  if(!board.in_bounds_and_has_tile(square)){
    if(node->is_final){
      legal_moves.push_back(partial_move);
    }
      
    // scanning right
    map<char, std::shared_ptr<Dictionary::TrieNode>>::iterator it;
    // for each possible next letter 
    for(it = node->nexts.begin(); it != node->nexts.end(); it++){
      try{
        // throws exception if tile doesn't exist in tile bag
        TileKind current_tile = remaining_tiles.lookup_tile(it->first);
        // remove it from the tile bag
        remaining_tiles.remove_tile(current_tile);
        // update partial_move
        partial_move.tiles.push_back(current_tile);
        
        extend_right(square.translate(partial_move.direction), partial_word + it->first, partial_move, it->second, remaining_tiles, legal_moves, board);

        // add tile back if it returns 
        remaining_tiles.add_tile(current_tile);
        // revert partial_move
        partial_move.tiles.pop_back();
        
      }
      catch(out_of_range &e){
        // if an exception is caught, it means that tile doesn't exist in the bag an
      }
      try{
        // throws exception if tile doesn't exist in tile bag
        TileKind blank_tile = remaining_tiles.lookup_tile('?');
        blank_tile.assigned = it->first;

        // remove it from the tile bag
        remaining_tiles.remove_tile(blank_tile);
        // update partial_move
        partial_move.tiles.push_back(blank_tile);

        extend_right(square.translate(partial_move.direction), partial_word + it->first, partial_move, it->second, remaining_tiles, legal_moves, board);

        // add tile back if it returns 
        remaining_tiles.add_tile(blank_tile);
        // revert partial_move
        partial_move.tiles.pop_back();
      }
      catch(out_of_range &e){
        // if an exception is caught, it means that tile doesn't exist in the bag anymore
      }
    }
  }
  // if there's already a tile
  else{
    map<char, std::shared_ptr<Dictionary::TrieNode>>::iterator it = node->nexts.find(board.letter_at(square));
    if(it != node->nexts.end()){
      char new_letter = board.letter_at(square);
      extend_right(square.translate(partial_move.direction), partial_word + new_letter, partial_move, it->second, remaining_tiles, legal_moves, board);
    }
  }
}

Move ComputerPlayer::get_move(const Board& board, const Dictionary& dictionary) const {
  std::vector<Move> legal_moves;
  std::vector<Board::Anchor> anchors = board.get_anchors();
  // HW5: IMPLEMENT THIS

  for(int i=0; i<(int)anchors.size(); i++){
    // if there are empty tiles to the left
    if(anchors[i].limit > 0){
      std::vector<TileKind> new_tiles;
      Move current_move = Move(new_tiles, anchors[i].position.row, anchors[i].position.column, anchors[i].direction);

      // make a copy of the hand because this function is const
      TileCollection temp_hand(tiles);

      left_part(anchors[i].position, "", current_move, dictionary.get_root(), anchors[i].limit, temp_hand, legal_moves, board);
    }
    
    // if there are no empty tiles to the left
    if(anchors[i].limit == 0){
      // check tiles that already exist to the left
      string partial_word = "";
            
      Board::Position temp = anchors[i].position;
      
      // scan left 
      temp.translate(anchors[i].direction, -1);
      while(board.in_bounds_and_has_tile(temp)){
        partial_word = board.letter_at(temp) + partial_word; 
        temp.translate(anchors[i].direction, -1);
      }
      shared_ptr<Dictionary::TrieNode> current_node = dictionary.find_prefix(partial_word);

      // if the pre-existing prefix is invalid 
      if(!current_node) continue; 

      std::vector<TileKind> new_tiles;
      Move partial_move = Move(new_tiles, anchors[i].position.row, anchors[i].position.column, anchors[i].direction);

      TileCollection temp_hand(tiles);

      extend_right(anchors[i].position, partial_word, partial_move, current_node,temp_hand, legal_moves, board);
    }
  }

  return get_best_move(legal_moves, board, dictionary);
}

Move ComputerPlayer::get_best_move(std::vector<Move> legal_moves, const Board& board, const Dictionary& dictionary) const {
    Move best_move = Move(); // Pass if no move found 
  // HW5: IMPLEMENT THIS
  
  int max = 0;
  int max_index;
  int valid_results = 0;
  for(int i=0; i<(int)legal_moves.size(); i++){
    PlaceResult result = board.test_place(legal_moves[i]);

    // check if all words created are in the dictionary
    for(int j=0; j<(int)result.words.size(); j++){
      if(!dictionary.is_word(result.words[j])){
        result.valid = false;
      }
    }

    // if result wasn't valid, just continue
    if(!result.valid) continue;
    valid_results ++;

    int total_points = result.points;

    if(legal_moves[i].tiles.size() == get_hand_size())
      total_points += 50;
    if(total_points > max){
      max = total_points;
      max_index = i;
    }
  }
  if(!valid_results == 0) best_move = legal_moves[max_index];

  return best_move; 
}
