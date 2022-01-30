#pragma once

#include <vector>
#include <stdexcept>
#include <memory>
#include <cstdlib>
#include <ctime>
#include <ratio>
#include <chrono>
#include <algorithm>
#include <random>
#include <climits>
#include <cmath>
#include <iostream>
#include <string>
#include "tool.h"
#include "database_communicator.h"

using namespace std;

using namespace std::chrono;

struct coordinate
{
    int row;
    int col;
};

struct treasure_spot
{
    // stores the coordinate of a square that, if filled, makes a 3-in-a-row or 4-in-a-row
    // Also stores the coordinates of the two squares that would extend the newly formed hypothetical line of pieces
        // (which is formed after current_square gets filled).

    coordinate current_square;
    coordinate next_square;
    coordinate other_next_square; // Version 19!

    bool is_weak_2_square = false; // Assumed to be false by default, but if I do get a
                                   // weak 2 square, set this attribute to true.
};

struct position_info_for_TT // The key info of a position that will be stored in the transposition table.
{
    string board;
    int evaluation;
    int calculation_depth_from_this_position; // stores how far ahead the computer calculated for getting the position's evaluation.
    vector<coordinate> possible_moves_sorted; // stores the position's possible moves, sorted from probable best to probable worst.
    bool is_evaluation_indisputable; // stores true if there this position's evaluation will not change by calculating deeper...
                                     // someone has a forced win/forced draw. Someone could have just won/drawn in this position too.
    bool is_comp_turn; // stores true if it's the computer's turn in the position.
};

struct coordinate_and_value
{
    coordinate square; // stores the coordinates of the amplifying square.
    int value; // stores the value of the amplifying square, determined by the heuristics in smart_evaluation() and the function it calls.
};

struct coordinate_and_double // Object for storing amplifying squares and their value
{
    coordinate square;
    double value;
};

bool operator==(const coordinate& first, const coordinate& second) // function tests for equality between two coordiate objects
{
    return (first.row == second.row && first.col == second.col);
}

bool operator!=(const coordinate& first, const coordinate& second) // function tests for inequality between two coordiate objects
{
    return !(first == second);
}

class position : public tool
{
public:
    // Constructors:

    // PROGRAMMER CALLS TO START THE GAME.
    position(bool is_comp_turnP);

    // PROGRAMMER CALLS WHEN COMP/USER MAKES A MOVE IN GAME.
    position(const string& boardP, bool is_comp_turnP, coordinate last_moveP,
             const vector<treasure_spot>& squares_amplifying_comp_2P, const vector<treasure_spot>& squares_amplifying_comp_3P,
             const vector<treasure_spot>& squares_amplifying_user_2P, const vector<treasure_spot>& squares_amplifying_user_3P);

    // COMPUTER CALLS RECURSIVELY IN ITS MINIMAX CALCULATIONS.
    position(shared_ptr<string> boardP, bool is_comp_turnP,
             int depthP, int number_of_piecesP, coordinate last_moveP,
             const vector<coordinate>& possible_movesP, int possible_moves_index,
             int alphaP, int betaP,
             shared_ptr<vector<treasure_spot>> squares_amplifying_comp_2P, shared_ptr<vector<treasure_spot>> squares_amplifying_comp_3P,
             shared_ptr<vector<treasure_spot>> squares_amplifying_user_2P, shared_ptr<vector<treasure_spot>> squares_amplifying_user_3P,
             int hash_value_of_positionP, shared_ptr<vector<int>> num_pieces_per_columnP,
             bool did_comp_go_first_in_the_gameP);
    // No param for evaluation is sent to constructor, as this is figured out by the computer via minimax.
    // No param for future_positions is sent to constructor, as this is figured out by the computer via minimax.

    // Getters:
    vector<vector<char>> get_board() const;
//    vector <unique_ptr<position>> get_future_positions() const;
    int get_evaluation() const;
    bool get_is_comp_turn() const;
    int get_depth() const;
    int get_number_of_pieces() const;
    unique_ptr<position> get_a_future_position(int i); // MOVES the position object at index i of future_positions and returns!
    vector<unique_ptr<position>> get_future_positions(); // MOVES the future_positions vector and returns it!
    coordinate get_last_move() const;
    coordinate find_best_move_for_comp(); // finds the best move to play in this current position, for the comp, and returns.
                                          // This function finds the move the comp should play against user in the game.
    vector<treasure_spot> get_squares_amplifying_comp_2() const; // returns the squares_amplifying_comp_2 vector.
    vector<treasure_spot> get_squares_amplifying_comp_3() const; // returns the squares_amplifying_comp_3 vector.
    vector<treasure_spot> get_squares_amplifying_user_2() const; // returns the squares_amplifying_user_2 vector.
    vector<treasure_spot> get_squares_amplifying_user_3() const; // returns the squares_amplifying_user_3 vector.

    coordinate get_best_move_from_DB() const;

    // Setters:
    void set_board(const vector<vector<char>>& boardP);
//    void set_future_positions(const vector<unique_ptr<position>>& future_positionsP);
    void set_evaluation (int evalP);
    void set_is_comp_turn (bool turnP);
    void set_depth(int depthP);
    void randomize_order_of_possible_moves(); // randomizes the order of elements in the possible_moves vector.
    void clean_up_amplifying_vectors(); // removes all elements in the 4 amplifying vectors that are no longer ' ' chars.
    void rearrange_possible_moves(const vector<coordinate>& front_moves); // puts the moves in front_moves at the front of
                                                                          // the possible_moves vector of the calling object.
                                                                          // All these moves should already be in possible_moves.

    void add_position_to_transposition_table(bool is_evaluation_indisputable);
    // Adds this position's board (the key) and evaluation to the appropriate index in
    // the static transposition table (i.e., the hash_value of the position).

    void set_best_move_from_DB(const coordinate& val);

    // Helpers:
    bool did_computer_win() const; // returns true if the computer has won the game with a 4-in-a-row in the current position.
    bool did_opponent_win() const; // returns true if the user has won the game with a 4-in-a-row in the current position.
    bool is_game_drawn() const; // returns true if the game is drawn in the current position.
    bool evaluation_in_future_positions(int eval) const; // returns true if at least 1 future position has the evaluation value
                                                         // of the eval param.
    bool is_valid_move(string column) const; // Checks if the move entered by the user is valid.

    void remove_treasure_spot_objects_from_vector(shared_ptr<vector<treasure_spot>>& vec);
    // removes the treasure_spot objects from vec whose current_square attribute DOESN'T store
    // the location of an empty square in board (i.e., isn't ' ').

    void print_amplifying_vectors(); // prints the contents of all 4 amplifying vectors to the screen (this is just for testing!)

    void find_critical_moves(vector<coordinate>& critical_moves); // Fills vector with moves that can be played now, which
                                                                  // lead to a 4-in-a-row.

    void find_critical_moves_in_amplifying_vector(vector<coordinate>& critical_moves,
                                                  const shared_ptr<vector<treasure_spot>> amplifying_vector,
                                                  bool are_3_pieces, char piece);
    // Fills critical_moves vector with moves/squares in the amplifying vector that can be played now, which lead to a
    // 4-in-a-row.
    // bool are_3_pieces param stores true if the amplifying vector is for squares amplifying 3-in-a-rows, and false if
    // it is for 2-in-a-rows.
    // char piece param is simply the type of char ('C' or 'U') being dealt with.

    bool is_in_bounds(const coordinate& square) const; // returns true if square is in-bounds in board.

    bool in_coordinate_vector(const vector<coordinate>& vec, const coordinate& element); // returns true if element is in vec.

    void remove_duplicates(vector<coordinate>& vec); // Removes duplicate elements from the vector.

    void initialize_row_barriers(int& lowest_comp_square_D, int& lowest_good_comp_square_D,
                                 int& lowest_user_square_D, int& lowest_good_user_square_D);
                                    // Initializes row_barriers (private member vector) with 7 elements.
                                    // 1st element is row index of highest row (visually) allowed in column 0.
                                    // Also stores any squares in the 4 bool params as appropriate.

    void find_winning_squares(vector<coordinate>& vec, const shared_ptr<vector<treasure_spot>> squares_amplifying_3,
                              const shared_ptr<vector<treasure_spot>> squares_amplifying_2, char piece,
                              int& lowest_square_D, int& lowest_good_square_D, bool evens_are_good);
    // Function finds squares making a 4-in-a-row of piece and stores them in vec.

    void update_D_squares(int& lowest_square_D, int& lowest_good_square_D,
                          bool evens_are_good, const coordinate& candidate_square);

    // Functions specifically designed for, and only used in, the Versus Sim:

    double get_static_thinking_time() const;

    void set_static_thinking_time(double val);

    unique_ptr<tool> call_static_think_on_game_position(const vector<vector<char>>& boardP, bool is_comp_turnP,
                                                        coordinate last_moveP, const vector<treasure_spot>& squares_amplifying_comp_2P,
                                                        const vector<treasure_spot>& squares_amplifying_comp_3P,
                                                        const vector<treasure_spot>& squares_amplifying_user_2P,
                                                        const vector<treasure_spot>& squares_amplifying_user_3P, bool starting_new_game);

    unique_ptr<tool> call_static_think_on_game_position(bool is_comp_turnP, bool starting_new_game);

    coordinate_and_value find_quick_winning_move(int max_number_moves_acceptable) const;

    coordinate return_a_move_that_wins_immediately() const;

    bool can_create_threat_with_D(char piece, int highest_row) const;
                                              // Returns true if player with piece could possibly create a 4-in-a-row threat
                                              // at or below highest_row in column D.

    bool can_square_be_involved_in_horizontal_win(char piece, char opponent_piece, int row, int col) const;

    bool can_square_be_involved_in_upper_diagonal_win(char piece, char opponent_piece, int row, int col) const;

    bool can_square_be_involved_in_lower_diagonal_win(char piece, char opponent_piece, int row, int col) const;


    // Public static variables:
    static const int UNDEFINED; // used when evaluation, alpha, or beta is unknown.
    static const int max_row_index; // the max row index of board (i.e., 5, since there are 6 rows).
    static const int max_col_index; // the max col index of board (i.e., 6, since there are 7 columns).
    static int depth_limit; // the depth of the computer's calculation abilities.
    static bool stop_signal; // used for multithreading (thinking while waiting for the user to move).

    static const int max_hash_value;

    static vector<vector<position_info_for_TT>> transposition_table; // A position's key & evaluation get stored here, at the appropriate
                                                                     // index (i.e., its hash value). The inner vector is to deal with possible
                                                                     // collisions. Multiple positions can be stored at the same
                                                                     // index in the outer vector via the inner vector.

    static vector<int> indices_of_elements_in_TT; // Stores the indices of which inner vectors in the TT actually store data.

    static vector<vector<int>> random_values_for_squares_with_C;
    static vector<vector<int>> random_values_for_squares_with_U;
    // These vectors each contain 42 random numbers, to be used for zobrist hashing.

    static bool surpassed_DB;

    static int counter; // counts how many times the position class is instantiated. PURELY FOR TESTING!
    static int counter_of_TT_usefulness; // counts how many times the TT is actually useful. PURELY FOR TESTING!

    static double thinking_time; // Comp spends this long thinking, plus the time it spends on the last iteration of the
                                 // iterative deepening while loop.

    static vector<treasure_spot> empty_amplifying_vector;

    static vector<vector<bool>> board_of_squares_winning_for_comp; // 2-D board that stores true for a square if the comp wins after filling it in.
    static vector<vector<bool>> board_of_squares_winning_for_user; // 2-D board that stores true for a square if the user wins after filling it in.

    // Public static methods:

    static vector<vector<int>> get_board_of_random_ints();
    // Returns a 7x6 2-D vector of ints, storing a random value for each square.

    static void reset_transposition_table(); // Resets the transposition table to only store empty inner vectors.
                                             // Uses the indices_of_elements_in_TT vector to do this resetting task efficiently.
                                             // Also, this function will make the indices_of_elements_in_TT vector then be empty.

    static bool compare_future_positions_by_evaluation(const unique_ptr<position>& first_pos, const unique_ptr<position>& second_pos);
    // Function returns true if first_pos would be better than second_pos for the player. This of course depends on
    // on whose turn it is in the calling object/position, which has to be determined by looking at first_pos or second_pos,
    // since this is a static function.

    static position_info_for_TT find_duplicate_in_TT(const unique_ptr<position>& pt);
    // Searches through the TT for a duplicate of pt, and returns it.

    static coordinate find_legal_square(const string& boardP, int col);

    static unique_ptr<position> think_on_game_position(const vector<vector<char>>& boardP, bool is_comp_turnP, coordinate last_moveP,
                                    const vector<treasure_spot>& squares_amplifying_comp_2P, const vector<treasure_spot>& squares_amplifying_comp_3P,
                                    const vector<treasure_spot>& squares_amplifying_user_2P, const vector<treasure_spot>& squares_amplifying_user_3P,
                                    bool starting_new_game, coordinate& best_move, bool generate_best_move);
    // Function thinks and returns a unique_ptr that points to a position object of all its calculations.
    // THIS FUNCTION SHOULD BE THE ONE CALLED FROM MAIN WHENEVER THE ENGINE NEEDS TO THINK! Or the function below for starting a new game...

    static unique_ptr<position> think_on_game_position(bool is_comp_turnP, bool starting_new_game,
                                                       coordinate& best_move, bool generate_best_move);

    static string create_starting_board_string();

    static vector<vector<char>> create_starting_board_2D_vector();

    static void read_amplifying_squares_into_pointer(shared_ptr<vector<treasure_spot>>& the_pointer, const vector<treasure_spot>& amplifying_vectorP);

    static vector<vector<bool>> create_board_of_bools(); // Returns a board storing false for each square.

    static void reset_board_of_bools(vector<vector<bool>>& board_of_bools, const vector<coordinate_and_double>& squares);

    static int index(int row, int col);
    // This function has the inline keyword, but it's only written in the function definition, not
    // the declaration here. See https://isocpp.org/wiki/faq/inline-functions#inline-member-fns.
    // Also, it doesn't matter whether an inline function is static or not, provided
    // its a member of a class (like index is). See the quote from 12.2.1 in:
    // https://stackoverflow.com/questions/44788412/c-inline-functions-declare-as-such-define-as-such-or-both-why

private:
    // Private variables:
    shared_ptr<string> board; 
    // stores C's and U's and ' ', representing the computer and user's pieces and empty squares.
    // The 'a' column of the connect four board is represented by the chars at indices 0 to 5, where index
    // 0 represents the top of the column, and index 5 represents the bottom. The 'b' column is
    // represented by indices 6 to 11, etc. Storing rows of the connect four board together is also
    // of course an option, but squares in columns may be more often used together (e.g., spaces above
    // an empty square in a column must also be empty).
    // The board can be indexed similar to how the 2D vector was. For any row and col values you were using,
    // instead of doing [row][col], just replace it by indexing the string board with [index_board(row, col)]. 
    // This hides any abstraction done, which is simply the line col * 6 + row. Using this function,
    // the string board can be treated with the same mental model as the 2D vector, for representing the board.
    bool is_comp_turn; // stores true if it's the computer's turn, and false if it's the user's turn.
    bool did_comp_go_first_in_the_game; // stores true if the computer started out moving first at the root node.
    int depth; // stores how deep this position is in the computer's calculations.
    int number_of_pieces; // stores how many pieces are in the current board. Only 'C' and 'U' count as pieces (not ' ').
    coordinate last_move; // last_move, an object of the coordinate struct, stores the row and col of the last move (made in
                          // the previous node).
    vector <coordinate> possible_moves; // stores all the possible moves in this current position. Usually there are 7.
    int alpha; // stores the best alternative found so far FOR THE COMPUTER at this time in the entire search. (i.e., highest val).
    int beta; // stores the best alternative found so far FOR THE USER at this time in the entire search (i.e., lowest val).
    int evaluation; // stores -1 if the computer is losing, 0 if the game is drawn, and +1 if the computer is winning.

    vector <unique_ptr<position>> future_positions; // stores pointers to all future positions one move ahead.
    // stored as pointers in order to be efficient with memory, as position objects are huge.

    shared_ptr<vector<treasure_spot>> squares_amplifying_comp_2; // squares that, if filled, turn the comp's 2-in-a-row into a 3-in-a-row.
    shared_ptr<vector<treasure_spot>> squares_amplifying_comp_3; // squares that, if filled, turn the comp's 3-in-a-row into a 4-in-a-row.
    shared_ptr<vector<treasure_spot>> squares_amplifying_user_2; // squares that, if filled, turn the user's 2-in-a-row into a 3-in-a-row.
    shared_ptr<vector<treasure_spot>> squares_amplifying_user_3; // squares that, if filled, turn the user's 3-in-a-row into a 4-in-a-row.

    vector<int> row_barriers; // stores the highest (visually) rows allowed for play in each column of the board,
                              // due to a square allowing both comp AND user to win (this is "finished column" algorithm).
                              // row_barriers[0] stores highest row (visually) allowed for play in column 0.
                              // row_barriers[6] stores highest row (visually) allowed for play in column 6.
                              // NOTE: This member is only initialized in smart_evaluation(), aka when depth limit is reached,
                              // since it has no use before then.

    int hash_value_of_position;

    bool is_a_pruned_branch; // Initialized to false - stores true if this node (and all its children) gets pruned via alpha-beta pruning.

    bool got_value_from_pruned_child; // Initialized to false - stores true if this node's final evaluation value came from
                                      // a child that got pruned (which means the child's value isn't accurate). To know
                                      // if a child got pruned, refer to the child's bool variable above ("is_a_pruned_branch").

    shared_ptr<vector<int>> num_pieces_per_column; // index 0 stores number of pieces in column 0. All the way up to...
                                                   // index 6 stores number of pieces in column 6.

    int calculation_depth_from_this_position; // stores how many moves ahead the comp will calculate from this current position.

    coordinate best_move_from_DB;

    // Private methods:
    void analyze_last_move(); // analyzes the last move to see if anyone won and to add anything to the above 4 vectors
                              // storing squares that allow 3-in-a-rows or 2-in-a-rows to be amplifyed.
    void analyze_horizontal_perspective_of_last_move(); // is the horizontal perspective of "analyze_last_move()".
    void analyze_vertical_perspective_of_last_move(); // is the vertical perspective of "analyze_last_move()".
    void analyze_positive_slope_diagonal_perspective_of_last_move(); // is the +slope diagonal perspective of
                                                                     // "analyze_last_move()".
    void analyze_negative_slope_diagonal_perspective_of_last_move(); // is the -slope diagonal perspective of
                                                                     // "analyze_last_move()".
    void add_to_appropriate_amplifying_vector(int num_pieces_in_a_row, const treasure_spot& empty_square);
    // function adds empty_square to one of the four amplifying vectors, depending on num_pieces_in_a_row and whose turn it is.
    void minimax(); // Employs the minimax algorithm...
                    // fills the future_positions vector with all positions one move ahead.
                    // eventually gives the evaluation attribute a value.
    double find_revised_player_evaluation(const vector<coordinate_and_double>& info_for_player_amplifying_squares,
                                          const vector<vector<bool>>& squares_winning_for_player,
                                          const vector<vector<bool>>& squares_winning_for_opponent, bool is_player_the_comp) const;
    void smart_evaluation(); // evaluates the position at depth_limit, if no one has won. Gives the evaluation attribute a value.
    void find_individual_player_evaluation(const shared_ptr<vector<treasure_spot>> squares_amplifying_3,
                                           const shared_ptr<vector<treasure_spot>> squares_amplifying_2, char piece,
                                           vector<vector<bool>>& board_of_squares_winning_for_player, vector<coordinate_and_double>& recorder) const;
                                           // Goes through the amplifying vectors, and evaluates each unique square.
                                           // The coordinates and value for each unique amplifying square are stored in recorder,
                                           // which is passed by reference.
                                           // All squares making a 4-in-a-row are given an 'A' in copy_board, which is also passed by reference.
    bool is_vertically_adjacent_to_a_winning_square(const vector<vector<bool>>& the_board, const coordinate& square) const;
    bool did_someone_win() const; // returns true if there is a 4-in-a-row in board, meaning someone won.
    bool horizontal_four_combo() const; // returns true if there is a horizontal 4-in-a-row in board.
    bool vertical_four_combo() const; // returns true if there is a vertical 4-in-a-row in board.
    bool positive_slope_diagonal_four_combo() const; // returns true if there is a positive slope diagonal 4-in-a-row in board.
    bool negative_slope_diagonal_four_combo() const; // returns true if there is a negative slope diagonal 4-in-a-row in board.
    bool is_acceptable_letter(char c) const; // returns true if char c is a letter from a-g (uppercase OR lowercase).
    bool is_element_in_vector(const vector<string>& vec, const string& element) const;
    // returns true if element is in vector vec. *Note*: vec is just a vector storing boards,
    // and element is a board (not necessarily the private board attribute of the calling object though!).
    coordinate find_starting_horizontal_point() const; // finds left-most connected square from last_move.
    coordinate find_ending_horizontal_point() const; // finds right_most connected square from last_move.
    coordinate find_ending_vertical_point() const; // finds the bottom-most connected square from last_move.
    coordinate find_starting_positive_slope_diagonal_point() const; // finds the bottom-left-most connected square from last_move.
    coordinate find_ending_positive_slope_diagonal_point() const; // finds the top_right-most connected sqaure from last_move.
    coordinate find_starting_negative_slope_diagonal_point() const; // finds the top-left-most connected square from last_move.
    coordinate find_ending_negative_slope_diagonal_point() const; // finds the bottom-right-most connected square from last_move.

    void display_board();
};

// Initializing the static variables:

const int position::UNDEFINED = INT_MAX - 1; // just a random value.
const int position::max_row_index = 5;
const int position::max_col_index = 6;
int position::depth_limit = 1; // starts off at 1 every time the Engine thinks (iterative deepening).
bool position::stop_signal = false;

const int position::max_hash_value = pow(2, 20) - 1; // If you want to increase the range of the random numbers and the hash
// table size, just increase 20 to some integer a bit greater. The 20 represents the number of bits in the range of the random
// numbers / TT size.
// E.g., change the 20 above to 24, for a TT size of over 16 million.

vector<vector<position_info_for_TT>> position::transposition_table(max_hash_value + 1);
vector<int> position::indices_of_elements_in_TT;

vector<vector<int>> position::random_values_for_squares_with_C = get_board_of_random_ints();
vector<vector<int>> position::random_values_for_squares_with_U = get_board_of_random_ints();

bool position::surpassed_DB = false;

int position::counter = 0;
int position::counter_of_TT_usefulness = 0;

double position::thinking_time = 0.30;

vector<treasure_spot> position::empty_amplifying_vector;

vector<vector<bool>> position::board_of_squares_winning_for_comp = create_board_of_bools();
vector<vector<bool>> position::board_of_squares_winning_for_user = create_board_of_bools();

// CONSTRUCTORS:

position::position(bool is_comp_turnP)
{
    best_move_from_DB = {UNDEFINED, UNDEFINED};

    board = make_shared<string>(42, ' ');

    // Initialize num_pieces_per_column:

    num_pieces_per_column = make_shared<vector<int>>();

    for (int col = 0; col <= max_col_index; col++)
    {
        num_pieces_per_column->push_back(0);
    }

    squares_amplifying_comp_2 = make_shared<vector<treasure_spot>>();

    squares_amplifying_comp_3 = make_shared<vector<treasure_spot>>();

    squares_amplifying_user_2 = make_shared<vector<treasure_spot>>();

    squares_amplifying_user_3 = make_shared<vector<treasure_spot>>();

    is_comp_turn = is_comp_turnP;

    did_comp_go_first_in_the_game = is_comp_turn;

    depth = 0;

    calculation_depth_from_this_position = depth_limit - depth;

    number_of_pieces = 0;

    last_move.row = UNDEFINED;
    last_move.col = UNDEFINED;

    // Giving last_move.row and last_move.col the UNDEFINED value, since this constructor is for starting the entire game.
    // No one has made a move yet.
    // I'll just not call any of the methods which check if the game has ended (it's impossible for it to end when it starts!).
    // The methods that check if someone won rely heavily on last_move.

    // INITIALIZE POSSIBLE_MOVES VECTOR:

    for (int c = 0; c <= max_col_index; c++)
    {
        coordinate temp;
        temp.col = c;
        temp.row = max_row_index;
        possible_moves.push_back(temp);
    }
    //   randomize_order_of_possible_moves(); No longer using randomness here.
    // Besides, in most cases possible_moves will just be set to a possible moves vector from an earlier duplicate in the TT.

    alpha = UNDEFINED;

    beta = UNDEFINED;

    evaluation = UNDEFINED;

    hash_value_of_position = 0;

    // Now, I don't need to check if someone won, since this constructor starts the entire game.
    // I also don't need to call the analyze_last_move() function, since there is no last_move yet!

    is_a_pruned_branch = false;
    got_value_from_pruned_child = false;

    // So, call minimax() now:

    minimax();
}

position::position(const string& boardP, bool is_comp_turnP, coordinate last_moveP,
                   const vector<treasure_spot>& squares_amplifying_comp_2P, const vector<treasure_spot>& squares_amplifying_comp_3P,
                   const vector<treasure_spot>& squares_amplifying_user_2P, const vector<treasure_spot>& squares_amplifying_user_3P)
{
    best_move_from_DB = {UNDEFINED, UNDEFINED};

    board = make_shared<string>(boardP);

    is_comp_turn = is_comp_turnP;

    depth = 0;

    calculation_depth_from_this_position = depth_limit - depth;

    number_of_pieces = 42 - count((*board).begin(), (*board).end(), ' ');

    // Initialize num_pieces_per_column:

    num_pieces_per_column = make_shared<vector<int>>(7, 0);

    // Now figure out the number of pieces in each column, to initialize the num_pieces_per_column vector:
    for (int col = 0; col <= max_col_index; ++col)
    {
        for (int row = 0; row <= max_row_index; ++row)
        {
            if ((*board)[index(row, col)] != ' ')
            {
                (*num_pieces_per_column)[col] ++;
            }
        }
    }

    last_move = last_moveP;

    // Figure out the possible moves in this position, and then randomize their order:

    for (int col = 0; col <= max_col_index; col++)
    {
        for (int row = max_row_index; row >= 0; row--)
        {
            if ((*board)[index(row, col)] == ' ') // found the legal move in this column:
            {
                coordinate temp;
                temp.row = row;
                temp.col = col;
                possible_moves.push_back(temp);
                break; // move on to next column via the outer for loop, since I found the one legal move of this column.
            }
        }
    }
 //   randomize_order_of_possible_moves();

    alpha = UNDEFINED; // just some random value to signify that there is no alpha value yet.

    beta = UNDEFINED; // just some random value to signify that there is no beta value yet.

    evaluation = UNDEFINED; // just some random value to signify that there is no evaluation value yet.

    squares_amplifying_comp_2 = make_shared<vector<treasure_spot>>();

    squares_amplifying_comp_3 = make_shared<vector<treasure_spot>>();

    squares_amplifying_user_2 = make_shared<vector<treasure_spot>>();

    squares_amplifying_user_3 = make_shared<vector<treasure_spot>>();

    read_amplifying_squares_into_pointer(squares_amplifying_comp_2, squares_amplifying_comp_2P);

    read_amplifying_squares_into_pointer(squares_amplifying_comp_3, squares_amplifying_comp_3P);

    read_amplifying_squares_into_pointer(squares_amplifying_user_2, squares_amplifying_user_2P);

    read_amplifying_squares_into_pointer(squares_amplifying_user_3, squares_amplifying_user_3P);

    clean_up_amplifying_vectors(); // gets rid of obsolete treasure_spot objects in the 4 amplifying vectors,
                                   // making it faster for comp to evaluate in its calculations later at depth_limit.
                                   // Note that I can call this clean-up function in this constructor (2nd), since this
                                   // constructor is only called once per move from main.cpp. The 3rd constructor is
                                   // called thousands of times by the computer during minimax, so I will not do
                                   // clean-up there (not efficient!).

    // Figure out the hash value by initializing it to 0, and then XORing it with all the non-empty squares
    // in the position.

    hash_value_of_position = 0;

    for (int row = 0; row < 6; ++row) {
        for (int col = 0; col < 7; ++col) {
            if ((*board)[index(row, col)] == 'C') {
                hash_value_of_position ^= random_values_for_squares_with_C[row][col];
            } else if ((*board)[index(row, col)] == 'U') {
                hash_value_of_position ^= random_values_for_squares_with_U[row][col];
            }
        }
    }

    is_a_pruned_branch = false;
    got_value_from_pruned_child = false;

    // Initialize the bool variable storing whether comp moved first at the start of the game.
    // Figure out by deduction...

    if (number_of_pieces % 2 == 0)
    {
        // Even number of pieces.

        did_comp_go_first_in_the_game = is_comp_turn;
    }

    else // odd number of pieces:
    {
        did_comp_go_first_in_the_game = !is_comp_turn;
    }

    analyze_last_move(); // will analyze the last_move, and then call minimax() if the game isn't over.
}

position::position(shared_ptr<string> boardP, bool is_comp_turnP,
                   int depthP, int number_of_piecesP, coordinate last_moveP,
                   const vector<coordinate>& possible_movesP, int possible_moves_index,
                   int alphaP, int betaP,
                   shared_ptr<vector<treasure_spot>> squares_amplifying_comp_2P, shared_ptr<vector<treasure_spot>> squares_amplifying_comp_3P,
                   shared_ptr<vector<treasure_spot>> squares_amplifying_user_2P, shared_ptr<vector<treasure_spot>> squares_amplifying_user_3P,
                   int hash_value_of_positionP, shared_ptr<vector<int>> num_pieces_per_columnP,
                   bool did_comp_go_first_in_the_gameP)
{
    best_move_from_DB = {UNDEFINED, UNDEFINED};

    did_comp_go_first_in_the_game = did_comp_go_first_in_the_gameP;

    board = boardP; // Still have to make last_move.

    if ((*board)[index(last_moveP.row, last_moveP.col)] != ' ') // shouldn't have made last move yet...
    {
        throw runtime_error("Board with last_move already made was passed to third constructor.\n");
    }

    (*board)[index(last_moveP.row, last_moveP.col)] = is_comp_turnP ? 'U' : 'C';

    is_comp_turn = is_comp_turnP;

    depth = depthP;
    calculation_depth_from_this_position = depth_limit - depth;
    number_of_pieces = number_of_piecesP;
    last_move = last_moveP;

    num_pieces_per_column = num_pieces_per_columnP;
    (*num_pieces_per_column)[last_move.col] ++;

    possible_moves = possible_movesP;
    possible_moves[possible_moves_index].row --;
    if (possible_moves[possible_moves_index].row == -1)
    {
        possible_moves.erase(possible_moves.begin() + possible_moves_index);
    }
 //   randomize_order_of_possible_moves();

    alpha = alphaP;
    beta = betaP;
    evaluation = UNDEFINED; // just some random value to signify there is no evaluation value yet.

    // 4 variables below will be used to reset the 4 amplifying vector pointers to what they were when they were passed to this function.
    // Purpose of this is to get them ready for the parent node to use them.
    // I'm using chars to store the size in order to optimize efficiency. The size of an amplifying vector couldn't be larger
    // than 128...
    char old_size_of_squares_amplifying_comp_2 = static_cast<char>(squares_amplifying_comp_2P->size());
    char old_size_of_squares_amplifying_comp_3 = static_cast<char>(squares_amplifying_comp_3P->size());
    char old_size_of_squares_amplifying_user_2 = static_cast<char>(squares_amplifying_user_2P->size());
    char old_size_of_squares_amplifying_user_3 = static_cast<char>(squares_amplifying_user_3P->size());

    squares_amplifying_comp_2 = squares_amplifying_comp_2P;

    squares_amplifying_comp_3 = squares_amplifying_comp_3P;

    squares_amplifying_user_2 = squares_amplifying_user_2P;

    squares_amplifying_user_3 = squares_amplifying_user_3P;

    hash_value_of_position = hash_value_of_positionP;

    if ((*board)[index(last_move.row, last_move.col)] == 'C') {
        hash_value_of_position ^= random_values_for_squares_with_C[last_move.row][last_move.col];
    } else {
        hash_value_of_position ^= random_values_for_squares_with_U[last_move.row][last_move.col];
    }

    is_a_pruned_branch = false;
    got_value_from_pruned_child = false;

    analyze_last_move(); // will analyze the last_move, and then call minimax() if the game isn't over.

    (*board)[index(last_move.row, last_move.col)] = ' '; // since board is a pointer. 
    // This is repairing board for the parent node to use it.

    squares_amplifying_comp_2->resize(static_cast<int>(old_size_of_squares_amplifying_comp_2));
    squares_amplifying_comp_3->resize(static_cast<int>(old_size_of_squares_amplifying_comp_3));
    squares_amplifying_user_2->resize(static_cast<int>(old_size_of_squares_amplifying_user_2));
    squares_amplifying_user_3->resize(static_cast<int>(old_size_of_squares_amplifying_user_3));

    (*num_pieces_per_column)[last_move.col] --;
}

// GETTERS:

vector<vector<char>> position::get_board() const
{
    // Return a copy of the board:

    vector<vector<char>> board_as_vec(6, vector<char>(7));

    for (int r = 0; r < 6; ++r) {
        for (int c = 0; c < 7; ++c) {
            board_as_vec[r][c] = (*board)[index(r, c)];
        }
    }

    return board_as_vec;
}

int position::get_evaluation() const
{
    return evaluation;
}

bool position::get_is_comp_turn() const
{
    return is_comp_turn;
}

int position::get_depth() const
{
    return depth;
}

int position::get_number_of_pieces() const
{
    return number_of_pieces;
}

unique_ptr<position> position::get_a_future_position(int i)
{
    return move(future_positions[i]);
}

vector <unique_ptr<position>> position::get_future_positions()
{
    return move(future_positions);
}

coordinate position::get_last_move() const
{
    return last_move;
}

coordinate position::find_best_move_for_comp()
{
    if (depth_limit != 1)
    {
        throw runtime_error("depth limit does not equal 1 in find_best_move_for_comp()");
    }

    // First, check if best_move_from_DB is not undefined. If so, return that,
    // because the DB was used.

    if (best_move_from_DB.row != UNDEFINED && best_move_from_DB.col != UNDEFINED)
    {
        if (number_of_pieces > 9)
        {
            throw runtime_error("Trying to use the DB with > 9 pieces\n");
        }

        return best_move_from_DB;
    }

    // Find and return the last_move attribute of the best future_position.
    // The problem is choosing which is the best future_position.

    // First, it's possible that the calling object could have an empty future_positions vector since the calling object
    // automatically accepted an indisputable evaluation from a duplicate in the TT.
        // If this is the case, just access this duplicate position from the TT and pick the best of its possible moves.

    if (future_positions.empty())
    {
        // This position must have a forced win, but it got its evaluation immediately from the TT and doesn't have a future positions vector.

        // So, create a new search right here with depth_limit = 1 or 2. The TT will be used in the search process implicitly, as it
        // would normally.

        // Then, move the new pointer being created here into the future_positions vector.

        for (const coordinate& current: possible_moves)
        {
            string copy_board = *board;

            copy_board[index(current.row, current.col)] = 'C';

            unique_ptr<position> pt = make_unique<position>(copy_board, !is_comp_turn, current,
                                                            *squares_amplifying_comp_2, *squares_amplifying_comp_3,
                                                            *squares_amplifying_user_2, *squares_amplifying_user_3);
                // NOTE: The static method for returning a unique_ptr isn't being called here, as I don't care
                // about iterative deepening. I know there's a forced win, and the TT will be used at depth = 1.

            future_positions.push_back(move(pt));
        }
    }

    if (evaluation == INT_MAX) // Comp is winning, so see if there's a solution to win in <= 9 moves:
    {
        coordinate_and_value quick_winning_move = find_quick_winning_move(9); // UNDEFINED returned for value field if no solution in <= 9 moves.

        if (quick_winning_move.value != UNDEFINED)
        {
            return quick_winning_move.square;
        }
    }

    if (evaluation == INT_MIN) // Comp is losing, so find the move that makes the comp work the longest, or at least work for 7 moves.
    {
        // Translation: Is there a comp move such that find_quick_winning_move(7) FOR USER returns UNDEFINED for the value field?
        // If not, pick the move that survives the longest.

        coordinate_and_value most_stubborn_defense; // coordinate stores the move, and value stores how many moves the user will need to win after.
        most_stubborn_defense.value = 1; // Starting off with the worst possible value to have - it says the user wins right on the spot.
        most_stubborn_defense.square = {UNDEFINED, UNDEFINED};

        for (const coordinate& current_move: possible_moves)
        {
            string assisting_board = *board;

            assisting_board[index(current_move.row, current_move.col)] = 'C';

            unique_ptr<position> pt = make_unique<position>(assisting_board, !is_comp_turn, current_move, empty_amplifying_vector,
                                                            empty_amplifying_vector, empty_amplifying_vector, empty_amplifying_vector);

            int number_of_moves_user_wins_in = pt->find_quick_winning_move(7).value;

            if (number_of_moves_user_wins_in == UNDEFINED) // user will have to work for > 7 moves, so pick this option immediately!
            {
                return current_move;
            }

            else if (most_stubborn_defense.square.row == UNDEFINED || most_stubborn_defense.value < number_of_moves_user_wins_in)
            {
                most_stubborn_defense.value = number_of_moves_user_wins_in;
                most_stubborn_defense.square = current_move;
            }
        }

        if (most_stubborn_defense.square.row == UNDEFINED)
        {
            throw runtime_error("No possible moves?\n");
        }

        return most_stubborn_defense.square;
    }

    // Randomly pick a move with the same evaluation as the calling position object.

    vector <int> indices; // will store all the possible indices of future_positions vector.

    for (int i = 0; i < future_positions.size(); i++)
    {
        indices.push_back(i);
    }

    // Shuffle indices vector:

    auto rng = default_random_engine {};
    shuffle(begin(indices), end(indices), rng);

    for (int index: indices) // index is the current ELEMENT in indices, and acts as an INDEX for the future_positions vector.
    {
        if (future_positions[index]->evaluation >= evaluation)
        {
            return (future_positions[index]->last_move);
        }
    }

    // If control reached this point, the position has a situation with a row barrier in column D where the computer mathematically
    // derived someone had a forced win and stopped calculating.

    // First check if comp can win on the spot:
    for (const coordinate& current_move: possible_moves)
    {
        (*board)[index(current_move.row, current_move.col)] = 'C';

        coordinate old_last_move = last_move;

        last_move = current_move;

        bool does_move_win = did_someone_win();

        (*board)[index(current_move.row, current_move.col)] = ' ';

        last_move = old_last_move;

        if (does_move_win)
        {
            return current_move;
        }
    }

    // Now try to play on D, unless the square above lets either player win:

    for (const coordinate& current_move: possible_moves)
    {
        if (current_move.col == 3)
        {
            // (*board)[current_move.row][current_move.col] = 'C'; no need to place a piece here for checking the square above.

            (*board)[index(current_move.row-1, current_move.col)] = 'U';

            coordinate old_last_move = last_move;

            last_move = {current_move.row-1, current_move.col};

            bool did_opponent_have_a_square_above = did_someone_win();

            (*board)[index(current_move.row-1, current_move.col)] = 'C';

            bool did_computer_have_a_square_above = did_someone_win();

            (*board)[index(current_move.row-1, current_move.col)] = ' ';
            (*board)[index(current_move.row, current_move.col)] = ' '; // redundant now since already empty, but leaving in anyway.

            last_move = old_last_move;

            if (!did_opponent_have_a_square_above && !did_computer_have_a_square_above)
            {
                return current_move;
            }
        }
    }

    // Now "settle" for any move that doesn't move in D, since moving in D loses on the spot.
    for (const coordinate& current_move: possible_moves)
    {
        if (current_move.col != 3)
        {
            return current_move;
        }
    }

    // Now should be only 1 possible move left: D. It would have been nice to avoid this, but no other choice:

    if (possible_moves.size() == 1)
    {
        return possible_moves[0];
    }

    cout << "\n\n\nPROBLEM:\n\n\n";

    display_board();

    cout << "Possible moves: ";

    for (coordinate current_move: possible_moves)
    {
        cout << "(" << current_move.row << "," << current_move.col << "), ";
    }

    throw runtime_error("Error - Control reached the end of the function in position::find_best_move_for_comp()\n");
}

vector<treasure_spot> position::get_squares_amplifying_comp_2() const
{
    return *squares_amplifying_comp_2;
}

vector<treasure_spot> position::get_squares_amplifying_comp_3() const
{
    return *squares_amplifying_comp_3;
}

vector<treasure_spot> position::get_squares_amplifying_user_2() const
{
    return *squares_amplifying_user_2;
}

vector<treasure_spot> position::get_squares_amplifying_user_3() const
{
    return *squares_amplifying_user_3;
}

coordinate position::get_best_move_from_DB() const
{
    return best_move_from_DB;
}

// SETTERS:

void position::set_board(const vector<vector<char>>& boardP)
{
    for (int r = 0; r < 6; ++r) {
        for (int c = 0; c < 7; ++c) {
            (*board)[index(r, c)] = boardP[r][c];
        }
    }
}

void position::set_evaluation (int evalP)
{
    evaluation = evalP;
}

void position::set_is_comp_turn (bool turnP)
{
    is_comp_turn = turnP;
}

void position::set_depth(int depthP)
{
    depth = depthP;
}

void position::randomize_order_of_possible_moves()
{
    // I want to run through the first FOUR elements and swap them with a random element in the vector.

    for (int i = 0; i < possible_moves.size() && i < 4; i++)
    {
        int random_index = rand() % possible_moves.size();

        coordinate temp = possible_moves[random_index];

        possible_moves[random_index] = possible_moves[i];

        possible_moves[i] = temp;
    }
}

void position::clean_up_amplifying_vectors()
{
    remove_treasure_spot_objects_from_vector(squares_amplifying_comp_2);
    remove_treasure_spot_objects_from_vector(squares_amplifying_comp_3);
    remove_treasure_spot_objects_from_vector(squares_amplifying_user_2);
    remove_treasure_spot_objects_from_vector(squares_amplifying_user_3);
}

void position::rearrange_possible_moves(const vector<coordinate>& front_moves)
{
    int start_size = possible_moves.size();

    vector<coordinate> replacement = front_moves; // possible_moves will be set to this vector at the end of the function.

    for (const coordinate& temp: possible_moves)
    {
        if (!in_coordinate_vector(front_moves, temp)) // not in front_moves, so add to replacement:
        {
            replacement.push_back(temp);
        }
    }

    possible_moves = replacement;

    if (possible_move.size() != start_size)
    {
        throw runtime_error("possible_moves.size changes.\n");
    }
}

void position::add_position_to_transposition_table(bool is_evaluation_indisputable)
{
    if (stop_signal)
    {
        return;
    }

    position_info_for_TT temp;
    temp.board = *board; // TODO - continue here for changing board usage from a 2D vector to a string.
    temp.evaluation = evaluation;
    temp.calculation_depth_from_this_position = calculation_depth_from_this_position;
    temp.is_evaluation_indisputable = is_evaluation_indisputable;
    temp.is_comp_turn = is_comp_turn;

    // If the evaluation is indisputable, no position that uses this position in the TT needs to look at its possible moves.
    // That position should automatically just accept this position's evaluation, "no questions asked".

    // If the evaluation isn't indisputable, then any position that uses this position in the TT should get access to
    // possible moves ordered, allowing that position to efficiently conduct its own deeper search.

    if (!is_evaluation_indisputable)
    {
        sort(future_positions.begin(), future_positions.end(), compare_future_positions_by_evaluation);

        for (const unique_ptr<position>& pos: future_positions)
        {
            temp.possible_moves_sorted.push_back(pos->get_last_move());
        }
    }

    // else, temp's possible_moves_sorted vector is simply left empty.

    // Now to run through the appropriate index in the TT, and replace an earlier duplicate position of temp, if one exists.

    bool does_a_duplicate_exist = false;

    for (position_info_for_TT& current: transposition_table[hash_value_of_position]) // by reference is deliberate.
    {
        if (current.board == temp.board && current.is_comp_turn == temp.is_comp_turn)
        {
            does_a_duplicate_exist = true;

            if (temp.calculation_depth_from_this_position > current.calculation_depth_from_this_position)
            {
                current = temp;
            }

            break;
        }
    }

    if (!does_a_duplicate_exist)
    {
        // Since if a duplicate did exist, it would have either been replaced by temp, or temp would have had a lower calculation_depth
        // and wouldn't be "worthy" enough to replace it. Either way, temp should not be push_backed in that scenario.

        // On the other hand, if a duplicate doesn't exist, obviously temp should obviously be added to the TT.

        transposition_table[hash_value_of_position].push_back(temp);

        if (transposition_table[hash_value_of_position].size() == 1)
        {
            // So temp is the only element in the vector. Therefore, it was the first element to have been put there:

            indices_of_elements_in_TT.push_back(hash_value_of_position);
        }
    }
}

void position::set_best_move_from_DB(const coordinate& val)
{
    best_move_from_DB = val;
}

// HELPERS:

bool position::did_computer_win() const
{
    return !is_comp_turn && did_someone_win();
}

bool position::did_opponent_win() const
{
    return is_comp_turn && did_someone_win();
}

// Pre-condition: It has already been checked that no one has won the game.
// Post-condition: The function will return true if the board is full... it is not guaranteed no one has four-in-a-row.
bool position::is_game_drawn() const
{
    return number_of_pieces == 42; // since a 7x6 board will have 42 pieces when filled up completely.
}

bool position::evaluation_in_future_positions(int eval) const
{
    for (int i = 0; i < future_positions.size(); i++)
    {
        if (future_positions[i]->evaluation == eval)
        {
            return true;
        }
    }

    return false;
}

bool position::is_valid_move(string column) const
{
    // First, check if column is only 1 in size:

    if (column.size() != 1)
    {
        return false;
    }

    char letter = column[0]; // just easier to deal with chars.

    // Now check if letter is between 'A' - 'G' (or 'a' - 'g')

    if (!is_acceptable_letter(letter))
    {
        return false;
    }

    // Now to figure out what column is represented by letter:

    int col = 0;

    if (letter >= 'a') // lowercase...
    {
        col = letter - 'a';
    }

    else // uppercase...
    {
        col = letter - 'A';
    }

    // Now to check if the top spot of col in board is empty:

    return ((*board)[0][col] == ' ');
}

void position::remove_treasure_spot_objects_from_vector(shared_ptr<vector<treasure_spot>>& vec)
{
    shared_ptr<vector<treasure_spot>> updated_vec = make_shared<vector<treasure_spot>>();
    // will store the good elements of vec (the ones that shouldn't be removed from vec).

    for (const treasure_spot& element: (*vec))
    {
        if ((*board)[element.current_square.row][element.current_square.col] == ' ') // square is safe, since its location in board is empty (stores ' ').
        {
            updated_vec->push_back(move(element));
        }
    }

    vec = updated_vec;
}

void position::print_amplifying_vectors()
{
    for (const treasure_spot& temp: *squares_amplifying_comp_2)
    {
        coordinate square = temp.current_square;

        cout << char('A' + square.col) << (6 - square.row) << "\n";
    }

    for (const treasure_spot& temp: *squares_amplifying_comp_3)
    {
        coordinate square = temp.current_square;

        cout << char('A' + square.col) << (6 - square.row) << "\n";
    }

    for (const treasure_spot& temp: *squares_amplifying_user_2)
    {
        coordinate square = temp.current_square;

        cout << char('A' + square.col) << (6 - square.row) << "\n";
    }

    for (const treasure_spot& temp: *squares_amplifying_user_3)
    {
        coordinate square = temp.current_square;

        cout << char('A' + square.col) << (6 - square.row) << "\n";
    }
}

void position::find_critical_moves(vector<coordinate>& critical_moves)
{
    // If it is the comp's turn in this position, I'll want to first add any moves that win for the comp to the
    // critical_moves vector first. This is because the critical_moves will be put at the front of possible_moves vector,
    // and if the comp can win then I want it to examine it right away (to allow minimax to prune other moves immediately).

    if (is_comp_turn)
    {
        find_critical_moves_in_amplifying_vector(critical_moves, squares_amplifying_comp_2, false, 'C');
        find_critical_moves_in_amplifying_vector(critical_moves, squares_amplifying_comp_3, true, 'C');
        find_critical_moves_in_amplifying_vector(critical_moves, squares_amplifying_user_2, false, 'U');
        find_critical_moves_in_amplifying_vector(critical_moves, squares_amplifying_user_3, true, 'U');
    }

    else // user's turn, so look at moves that win for the user first...
    {
        find_critical_moves_in_amplifying_vector(critical_moves, squares_amplifying_user_2, false, 'U');
        find_critical_moves_in_amplifying_vector(critical_moves, squares_amplifying_user_3, true, 'U');
        find_critical_moves_in_amplifying_vector(critical_moves, squares_amplifying_comp_2, false, 'C');
        find_critical_moves_in_amplifying_vector(critical_moves, squares_amplifying_comp_3, true, 'C');
    }

    remove_duplicates(critical_moves);
}

void position::find_critical_moves_in_amplifying_vector(vector<coordinate>& critical_moves,
                                                        const shared_ptr<vector<treasure_spot>> amplifying_vector,
                                                        bool are_3_pieces, char piece)
{
    for (const treasure_spot& temp: *amplifying_vector)
    {
        if ((*board)[temp.current_square.row][temp.current_square.col] == ' ') // so the square is still a valid amplifying square...
        {
            // Now to see if the square can be filled in one move...

            if (temp.current_square.row == max_row_index || (*board)[temp.current_square.row + 1][temp.current_square.col] != ' ')
            {
                // Now to see if the square creates a 4-in-a-row, if filled...

                if (are_3_pieces || (is_in_bounds(temp.next_square) && (*board)[temp.next_square.row][temp.next_square.col] == piece) ||
                    (is_in_bounds(temp.other_next_square) && (*board)[temp.other_next_square.row][temp.other_next_square.col] == piece))
                {
                    // Since the square/move either amplifies a 3-in-a-row or connects a 2-in-a-row with a piece,
                    // add it to the critical_moves vector (since it creates a 4-in-a-row):

                    critical_moves.push_back(temp.current_square);
                }
            }
        }
    }
}

bool position::is_in_bounds(const coordinate& square) const
{
     return square.row >= 0 && square.row <= max_row_index && square.col >= 0 && square.col <= max_col_index;
}

bool position::in_coordinate_vector(const vector<coordinate>& vec, const coordinate& element)
{
     for (const coordinate& temp: vec)
     {
         if (temp == element)
         {
             return true;
         }
     }

     return false;
}

void position::remove_duplicates(vector<coordinate>& vec)
{
     vector<coordinate> replacement; // vec will be set equal to replacement at the end of this function.

     for (const coordinate& temp: vec)
     {
         if (!in_coordinate_vector(replacement, temp))
         {
             replacement.push_back(temp);
         }
     }

     vec = replacement;
}

void position::initialize_row_barriers(int& lowest_comp_square_D, int& lowest_good_comp_square_D,
                                       int& lowest_user_square_D, int& lowest_good_user_square_D)
{
    // Find all squares that give both comp AND user a 4-in-a-row. Could be in 2-in-a-row vectors too.

    vector<coordinate> squares_winning_for_comp;
    find_winning_squares(squares_winning_for_comp, squares_amplifying_comp_3, squares_amplifying_comp_2, 'C',
                         lowest_comp_square_D, lowest_good_comp_square_D, !did_comp_go_first_in_the_game);

    vector<coordinate> squares_winning_for_user;
    find_winning_squares(squares_winning_for_user, squares_amplifying_user_3, squares_amplifying_user_2, 'U',
                         lowest_user_square_D, lowest_good_user_square_D, did_comp_go_first_in_the_game);

    // Now parse through squares_winning_for_comp vector, and for each square check if it's also
    // in the squares_winning_for_user vector. If so, it's a "barricade" square.

    vector<coordinate> barricade_squares;

    for (const coordinate& temp: squares_winning_for_comp)
    {
        if (in_coordinate_vector(squares_winning_for_user, temp))
        {
            barricade_squares.push_back(temp);
        }
    }

    // Now, get the lowest barricade_squares (visually) for each column in barricade_squares vector,
    // extract their row index, and store these indices in the row_barriers private member (the whole point of this function).

    for (int i = 0; i <= max_col_index; i++) // since row_barries isn't even initialized yet. Give it 7 UNDEFINED ints.
    {
        row_barriers.push_back(UNDEFINED);
    }

    for (const coordinate& temp: barricade_squares)
    {
        // square temp's col value corresponds to an index in row_barriers:

        if (row_barriers[temp.col] == UNDEFINED || row_barriers[temp.col] < temp.row)
        {
            // Either that index in row_barriers has no value (undefined), or it stores a row
            // value that is higher up (visually) on the board than square temp's row value.
                // Since lower row values barricade more of a column, I should switch to that:

            row_barriers[temp.col] = temp.row;
        }
    }
}

void position::update_D_squares(int& lowest_square_D, int& lowest_good_square_D,
                                bool evens_are_good, const coordinate& candidate_square)
{
    // Pre-condition: It's already been checked that the candidate_square is in column D.

    if (candidate_square.col != 3)
    {
        throw runtime_error("candidate square not in column D, in update_D_squares function");
    }

    if (lowest_square_D == UNDEFINED || lowest_square_D < candidate_square.row)
    {
        lowest_square_D = candidate_square.row;
    }

    // Now to see if the square should be the new lowest_good_square_D:

    if ((candidate_square.row % 2 == 0) == evens_are_good)
    {
        if (lowest_good_square_D == UNDEFINED || lowest_good_square_D < candidate_square.row)
        {
            lowest_good_square_D = candidate_square.row;
        }
    }
}

void position::find_winning_squares(vector<coordinate>& vec, const shared_ptr<vector<treasure_spot>> squares_amplifying_3,
                                    const shared_ptr<vector<treasure_spot>> squares_amplifying_2, char piece,
                                    int& lowest_square_D, int& lowest_good_square_D, bool evens_are_good)
{
    for (const treasure_spot& temp: *squares_amplifying_3)
    {
        if ((*board)[temp.current_square.row][temp.current_square.col] == ' ')
        {
            vec.push_back(temp.current_square);

            if (temp.current_square.col == 3)
            {
                update_D_squares(lowest_square_D, lowest_good_square_D, evens_are_good, temp.current_square);
            }
        }
    }

    for (const treasure_spot& temp: *squares_amplifying_2)
    {
        if ((*board)[temp.current_square.row][temp.current_square.col] == ' ' &&
            ((is_in_bounds(temp.next_square) && (*board)[temp.next_square.row][temp.next_square.col] == piece) ||
             (is_in_bounds(temp.other_next_square) && (*board)[temp.other_next_square.row][temp.other_next_square.col] == piece)))
        {
            vec.push_back(temp.current_square);

            if (temp.current_square.col == 3)
            {
                update_D_squares(lowest_square_D, lowest_good_square_D, evens_are_good, temp.current_square);
            }
        }
    }

    remove_duplicates(vec);
}

bool position::can_create_threat_with_D(char piece, int highest_row) const
{
    // See if piece can create a threat in/using column D, anywhere at or below the highest_row.

    char opponent_piece = piece == 'U' ? 'C' : 'U';

/*    int row = 0;

    if (row_barriers[3] != UNDEFINED)
    {
        row = row_barriers[3] + 1;
    } */

    for (int row = highest_row; row < 6; row++)
    {
        if ((*board)[row][3] == opponent_piece)
        {
            continue;
        }

        else if (can_square_be_involved_in_horizontal_win(piece, opponent_piece, row, 3) ||
                 can_square_be_involved_in_upper_diagonal_win(piece, opponent_piece, row, 3) ||
                 can_square_be_involved_in_lower_diagonal_win(piece, opponent_piece, row, 3))
        {
            return true;
        }
    }

    return false;
}

// Pre-condition: square is somewhere in column D.
bool position::can_square_be_involved_in_horizontal_win(char piece, char opponent_piece, int row, int col) const
{
    // Don't need to bounds check since only traversing horizontally, and the square is in D.

    if ((*board)[row][col-1] == opponent_piece)
    {
        return ((*board)[row][col+1] != opponent_piece && (*board)[row][col+2] != opponent_piece && (*board)[row][col+3] != opponent_piece);
    }

    // [row][col-1] doesn't store opponent's piece, so it's a valid square.

    else if ((*board)[row][col+1] == opponent_piece)
    {
        return ((*board)[row][col-2] != opponent_piece && (*board)[row][col-3] != opponent_piece);
    }

    // [row][col+1] is also a valid square.

    else
    {
        return ((*board)[row][col+2] != opponent_piece || (*board)[row][col-2] != opponent_piece);
    }
}

// Pre-condition: square is somewhere in column D.
bool position::can_square_be_involved_in_upper_diagonal_win(char piece, char opponent_piece, int row, int col) const
{
    if (row + 1 > max_row_index || (*board)[row+1][col-1] == opponent_piece)
    {
        return (row - 3 >= 0 && (*board)[row-1][col+1] != opponent_piece &&
                (*board)[row-2][col+2] != opponent_piece && (*board)[row-3][col+3] != opponent_piece);
    }

    // [row+1][col-1] doesn't store opponent's piece, so it's a valid square.

    else if (row - 1 < 0 || (*board)[row-1][col+1] == opponent_piece)
    {
        return (row + 3 <= max_row_index && (*board)[row+2][col-2] != opponent_piece && (*board)[row+3][col-3] != opponent_piece);
    }

    // [row-1][col+1] is also a valid square.

    else
    {
        return ((row + 2 <= max_row_index && (*board)[row+2][col-2] != opponent_piece) ||
                (row - 2 >= 0 && (*board)[row-2][col+2] != opponent_piece));
    }
}

// Pre-condition: square is somewhere in column D.
bool position::can_square_be_involved_in_lower_diagonal_win(char piece, char opponent_piece, int row, int col) const
{
    if (row - 1 < 0 || (*board)[row-1][col-1] == opponent_piece)
    {
        return (row + 3 <= max_row_index && (*board)[row+1][col+1] != opponent_piece &&
                (*board)[row+2][col+2] != opponent_piece && (*board)[row+3][col+3] != opponent_piece);
    }

    // [row-1][col-1] doesn't store opponent's piece, so it's a valid square.

    else if (row + 1 > max_row_index || (*board)[row+1][col+1] == opponent_piece)
    {
        return (row - 3 >= 0 && (*board)[row-2][col-2] != opponent_piece && (*board)[row-3][col-3] != opponent_piece);
    }

    // [row+1][col+1] is also a valid square.

    else
    {
        return ((row + 2 <= max_row_index && (*board)[row+2][col+2] != opponent_piece) ||
                (row - 2 >= 0 && (*board)[row-2][col-2] != opponent_piece));
    }
}

// FUNCTIONS SPECIFICALLY FOR THE VERSUS SIM:

double position::get_static_thinking_time() const
{
    return position::thinking_time;
}

void position::set_static_thinking_time(double val)
{
    position::thinking_time = val;
}

unique_ptr<tool> position::call_static_think_on_game_position(const vector <vector<char>>& boardP, bool is_comp_turnP,
                                                              coordinate last_moveP, const vector<treasure_spot>& squares_amplifying_comp_2P,
                                                              const vector<treasure_spot>& squares_amplifying_comp_3P,
                                                              const vector<treasure_spot>& squares_amplifying_user_2P,
                                                              const vector<treasure_spot>& squares_amplifying_user_3P, bool starting_new_game)
{
    coordinate placeholder = {UNDEFINED, UNDEFINED};

    return move(think_on_game_position(boardP, is_comp_turnP, last_moveP, squares_amplifying_comp_2P, squares_amplifying_comp_3P,
                                       squares_amplifying_user_2P, squares_amplifying_user_3P, starting_new_game,
                                       placeholder, false));
}

unique_ptr<tool> position::call_static_think_on_game_position(bool is_comp_turnP, bool starting_new_game)
{
    coordinate placeholder = {UNDEFINED, UNDEFINED};

    return move(think_on_game_position(is_comp_turnP, starting_new_game, placeholder, false));
}

coordinate_and_value position::find_quick_winning_move(int max_number_moves_acceptable) const
{
    // Returns the coordinate for a move winning in <= max_number_moves_acceptable, and returns the number of moves it wins in
    // (this is why I'm returning a coordinate_and_value object).

    coordinate_and_value solution; // WILL BE RETURNED.
    solution.square = {UNDEFINED, UNDEFINED};
    solution.value = UNDEFINED;

    if (max_number_moves_acceptable < 1) // Cannot do anything in less than 1 move...
    {
        return solution; // deliberately still UNDEFINED
    }

    // Now do a preliminary pass over all possible_moves, just checking at depth 1. Purpose is to see if any win immediately,
    // which saves having to recursively delve into each of them in the rest of this function.

    coordinate immediate_win = return_a_move_that_wins_immediately(); // Function returns {UNDEFINED, UNDEFINED} if no immediate win.

    if (immediate_win.row != UNDEFINED) // An immediate win was found!
    {
        solution.square = immediate_win;
        solution.value = 1;

        return solution;
    }

    else if (max_number_moves_acceptable < 3)
    {
        // Due to the else if, immediate_win = {UNDEFINED, UNDEFINED}.
        // So there's no immediate win, yet max_number_of_moves_acceptable is less than 3. This only gives player 1 move to win.
        // Since I've verified this isn't the case, return solution, which is still UNDEFINED.

        return solution;
    }

    vector<vector<char>> assisting_board = *board;

    for (const coordinate& current_move: possible_moves)
    {
        char piece_to_be_played = 'C';

        if (!is_comp_turn)
        {
            piece_to_be_played = 'U';
        }

        assisting_board[current_move.row][current_move.col] = piece_to_be_played;

        if (piece_to_be_played == 'C')
        {
            piece_to_be_played = 'U';
        }

        else
        {
            piece_to_be_played = 'C';
        }

        unique_ptr<position> p1 = make_unique<position>(assisting_board, !is_comp_turn, current_move, empty_amplifying_vector,
                                                        empty_amplifying_vector, empty_amplifying_vector, empty_amplifying_vector);

        // NOTE: I know that player couldn't have won with this move, since I already checked if the player could win immediately
        // with any of the moves in possible_moves.

        // Check if p1 is in the TT. IF it is, make sure the TT says the position's evaluation is winning for the player.

        bool is_player_winning = true;

        for (const position_info_for_TT& current: transposition_table[p1->hash_value_of_position])
        {
            if (current.board == *(p1->board) && current.is_comp_turn == p1->is_comp_turn &&
                ((is_comp_turn && current.evaluation != INT_MAX) || (!is_comp_turn && current.evaluation != INT_MIN)))
            {
                is_player_winning = false;

                break;
            }
        }

        if (!is_player_winning)
        {
            // current_move is a failure, so continue to the next move in for loop:

            assisting_board[current_move.row][current_move.col] = ' ';

            continue;
        }

        // Now run through each of the opponent's responses, and see if player can win against all of them within the bound of moves.
        // This will involve recursion.

        int players_worst_performance_against_all_opponent_responses = 1;

        bool can_opponent_win = false; // set to true if the opponent has a move that wins.

        for (const coordinate& opponent_response: p1->possible_moves)
        {
            assisting_board[opponent_response.row][opponent_response.col] = piece_to_be_played;

            unique_ptr<position> p2 = make_unique<position>(assisting_board, !p1->is_comp_turn, opponent_response, empty_amplifying_vector,
                                                            empty_amplifying_vector, empty_amplifying_vector, empty_amplifying_vector);

            // Now, check if the opponent is winning:

            if ((is_comp_turn && p2->evaluation == INT_MIN) || (!is_comp_turn && p2->evaluation == INT_MAX))
            {
                can_opponent_win = true;

                assisting_board[opponent_response.row][opponent_response.col] = ' ';

                break; // No need to consider other moves for the opponent, as there is one move that wins for the opponent.
                       // current_move is a failure.
            }

            int number_of_moves_player_can_win_from_here = p2->find_quick_winning_move(max_number_moves_acceptable - 2).value;

            if (number_of_moves_player_can_win_from_here == UNDEFINED ||
                number_of_moves_player_can_win_from_here > players_worst_performance_against_all_opponent_responses)
            {
                players_worst_performance_against_all_opponent_responses = number_of_moves_player_can_win_from_here;
            }

            assisting_board[opponent_response.row][opponent_response.col] = ' ';

            if (players_worst_performance_against_all_opponent_responses == UNDEFINED)
            {
                break; // current_move is a failure.
            }
        }

        if (!can_opponent_win && players_worst_performance_against_all_opponent_responses != UNDEFINED)
        {
            if (solution.value == UNDEFINED || players_worst_performance_against_all_opponent_responses + 2 < solution.value)
            {
                solution.value = players_worst_performance_against_all_opponent_responses + 2;
                solution.square = current_move;
            }
        }

        assisting_board[current_move.row][current_move.col] = ' ';
    }

    return solution;
}

coordinate position::return_a_move_that_wins_immediately() const
{
    // NOTE: if such a move doesn't exist, return {UNDEFINED, UNDEFINED}.

    vector<vector<char>> assisting_board = *board;

    char piece = 'C';

    if (!is_comp_turn)
    {
        piece = 'U';
    }

    for (const coordinate& current_move: possible_moves)
    {
        assisting_board[current_move.row][current_move.col] = piece;

        unique_ptr<position> pt = make_unique<position>(assisting_board, !is_comp_turn, current_move, empty_amplifying_vector,
                                                        empty_amplifying_vector, empty_amplifying_vector, empty_amplifying_vector);

        if ((is_comp_turn && pt->did_computer_win()) || (!is_comp_turn && pt->did_opponent_win()))
        {
            // pt->did_opponent_win() really means "pt->did_user_win()"

            return current_move;
        }

        assisting_board[current_move.row][current_move.col] = ' ';
    }

    return {UNDEFINED, UNDEFINED};
}

// PUBLIC STATIC METHODS:

vector<vector<int>> position::get_board_of_random_ints()
{
    https://stackoverflow.com/questions/13445688/how-to-generate-a-random-number-in-c
    // Using the method described in the top answer for generating random numbers, as opposed to something like rand().

    random_device dev;
    mt19937 rng(dev());
    uniform_int_distribution<mt19937::result_type> dist(1, max_hash_value);

    vector<vector<int>> vec(6, vector<int>(7));
    for (int r = 0; r < 6; ++r) {
        for (int c = 0; c < 7; ++c) {
            vec[r][c] = dist(rng);
        }
    }
    return vec;
}

void position::reset_transposition_table()
{
    for (int index: indices_of_elements_in_TT)
    {
        // Delete all the elements in the inner vector of the TT at index.

        transposition_table[index].clear();
    }

    indices_of_elements_in_TT.clear();
}

bool position::compare_future_positions_by_evaluation(const unique_ptr<position>& first_pos, const unique_ptr<position>& second_pos)
{
    if (first_pos->is_comp_turn)
    {
        // User's turn in the calling position/object (since its the comp's turn one move later), so the user is the player here.

        // The user would like to look at positions with a smaller evaluation:

        return (first_pos->evaluation <= second_pos->evaluation);
    }

    else
    {
        // Comp's turn in the calling position/object, so the comp is the player here.

        // The comp would like to look at positions with a greater evaluation:

        return (first_pos->evaluation >= second_pos->evaluation);
    }
}

position_info_for_TT position::find_duplicate_in_TT(const unique_ptr<position>& pt)
{
    for (const position_info_for_TT& current: transposition_table[pt->hash_value_of_position])
    {
        if (current.board == *(pt->board) && current.is_comp_turn == pt->is_comp_turn)
        {
            return current;
        }
    }

    // At this point, a runtime error could be thrown (this was done in versions
    // prior to V.48). However, this might not be the best course of action.
    // I'm not sure why there isn't a duplicate in the TT, but just because
    // there isn't doesn't mean the engine is broken by any means.

    // This function is only called in the while loop in think, where it
    // checks if a previous duplicate (from iterative deepening) says that
    // this position's evaluation is indisputable, and thus there is
    // no point in continuing to think.

    // Since a duplicate doesn't exist, I'll make it so that it's "false"
    // that the evaluation is indisputable.

    // To do this, I'll create a dummy object with just this attribute set.

    position_info_for_TT temp;

    temp.is_evaluation_indisputable = false;

    return temp;

    // throw runtime_error("Did not find a duplicate in find_duplicate_in_TT()\n");
}

coordinate position::find_legal_square(const vector<vector<char>>& boardP, int col)
{
    for (int r = 5; r >= 0; r--)
    {
        if (boardP[r][col] == ' ')
        {
            coordinate result = {r, col};

            return result;
        }
    }

    throw runtime_error("The column is full\n");
}

unique_ptr<position> position::think_on_game_position(const vector<vector<char>>& boardP, bool is_comp_turnP, coordinate last_moveP,
                                    const vector<treasure_spot>& squares_amplifying_comp_2P, const vector<treasure_spot>& squares_amplifying_comp_3P,
                                    const vector<treasure_spot>& squares_amplifying_user_2P, const vector<treasure_spot>& squares_amplifying_user_3P,
                                    bool starting_new_game, coordinate& best_move, bool generate_best_move)
{
    // TODO - Make a string board, and make it represent the board that boardP does.
    // Then use this string board in this function, instead of boardP.

    const int max_depth_limit = UNDEFINED;
    // If this engine is doing a time_limited think, set the value of
    // this variable to UNDEFINED.

    // Otherwise, if this engine is doing a depth_limited think, set
    // the value of this variable to the max depth limit you want
    // for iterative deepening.

    // This max_depth_limit variable is the only variable you have to
    // set in the think function for the Versus Sim. Other than this,
    // just get rid of the auto rand thing too.

    if (starting_new_game)
    {
        reset_transposition_table();
        random_values_for_squares_with_C = get_board_of_random_ints();
        random_values_for_squares_with_U = get_board_of_random_ints();
        surpassed_DB = false;
    }

    const bool is_this_the_starting_position = boardP == create_starting_board_string();

    // this variable says whether to call constructor 1/2.

    unique_ptr<position> pt;

    steady_clock::time_point start_time = steady_clock::now();

    if (is_this_the_starting_position)
    {
        if (!(last_moveP.row == UNDEFINED && last_moveP.col == UNDEFINED &&
              squares_amplifying_comp_2P.empty() &&
              squares_amplifying_comp_3P.empty() &&
              squares_amplifying_user_2P.empty() &&
              squares_amplifying_user_3P.empty()))
         {
             throw runtime_error("Incorrect data for the starting position.\n");
         }

        pt = make_unique<position>(is_comp_turnP);
    }

    else
    {
        pt = make_unique<position>(boardP, is_comp_turnP, last_moveP,
                                   squares_amplifying_comp_2P,
                                   squares_amplifying_comp_3P,
                                   squares_amplifying_user_2P,
                                   squares_amplifying_user_3P);
    }

    duration<double> time_span = duration_cast<duration<double>>(steady_clock::now() - start_time);

    if (max_depth_limit != UNDEFINED && thinking_time < 0.000001)
    {
        // The engine does depth_limited_thinks, and the thinking_time is 0.
        // This means the function should return now, since I only called it
        // to update the amplifying vectors.

        // No need to use the DB, as finding a good move wasn't the
        // intention of calling this function right now.

        depth_limit = 1;

        return move(pt);
    }

    const int ply_of_root_node = pt->number_of_pieces;

    int final_depth_limit_used = depth_limit;
    // Will be incremented in the while loop.

    // The first condition of the below while loop, a large compound statement, checks
    // whether the depth_limit/time_limit has been exceeded, depending on whether
    // max_depth_limit == UNDEFINED (since this says whether the engine thinks
    // according to a depth_limit or time_limit).

    // For the last part of the sub-condition where max_depth_limit != UNDEFINED
    // (i.e., this is a depth_limit search), the disjunction is equivalent to
    // stating the following implication must hold (using p v q <==> !p --> q):

    // "If the root node is in the DB, then the max ply that can be reached
    //  in the search must exceed 18".

    // If this implication is met, then either:
        // 1) The root node is not in the DB.
        // 2) Or, the root node is in the DB, but the max ply reached in the search
        //    will exceed 18.

        // In either of these cases, I want the loop to run. In the first case, it
        // it is the only option; in the second case the DB is an option, but the
        // while loop should be the better option since it will exceed 18 ply.

    // However, if the implication is not met, then:
        // The root node is in the DB, but the max ply of the search will
        // be 18 or less.

        // In this case, the DB is an option. And, the search won't exceed 18
        // so the DB is the better option (or at least as good). Therefore,
        // do not run the while loop and just use the DB immediately.

        // While not executing the while loop  in this scenario saves time, the
        // main reason is to actually make the engine stronger, by not adding
        // to the TT (not sure why this helps).

    while (((max_depth_limit == UNDEFINED && time_span.count() < thinking_time) ||
            (max_depth_limit != UNDEFINED && depth_limit < max_depth_limit &&
             (ply_of_root_node > 9 || ply_of_root_node + max_depth_limit > 18)))
           && !find_duplicate_in_TT(pt).is_evaluation_indisputable
           && pt->number_of_pieces + depth_limit <= 43
           && !stop_signal)
    {
        depth_limit ++; // Iterative deepening.

        if (is_this_the_starting_position)
        {
            pt = make_unique<position>(is_comp_turnP);
        }

        else
        {
            pt = make_unique<position>(boardP, is_comp_turnP, last_moveP,
                                       squares_amplifying_comp_2P,
                                       squares_amplifying_comp_3P,
                                       squares_amplifying_user_2P,
                                       squares_amplifying_user_3P);
        }

        time_span = duration_cast<duration<double>>(steady_clock::now() - start_time);

        if (generate_best_move && time_span.count() >= 3.0 && !stop_signal)
        {
            int old_value_of_depth_limit = depth_limit;

            depth_limit = 1;

            best_move = pt->find_best_move_for_comp();

            depth_limit = old_value_of_depth_limit;
        }

        if (!stop_signal)
        {
            final_depth_limit_used = depth_limit;
        }

        // See if I should break from this while loop early. This depends on
        // whether the calculations here have a reasonable
        // chance of surpassing what the database has.

        if (ply_of_root_node <= 9 && depth_limit >= 3)
        {
            // ply_of_root_node <= 9 means this position is in the database.

            // depth_limit >= 3 is the minimum I want to engine to search,
            // before potentially giving up and just using the DB.

            // doesn't matter how much it searches in the while loop though,
            // if the DB is going to be used instead.

            if (ply_of_root_node <= 5)
            {
                break;
            }

            // Removed the if statement testing whether to break if
            // max_depth_limit != UNDEFINED (i.e., depth_limit engine),
            // since this is already decided in the while loop header before
            // running any iterations.

            if (max_depth_limit == UNDEFINED &&
                time_span.count() * 3.0 >= thinking_time &&
                ply_of_root_node + depth_limit <= 16)
            {
                // The engine thinks according to a time_limit,
                // and at least a third of the thinking_time has been used,
                // and the ply reached is still at least 3 under
                // what the database achieved (18).

                break;
            }
        }
    }

    if (ply_of_root_node <= 9 && final_depth_limit_used + ply_of_root_node <= 18)
    {
        // So, use the database. Also, reset the TT because the think didn't
        // do better than the DB. The exception is if the DB was surpassed in the
        // past - in this case don't reset the TT, since this think here
        // is likely due to a generate_best_move think being cut short.

        if (!surpassed_DB)
        {
            reset_transposition_table();
        }

        int db_evaluation;

        int col_of_best_move;

        Database_Functions::get_info_for_position(boardP, is_comp_turnP,
                                                  db_evaluation, col_of_best_move);

        if (col_of_best_move != UNDEFINED)
        {
            pt->set_best_move_from_DB(find_legal_square(boardP, col_of_best_move));
        }

        else if (!pt->did_someone_win())
        {
            // There is no best move for this position in the DB, which
            // would be fine if someone just won (then best_move_from_DB
            // just remains UNDEFINED). But here it appears someone didn't win.

            throw runtime_error("Undefined best_move, in an ongoing position\n");
        }

        pt->set_evaluation(db_evaluation);

        if (generate_best_move)
        {
            // So, called this think function to find a response to the predicted
            // user move. However, the think done was beaten by the DB, since the
            // think didn't reach 18 ply.

            if (stop_signal)
            {
                // The stop_signal was pulled, which could possibly be why
                // the DB beat the engine here. So let the engine think on
                // its actual turn coming up, and then decide whether
                // to use that think, or the DB.

                best_move = {UNDEFINED, UNDEFINED};
            }

            else
            {
                // The stop signal wasn't pulled, which means the DB beat the
                // engine in this think "fairly". The engine's think in the actual
                // game will be at least worse, so the DB will win again there.
                // Therefore, use the DB for the best_move:

                best_move = pt->get_best_move_from_DB();
            }
        }
    }

    else
    {
        surpassed_DB = true;
    }

    depth_limit = 1; // in preparation for the next time the engine thinks.

    return move(pt);
}

unique_ptr<position> position::think_on_game_position(bool is_comp_turnP, bool starting_new_game,
                                                      coordinate& best_move, bool generate_best_move)
{
    coordinate undefined_move = {UNDEFINED, UNDEFINED};

    vector<treasure_spot> empty_vector;

    return (move(think_on_game_position(create_starting_board_2D_vector(), is_comp_turnP,
                                        undefined_move, empty_vector, empty_vector,
                                        empty_vector, empty_vector, starting_new_game,
                                        best_move, generate_best_move)));
}

vector<vector<char>> position::create_starting_board_2D_vector()
{
    vector<vector<char>> result;

    vector<char> row;

    for (int c = 0; c < 7; c++)
    {
        row.push_back(' ');
    }

    for (int r = 0; r < 6; r++)
    {
        result.push_back(row);
    }

    return result;
}

string position::create_starting_board_string()
{
    string result(42, ' ');
    return result;
}

void position::read_amplifying_squares_into_pointer(shared_ptr<vector<treasure_spot>>& the_pointer, const vector<treasure_spot>& amplifying_vectorP)
{
    for (const treasure_spot& current_spot: amplifying_vectorP)
    {
        the_pointer->push_back(current_spot);
    }
}

vector<vector<bool>> position::create_board_of_bools()
{
    vector<vector<bool>> board_of_bools;

    vector<bool> row;

    for (int i = 0; i <= max_col_index; i++)
    {
        row.push_back(false);
    }

    for (int i = 0; i <= max_row_index; i++)
    {
        board_of_bools.push_back(row);
    }

    return board_of_bools;
}

void position::reset_board_of_bools(vector<vector<bool>>& board_of_bools, const vector<coordinate_and_double>& squares)
{
    for (const coordinate_and_double& current: squares)
    {
        board_of_bools[current.square.row][current.square.col] = false;
    }
}

inline int position::index(int row, int col) {
    return col * 6 + row;
}

// PRIVATE METHODS:

void position::analyze_last_move()
{
    // First, check if this position has already been analyzed, and has an evaluation in the transposition table.

    // If so, accept this evaluation if one of two conditions are met:
        // 1) The evaluation is indisputable (i.e., forced).
        // 2) The duplicate position in the hash table has a >= "calculation_depth_from_this_position" than the calling object.

    for (const position_info_for_TT& current: transposition_table[hash_value_of_position])
    {
        if (current.board == *board && current.is_comp_turn == is_comp_turn &&
            (current.is_evaluation_indisputable || current.calculation_depth_from_this_position >= calculation_depth_from_this_position))
        {
            evaluation = current.evaluation;

          //  counter_of_TT_usefulness ++;

            return; // All done for this position entirely!
        }
    }

    // See how many pieces are in a row horizontally due to last_move:

    analyze_horizontal_perspective_of_last_move(); // sets evaluation to INT_MAX/INT_MIN if someone won,
                                                   // and updates one of the amplifying vectors if appropriate.

    if (evaluation == INT_MAX || evaluation == INT_MIN) // someone won...
    {
        add_position_to_transposition_table(true);

        return;
    }

    analyze_vertical_perspective_of_last_move(); // sets evaluation to INT_MAX/INT_MIN if someone won,
                                                 // and updates one of the amplifying vectors if appropriate.

    if (evaluation == INT_MAX || evaluation == INT_MIN) // someone won...
    {
        add_position_to_transposition_table(true);

        return;
    }

    analyze_positive_slope_diagonal_perspective_of_last_move(); // sets evaluation to INT_MAX/INT_MIN if someone won,
                                                                // and updates one of the amplifying vectors if appropriate.

    if (evaluation == INT_MAX || evaluation == INT_MIN) // someone won...
    {
        add_position_to_transposition_table(true);

        return;
    }

    analyze_negative_slope_diagonal_perspective_of_last_move(); // sets evaluation to INT_MAX/INT_MIN if someone won,
                                                                // and updates one of the amplifying vectors if appropriate.

    if (evaluation == INT_MAX || evaluation == INT_MIN) // someone won...
    {
        add_position_to_transposition_table(true);

        return;
    }

    // Before calling minimax(), check if the game is drawn,
    // or if the depth_limit is reached (meaning the computer should evaluate the position now if it is in a quiescent state):

    if (number_of_pieces == 42)
    {
        evaluation = 0;

        add_position_to_transposition_table(true);

        return;
    }

    vector<coordinate> critical_moves; // stores moves (for either side) that make a 4-in-a-row, that can be played now.

    find_critical_moves(critical_moves); // passed by reference.

    if (depth >= depth_limit && critical_moves.size() == 0) // Quiescent state reached at depth_limit (or beyond).
    {
        // So, smart_evaluation() is ready to evaluate the position:

        smart_evaluation(); // gives the evaluation attribute a value.

        if (evaluation == INT_MAX || evaluation == INT_MIN)
        {
            add_position_to_transposition_table(true);
        }

        else
        {
            add_position_to_transposition_table(false);
        }

        return;
    }

    // Now see if there's an earlier duplicate of position in the TT, that has a non-empty possible moves vector.
    // If so, set this calling object's possible_moves vector to equal it.
    // Note that this is where nearly all the speed of the TT comes to fruition!

    bool found_earlier_duplicate_in_TT = false;

    for (const position_info_for_TT& current: transposition_table[hash_value_of_position])
    {
        if (current.board == *board && current.is_comp_turn == is_comp_turn && !current.possible_moves_sorted.empty())
        {
            possible_moves = current.possible_moves_sorted;

            found_earlier_duplicate_in_TT = true;

            break;
        }
    }

    if (!found_earlier_duplicate_in_TT)
    {
        rearrange_possible_moves(critical_moves); // Function puts the critical_moves in possible_moves at the front
                                                  // of possible_moves.
    }

    minimax();
}

void position::analyze_horizontal_perspective_of_last_move()
{
    coordinate start_point = find_starting_horizontal_point();
    coordinate end_point = find_ending_horizontal_point();
    int num_pieces_in_a_row = end_point.col - start_point.col + 1;

    if (num_pieces_in_a_row == 1) // "1-in-a-row"... Version 20 values it!
    {
        char piece = (*board)[last_move.row][last_move.col];

        // By definition, there are empty squares on either side of last_move, or out-of-bounds.
            // I don't need to check if such a square is empty, due to num_pieces_in_a_row = 1.
            // I don't need to explicitly check if such a square is in-bounds, if I check the square two over from last_move is in-bounds.

        // See if the square to the left amplifies a "2-in-a-row":

        if (last_move.col - 2 >= 0 && (*board)[last_move.row][last_move.col - 2] == piece)
        {
            treasure_spot temp;

            temp.current_square = last_move;
            temp.current_square.col--;

            temp.next_square = temp.current_square;
            temp.next_square.col -= 2;

            temp.other_next_square = temp.current_square;
            temp.other_next_square.col += 2;

            add_to_appropriate_amplifying_vector(2, temp);
        }

        else if (last_move.col - 3 >= 0 && (*board)[last_move.row][last_move.col - 3] == piece)
        {
            // So we have a situation like this:

            //    | |X| | |X| |

            // Treating a square in the middle as a square amplifying a 2-in-a-row square is the idea of V.41.

            // However, here there's only one next_square (i.e., other square in the middle), not another.

            treasure_spot temp;

            temp.current_square = last_move;
            temp.current_square.col--;

            temp.next_square = temp.current_square;
            temp.next_square.col--;

            temp.other_next_square.row = UNDEFINED;
            temp.other_next_square.col = UNDEFINED;

            temp.is_weak_2_square = true;

            add_to_appropriate_amplifying_vector(2, temp);
        }

        if (last_move.col + 2 <= max_col_index && (*board)[last_move.row][last_move.col + 2] == piece)
        {
            treasure_spot temp;

            temp.current_square = last_move;
            temp.current_square.col++;

            temp.next_square = temp.current_square;
            temp.next_square.col += 2;

            temp.other_next_square = temp.current_square;
            temp.other_next_square.col -= 2;

            add_to_appropriate_amplifying_vector(2, temp);
        }

        else if (last_move.col + 3 <= max_col_index && (*board)[last_move.row][last_move.col + 3] == piece)
        {
            treasure_spot temp;

            temp.current_square = last_move;
            temp.current_square.col++;

            temp.next_square = temp.current_square;
            temp.next_square.col++;

            temp.other_next_square.row = UNDEFINED;
            temp.other_next_square.col = UNDEFINED;

            temp.is_weak_2_square = true;

            add_to_appropriate_amplifying_vector(2, temp);
        }

        return;
    }

    if (num_pieces_in_a_row >= 4)
    {
        if (is_comp_turn) // comp's turn now, so user just moved and got 4-in-a-row:
        {
            evaluation = INT_MIN;
            return;
        }

        evaluation = INT_MAX;
        return;
    }

    // num_pieces_in_a_row must be 2 or 3. This means I may want to add to one of the amplifying vectors.
    // Create a treasure_spot object to possibly do this:

    treasure_spot preceding_point; // space to the left of start_point on board.

    preceding_point.current_square = start_point;
    preceding_point.current_square.col--;

    preceding_point.next_square = preceding_point.current_square;
    preceding_point.next_square.col--;

    treasure_spot succeeding_point; // space to the right of end_point on board.

    succeeding_point.current_square = end_point;
    succeeding_point.current_square.col++;

    succeeding_point.next_square = succeeding_point.current_square;
    succeeding_point.next_square.col++;

    // Version 19 additions:

    preceding_point.other_next_square = succeeding_point.current_square;
    succeeding_point.other_next_square = preceding_point.current_square;

    if (preceding_point.current_square.col >= 0 &&
        (*board)[preceding_point.current_square.row][preceding_point.current_square.col] == ' ')
    {
        add_to_appropriate_amplifying_vector(num_pieces_in_a_row, preceding_point);
    }

    if (succeeding_point.current_square.col <= max_col_index &&
        (*board)[succeeding_point.current_square.row][succeeding_point.current_square.col] == ' ')
    {
        add_to_appropriate_amplifying_vector(num_pieces_in_a_row, succeeding_point);
    }
}

void position::analyze_vertical_perspective_of_last_move()
{
    //coordinate start_point = find_starting_vertical_point();
    coordinate start_point = last_move;
    coordinate end_point = find_ending_vertical_point();
    int num_pieces_in_a_row = end_point.row - start_point.row + 1;

    if (num_pieces_in_a_row == 1) // "1-in-a-row"... adds no value vertically, and not worth recording:
    {
        return;
    }

    if (num_pieces_in_a_row >= 4)
    {
        if (is_comp_turn) // comp's turn now, so user just moved and got 4-in-a-row:
        {
            evaluation = INT_MIN;
            return;
        }

        evaluation = INT_MAX;
        return;
    }

    // num_pieces_in_a_row must be 2 or 3. This means I may want to add to one of the amplifying vectors:
    // Create a treasure_spot object to possibly do this:

    treasure_spot preceding_point; // space above start_point on board.

    preceding_point.current_square = start_point;
    preceding_point.current_square.row--;

    preceding_point.next_square = preceding_point.current_square;
    preceding_point.next_square.row--;

    // Version 19 addition:

    preceding_point.other_next_square.row = UNDEFINED;
    preceding_point.other_next_square.col = UNDEFINED;
    // Since vertically, there is no other next square, only the one above. Below is filled with pieces.

    if (preceding_point.current_square.row >= 0) // don't need to also check if the square is empty since it's guaranteed to be.
    {
        add_to_appropriate_amplifying_vector(num_pieces_in_a_row, preceding_point);
    }

    // Note that I only need to check above the start_point. Checking below the end_point is pointless since the
    // column is filled with pieces under last_move (there are no empty squares).
}

void position::analyze_positive_slope_diagonal_perspective_of_last_move()
{
    coordinate start_point = find_starting_positive_slope_diagonal_point();
    coordinate end_point = find_ending_positive_slope_diagonal_point();
    int num_pieces_in_a_row = end_point.col - start_point.col + 1; // could also have used start_point.row - end_point.row + 1.

    if (num_pieces_in_a_row == 1) // "1-in-a-row"... Version 20 values it!
    {
        char piece = (*board)[last_move.row][last_move.col];

        // By definition, there are empty squares on either side of last_move, or out-of-bounds.
            // I don't need to check if such a square is empty, due to num_pieces_in_a_row = 1.
            // I don't need to explicitly check if such a square is in-bounds, if I check the square two over from last_move is in-bounds.

        // See if the square to the down-left amplifies a "2-in-a-row":

        if (last_move.col - 2 >= 0 && last_move.row + 2 <= max_row_index && (*board)[last_move.row + 2][last_move.col - 2] == piece)
        {
            treasure_spot temp;

            temp.current_square = last_move;
            temp.current_square.col--;
            temp.current_square.row++;

            temp.next_square = temp.current_square;
            temp.next_square.col -= 2;
            temp.next_square.row += 2;

            temp.other_next_square = temp.current_square;
            temp.other_next_square.col += 2;
            temp.other_next_square.row -= 2;

            add_to_appropriate_amplifying_vector(2, temp);
        }

        else if (last_move.col - 3 >= 0 && last_move.row + 3 <= max_row_index &&
                 (*board)[last_move.row + 3][last_move.col - 3] == piece)
        {
            treasure_spot temp;

            temp.current_square = last_move;
            temp.current_square.col--;
            temp.current_square.row++;

            temp.next_square = temp.current_square;
            temp.next_square.col--;
            temp.next_square.row++;

            temp.other_next_square.row = UNDEFINED;
            temp.other_next_square.col = UNDEFINED;

            temp.is_weak_2_square = true;

            add_to_appropriate_amplifying_vector(2, temp);
        }

        // See if the square to the up-right amplifies a "2-in-a-row":

        if (last_move.col + 2 <= max_col_index && last_move.row - 2 >= 0 && (*board)[last_move.row - 2][last_move.col + 2] == piece)
        {
            treasure_spot temp;

            temp.current_square = last_move;
            temp.current_square.col++;
            temp.current_square.row--;

            temp.next_square = temp.current_square;
            temp.next_square.col += 2;
            temp.next_square.row -= 2;

            temp.other_next_square = temp.current_square;
            temp.other_next_square.col -= 2;
            temp.other_next_square.row += 2;

            add_to_appropriate_amplifying_vector(2, temp);
        }

        else if (last_move.col + 3 <= max_col_index && last_move.row - 3 >= 0 &&
                 (*board)[last_move.row - 3][last_move.col + 3] == piece)
        {
            treasure_spot temp;

            temp.current_square = last_move;
            temp.current_square.col++;
            temp.current_square.row--;

            temp.next_square = temp.current_square;
            temp.next_square.col++;
            temp.next_square.row--;

            temp.other_next_square.row = UNDEFINED;
            temp.other_next_square.col = UNDEFINED;

            temp.is_weak_2_square = true;

            add_to_appropriate_amplifying_vector(2, temp);
        }

        return;
    }

    if (num_pieces_in_a_row >= 4)
    {
        if (is_comp_turn) // comp's turn now, so user just moved and got 4-in-a-row:
        {
            evaluation = INT_MIN;
            return;
        }

        evaluation = INT_MAX;
        return;
    }

    // num_pieces_in_a_row must be 2 or 3. This means I may want to add to one of the amplifying vectors:
    // Create a treasure_spot object to possibly do this:

    treasure_spot preceding_point; // space to the down-left of start_point on board.

    preceding_point.current_square = start_point;
    preceding_point.current_square.col--;
    preceding_point.current_square.row++;

    preceding_point.next_square = preceding_point.current_square;
    preceding_point.next_square.col--;
    preceding_point.next_square.row++;

    treasure_spot succeeding_point; // space to the up-right of end_point on board.

    succeeding_point.current_square = end_point;
    succeeding_point.current_square.col++;
    succeeding_point.current_square.row--;

    succeeding_point.next_square = succeeding_point.current_square;
    succeeding_point.next_square.col++;
    succeeding_point.next_square.row--;

    // Version 19 additions:

    preceding_point.other_next_square = succeeding_point.current_square;
    succeeding_point.other_next_square = preceding_point.current_square;

    if (preceding_point.current_square.col >= 0 &&
        preceding_point.current_square.row <= max_row_index &&
        (*board)[preceding_point.current_square.row][preceding_point.current_square.col] == ' ')
    {
        add_to_appropriate_amplifying_vector(num_pieces_in_a_row, preceding_point);
    }

    if (succeeding_point.current_square.col <= max_col_index &&
        succeeding_point.current_square.row >= 0 &&
        (*board)[succeeding_point.current_square.row][succeeding_point.current_square.col] == ' ')
    {
        add_to_appropriate_amplifying_vector(num_pieces_in_a_row, succeeding_point);
    }
}

void position::analyze_negative_slope_diagonal_perspective_of_last_move()
{
    coordinate start_point = find_starting_negative_slope_diagonal_point();
    coordinate end_point = find_ending_negative_slope_diagonal_point();
    int num_pieces_in_a_row = end_point.col - start_point.col + 1; // could also have used end_point.row - start_point.row + 1.

    if (num_pieces_in_a_row == 1) // "1-in-a-row"... Version 20 values it!
    {
        char piece = (*board)[last_move.row][last_move.col];

        // By definition, there are empty squares on either side of last_move, or out-of-bounds.
            // I don't need to check if such a square is empty, due to num_pieces_in_a_row = 1.
            // I don't need to explicitly check if such a square is in-bounds, if I check the square two over from last_move is in-bounds.

        // See if the square to the up-left amplifies a "2-in-a-row":

        if (last_move.col - 2 >= 0 && last_move.row - 2 >= 0 && (*board)[last_move.row - 2][last_move.col - 2] == piece)
        {
            treasure_spot temp;

            temp.current_square = last_move;
            temp.current_square.col--;
            temp.current_square.row--;

            temp.next_square = temp.current_square;
            temp.next_square.col -= 2;
            temp.next_square.row -= 2;

            temp.other_next_square = temp.current_square;
            temp.other_next_square.col += 2;
            temp.other_next_square.row += 2;

            add_to_appropriate_amplifying_vector(2, temp);
        }

        else if (last_move.col - 3 >= 0 && last_move.row - 3 >= 0 &&
                 (*board)[last_move.row - 3][last_move.col - 3] == piece)
        {
            treasure_spot temp;

            temp.current_square = last_move;
            temp.current_square.col--;
            temp.current_square.row--;

            temp.next_square = temp.current_square;
            temp.next_square.col--;
            temp.next_square.row--;

            temp.other_next_square.row = UNDEFINED;
            temp.other_next_square.col = UNDEFINED;

            temp.is_weak_2_square = true;

            add_to_appropriate_amplifying_vector(2, temp);
        }

        // See if the square to the down-right amplifies a "2-in-a-row":

        if (last_move.col + 2 <= max_col_index && last_move.row + 2 <= max_row_index && (*board)[last_move.row + 2][last_move.col + 2] == piece)
        {
            treasure_spot temp;

            temp.current_square = last_move;
            temp.current_square.col++;
            temp.current_square.row++;

            temp.next_square = temp.current_square;
            temp.next_square.col += 2;
            temp.next_square.row += 2;

            temp.other_next_square = temp.current_square;
            temp.other_next_square.col -= 2;
            temp.other_next_square.row -= 2;

            add_to_appropriate_amplifying_vector(2, temp);
        }

        else if (last_move.col + 3 <= max_col_index && last_move.row + 3 <= max_row_index &&
                 (*board)[last_move.row + 3][last_move.col + 3] == piece)
        {
            treasure_spot temp;

            temp.current_square = last_move;
            temp.current_square.col++;
            temp.current_square.row++;

            temp.next_square = temp.current_square;
            temp.next_square.col++;
            temp.next_square.row++;

            temp.other_next_square.row = UNDEFINED;
            temp.other_next_square.col = UNDEFINED;

            temp.is_weak_2_square = true;

            add_to_appropriate_amplifying_vector(2, temp);
        }

        return;
    }

    if (num_pieces_in_a_row >= 4)
    {
        if (is_comp_turn) // comp's turn now, so user just moved and got 4-in-a-row:
        {
            evaluation = INT_MIN;
            return;
        }

        evaluation = INT_MAX;
        return;
    }

    // num_pieces_in_a_row must be 2 or 3. This means I may want to add to one of the amplifying vectors:
    // Create a treasure_spot object to possibly do this:

    treasure_spot preceding_point; // space to the up-left of start_point on board.

    preceding_point.current_square = start_point;
    preceding_point.current_square.col--;
    preceding_point.current_square.row--;

    preceding_point.next_square = preceding_point.current_square;
    preceding_point.next_square.col--;
    preceding_point.next_square.row--;

    treasure_spot succeeding_point; // space to the down-right of end_point on board.

    succeeding_point.current_square = end_point;
    succeeding_point.current_square.col++;
    succeeding_point.current_square.row++;

    succeeding_point.next_square = succeeding_point.current_square;
    succeeding_point.next_square.col++;
    succeeding_point.next_square.row++;

    // Version 19 additions:

    preceding_point.other_next_square = succeeding_point.current_square;
    succeeding_point.other_next_square = preceding_point.current_square;

    if (preceding_point.current_square.col >= 0 &&
        preceding_point.current_square.row >= 0 &&
        (*board)[preceding_point.current_square.row][preceding_point.current_square.col] == ' ')
    {
        add_to_appropriate_amplifying_vector(num_pieces_in_a_row, preceding_point);
    }

    if (succeeding_point.current_square.col <= max_col_index &&
        succeeding_point.current_square.row <= max_row_index &&
        (*board)[succeeding_point.current_square.row][succeeding_point.current_square.col] == ' ')
    {
        add_to_appropriate_amplifying_vector(num_pieces_in_a_row, succeeding_point);
    }
}

void position::add_to_appropriate_amplifying_vector(int num_pieces_in_a_row, const treasure_spot& empty_square)
{
    if (is_comp_turn && num_pieces_in_a_row == 2) // user just moved, and has 2 pieces in a row from their last_move:
    {
        squares_amplifying_user_2->push_back(empty_square);
    }

    else if (!is_comp_turn && num_pieces_in_a_row == 2) // comp just moved, and has 2 pieces in a row from its last_move:
    {
        squares_amplifying_comp_2->push_back(empty_square);
    }

    else if (is_comp_turn && num_pieces_in_a_row == 3) // user just moved, and has 3 pieces in a row from their last_move:
    {
        squares_amplifying_user_3->push_back(empty_square);
    }

    else if (!is_comp_turn && num_pieces_in_a_row == 3) // comp just moved, and has 3 pieces in a row from its last_move:
    {
        squares_amplifying_comp_3->push_back(empty_square);
    }
}

void position::minimax()
{
    // Here's where all the magic happens.

    // The game is not over, so look at all positions one move ahead.
    // Then, set evaluation accordingly, using the minimax algorithm...

    for (int i = 0; i < possible_moves.size(); i++) // running through the possible_moves vector to play out each move.
    {
        if (stop_signal)
        {
            return; // Since this means the computer was just thinking while waiting for the user to make their move.
        }

        // Now to make a new position object, with this updated board that's one move ahead.

        unique_ptr<position> pt = make_unique<position>(board, !is_comp_turn, depth + 1,
                                                        number_of_pieces + 1, possible_moves[i],
                                                        possible_moves, i, alpha, beta,
                                                        squares_amplifying_comp_2, squares_amplifying_comp_3,
                                                        squares_amplifying_user_2, squares_amplifying_user_3,
                                                        hash_value_of_position, num_pieces_per_column, did_comp_go_first_in_the_game);
                                                        // Note that this position's 4 amplifying vectors are being sent.
                                                        // Any necessary additions to be made to the amplifying vectors
                                                        // due to last_move (represented by possible_moves[i] here) will be
                                                        // handled in the constructor.
                                                        // Also, this position's hash_value_of_position variable is being sent.
                                                        // It will be updated appropriately in the constructor of the child position node.
                                                        // Finally, num_pieces_per_column for this position is being sent,
                                                        // and then necessary change due to last_move will be taken care of in constructor.

        int future_evaluation = pt->evaluation;

        bool is_child_pruned = pt->is_a_pruned_branch; // stores true if pt (this node's child) got pruned from alpha-beta.
                                                       // This is important to know since if this node's final evaluation actually
                                                       // = child's, it is unstable (and shouldn't be stored in the TT).

        future_positions.push_back(move(pt));

        // Test if a winning move was found for the comp or user:

        if (future_evaluation == INT_MAX && is_comp_turn) // so the comp can make a move that wins...
        {
            evaluation = INT_MAX;

            add_position_to_transposition_table(true);

            return;
        }

        if (future_evaluation == INT_MIN && !is_comp_turn) // so the user can make a move that wins for them...
        {
            evaluation = INT_MIN;

            add_position_to_transposition_table(true);

            return;
        }

        if (evaluation == UNDEFINED) // no evaluation for this position yet, so for now:
        {
            evaluation = future_evaluation;

            if (is_child_pruned)
            {
                got_value_from_pruned_child = true; // note that this might change, since there are likely other child nodes of this node to look at.
            }
        }

        else // this position already has an evaluation from a future position previously examined, so I need to see if
        {    // I should replace it with an updated evaluation value from the current future position being examined now.
            if ((future_evaluation > evaluation && is_comp_turn) || (future_evaluation < evaluation && !is_comp_turn))
            {
                evaluation = future_evaluation;

                if (is_child_pruned)
                {
                    got_value_from_pruned_child = true;
                }

                else
                {
                    got_value_from_pruned_child = false;
                }
            }
        }

        // ALPHA-BETA PRUNING:

        // Let's check if this position is a MAX block (comp's turn) or MIN block (user's turn):

        if (is_comp_turn) // This is a MAX block... check beta to try to prune, and possibly reset alpha.
        {
            // SEE IF I CAN PRUNE:
            if (beta != UNDEFINED && beta <= evaluation) // there is a real beta value, and it is <= evaluation.
            {
                // This position's evaluation can only stay the same or get higher, since the comp will find the position
                // with the highest evaluation (this is a MAX node).

                // Already, beta is <= evaluation.

                // Therefore, the user (in the previous MIN node) would not have picked this branch. They
                // would have picked the branch with the value of beta (i.e., lowest value).

                // So, this branch will be TRIMMED.

                evaluation++; // To ensure this branch is not favoured over the previous good branch
                              // with the value of beta (since beta could = evaluation right now). The parent MIN node of this current MAX node will
                              // not choose this node since it can choose a node with at least 1 lower evaluation than this node.

                is_a_pruned_branch = true;

                return;
            }

            // SEE IF ALPHA SHOULD BE RESET (or given a value, if it doesn't have one yet):
            if (alpha == UNDEFINED || evaluation > alpha)
            {
                alpha = evaluation;
            }
        }

        else // user's turn, so MIN block... check alpha to see if branch should be pruned, and possibly reset beta.
        {
            // SEE IF I CAN PRUNE:
            if (alpha != UNDEFINED && alpha >= evaluation) // there is a real alpha value, and it is >= evaluation.
            {
                // This position's evaluation can only stay the same or get lower, since the user will find the position
                // with the lowest evaluation (this is a MIN block).

                // Already, alpha is >= evaluation.

                // Therefore, the comp (in the previous MAX node) would not have picked this branch. It would have picked
                // the branch with the value of alpha (i.e., highest value).

                // So, this branch will be TRIMMED.

                evaluation--; // To ensure this branch is not favoured over the previous good branch
                              // with the value of alpha. The parent MAX node of this current MIN node will
                              // not choose this node since there's another node with at least 1 greater evaluation than this node.

                is_a_pruned_branch = true;

                return;
            }

            // SEE IF BETA SHOULD BE RESET (or given a value, if it doesn't have one yet):
            if (beta == UNDEFINED || evaluation < beta)
            {
                beta = evaluation;
            }
        }
    }

    // Now, before adding this position to the transposition table, ensure that it didn't get its evaluation from a pruned child node:

    if (!got_value_from_pruned_child)
    {
        add_position_to_transposition_table(false); // since at the end of this minimax() function, evaluation has been finalized.
    }
}

double position::find_revised_player_evaluation(const vector<coordinate_and_double>& info_for_player_amplifying_squares,
                                                const vector<vector<bool>>& squares_winning_for_player,
                                                const vector<vector<bool>>& squares_winning_for_opponent,
                                                bool is_player_the_comp) const
{
    double temp_evaluation_as_double = 0.0;

    bool did_player_move_first_in_the_game = ((is_player_the_comp && did_comp_go_first_in_the_game) ||
                                              (!is_player_the_comp && !did_comp_go_first_in_the_game));

    for (coordinate_and_double current: info_for_player_amplifying_squares)
    {
        if ((current.square.row % 2 != 0) == did_player_move_first_in_the_game)
        {
            // I used to only increase the square's value if it was a winning square.
            // But now in V.35, I'm doing it for squares amplifiying a 2-in-a-row too.

            current.value *= 1.75;
        }

        if (current.square.row + 1 <= max_row_index && squares_winning_for_opponent[current.square.row + 1][current.square.col])
        {
            current.value *= 0.25;
        }

        else if (current.square.row + 2 <= max_row_index && squares_winning_for_player[current.square.row + 2][current.square.col])
        {
            current.value *= 0.75;
        }

        temp_evaluation_as_double += current.value;
    }

    return temp_evaluation_as_double;
}

void position::smart_evaluation()
{
    int lowest_comp_square_D = UNDEFINED;
    int lowest_good_comp_square_D = UNDEFINED;
    int lowest_user_square_D = UNDEFINED;
    int lowest_good_user_square_D = UNDEFINED;

    initialize_row_barriers(lowest_comp_square_D, lowest_good_comp_square_D,
                            lowest_user_square_D, lowest_good_user_square_D);
                               // implements finished column algorithm, by finding the squares in each
                               // column that is as far as play can possibly go (due to a square allowing comp AND user to win).
                               // These squares will be stored in the private member, "row_barriers", and
                               // row_barriers has 7 elements. row_barriers[0] is the row value of the
                               // highest square allowed for play in column 0,.... row_barriers[6] is the row value of the
                               // highest square allowed for play in column 6.

                               // The function is also passed 4 coordinate objects by reference, to store specific squares
                               // in column D.

    //  VERSION 40:

    if (lowest_good_comp_square_D != UNDEFINED &&
        lowest_good_comp_square_D >= lowest_user_square_D)
    {
        // The comp has a good square, and the user has no square below it.

        if (!can_create_threat_with_D('U', lowest_good_comp_square_D + 1))
        {
            evaluation = INT_MAX;

            return;
        }
    }

    if (lowest_good_user_square_D != UNDEFINED &&
        lowest_good_user_square_D >= lowest_comp_square_D)
    {
        if (!can_create_threat_with_D('C', lowest_good_user_square_D + 1))
        {
            evaluation = INT_MIN;

            return;
        }
    }

    vector<coordinate_and_double> info_for_comp_valid_amplifying_squares;
    // Stores the coordinates of each of the comp's valid amplifying squraes (both for 2-in-a-rows and 3-in-a-rows), along with each square's
    // respective value (determined in find_individual_player_evaluation()).

    vector<coordinate_and_double> info_for_user_valid_amplifying_squares;
    // Stores the coordinates of each of the user's valid amplifying squraes (both for 2-in-a-rows and 3-in-a-rows), along with each square's
    // respective value (determined in find_individual_player_evaluation()).

    find_individual_player_evaluation(squares_amplifying_comp_3, squares_amplifying_comp_2, 'C',
                                      board_of_squares_winning_for_comp, info_for_comp_valid_amplifying_squares);
    find_individual_player_evaluation(squares_amplifying_user_3, squares_amplifying_user_2, 'U',
                                      board_of_squares_winning_for_user, info_for_user_valid_amplifying_squares);

    evaluation = round(find_revised_player_evaluation(info_for_comp_valid_amplifying_squares,
                                                      board_of_squares_winning_for_comp, board_of_squares_winning_for_user, true) -
                       find_revised_player_evaluation(info_for_user_valid_amplifying_squares,
                                                      board_of_squares_winning_for_user, board_of_squares_winning_for_comp, false));

    // The statement finds the final individual evaluation for the computer and user. It then finds the difference between these two
    // values, and returns this difference rounded to an int.

    reset_board_of_bools(board_of_squares_winning_for_comp, info_for_comp_valid_amplifying_squares);

    reset_board_of_bools(board_of_squares_winning_for_user, info_for_user_valid_amplifying_squares);
}

void position::find_individual_player_evaluation(const shared_ptr<vector<treasure_spot>> squares_amplifying_3,
                                                 const shared_ptr<vector<treasure_spot>> squares_amplifying_2, char piece,
                                                 vector<vector<bool>>& board_of_squares_winning_for_player,
                                                 vector<coordinate_and_double>& recorder) const
{
    // board_of_squares_winning_for_player should be worked on, storing true for the squares creating a 4-in-a-row.
    // recorder is used to store the coordinates and derived values of all unique amplifying squares (both for 2-in-a-row and 3-in-a-row).

    // Use row_barriers private member vector when seeing if an amplifying square should be counted.

    const double big_amount = 10.0; // how many points to add if there is an amplifying square completes a 3-in-a-row.
    const double small_amount = 3.0; // NOTE: if you change this to a double, recorder (and its parent in prev. function) should store coordinate and DOUBLE.
                                // and calculations done in this function should be in doubles, not ints whatsoever. how many points to add if an amplifying square completes a 2-in-a-row.
    const double stacked_threat_coefficient = 5.0; // What to multiply the normal value of a square that let's a 3-in-a-row be
                                              // completed into a 4-in-a-row, if there's a similar square above/below it.

    for (const treasure_spot& space: *squares_amplifying_3) // running through squares completing a 3-in-a-row.
    {
        coordinate current_square = space.current_square;

        if ((*board)[current_square.row][current_square.col] == ' ' &&
            !board_of_squares_winning_for_player[current_square.row][current_square.col] &&
            (row_barriers[current_square.col] == UNDEFINED || row_barriers[current_square.col] <= current_square.row))
            // Second line applies the finished column algorithm.
        {
            coordinate_and_double temp;

            temp.square = current_square;

            temp.value = big_amount * static_cast<double>(current_square.row + 1 + (*num_pieces_per_column)[current_square.col]);

            // I want to check if the square above or below has an 'A' in it, since this means I must have already
            // examined it and found it led to a 4-in-a-row. If this is the case, the computer will have two threats to
            // make a 4-in-a-row lined up on top of each over (which is extremely powerful).

            if ((current_square.row + 1 <= max_row_index && board_of_squares_winning_for_player[current_square.row + 1][current_square.col]) ||
                (current_square.row - 1 >= 0 && board_of_squares_winning_for_player[current_square.row - 1][current_square.col]))
            {
                temp.value *= stacked_threat_coefficient; // since the threat is much more powerful now.
            }

            board_of_squares_winning_for_player[current_square.row][current_square.col] = true;

            recorder.push_back(temp);
        }
    }

    vector<treasure_spot> normal_squares_amplifying_2; // Will store all squares that only amplify a 2-in-a-row, and don't make a 4-in-a-row.
                                                       // These squares will be looked at on a final pass, AFTER the below for loop.

    for (const treasure_spot& space: *squares_amplifying_2) // running through spaces completing a 2-in-a-row.
    {
        coordinate current_square = space.current_square;
        coordinate next_square = space.next_square;
        coordinate other_next_square = space.other_next_square;

        if ((*board)[current_square.row][current_square.col] == ' ' &&
            !board_of_squares_winning_for_player[current_square.row][current_square.col] &&
            (row_barriers[current_square.col] == UNDEFINED || row_barriers[current_square.col] <= current_square.row))
        {
            bool is_next_square_in_bounds = is_in_bounds(next_square);
            bool is_other_next_square_in_bounds = is_in_bounds(other_next_square);

            if ((is_next_square_in_bounds && (*board)[next_square.row][next_square.col] == piece) ||
                (is_other_next_square_in_bounds && (*board)[other_next_square.row][other_next_square.col] == piece))
            {
                coordinate_and_double temp;

                temp.square = current_square;

                // Treat current_square as if it completed a 3-in-a-row into a 4-in-a-row, since filling it ==> wins the game.

                temp.value = big_amount * static_cast<double>(current_square.row + 1 + (*num_pieces_per_column)[current_square.col]);

                // I want to check if the square above or below has an 'A' in it, since this means I must have already
                // examined it and found it led to a 4-in-a-row if filled. If this is the case, the computer will have two threats to
                // make a 4-in-a-row lined up on top of each over (which is extremely powerful).

                if ((current_square.row + 1 <= max_row_index && board_of_squares_winning_for_player[current_square.row + 1][current_square.col]) ||
                    (current_square.row - 1 >= 0 && board_of_squares_winning_for_player[current_square.row - 1][current_square.col]))
                {
                    temp.value *= stacked_threat_coefficient; // since the threat is much more powerful now.
                }

                board_of_squares_winning_for_player[current_square.row][current_square.col] = true;

                recorder.push_back(temp);
            }

            else
            {
                normal_squares_amplifying_2.push_back(space);
            }
        }
    }

    for (const treasure_spot& space: normal_squares_amplifying_2)
    {
        coordinate current_square = space.current_square;
        coordinate next_square = space.next_square;
        coordinate other_next_square = space.other_next_square;

        if (!board_of_squares_winning_for_player[current_square.row][current_square.col])
        {
            if (space.is_weak_2_square)
            {
                // If this is the case, I want to see if the two pieces on the ends of the two squares
                // are by themselves.
                // If they're not, skip this square.

                int row_diff = (next_square.row - current_square.row) * 2;
                int col_diff = (next_square.col - current_square.col) * 2;

                coordinate outside_square_1;
                outside_square_1.row = next_square.row + row_diff;
                outside_square_1.col = next_square.col + col_diff;

                coordinate outside_square_2;
                outside_square_2.row = current_square.row - row_diff;
                outside_square_2.col = current_square.col - col_diff;

                if (is_in_bounds(outside_square_1) &&
                    (*board)[outside_square_1.row][outside_square_1.col] == piece)
                {
                    continue;
                }

                if (is_in_bounds(outside_square_2) &&
                    (*board)[outside_square_2.row][outside_square_2.col] == piece)
                {
                    continue;
                }
            }

            bool is_next_square_empty = (is_in_bounds(next_square) && (*board)[next_square.row][next_square.col] == ' ' &&
                                         !board_of_squares_winning_for_player[next_square.row][next_square.col] &&
                                         (row_barriers[next_square.col] == UNDEFINED || row_barriers[next_square.col] <= next_square.row));

            bool is_other_next_square_empty = (is_in_bounds(other_next_square) && (*board)[other_next_square.row][other_next_square.col] == ' ' &&
                                               !board_of_squares_winning_for_player[other_next_square.row][other_next_square.col] &&
                                               (row_barriers[other_next_square.col] == UNDEFINED ||
                                               row_barriers[other_next_square.col] <= other_next_square.row));

            // Two bools above each store true if next_square/other_next_square is in-bounds, empty, and not above a row barrier.

            if (is_next_square_empty)
            {
                // At least one next square is empty:

                coordinate_and_double temp;

                temp.square = current_square;

                temp.value = small_amount * static_cast<double>(current_square.row + 1 + (*num_pieces_per_column)[current_square.col]);

                if (is_other_next_square_empty)
                {
                    // BOTH next squares are empty. Multiply temp.value by some constant:

                    temp.value *= 1.2; // This constant did best in the playing sim I ran on Linux.

                    if ((is_vertically_adjacent_to_a_winning_square(board_of_squares_winning_for_player, other_next_square)) ||
                        (is_vertically_adjacent_to_a_winning_square(board_of_squares_winning_for_player, next_square)))
                    {
                        temp.value *= 1.5;
                    }
                }

                else // only next_square is empty.
                {
                    if (is_vertically_adjacent_to_a_winning_square(board_of_squares_winning_for_player, next_square))
                    {
                        temp.value *= 1.5;
                    }
                }

                recorder.push_back(temp);
            }

            else if (is_other_next_square_empty)
            {
                // Only other_next_square is empty (next_square cannot be empty due to this being an else if):

                coordinate_and_double temp;

                temp.square = current_square;

                temp.value = small_amount * static_cast<double>(current_square.row + 1 + (*num_pieces_per_column)[current_square.col]);


                if (is_vertically_adjacent_to_a_winning_square(board_of_squares_winning_for_player, other_next_square))
                {
                    temp.value *= 1.5;
                }

                recorder.push_back(temp);
            }
        }
    }
}

bool position::is_vertically_adjacent_to_a_winning_square(const vector<vector<bool>>& the_board, const coordinate& square) const
{
    // return true if the board stores a "true" value above or below square.

    return ((square.row > 0 && the_board[square.row - 1][square.col]) ||
            (square.row < max_row_index && the_board[square.row + 1][square.col]));
}

bool position::did_someone_win() const
{
    // First, ensure that last_move has been initialized.

    if (last_move.row == UNDEFINED || last_move.col == UNDEFINED)
    {
        return false; // this is the starting position...
    }

    // I need to check for 4-in-a-row horizontally, vertically, positive-slope diagonally, negative-slope diagonally.

    // RETURN BASED OFF CALLS TO ALL 4 OF THESE METHODS:

    return (horizontal_four_combo() ||
            vertical_four_combo() ||
            positive_slope_diagonal_four_combo() ||
            negative_slope_diagonal_four_combo());
}

bool position::horizontal_four_combo() const
{
    if (last_move.col + 1 > max_col_index || (*board)[last_move.row][last_move.col + 1] != (*board)[last_move.row][last_move.col])
    {
        return (last_move.col - 3 >= 0 &&
                (*board)[last_move.row][last_move.col - 3] == (*board)[last_move.row][last_move.col] &&
                (*board)[last_move.row][last_move.col - 2] == (*board)[last_move.row][last_move.col] &&
                (*board)[last_move.row][last_move.col - 1] == (*board)[last_move.row][last_move.col]);
    }

    if (last_move.col - 1 < 0 || (*board)[last_move.row][last_move.col - 1] != (*board)[last_move.row][last_move.col])
    {
        return (last_move.col + 3 <= max_col_index &&
                (*board)[last_move.row][last_move.col + 3] == (*board)[last_move.row][last_move.col] &&
                (*board)[last_move.row][last_move.col + 2] == (*board)[last_move.row][last_move.col]);
    }

    return ((last_move.col - 2 >= 0 && (*board)[last_move.row][last_move.col - 2] == (*board)[last_move.row][last_move.col]) ||
            (last_move.col + 2 <= max_col_index && (*board)[last_move.row][last_move.col + 2] == (*board)[last_move.row][last_move.col]));
}

bool position::vertical_four_combo() const
{
    // This one is easy to check, since there can be no pieces above the coordinates of the last move.

    // Therefore, I only need to check if there are 3 pieces under the last move, and if these pieces match the last move.

    return (last_move.row + 3 <= max_row_index &&
            (*board)[last_move.row + 3][last_move.col] == (*board)[last_move.row][last_move.col] &&
            (*board)[last_move.row + 2][last_move.col] == (*board)[last_move.row][last_move.col] &&
            (*board)[last_move.row + 1][last_move.col] == (*board)[last_move.row][last_move.col]);
}

bool position::positive_slope_diagonal_four_combo() const
{
    if (last_move.row - 1 < 0 ||
        last_move.col + 1 > max_col_index ||
        (*board)[last_move.row - 1][last_move.col + 1] != (*board)[last_move.row][last_move.col])
    {
        return (last_move.row + 3 <= max_row_index &&
                last_move.col - 3 >= 0 &&
                (*board)[last_move.row + 3][last_move.col - 3] == (*board)[last_move.row][last_move.col] &&
                (*board)[last_move.row + 2][last_move.col - 2] == (*board)[last_move.row][last_move.col] &&
                (*board)[last_move.row + 1][last_move.col - 1] == (*board)[last_move.row][last_move.col]);
    }

    if (last_move.row + 1 > max_row_index ||
        last_move.col - 1 < 0 ||
        (*board)[last_move.row + 1][last_move.col - 1] != (*board)[last_move.row][last_move.col])
    {
        return (last_move.row - 3 >= 0 &&
                last_move.col + 3 <= max_col_index &&
                (*board)[last_move.row - 3][last_move.col + 3] == (*board)[last_move.row][last_move.col] &&
                (*board)[last_move.row - 2][last_move.col + 2] == (*board)[last_move.row][last_move.col]);
    }

    return ((last_move.row + 2 <= max_row_index &&
             last_move.col - 2 >= 0 &&
             (*board)[last_move.row + 2][last_move.col - 2] == (*board)[last_move.row][last_move.col])
            ||
            (last_move.row - 2 >= 0 &&
             last_move.col + 2 <= max_col_index &&
             (*board)[last_move.row - 2][last_move.col + 2] == (*board)[last_move.row][last_move.col]));
}

bool position::negative_slope_diagonal_four_combo() const
{
    if (last_move.row + 1 > max_row_index ||
        last_move.col + 1 > max_col_index ||
        (*board)[last_move.row + 1][last_move.col + 1] != (*board)[last_move.row][last_move.col])
    {
        return (last_move.row - 3 >= 0 &&
                last_move.col - 3 >= 0 &&
                (*board)[last_move.row - 3][last_move.col - 3] == (*board)[last_move.row][last_move.col] &&
                (*board)[last_move.row - 2][last_move.col - 2] == (*board)[last_move.row][last_move.col] &&
                (*board)[last_move.row - 1][last_move.col - 1] == (*board)[last_move.row][last_move.col]);
    }

    if (last_move.row - 1 < 0 ||
        last_move.col - 1 < 0 ||
        (*board)[last_move.row - 1][last_move.col - 1] != (*board)[last_move.row][last_move.col])
    {
        return (last_move.row + 3 <= max_row_index &&
                last_move.col + 3 <= max_col_index &&
                (*board)[last_move.row + 3][last_move.col + 3] == (*board)[last_move.row][last_move.col] &&
                (*board)[last_move.row + 2][last_move.col + 2] == (*board)[last_move.row][last_move.col]);
    }

    return ((last_move.row - 2 >= 0 &&
             last_move.col - 2 >= 0 &&
             (*board)[last_move.row - 2][last_move.col - 2] == (*board)[last_move.row][last_move.col])
            ||
            (last_move.row + 2 <= max_row_index &&
             last_move.col + 2 <= max_col_index &&
             (*board)[last_move.row + 2][last_move.col + 2] == (*board)[last_move.row][last_move.col]));
}

bool position::is_acceptable_letter(char c) const
{
    return (c >= 'A' && c <= 'G') || (c >= 'a' && c <= 'g');
}

bool position::is_element_in_vector(const vector<vector<vector<char>>>& vec, const vector<vector<char>>& element) const
{
    for (int i = 0; i < vec.size(); i++)
    {
        if (vec[i] == element)
        {
            return true;
        }
    }

    return false;
}

coordinate position::find_starting_horizontal_point() const
{
    char piece = (*board)[last_move.row][last_move.col];

    coordinate left_most_point = last_move;

    while (true)
    {
        if (left_most_point.col - 1 < 0 || (*board)[left_most_point.row][left_most_point.col - 1] != piece)
        {
            return left_most_point; // since the next square over left is either out-of-bounds or not equal to piece.
        }

        left_most_point.col --;
    }
}

coordinate position::find_ending_horizontal_point() const
{
    char piece = (*board)[last_move.row][last_move.col];

    coordinate right_most_point = last_move;

    while (true)
    {
        if (right_most_point.col + 1 > max_col_index || (*board)[right_most_point.row][right_most_point.col + 1] != piece)
        {
            return right_most_point; // since the next square over right is either out-of-bounds or not equal to piece.
        }

        right_most_point.col ++;
    }
}

coordinate position::find_ending_vertical_point() const
{
    char piece = (*board)[last_move.row][last_move.col];

    coordinate bottom_most_point = last_move;

    while (true)
    {
        if (bottom_most_point.row + 1 > max_row_index || (*board)[bottom_most_point.row + 1][bottom_most_point.col] != piece)
        {
            return bottom_most_point; // since the next square down under is either out-of-bounds or not equal to piece.
        }

        bottom_most_point.row ++;
    }
}

coordinate position::find_starting_positive_slope_diagonal_point() const
{
    char piece = (*board)[last_move.row][last_move.col];

    coordinate bottom_left_most_point = last_move;

    while (true)
    {
        if (bottom_left_most_point.row + 1 > max_row_index ||
            bottom_left_most_point.col - 1 < 0 ||
            (*board)[bottom_left_most_point.row + 1][bottom_left_most_point.col - 1] != piece)
        {
            return bottom_left_most_point; // since the next square over down-left is either out-of-bounds or not equal to piece.
        }

        bottom_left_most_point.row ++;
        bottom_left_most_point.col --;
    }
}

coordinate position::find_ending_positive_slope_diagonal_point() const
{
    char piece = (*board)[last_move.row][last_move.col];

    coordinate top_right_most_point = last_move;

    while (true)
    {
        if (top_right_most_point.row - 1 < 0 ||
            top_right_most_point.col + 1 > max_col_index ||
            (*board)[top_right_most_point.row - 1][top_right_most_point.col + 1] != piece)
        {
            return top_right_most_point; // since the next square up-right is either out-of-bounds or not equal to piece.
        }

        top_right_most_point.row --;
        top_right_most_point.col ++;
    }
}

coordinate position::find_starting_negative_slope_diagonal_point() const
{
    char piece = (*board)[last_move.row][last_move.col];

    coordinate top_left_most_point = last_move;

    while (true)
    {
        if (top_left_most_point.row - 1 < 0 ||
            top_left_most_point.col - 1 < 0 ||
            (*board)[top_left_most_point.row - 1][top_left_most_point.col - 1] != piece)
        {
            return top_left_most_point; // since the next square up-left is either out-of-bounds or not equal to piece.
        }

        top_left_most_point.row --;
        top_left_most_point.col --;
    }
}

coordinate position::find_ending_negative_slope_diagonal_point() const
{
    char piece = (*board)[last_move.row][last_move.col];

    coordinate bottom_right_most_point = last_move;

    while (true)
    {
        if (bottom_right_most_point.row + 1 > max_row_index ||
            bottom_right_most_point.col + 1 > max_col_index ||
            (*board)[bottom_right_most_point.row + 1][bottom_right_most_point.col + 1] != piece)
        {
            return bottom_right_most_point; // since the next square over down-right is either out-of-bounds or not equal to piece.
        }

        bottom_right_most_point.row ++;
        bottom_right_most_point.col ++;
    }
}

void position::display_board()
{
    cout << "\n    A   B   C   D   E   F   G\n\n";

    for (int row = 0; row <= 5; row++)
    {
        cout << (6 - row) << " | "; // Since I want row numbers to be displayed increasing from bottom up, not top down.

        for (int col = 0; col <= 6; col++)
        {
            cout << (*board)[row][col] << " | ";
        }

        cout << "\n" << "  |---|---|---|---|---|---|---|\n";
    }
}
