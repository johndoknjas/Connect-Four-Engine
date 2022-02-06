#pragma once

#include <sqlite3.h>
#include <stdexcept>
#include <cstdlib>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <random>

using namespace std;

class Wrapper
{
    // By wrapping the sqlite3* pointer in this Wrapper class, I can automatically
    // initialize it (with sqlite3_open) and close it (with sqlite3_close), using
    // the constructor and destructor.

    // This allows me to create a static object of Wrapper, the Database_Functions class,
    // without extra hassle. If I had created a static sqlite3* pointer in the
    // Database_Functions class instead, it would be trickier to open and close
    // the associated database, as it's a static object (with no constructor/destructor).

public:
    Wrapper();
    ~Wrapper();

    sqlite3* DB_pointer;
};

Wrapper::Wrapper()
{
    // Open up the file "database_directory.txt", to find the
    // absolute directory of where Database C is.

    ifstream fin("directory.txt");

    string absolute_path;

    if (fin.good())
    {
        getline(fin, absolute_path);
    }

    else
    {
        throw runtime_error("!fin.good()\n");
    }

    fin.close();

    if (absolute_path[absolute_path.size()-1] == ' ' ||
        absolute_path[absolute_path.size()-1] == '\t')
    {
        throw runtime_error("Bad file data\n");
    }

    int signal;

    signal = sqlite3_open(/*"Database_C.db"*/absolute_path.c_str(), &DB_pointer);

    if (signal)
    {
        throw runtime_error("Problem opening database C.\n");
    }
}

Wrapper::~Wrapper()
{
    sqlite3_close(DB_pointer);
}

class Database_Functions
{
public:
    static Wrapper DB;

    static int callback(void* data, int argc, char** argv, char** azColName);

    static void get_info_for_position(const vector<vector<char>>& boardP,
                                      bool is_comp_turnP, int& evaluation,
                                      int& best_move_col_index);

    static string convert_board_to_string(const vector<vector<char>>& boardP);

    static vector<vector<int>> get_starting_sets(int ply, int low_eval_bound,
                                                 int high_eval_bound, int num_sets_wanted);

};

Wrapper Database_Functions::DB; // calls the default constructor of Wrapper.

int Database_Functions::callback(void* data, int argc, char** argv, char** azColName)
{
    vector<string> row_being_examined;

    for (int i = 0; i < argc; i++) {

        row_being_examined.push_back(argv[i]);
    }

    ((vector<vector<string>>*)data)->push_back(row_being_examined);

    return 0; // Necessary to avoid sql saying there is an error.
              // The zero says that the function worked fine.
}

void Database_Functions::get_info_for_position(const vector<vector<char>>& boardP,
                                               bool is_comp_turnP, int& evaluationP,
                                               int& best_move_col_index)
{
    string string_rep = convert_board_to_string(boardP);

    char whose_turn = is_comp_turnP ? 'C' : 'U';

    string select_statement = "";
    select_statement += "SELECT evaluation, best_move FROM Positions ";
    select_statement += "WHERE string_rep = '";
    select_statement += string_rep;
    select_statement += "' and whose_turn = '";
    select_statement += whose_turn;
    select_statement += "';";

    vector<vector<string>> result_from_select;

    vector<vector<string>>* vec_ptr = &result_from_select;

    int signal;

    char* error_message;

    signal = sqlite3_exec(DB.DB_pointer, select_statement.c_str(), callback,
                          vec_ptr, &error_message);

    if (signal != SQLITE_OK)
    {
        cout << "Error - from sqlite: " << error_message << "\n";

        throw runtime_error("Something went wrong in get_info_for_position()\n");
    }

    if (result_from_select.size() != 1 || result_from_select[0].size() != 2)
    {
        throw runtime_error("Problem with the size of the SELECT\n");
    }

    evaluationP = stoi(result_from_select[0][0]);

    best_move_col_index = stoi(result_from_select[0][1]);
}

string Database_Functions::convert_board_to_string(const vector<vector<char>>& boardP)
{
    string result;

    for (int r = 0; r < 6; r++)
    {
        for (int c = 0; c < 7; c++)
        {
            result += boardP[r][c];
        }
    }

    return result;
}

vector<vector<int>> Database_Functions::get_starting_sets(int ply, int low_eval_bound,
                                                          int high_eval_bound, int num_sets_wanted)
{
    string select_statement = "";
    select_statement += "SELECT string_of_moves FROM Positions ";
    select_statement += "WHERE ply_number = ";
    select_statement += to_string(ply);
    select_statement += " and evaluation >= ";
    select_statement += to_string(low_eval_bound);
    select_statement += " and evaluation <= ";
    select_statement += to_string(high_eval_bound);
    select_statement += " and whose_turn = 'C';";
    // Only reason for whose_turn = 'C' is to avoid duplicate strings of moves,
    // since there is another string where it's the user's turn in the
    // resulting position.
        // This function just returns the string of moves, so I could have
        // written whose_turn = 'U' instead and it would be fine.

    vector<vector<string>> result_from_select;

    vector<vector<string>>* vec_ptr = &result_from_select;

    int signal;

    char* error_message;

    signal = sqlite3_exec(DB.DB_pointer, select_statement.c_str(), callback,
                          vec_ptr, &error_message);

    random_device rd;
    mt19937 g(rd());
    shuffle(result_from_select.begin(), result_from_select.end(), g);

    result_from_select.resize(num_sets_wanted);

    if (signal != SQLITE_OK)
    {
        cout << "Error - from sqlite: " << error_message << "\n";

        throw runtime_error("Something went wrong in get_starting_sets()\n");
    }

    if (result_from_select[0].size() != 1 ||
        result_from_select[result_from_select.size()-1].size() != 1)
    {
        throw runtime_error("Not 1 element in a SELECT\n");
    }

    vector<vector<int>> return_object;

    for (const vector<string>& current: result_from_select)
    {
        string string_of_moves = current[0]; // Only element in this sub-list.

        vector<int> new_form;

        for (char c: string_of_moves)
        {
            new_form.push_back(c - '0');
        }

        return_object.push_back(new_form);
    }

    return return_object;
}

