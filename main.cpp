#include <iostream>
#include <vector>
#include <stdexcept>
#include <memory>
#include <time.h>
#include <cstdlib>
#include <algorithm>
#include <climits>
#include <fstream>
#include <ctime>
#include <ratio>
#include <chrono>
#include <thread>
#include <atomic>

#include "position.h"

using namespace std;

bool talk_with_python = true;
// If this is false, then I'm in testing mode and the UI is still in C++ here.

const int initial_textfile_num = 1587992;

int current_textfile_num = initial_textfile_num;
// The first textfile used to communicate with Python will be "1587992.txt".
// Then, it will become "1587993.txt", and so on.
// Whenever writing to reading from the current textfile, increment this variable.

string current_textfile_name = to_string(current_textfile_num) + ".txt";

atomic<bool> received_input(false);

bool should_crash_program = false; // If true, the whole program stops.

coordinate placeholder = {position::UNDEFINED, position::UNDEFINED};

int num_other_threads_running = 0;
// Whenever creating a new thread, increment this variable.
// Then, when the thread finishes, decrement this variable.

void display_board(vector<vector<char>> board, bool x_represents_user,
                   bool is_starting_position, coordinate last_move)
{
    cout << "\n\nTime the computer has spent in functions:\n";
    for (int i = 0; i < position::times.size(); i++) {
        if (position::times[i] > 0.00000000001) {
            cout << "times[" << i << "]: " << position::times[i] << " seconds\n";
        }
    }
    
    // First, change the 'C' and 'U' in board to 'X' and 'O', depending on if 'X' or 'O' represents the user...

    for (int row = 0; row <= 5; row++)
    {
        for (int col = 0; col <= 6; col++)
        {
            if (board[row][col] == 'U')
            {
                if (x_represents_user)
                {
                    board[row][col] = 'X';
                }

                else
                {
                    board[row][col] = 'O';
                }
            }

            else if (board[row][col] == 'C')
            {
                if (x_represents_user) // so 'O' represents the computer:
                {
                    board[row][col] = 'O';
                }

                else
                {
                    board[row][col] = 'X';
                }
            }
        }
    }

    // Print who moved, and to where, assuming there is a last move (only not the case when this is the starting position).

    if (!is_starting_position)
    {
        if (last_move.row == position::UNDEFINED)
        {
            throw runtime_error("last_move is UNDEFINED even though this isn't the starting position.\n");
        }

        if ((board[last_move.row][last_move.col] == 'X') == (x_represents_user)) // testing for a logical equivalence between two boolean values.
        {
            cout << "\nYou just moved to square: ";
        }

        else
        {
            cout << "The computer just moved to square: ";
        }

        cout << char('A' + last_move.col) << (6 - last_move.row) << "\n";
    }

    cout << "\n    A   B   C   D   E   F   G\n\n";

    for (int row = 0; row <= 5; row++)
    {
        cout << (6 - row) << " | "; // Since I want row numbers to be displayed increasing from bottom up, not top down.

        for (int col = 0; col <= 6; col++)
        {
            cout << board[row][col] << " | ";
        }

        cout << "\n" << "  |---|---|---|---|---|---|---|\n";
    }
}

vector<coordinate> get_all_squares_filled_by_piece(const vector<vector<char>>& board,
                                                   bool x_represents_user,
                                                   bool looking_for_x)
{
    // Board has 'U' and 'C' pieces. Return the squares containing X's or O's,
    // depending on the truth value of the "looking_for_x" parameter.

    vector<coordinate> squares;

    for (int r = 0; r < 6; r++)
    {
        for (int c = 0; c < 7; c++)
        {
            if ((board[r][c] == 'U' && looking_for_x == x_represents_user) ||
                (board[r][c] == 'C' && looking_for_x != x_represents_user))
            {
                squares.push_back({r,c});
            }
        }
    }

    return squares;
}

void remove_set_at_index(vector<vector<coordinate>>& vec, int index)
{
    if (vec.empty())
    {
        throw runtime_error("vec is empty in remove_set_at_index()\n");
    }

    vec[index] = vec[vec.size()-1];

    vec.pop_back();
}

unique_ptr<position> get_to_chosen_starting_position(bool does_comp_go_first, const vector<coordinate> set_of_moves,
                                                     bool set_of_moves_supposed_to_be_empty)
{
    // Get the Engine to play out the set_of_moves param, and only think for a reasonable amount of time
    // on the last move (since that's where the game begins).

    if (set_of_moves_supposed_to_be_empty && set_of_moves.empty())
    {
        // So the game is supposed to begin in the empty starting position.

        unique_ptr<position> pt = position::think_on_game_position(does_comp_go_first, true,
                                                                   placeholder, false);

        return move(pt);
    }

    if (set_of_moves.empty() || set_of_moves_supposed_to_be_empty)
    {
        // This basically checks that set_of_moves_supposed_to_be_empty and the size of set of moves equaling 0 are not
        // mutually exclusive. Either both are true, or neither are. If only one are true, and error happened.

        throw runtime_error("set_of_moves is empty in get_to_chosen_starting_position()\n");
    }

    const double old_thinking_time = position::thinking_time;

    position::thinking_time = 0; // temporarily reducing it, until reaching the actual starting position for the game.

    // Now, I need to figure out if a 'C' or 'U' should be played first on the empty board.
    // The "does_comp_go_first" param stores who goes first in the actual starting position played, 4-9 moves later.

    bool does_comp_move_first_in_empty_board = true;

    if ((does_comp_go_first && set_of_moves.size() % 2 != 0) || (!does_comp_go_first && set_of_moves.size() % 2 == 0))
    {
        // In either case, 'U' should be the first piece placed on the empty board:

        does_comp_move_first_in_empty_board = false;
    }

    unique_ptr<position> pt = position::think_on_game_position(does_comp_move_first_in_empty_board, true,
                                                               placeholder, false);

    for (int i = 0; i < set_of_moves.size(); i++)
    {
        if (i == set_of_moves.size()-1) // On the move that yields the starting position, so return thinking_time to its original value:
        {
            position::thinking_time = old_thinking_time;
        }

        vector<vector<char>> temp_board = pt->get_board();

        if (pt->get_is_comp_turn())
        {
            temp_board[set_of_moves[i].row][set_of_moves[i].col] = 'C';
        }

        else
        {
            temp_board[set_of_moves[i].row][set_of_moves[i].col] = 'U';
        }

        pt = position::think_on_game_position(temp_board, !pt->get_is_comp_turn(), set_of_moves[i], pt->get_squares_amplifying_comp_2(),
                                              pt->get_squares_amplifying_comp_3(), pt->get_squares_amplifying_user_2(),
                                              pt->get_squares_amplifying_user_3(), true, placeholder, false);
                                              // Sending "true" for starting new game since I don't want the TT used. In order to be fair, the comp
                                              // shouldn't be able to use calculations it did to get to the starting position.
    }

    if (position::thinking_time != old_thinking_time)
    {
        throw runtime_error("thinking_time was not reset to its standard value!\n");
    }

    // Now ensure that the position's evaluation is within [-10,10] in the DB.

    int evaluation_of_position;
    int purposeless;

    Database_Functions::get_info_for_position(pt->get_board(), pt->get_is_comp_turn(),
                                              evaluation_of_position, purposeless);

    if (evaluation_of_position < -10 || evaluation_of_position > 10)
    {
        throw "The evaluation of the starting position isn't within [-10, 10]";
    }

    return move(pt);
}

bool is_board_empty(const vector<vector<char>>& board)
{
    for (int r = 0; r < 6; r++)
    {
        for (int c = 0; c < 7; c++)
        {
            if (board[r][c] != ' ')
            {
                return false;
            }
        }
    }

    return true;
}

vector<vector<char>> flip_board(vector<vector<char>> board)
{
    for (int r = 0; r < 6; r++)
    {
        for (int c = 0; c < 7; c++)
        {
            if (board[r][c] == 'C')
            {
                board[r][c] = 'U';
            }

            else if (board[r][c] == 'U')
            {
                board[r][c] = 'C';
            }
        }
    }

    return board;
}

void think_while_user_is_thinking(vector<vector<char>> board, bool is_comp_turn,
                                  coordinate last_move, const vector<treasure_spot>& squares_amplifying_comp_2,
                                  const vector<treasure_spot>& squares_amplifying_comp_3,
                                  const vector<treasure_spot>& squares_amplifying_user_2,
                                  const vector<treasure_spot>& squares_amplifying_user_3,
                                  bool starting_new_game, coordinate* predicted_move, coordinate* best_response)
{
    num_other_threads_running ++;

    // First, try to predict what move the user will choose.
    // Do this by thinking for around 0.3 seconds, and then finding the best move.
    // Everything will have to be flipped to do this, since the find_best_move function only finds the best
    // move for the comp.

    double old_thinking_time = position::thinking_time;

    position::thinking_time = 0.25;

    unique_ptr<position> temp;

    if (is_board_empty(board))
    {
        temp = position::think_on_game_position(true, false, placeholder, false);
    }

    else
    {
        temp = position::think_on_game_position(flip_board(board), true, last_move,
                                                squares_amplifying_user_2, squares_amplifying_user_3,
                                                squares_amplifying_comp_2, squares_amplifying_comp_3,
                                                false, placeholder, false);

        // The amplifying vectors were sent in reverse order deliberately, since swapping everything.
    }

    position::thinking_time = old_thinking_time;

    *predicted_move = temp->find_best_move_for_comp(); // really finding the best move for the user.

    // Now to make the predicted_move, and then think to generate a reply:

    board[(*predicted_move).row][(*predicted_move).col] = 'U';

    position::thinking_time = 10.0;

    unique_ptr<position> pos = position::think_on_game_position(board, true, *predicted_move, squares_amplifying_comp_2,
                                                                squares_amplifying_comp_3, squares_amplifying_user_2,
                                                                squares_amplifying_user_3, false, *best_response, true);

    position::thinking_time = old_thinking_time;

    num_other_threads_running --;
}

int get_column_user_wants_to_move_in(const unique_ptr<position>& pt)
{
    // Function returns the column the user wants to move in.

    string user_input = "";

    cout << "Enter a column from 'A'-'G' (or 'a'-'g') to move: ";

    cin >> user_input;

    cin.clear();

    cin.ignore(INT_MAX, '\n');

    while (!pt->is_valid_move(user_input))
    {
        cout << "You entered an invalid move. Please try again: ";

        cin >> user_input;

        cin.clear();

        cin.ignore(INT_MAX, '\n');
    }

    // Now I know the user inputted a valid move, so figure out what col they meant:

    char letter = user_input[0];

    if (letter >= 'a') // lowercase:
    {
        return static_cast<int>(letter - 'a');
    }

    else // uppercase:
    {
        return static_cast<int>(letter - 'A');
    }
}

void wait(double waiting_time)
{
    steady_clock::time_point start = steady_clock::now();

    while (duration_cast<duration<double>>(steady_clock::now() - start).count() < waiting_time)
    {
        // Left unfilled deliberately - only purpose is to wait.
    }
}

void use_thread_to_reset_TT()
{
    num_other_threads_running ++;

    position::reset_transposition_table();

    num_other_threads_running --;
}

void crash_if_Python_ends()
{
    // A seprate thread will run this function while the main thread waits
    // for user input.

    // The function returns true if the entire program should crash.

    num_other_threads_running ++;

    int current_time_point = time(NULL);

    while (true)
    {
        if (received_input)
        {
            received_input = false; // In preparation for next time.

            num_other_threads_running --;

            return;
        }

        if (time(NULL) - current_time_point >= 2)
        {
            // It has been 4 seconds, so check again if the signal.txt file
            // has been created. This would mean the Python program ended
            // abruptly (by the user closing the GUI), which means
            // this C++ program should end now too.

            current_time_point = time(NULL); // for next time.

            if (ifstream("signal.txt"))
            {
                // Crash this program:

                break;
            }
        }
    }

    // If control reached this point, it means that the signal.txt file was
    // created, which in turn means that the Python program crashed.

    // Therefore, the global flag to crash this program should be set to true.

    should_crash_program = true;

    num_other_threads_running --;
}

void prepare_to_throw(string error_message, thread* thread_ptr)
{
    // This function is called when I'm about to throw a string and
    // end the program, but I want to make sure any other threads finish.

    position::stop_signal = true;

    wait(1.0);

    while (num_other_threads_running > 0)
    {
        wait(1.0);
    }

    if (thread_ptr != nullptr)
    {
        thread_ptr->join();
        // This should safely close the thread for good.
        // Now to end the program:
    }

    cout << num_other_threads_running << " other threads running\n"; // Should be 0.

    throw error_message; // Will be handled by the catch block at the end of main().
}

int get_value_from_Python_textfile(string input_description, thread* thread_ptr)
{
    // thread_ptr may or may not point to an actual thread (it could be just = nullptr).
    // If it does point to a thread, the point of passing it here is to call ->join()
    // on it if the user doesn't enter input, and the program has to crash.

    thread t1(crash_if_Python_ends);

    int value = -1;

    while (true)
    {
        if (should_crash_program)
        {
            string error_message = "The Python application ended because the ";
            error_message += "signal.txt file was created, so the C++ program ";
            error_message += " ended as well.";

            t1.join();

            prepare_to_throw(error_message, thread_ptr);
        }

        ifstream fin(current_textfile_name.c_str());

        if (fin.good())
        {
            // Check if there's any characters in the file:

            if (fin.peek() != ifstream::traits_type::eof())
            {
                // The file is not empty (since at least the next character
                // is not the EOF):

                fin >> value;

                current_textfile_num ++;
                current_textfile_name = to_string(current_textfile_num) + ".txt";

                fin.close();

                received_input = true;

                t1.join();

                return value;
            }

            fin.close();
        }
    }
}

void write_to_Python_textfile(int value)
{
    // Make a file with the name of "current_textfile_name", and write value to it.

    ofstream fout(current_textfile_name);
    fout << value;
    fout.close();

    current_textfile_num ++;
    current_textfile_name = to_string(current_textfile_num) + ".txt";
}

void write_squares_to_file(const vector<vector<char>>& board, bool x_represents_user)
{
    // The format of this file will have 42 lines.
    // Line 1 represents square [0,0], line 2 represents [0,1],
    // ...., line 42 represents [5,6].

    // Each line has one int. If the int is 0, the square is empty. If the
    // int is 1, the square has an X. If the int is 2, the square has an O.

    ofstream fout(current_textfile_name);

    int test_counter = 0;

    for (int r = 0; r < 6; r++)
    {
        for (int c = 0; c < 7; c++)
        {
            int value_to_write = 0;

            if ((board[r][c] == 'U' && x_represents_user) ||
                (board[r][c] == 'C' && !x_represents_user))
            {
                value_to_write = 1;
            }

            else if (board[r][c] != ' ')
            {
                // Board has a piece in it, but isn't an X. So it must be an O:

                value_to_write = 2;
            }

            fout << value_to_write;

            test_counter ++;

            if (!(r == 5 && c == 6))
            {
                // Not on the last entry for the file, so do a normal newline.
                fout << "\n";
            }
        }
    }

    fout.close();

    current_textfile_num ++;
    current_textfile_name = to_string(current_textfile_num) + ".txt";

    if (test_counter != 42)
    {
        // Something went wrong...

        ofstream fout("PROBLEM.txt");

        fout << "Only " << test_counter << " values outputted to squares file.";

        int current_time = time(NULL);

        fout << "\nCurrent time = " << current_time;

        fout.close();
    }
}

void set_pregame_data(bool& user_goes_first, bool& x_represents_user, thread* thread_ptr)
{
    if (talk_with_python)
    {
        user_goes_first = get_value_from_Python_textfile("who should go first",
                                                         thread_ptr);
        // Note that a value of 1 from Python means the user should go first,
        // and a value of 0 means the computer should.

        x_represents_user = get_value_from_Python_textfile("what piece to play",
                                                           thread_ptr);
    }

    else
    {
        cout << "Enter y to go first: ";

        string user_input = "";

        cin >> user_input;
        cin.clear();
        cin.ignore(INT_MAX, '\n');

        if (user_input == "y" || user_input == "Y")
        {
            user_goes_first = true;
        }

        else
        {
            user_goes_first = false;
        }

        cout << "Enter X to play X; otherwise, enter O (or anything else): ";

        cin >> user_input;
        cin.clear();
        cin.ignore(INT_MAX, '\n');

        if (user_input == "x" || user_input == "X")
        {
            x_represents_user = true;
        }

        else
        {
            x_represents_user = false;
        }
    }
}

void play_game(vector<vector<coordinate>>& moves_reaching_starting_positions, int& num_comp_wins_ongoing, int& num_user_wins_ongoing,
               int& num_draws_ongoing, bool play_from_the_starting_position, bool playing_first_game, bool& end_program)
{
    thread t1(use_thread_to_reset_TT);

    thread* t1_ptr = &t1;

    write_to_Python_textfile(playing_first_game);
    // A value of 1 would signify to the python interface that this is the first game.

    if (!playing_first_game)
    {
        int user_input;

        if (talk_with_python)
        {
            user_input = get_value_from_Python_textfile("whether to play again",
                                                        t1_ptr);
        }

        else
        {
            cout << "To play again, press 1 and enter: ";

            cin >> user_input;
        }

        if (user_input != 1)
        {
            end_program = true;

            t1.join();

            return;
        }
    }

    bool user_goes_first = false;
    bool x_represents_user = false;

    set_pregame_data(user_goes_first, x_represents_user, t1_ptr);

    // Now to pick a set of moves for the comp and user to play:

    int random_index = rand() % moves_reaching_starting_positions.size();

    vector<coordinate> chosen_set_of_moves = moves_reaching_starting_positions[random_index];

    remove_set_at_index(moves_reaching_starting_positions, random_index); // function will replace the bad set with the last set in the vector
                                                                          // and then pop_back.
    t1.join();

    unique_ptr<position> pos = get_to_chosen_starting_position(!user_goes_first, chosen_set_of_moves, play_from_the_starting_position);

    vector<vector<char>> assisting_board = pos->get_board();

    if (talk_with_python)
    {
        write_squares_to_file(assisting_board, x_represents_user);
    }

    else
    {
        // cout << "\nSTARTING POSITION:\n";

        display_board(assisting_board, x_represents_user, true, {position::UNDEFINED, position::UNDEFINED});
        // UNDEFINED for last_move since this is the starting position.
    }

    if (!user_goes_first)
    {
        wait(0.2);
    }

    bool on_first_loop_iteration = true;

    while (!pos->did_computer_win() && !pos->did_opponent_win() && !pos->is_game_drawn())
    {
        if (!on_first_loop_iteration && talk_with_python)
        {
            // Write to the python textfile that the game is not over:

            write_to_Python_textfile(0);
        }

        if (pos->get_is_comp_turn()) // computer's turn:
        {
            coordinate best_move = pos->find_best_move_for_comp();
            // This is the move the computer should play in this position.

            assisting_board[best_move.row][best_move.col] = 'C';

            double old_thinking_time = position::thinking_time;

            position::thinking_time = 0;

            pos = position::think_on_game_position(assisting_board, false, best_move, pos->get_squares_amplifying_comp_2(),
                                                   pos->get_squares_amplifying_comp_3(), pos->get_squares_amplifying_user_2(),
                                                   pos->get_squares_amplifying_user_3(), false, placeholder, false);

            position::thinking_time = old_thinking_time;

            if (talk_with_python)
            {
                write_squares_to_file(assisting_board, x_represents_user);
                /*
                if (!pos->did_computer_win() && !pos->did_opponent_win() &&
                    !pos->is_game_drawn())
                {
                    write_to_Python_textfile(0); // Game is not over.
                }

                else
                {
                    write_to_Python_textfile(1); // Game is over.
                }
                */
            }

            else
            {
                cout << "\n\n";

                display_board(assisting_board, x_represents_user, false, best_move);
            }
        }

        else // user's turn:
        {
            if (!talk_with_python)
            {
                cout << "\n\n\n";
            }

            int undefined_value = position::UNDEFINED;

            coordinate predicted_move = {position::UNDEFINED, position::UNDEFINED};

            coordinate* predicted_move_ptr = &predicted_move;

            coordinate best_reply = {position::UNDEFINED, position::UNDEFINED};

            coordinate* best_reply_ptr = &best_reply;

            thread t2(think_while_user_is_thinking, pos->get_board(), false, pos->get_last_move(),
                      pos->get_squares_amplifying_comp_2(), pos->get_squares_amplifying_comp_3(),
                      pos->get_squares_amplifying_user_2(), pos->get_squares_amplifying_user_3(), false,
                      predicted_move_ptr, best_reply_ptr);

            thread* t2_ptr = &t2;

            int col;

            if (talk_with_python)
            {
                col = get_value_from_Python_textfile("user's move", t2_ptr);

                /* while (assisting_board[0][col] != ' ')
                {
                    Commenting this portion of the code out because
                    in the Python program I'll already be checking if
                    the move was valid. No need to do it here.

                    write_to_Python_textfile(0); // signifying an invalid move.

                    // Invalid move, since the column the user entered is full.

                    cout << "That column is full, please try again.\n";

                    col = get_value_from_Python_textfile(60, "user's move", t2_ptr);
                } */

                // write_to_Python_textfile(1); // signifying a valid move.
            }

            else
            {
                col = get_column_user_wants_to_move_in(pos);
            }

            // Now to make the user's move on the assisting_board:

            int row = 5;

            coordinate move_chosen_by_user{undefined_value, undefined_value};

            while (row >= 0)
            {
                if (assisting_board[row][col] == ' ') // empty square... move a piece here:
                {
                    assisting_board[row][col] = 'U';

                    move_chosen_by_user.row = row;

                    move_chosen_by_user.col = col;

                    break;
                }

                row --;
            }

            if (row < 0 || move_chosen_by_user.row == position::UNDEFINED)
            {
                ofstream fout("ThisIsTheErrorYou'reLookingFor.txt");

                fout << "That error, \"row variable in play_game() is smaller than...\"";
                fout << "was thrown. The col index the user apparently wanted to move into was ";
                fout << col << ", and after the while loop the value of row was ";
                fout << row << "\n";

                fout.close();

                throw runtime_error("row variable in play_game() is smaller than 0, or there is no move chosen by the user recorded.\n");
            }

            if (talk_with_python)
            {
                write_squares_to_file(assisting_board, x_represents_user);
                /*
                if (!pos->did_computer_win() && !pos->did_opponent_win() &&
                    !pos->is_game_drawn())
                {
                    write_to_Python_textfile(0); // Game is not over.
                }

                else
                {
                    write_to_Python_textfile(1); // Game is over.
                }
                */
            }

            else
            {
                display_board(assisting_board, x_represents_user, false, move_chosen_by_user);

                cout << "\n";
            }

            position::stop_signal = true; // tells the computer to stop thinking while waiting for the user, since the
                                          // user has just made their move.

            t2.join();

            // cout << "Computer stopped musing\n";

            position::stop_signal = false;

            // Now, see if the computer predicted correctly, and if it had time to think of a good reponse:

            if (predicted_move == move_chosen_by_user && best_reply.row != position::UNDEFINED)
            {
                // The computer predicted correctly, and has a reply ready.
                // A corolarry of this is that the game must not be over.
                // So, send this to the Python file now, since the normal turn flow
                // of the while loop is altered here (on the loop's next iteration,
                // it should again be the user's turn).

                if (talk_with_python)
                {
                    write_to_Python_textfile(0); // Game is not over.
                }

                // cout << "predicted right and generated a response\n";

                // Now to make the user's move, and then the comp's move, both with thinking_time = 0 (to keep the
                // amplifying vectors updated).

                double old_thinking_time = position::thinking_time;

                position::thinking_time = 0;

                pos = position::think_on_game_position(assisting_board, true, move_chosen_by_user,
                                                       pos->get_squares_amplifying_comp_2(), pos->get_squares_amplifying_comp_3(),
                                                       pos->get_squares_amplifying_user_2(), pos->get_squares_amplifying_user_3(),
                                                       false, placeholder, false);

                if (best_reply.row < 0 || best_reply.row > 5 || best_reply.col < 0 || best_reply.col > 6 ||
                    assisting_board[best_reply.row][best_reply.col] != ' ')
                {
                    throw runtime_error("Computer came up with an illegal move");
                }

                assisting_board[best_reply.row][best_reply.col] = 'C';

                pos = position::think_on_game_position(assisting_board, false, best_reply,
                                                       pos->get_squares_amplifying_comp_2(), pos->get_squares_amplifying_comp_3(),
                                                       pos->get_squares_amplifying_user_2(), pos->get_squares_amplifying_user_3(),
                                                       false, placeholder, false);

                position::thinking_time = old_thinking_time;

                if (talk_with_python)
                {
                    write_squares_to_file(assisting_board, x_represents_user);
                    /*
                    if (!pos->did_computer_win() && !pos->did_opponent_win() &&
                        !pos->is_game_drawn())
                    {
                        write_to_Python_textfile(0); // Game is not over.
                    }

                    else
                    {
                        write_to_Python_textfile(1); // Game is over.
                    }
                    */
                }

                else
                {
                    cout << "\n\n";

                    display_board(assisting_board, x_represents_user, false, best_reply);
                }
            }

            else
            {
                if (predicted_move == move_chosen_by_user && best_reply.row == position::UNDEFINED)
                {
                    // cout << "predicted right but couldn't generate a response in time\n";
                }

                if (predicted_move != move_chosen_by_user)
                {
                   // cout << "did not predict the user's move\n";

                   // cout << "predicted move was (" << char('A' + predicted_move.col) << (6 - predicted_move.row) << ")\n";
                }

                pos = position::think_on_game_position(assisting_board, true, move_chosen_by_user,
                                                       pos->get_squares_amplifying_comp_2(), pos->get_squares_amplifying_comp_3(),
                                                       pos->get_squares_amplifying_user_2(), pos->get_squares_amplifying_user_3(),
                                                       false, placeholder, false);
            }
        }

        on_first_loop_iteration = false;
    }

    if (talk_with_python)
    {
        write_to_Python_textfile(1); // The game is over.
    }

    // At this point, the game has ended. I should display the winner:

    string message;

    int game_result;

    if (pos->did_computer_win())
    {
        message = "The computer won!\n\n";

        num_comp_wins_ongoing ++;

        game_result = 1;
    }

    else if (pos->did_opponent_win())
    {
        message = "You won!\n\n";

        num_user_wins_ongoing ++;

        game_result = 2;
    }

    else
    {
        message = "The game is a draw!\n\n";

        num_draws_ongoing ++;

        game_result = 3;
    }

    if (talk_with_python)
    {
        write_to_Python_textfile(game_result);
    }

    else
    {
        cout << "\n\n\n" << message;
    }
}

void read_file_into_vector(vector<vector<coordinate>>& vec)
{
    ifstream fin("MovesReachingPositions.txt");

    if (fin.fail())
    {
        throw runtime_error("fin failed to read file in read_file_into_vector() function.\n");
    }

    coordinate current_move_found;
    current_move_found.row = position::UNDEFINED;
    current_move_found.col = position::UNDEFINED;

    vector<coordinate> empty_vec;

    vec.push_back(empty_vec);

    char c = fin.get();

    while (!fin.eof())
    {
        if (c - '0' >= 0 && c - '0' <= 9) // is a digit, so work with it...
        {
            if (current_move_found.row == position::UNDEFINED)
            {
                current_move_found.row = c - '0';
            }

            else
            {
                current_move_found.col = c - '0';

                vec[vec.size()-1].push_back(current_move_found);

                // Now reset current_move_found:

                current_move_found.row = position::UNDEFINED;
                current_move_found.col = position::UNDEFINED;
            }
        }

        else if (c == '\n')
        {
            // About to be a new line in the file (representing a set of moves), so create a new spot in the param vector:

            vec.push_back(empty_vec);

            // Note that if that '\n' was the last newline character in the file, then vec's last element will be an empty vector.

            // So, I take care of this near the end of the function.
        }

        else if (c != '(' && c != ')' && c != ',' && c != ' ')
        {
            // c doesn't equal any of the other allowable characters in the file. Something's wrong:

            throw runtime_error("Found an unexpected character in the file in read_file_into_vector()\n");
        }

        c = fin.get();
    }

    if (vec[vec.size()-1].size() == 0)
    {
        // The last set of moves in vec is empty, which makes sense.

        vec.pop_back();
    }
}

bool are_all_moves_valid(const vector<vector<coordinate>>& vec)
{
    for (const vector<coordinate>& current_set: vec)
    {
        if (current_set.empty() || current_set.size() < 4 || current_set.size() > 9)
        {
            return false;
        }

        for (const coordinate& current_move: current_set)
        {
            if (current_move.row < 0 || current_move.row > 5 || current_move.col < 0 || current_move.col > 6)
            {
                return false;
            }
        }
    }

    return true;
}

void read_in_file_of_ongoing_stats(int& num_comp_wins_ongoing, int& num_user_wins_ongoing, int& num_draws_ongoing)
{
    ifstream fin("OngoingScore.txt");

    if (fin.fail())
    {
        throw runtime_error("Could not read in the OngoingScore.txt file.\n");
    }

    fin >> num_comp_wins_ongoing >> num_user_wins_ongoing >> num_draws_ongoing;

    fin.close();
}

void write_updated_score_to_file(int num_comp_wins_ongoing, int num_user_wins_ongoing, int num_draws_ongoing)
{
    ofstream fout("OngoingScore.txt");

    if (fout.fail())
    {
        throw runtime_error("Could not make an updated version of the OngoingScore.txt file.\n");
    }

    fout << num_comp_wins_ongoing << '\n' << num_user_wins_ongoing << '\n' << num_draws_ongoing << '\n';

    fout.close();
}

void remove_existing_communication_textfiles()
{
    int starting_num = initial_textfile_num;

    string filename = to_string(starting_num) + ".txt";

    while (remove(filename.c_str()) == 0)
    {
        // cout << "Removed " << filename << "\n";

        starting_num ++;

        filename = to_string(starting_num) + ".txt";
    }

    // this_thread::sleep_for(std::chrono::milliseconds(10000));
}

vector<vector<coordinate>> convert_to_coordinate_form(const vector<vector<int>>& vec)
{
    vector<vector<coordinate>> result;

    for (const vector<int>& current_sub_vec: vec)
    {
        // To figure out which square each col refers to, I need to
        // keep track of the current available square in each column.

        vector<coordinate> coordinate_form_of_sub_vec;

        vector<coordinate> available_squares;

        for (int i = 0; i < 7; i++)
        {
            available_squares.push_back({5, i});
        }

        // Here, available_squares[0] represents the available square in column 0,
        // while available_squares[6] represents the available square in column 6.

        for (int current_col_move: current_sub_vec)
        {
            if (available_squares[current_col_move].row < 0 ||
                available_squares[current_col_move].row > 6)
            {
                throw runtime_error("Bad square\n");
            }

            coordinate_form_of_sub_vec.push_back(available_squares[current_col_move]);

            available_squares[current_col_move].row--;
        }

        result.push_back(coordinate_form_of_sub_vec);
    }

    return result;
}

void merge_second_into_first(vector<vector<coordinate>>& first,
                             const vector<vector<coordinate>>& second)
{
    for (const vector<coordinate>& current: second)
    {
        first.push_back(current);
    }
}

void find_fair_starting_sets_using_DB(vector<vector<coordinate>>& sets_of_moves)
{
    // This function will return starting sets of moves, leading to
    // positions between 4 and 7 ply, that are between a -10 and 10 evaluation
    // (according to the DB).

    for (int ply = 4; ply <= 7; ply++)
    {
        vector<vector<int>> current_set_with_int_form =
        Database_Functions::get_starting_sets(ply, -10, 10, 20);

        vector<vector<coordinate>> current_set_with_coordinate_form =
        convert_to_coordinate_form(current_set_with_int_form);

        merge_second_into_first(sets_of_moves, current_set_with_coordinate_form);
    }

    if (sets_of_moves.size() != 80)
    {
        throw runtime_error("Not the right number of sets of moves\n");
    }
}

int main(int argc, char* argv[])
{
    // The point of putting the whole program in a try block is to catch a specific
    // error, which occurs when no communication has happened with the Python
    // interface for some time (since this likely means the user closed the program).

    // When this happens, I want the thrown error to bubble up to main, and then
    // I can cleanly end the whole program/exe in the catch block.

    try {

    if (argc == 1)
    {
        talk_with_python = false;
    }

    else if (argc == 2)
    {
        if (string(argv[1]) == "Python")
        {
            talk_with_python = true;
        }

        else
        {
            throw (string("The second command line argument is:\n") + argv[1]);
        }
    }

    else
    {
        throw ("Bad number of command line arguments.");
    }

    srand(time(NULL));

    vector<vector<coordinate>> moves_reaching_starting_positions;
    // Will store sets of fair starting moves, leading to positions between
    // 4 and 7 ply.

    // There will be 20 starting sets of moves for each ply.
    // So 80 sets total.

    thread db_thread(find_fair_starting_sets_using_DB,
                     ref(moves_reaching_starting_positions));

    remove_existing_communication_textfiles();

    int num_comp_wins_ongoing = 0;
    int num_user_wins_ongoing = 0;
    int num_draws_ongoing = 0;

    read_in_file_of_ongoing_stats(num_comp_wins_ongoing, num_user_wins_ongoing, num_draws_ongoing);

    if (talk_with_python)
    {
        write_to_Python_textfile(num_comp_wins_ongoing);
        write_to_Python_textfile(num_user_wins_ongoing);
        write_to_Python_textfile(num_draws_ongoing);
    }

    else
    {
        cout << "Current ongoing match score:\nComp has won " << num_comp_wins_ongoing;
        cout << " games.\nYou have won " << num_user_wins_ongoing;
        cout << " games.\nThere have been " << num_draws_ongoing << " draws.\n";

        cout << "Enter approximately how long you want the Engine to think on each move: ";
    }

    if (talk_with_python)
    {
        int time_in_milliseconds = get_value_from_Python_textfile("thinking time",
                                                                  nullptr);

        position::thinking_time = (double(time_in_milliseconds)) / 1000.0;
    }

    else
    {
        cin >> position::thinking_time;
    }

    db_thread.join();

    if (!are_all_moves_valid(moves_reaching_starting_positions))
    {
        throw runtime_error("Found invalid move(s) in the moves_reaching_starting_positions vector in main()\n");
    }

    char play_starting_position_input = ' ';

    if (talk_with_python)
    {
        int val = get_value_from_Python_textfile("starting position choice", nullptr);

        if (val == 1)
        {
            play_starting_position_input = 's';
        }
    }

    else
    {
        cout << "To play only the starting position, press s: ";

        cin >> play_starting_position_input;
    }

    bool only_playing_from_starting_position = play_starting_position_input == 's';

    if (only_playing_from_starting_position)
    {
        // Make the moves_reaching_starting_positions vector only contain moves to the starting board:

        moves_reaching_starting_positions.clear();

        vector<coordinate> empty_vec;

        for (int i = 0; i < 10000; i++)
        {
            moves_reaching_starting_positions.push_back(empty_vec);
        }
    }

    bool end_program = false;

    bool playing_first_game = true;

    while (true)
    {
        if (moves_reaching_starting_positions.empty()) // Somehow all starting positions have been played out, so read in file again...
        {
            // read_file_into_vector(moves_reaching_starting_positions);
            find_fair_starting_sets_using_DB(moves_reaching_starting_positions);
        }

        play_game(moves_reaching_starting_positions, num_comp_wins_ongoing, num_user_wins_ongoing, num_draws_ongoing,
                  only_playing_from_starting_position, playing_first_game, end_program);

        playing_first_game = false;

        if (end_program)
        {
            break;
        }

        if (!only_playing_from_starting_position)
        {
            write_updated_score_to_file(num_comp_wins_ongoing, num_user_wins_ongoing, num_draws_ongoing);

            if (!talk_with_python)
            {

                cout << "Current ongoing match score:\nComp has won ";
                cout << num_comp_wins_ongoing << " games.\nYou have won ";
                cout << num_user_wins_ongoing << " games.\nThere have been ";
                cout << num_draws_ongoing << " draws.\n\n";
            }
        }
    }

    }

    catch (string e) {
        // If control reaches here, it's a case of the user prematurely
        // closing the Python interface (or just not entering any input after some time).

        // Now, before ending the program, I want to make sure all other threads
        // have time to stop.

        // Other than waiting a few seconds, I will also set position::stop_signal
        // to true, since the thread that gets the computer to think while
        // the user thinks could be running in its 10 second calculation right now.

        // cout << "In catch block...\n";

        // Print the error message to a special file, denoted by whatever the current
        // time is.

        int current_time = time(NULL);

        string filename = "Crash report file at time " + to_string(current_time) + ".txt";

        ofstream fout(filename);

        fout << e;

        fout.close();

        remove_existing_communication_textfiles();

        return 0;
    }

    int current_time = time(NULL);

    string filename = "Crash report file at time " + to_string(current_time) + ".txt";

    ofstream fout(filename);

    fout << "The program reached its natural end.";

    fout.close();

    remove_existing_communication_textfiles();
}

