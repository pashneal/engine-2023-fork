mod ctypes;
use ctypes::*;


// Called whenever a new round starts
#[no_mangle]
pub extern "C" fn handle_new_round(game_info: GameInfo, round_info : RoundInfo) {
    println!("Round initialized!");
    println!("Game info: {:?}", game_info);
    println!("Round info: {:?}", round_info);
}
  
// Called whenever a round begins with a new deal of cards
//
// Returns the action that will be taken per turn
// this is where most of the meat of the code should end up calling into
#[no_mangle]
pub extern "C" fn handle_ongoing_round(game_info: GameInfo, round_info : RoundInfo) ->  Action {
    println!("{}", game_info.game_clock);
    let cards = round_info.player_cards();
    let first_rank = cards[0].rank();
    let second_rank = cards[1].rank();

    if first_rank == second_rank {
        return raise!(10)
    }

    if (first_rank == 'A' || first_rank == 'K' || first_rank == 'Q') 
        && (second_rank == 'A' || second_rank == 'K' || second_rank == 'Q')
    {
        return raise!(300)
    }


    fold!()
}


// Called whenever a round is over with results
// and available information about the round
#[no_mangle]
pub extern "C" fn handle_round_over(game_info: GameInfo, round_over_info : RoundOverInfo) {
    println!("Round over!");
    println!("Game info: {:?}", game_info);
    println!("Round over info: {:?}", round_over_info);

}

