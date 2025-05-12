#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <deque>
#include <utility>

using namespace std;

using Domino = pair<int, int>;

void shuffleDeck(vector<Domino>& deck, mt19937& gen) {
    shuffle(deck.begin(), deck.end(), gen);
}


void distributeCards(vector<Domino>& deck,
                     vector<Domino>& player1_cards,
                     vector<Domino>& player2_cards,
                     mt19937& gen) {
    uniform_int_distribution<> dis(0, 1); // 0 = player1, 1 = player2

    player1_cards.reserve(7);
    player2_cards.reserve(7);

    // Store the frequency of each value and suit for both players
    vector<int> player1_values_freq(7, 0), player2_values_freq(7, 0);
    int player1_suits = 0, player2_suits = 0;

    // Keep distributing cards until both players have 7 cards or the deck is empty
    while ((player1_cards.size() < 7 || player2_cards.size() < 7) && !deck.empty()) {

        // Randomly select a player to receive the next card
        int target = dis(gen); 

        const auto& card = deck.back();

        // Multiple checks are ran to ensure that the player can recieve the card to ensure that the game is fair
        // 1. The player must have 7 cards
        // 2. The player must not have more than 5 of the same value
        // 3. The player must not have more than 5 of the same suit
        if (target == 0 && player1_cards.size() < 7 && player1_values_freq[card.first] < 5 && player1_values_freq[card.second] < 5 && player1_suits < 5) {

            // Add the value to the frequency vector
            player1_values_freq[card.first]++;

            // Check if the card is a suit
            if (card.first == card.second) 
                player1_suits++; // increment the suit count
            else
                player1_values_freq[card.second]++; // this only runs if the card is not a suit, since the suit value is already added to the frequency vector
            
            
            player1_cards.push_back(card);
            deck.pop_back();

        } else if (target == 1 && player2_cards.size() < 7 && player2_values_freq[card.first] < 5 && player2_values_freq[card.second] < 5 && player2_suits < 5) {

            player2_values_freq[card.first]++;

            if (card.first == card.second) 
                player2_suits++; 
            else 
                player2_values_freq[card.second]++; 
            
            
            player2_cards.push_back(card);
            deck.pop_back();

        }
    }
}


void printDeck(const string& name, const vector<Domino>& deck) {
    cout << name << ":\n";
    for (const auto& card : deck) {
        cout << "[" << card.first << "|" << card.second << "] ";
    }
    cout << "\n\n";
}


bool canPlay(const Domino& d, int left, int right) {
    return d.first == left || d.second == left || d.first == right || d.second == right;
}


// Orient the domino based on the match and whether it's played on the left or right
Domino orient(const Domino& d, int match, bool left) {
    if (left) {
        if (d.second == match) return d;
        return {d.second, d.first};
    } else {
        if (d.first == match) return d;
        return {d.second, d.first};
    }
}


// This AI plays the game by always trying to play the smallest domino available and...
// ...prioritize playing on the left side of the board if possible
bool playSmallAI(vector<Domino>& hand, deque<Domino>& board, vector<Domino>& boneyard) {

    // Sort the hand in ascending order based on the sum of the domino values...
    // ...this is done to ensure that the player plays the smallest domino first
    sort(hand.begin(), hand.end(), [](const Domino& a, const Domino& b) {
        return (a.first + a.second) < (b.first + b.second);
    });


    while (true) {
        // Get the left and right ends of the board
        int left = board.front().first;
        int right = board.back().second;

        for (int i = 0; i < hand.size(); ++i) {
            Domino d = hand[i];

            if (canPlay(d, left, right)) { // check if the domino can be played

                if (d.second == left || d.first == left) 
                    board.push_front(orient(d, left, true));
                else
                    board.push_back(orient(d, right, false));
                
                hand.erase(hand.begin() + i);
                return true;
            }
        }

        // If no domino can be played, draw from the boneyard
        if (!boneyard.empty()) {
            hand.push_back(boneyard.back());
            boneyard.pop_back();
        } else {
            return false;
        }
    }
}

// This AI plays the game by always trying to play the largest domino available and...
// ...prioritize playing on the left side of the board if possible
bool playLargeAI(vector<Domino>& hand, deque<Domino>& board, vector<Domino>& boneyard) {

    // Sort the hand in descending order based on the sum of the domino values...
    // ...this is done to ensure that the player plays the largest domino first
    sort(hand.begin(), hand.end(), [](const Domino& a, const Domino& b) {
        return (a.first + a.second) > (b.first + b.second);
    });


    while (true) {
        // Get the left and right ends of the board
        int left = board.front().first;
        int right = board.back().second;

        for (int i = 0; i < hand.size(); ++i) {
            Domino d = hand[i];

            if (canPlay(d, left, right)) { // check if the domino can be played

                if (d.second == left || d.first == left) 
                    board.push_front(orient(d, left, true));
                else 
                    board.push_back(orient(d, right, false));
                
                hand.erase(hand.begin() + i);
                return true;
            }
        }

        // If no domino can be played, draw from the boneyard
        if (!boneyard.empty()) {
            hand.push_back(boneyard.back());
            boneyard.pop_back();
        } else {
            return false;
        }
    }
}


void gameManager(vector<Domino> player1, vector<Domino> player2, vector<Domino> boneyard) {
    deque<Domino> board;
    board.push_back(boneyard.back());
    boneyard.pop_back();

    cout << "Starting tile: [" << board.front().first << "|" << board.front().second << "]\n\n";

    bool p1Turn = true;
    int passCount = 0;

    while (true) {
        bool played = false;
        if (p1Turn) {
            played = playSmallAI(player1, board, boneyard);
            cout << "[Player 1] ";
        } else {
            played = playLargeAI(player2, board, boneyard);
            cout << "[Player 2] ";
        }

        for (const auto& d : board) cout << "[" << d.first << "|" << d.second << "] ";
        cout << (played ? "\n" : " - Pass\n");

        if (player1.empty()) {
            cout << "\n🏆 Player 1 wins!\n";
            break;
        }
        if (player2.empty()) {
            cout << "\n🏆 Player 2 wins!\n";
            break;
        }
        if (!played) passCount++;
        else passCount = 0;

        if (passCount >= 2) {
            cout << "\n🛑 Game ends in a draw.\n";
            break;
        }

        p1Turn = !p1Turn;
    }
}


int main() {
    // Seed RNG
    random_device rd;
    mt19937 gen(rd());

    vector<Domino> dominoes = {
        {0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5}, {0, 6},
        {1, 1}, {1, 2}, {1, 3}, {1, 4}, {1, 5}, {1, 6},
        {2, 2}, {2, 3}, {2, 4}, {2, 5}, {2, 6},
        {3, 3}, {3, 4}, {3, 5}, {3, 6},
        {4, 4}, {4, 5}, {4, 6},
        {5, 5}, {5, 6},
        {6, 6}
    };

    // Shuffle the deck randomly
    shuffleDeck(dominoes, gen);

    // Distribute cards to players
    vector<Domino> player1_cards, player2_cards;
    distributeCards(dominoes, player1_cards, player2_cards, gen);


    printDeck("Player 1", player1_cards);
    printDeck("Player 2", player2_cards);
    printDeck("Remaining dominoes (boneyard)", dominoes);


    // Start the game
    gameManager(player1_cards, player2_cards, dominoes);

    return 0;
}
