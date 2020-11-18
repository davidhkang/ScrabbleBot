#include "player.h"

using namespace std;



// Adds points to player's score
void Player::add_points(size_t points){
	this->points += points;
}	

// Subtracts points from player's score
void Player::subtract_points(size_t points){
	this->points -= points;
}

size_t Player::get_points() const{
	return this->points;
}

const string& Player::get_name() const{
	return this->name;
}

// Returns the number of tiles in a player's hand.
size_t Player::count_tiles() const{
	return this->tiles.count_tiles();
}

// Removes tiles from player's hand.
void Player::remove_tiles(const vector<TileKind>& tiles){
	// loop through tiles vector and remove from hand 
	for(size_t i=0; i<tiles.size(); i++){
		this->tiles.remove_tile(tiles[i]);
	}
}

// Adds tiles to player's hand.
void Player::add_tiles(const vector<TileKind>& tiles){
	// loop through tiles vector and add each tile to hand 
	for(size_t i=0; i<tiles.size(); i++){
		this->tiles.add_tile(tiles[i]);
	}
}

// Checks if player has a matching tile.
bool Player::has_tile(TileKind tile){
	try{
		// if lookup_tile returns successfully, it means the tile exists
		if(tile == this->tiles.lookup_tile(tile.letter)) return true;
	}catch(out_of_range &e){
		return false; 
	}
	return false;

}

// Returns the total points of all tiles in the players hand.
unsigned int Player::get_hand_value() const{
	return this->tiles.total_points();
}

size_t Player::get_hand_size() const{
	return this->hand_size;
}