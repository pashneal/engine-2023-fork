/// Module for defining the C types defined within the C
/// skeleton for the poker engine. Must be kept in sync
/// with the types found within main.cpp otherwise
/// silent data currption will occur.

use std::ffi::{c_char, c_int, c_double};

const MAX_LEGAL_ACTIONS : usize = 4;
const MAX_STREET_SIZE: usize = 20;

pub struct Card([char; 2]);


#[derive(Debug, Clone, Copy)]
#[repr(C)]
pub struct GameInfo {
  pub bankroll : c_int,
  pub game_clock: c_double,
  pub round_num : c_int,
}
    
#[derive(Debug, Clone, Copy)]
#[repr(C)]
pub enum ActionType { 
    Fold, 
    Call, 
    Check, 
    Raise,
}

#[derive(Debug, Clone, Copy)]
#[repr(C)]
pub struct Action {
    pub action_type : ActionType,
    pub amount : c_int,
}

#[derive(Debug, Clone, Copy)]
#[repr(C)]
pub struct RoundInfo {
    pub street : c_int, // Number of community cards currently face up in the middle of the board
    my_cards : [[c_char; 2];2], //  The two hole cards provided to the player
    board_cards : [[c_char; 2]; MAX_STREET_SIZE],  // The community cards on the board
    pub my_pip : c_int, // Number of chips contributed to the current round of betting
    pub opp_pip : c_int, // Number of opponent chips contributed to the current round of betting
    pub my_stack : c_int, // Number of chips the player has remaining for raises or bets 
    pub opp_stack : c_int, // Number of chips the opponent has remaining for raises or bets
    num_legal_actions : c_int, // The number of legal actions the player can take
    legal_actions : [Action; MAX_LEGAL_ACTIONS], // The set of legal actions 
    pub raise_bounds : [c_int; 2], // Smallest and largest numbers of chips for a legal raise
}

#[derive(Debug, Clone, Copy)]
#[repr(C)]
pub struct RoundOverInfo {
    pub delta : c_int, // The number of chips the player won or lost
    pub street : c_int, // The street at which the round ended
    my_cards : [[c_char; 2];2], // The two hole cards provided to the player    
    opponent_cards : [[c_char; 2];2] // opponent's cards or "XX" if not revealed
}


////////////////////////////
/// IMPLEMENTATIONS
////////////////////////////


impl Card {
    pub fn rank(&self) -> char {
        self.0[RANK]
    }

    pub fn suit(&self) -> char {
        self.0[SUIT]
    }
}

const RANK : usize = 0;
const SUIT : usize = 1;

impl RoundInfo {
    pub fn player_cards(&self) -> [Card; 2] {
        let mut ret = [['X'; 2]; 2];
        for i in 0..2 {
            for j in 0..2 {
                ret[i][j] = self.my_cards[i][j] as u8 as char;
            }
        }

        [Card(ret[0]), Card(ret[1])]
    }

    pub fn community_cards(&self) -> Vec<Card> {
        let board_size = self.street as usize;
        let mut ret = [['X'; 2]; MAX_STREET_SIZE];
        for i in 0..board_size {
            for j in 0..2 {
                ret[i][j] = self.board_cards[i][j] as u8 as char;
            }
        }

        let mut ret2 = Vec::new();
        for i in 0..board_size {
            ret2.push(Card(ret[i]));
        }
        ret2
    }

    pub fn legal_actions(&self) -> Vec<ActionType> {
        let mut ret = Vec::new();
        for i in 0..self.num_legal_actions {
            ret.push(self.legal_actions[i as usize].action_type.clone());
        }
        ret
    }
}


impl RoundOverInfo{
    pub fn player_cards(&self) -> [Card; 2] {
        let mut ret = [['X'; 2]; 2];
        for i in 0..2 {
            for j in 0..2 {
                ret[i][j] = self.my_cards[i][j] as u8 as char;
            }
        }

        [Card(ret[0]), Card(ret[1])]
    }

    pub fn opponent_cards(&self) -> Option<[Card; 2]> {
        if self.opponent_cards[0][0] as u8 as char == 'X' {
            None
        } else {
            let mut ret = [['X'; 2]; 2];
            for i in 0..2 {
                for j in 0..2 {
                    ret[i][j] = self.opponent_cards[i][j] as u8 as char;
                }
            }
            let ret = [Card(ret[0]), Card(ret[1])];
            Some(ret)
        }
    }
}

////////////////////////////
/// MACROS
////////////////////////////
#[macro_export]
macro_rules! raise {
    ( $x:expr) => {
        {
            Action{action_type: ActionType::Raise, amount: $x}
        }
    };

}
#[macro_export]
macro_rules! call {
    () => {
        {
            Action{action_type: ActionType::Call, amount: 0}
        }
    };
}
#[macro_export]
macro_rules! fold {
    () => {
        {
            Action{action_type: ActionType::Fold, amount: 0}
        }
    };
}
#[macro_export]
macro_rules! check {
    () => {
        {
            Action{action_type: ActionType::Check, amount: 0}
        }
    };
}
