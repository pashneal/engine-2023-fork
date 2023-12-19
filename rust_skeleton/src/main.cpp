#include <skeleton/actions.h>
#include <skeleton/constants.h>
#include <skeleton/runner.h>
#include <skeleton/states.h>
#include <iostream>


#define MAX_STREET_SIZE 20

using namespace pokerbots::skeleton;

extern "C" {

  // Friendlier types for the Rust FFI
  // make sure that the struct mirrors that which is found in Rust,
  // otherwise silent data corruption errors will occur
  struct RoundOverInfo{
    int delta;
    int street;
    char my_cards[2][2];
    char opp_cards[2][2];

    RoundOverInfo(int delta, int street, std::array<std::string, 2> my_cards, std::array<std::string, 2> opp_cards) {
      this->delta = delta;
      this->street = street;
      for (int i = 0; i < 2; i++) {
        this->my_cards[i][0] = my_cards[i][0];
        this->my_cards[i][1] = my_cards[i][1];
        if (opp_cards[i] == "") {
          this->opp_cards[i][0] = 'X';
          this->opp_cards[i][1] = 'X';
        } else {
          this->opp_cards[i][0] = opp_cards[i][0];
          this->opp_cards[i][1] = opp_cards[i][1];
        }
      }
    }
  };


  // Friendlier types for the Rust FFI
  // make sure that the struct mirrors that which is found in Rust,
  // otherwise silent data corruption errors will occur
  struct RoundInfo{
    int street;
    char my_cards[2][2];
    char board_cards[MAX_STREET_SIZE][2];;
    int my_pip;
    int opp_pip;
    int my_stack;
    int opp_stack;
    int num_legal_actions;
    Action legal_actions[4];
    
    int raise_bounds[2];
    
    RoundInfo(
      int street,
      std::array<std::string, 2> my_cards,
      std::array<std::string, MAX_STREET_SIZE> board_cards,
      int my_pip,
      int opp_pip,
      int my_stack,
      int opp_stack,
      int num_legal_actions,
      std::array<Action, 4> legal_actions,
      std::array<int, 2> raise_bounds
    ) {
      this->street = street;
      for (int i = 0; i < 2; i++) {
        this->my_cards[i][0] = my_cards[i][0];
        this->my_cards[i][1] = my_cards[i][1];
      }
      for (int i = 0; i < street; i++) {
        this->board_cards[i][0] = board_cards[i][0];
        this->board_cards[i][1] = board_cards[i][1];
      }
      this->my_pip = my_pip;
      this->opp_pip = opp_pip;
      this->my_stack = my_stack;
      this->opp_stack = opp_stack;
      this->num_legal_actions = num_legal_actions;
      for (int i = 0; i < 4; i++) {
        this->legal_actions[i] = legal_actions[i];
      }
      this->raise_bounds[0] = raise_bounds[0];
      this->raise_bounds[1] = raise_bounds[1];
    }

  };


  // Define all Rust FFI functions here with appropiate types
  void handle_round_over(GameInfo gameState, RoundOverInfo roundOverInfo);
  void handle_new_round(GameInfo gameState, RoundInfo roundInfo);
  Action handle_ongoing_round(GameInfo GameState, RoundInfo roundInfo);
}

struct Bot {
  /*
    Called when a new round starts. Called NUM_ROUNDS times.

    @param gameState The GameState object.
    @param roundState The RoundState object.
    @param active Your player's index.
  */
  void handleNewRound(GameInfoPtr gameState, RoundStatePtr roundState, int active) {
    auto legalActions = roundState->legalActions();  // the actions you are allowed to take
     int street = roundState->street;  // street int representing pre-flop, flop, turn, river, run# respectively
     auto myCards = roundState->hands[active];  // your cards
     auto boardCards = roundState->deck;  // the board cards
     int myPip = roundState->pips[active];  // the number of chips you have contributed to the pot this round of betting
     int oppPip = roundState->pips[1-active];  // the number of chips your opponent has contributed to the pot this round of betting
     int myStack = roundState->stacks[active];  // the number of chips you have remaining
     int oppStack = roundState->stacks[1-active];  // the number of chips your opponent has remaining
       
     int continueCost = oppPip - myPip;  // the number of chips needed to stay in the pot
     auto raiseBounds = roundState->raiseBounds();  // the smallest and largest numbers of chips for a legal bet/raise

    std::array<std::string, 2> myCardsBuffer;
    std::array<std::string, MAX_STREET_SIZE> boardCardsBuffer;
    std::array<Action, 4> legalActionsBuffer;
    std::array<int, 2> raiseBoundsBuffer;

    for (int i = 0; i < 2; i++) {
      myCardsBuffer[i] = myCards[i];
    }
    for (int i = 0; i < street; i++) {
      boardCardsBuffer[i] = boardCards.at(i).c_str();
    }
    int idx = 0;
    for (auto action : legalActions) {
      legalActionsBuffer[idx]=action;
      idx++;
    }
    raiseBoundsBuffer[0] = raiseBounds[0];
    raiseBoundsBuffer[1] = raiseBounds[1];

    auto roundInfo = new RoundInfo(
       street,
       myCardsBuffer,
       boardCardsBuffer,
       myPip,
       oppPip,
       myStack,
       oppStack,
       legalActions.size(),
       legalActionsBuffer,
       raiseBoundsBuffer
    );
    handle_new_round(*gameState.get(), *roundInfo);
    delete roundInfo;
  }

  /*
    Called when a round ends. Called NUM_ROUNDS times.

    @param gameState The GameState object.
    @param terminalState The TerminalState object.
    @param active Your player's index.
  */
  void handleRoundOver(GameInfoPtr gameState, TerminalStatePtr terminalState, int active) {
     int myDelta = terminalState->deltas[active];  // your bankroll change from this round
     auto previousState = std::static_pointer_cast<const RoundState>(terminalState->previousState);  // RoundState before payoffs
     int street = previousState->street;  // street int representing when this round ended
     auto myCards = previousState->hands[active];  // your cards
     auto oppCards = previousState->hands[1-active];  // opponent's cards or "" if not revealed
     auto roundOverInfo = new RoundOverInfo(myDelta, street, myCards, oppCards);
     handle_round_over(*gameState.get(), *roundOverInfo);
     delete roundOverInfo;
  }
  
  /*
    Where the magic happens - your code should implement this function.
    Called any time the engine needs an action from your bot.

    @param gameState The GameState object.
    @param roundState The RoundState object.
    @param active Your player's index.
    @return Your action.
  */
  Action getAction(GameInfoPtr gameState, RoundStatePtr roundState, int active) {
    auto legalActions = roundState->legalActions();  // the actions you are allowed to take
     int street = roundState->street;  // street int representing pre-flop, flop, turn, river, run# respectively
     auto myCards = roundState->hands[active];  // your cards
     auto boardCards = roundState->deck;  // the board cards
     int myPip = roundState->pips[active];  // the number of chips you have contributed to the pot this round of betting
     int oppPip = roundState->pips[1-active];  // the number of chips your opponent has contributed to the pot this round of betting
     int myStack = roundState->stacks[active];  // the number of chips you have remaining
     int oppStack = roundState->stacks[1-active];  // the number of chips your opponent has remaining
       
     int continueCost = oppPip - myPip;  // the number of chips needed to stay in the pot
     auto raiseBounds = roundState->raiseBounds();  // the smallest and largest numbers of chips for a legal bet/raise

    std::array<std::string, 2> myCardsBuffer;
    std::array<std::string, MAX_STREET_SIZE> boardCardsBuffer;
    std::array<Action, 4> legalActionsBuffer;
    std::array<int, 2> raiseBoundsBuffer;

    for (int i = 0; i < 2; i++) {
      myCardsBuffer[i] = myCards[i];
    }
    for (int i = 0; i < street; i++) {
      boardCardsBuffer[i] = boardCards.at(i).c_str();
    }
    int idx = 0;
    for (auto action : legalActions) {
      legalActionsBuffer[idx]=action;
      idx++;
    }
    raiseBoundsBuffer[0] = raiseBounds[0];
    raiseBoundsBuffer[1] = raiseBounds[1];

    auto roundInfo = new RoundInfo(
       street,
       myCardsBuffer,
       boardCardsBuffer,
       myPip,
       oppPip,
       myStack,
       oppStack,
       legalActions.size(),
       legalActionsBuffer,
       raiseBoundsBuffer
    );


    auto action = handle_ongoing_round(*gameState.get(), *roundInfo);
    delete roundInfo;
    return action;
  }
};

/*
  Main program for running a C++ pokerbot.
*/
int main(int argc, char *argv[]) {
  auto [host, port] = parseArgs(argc, argv);
  runBot<Bot>(host, port);
  return 0;
}
