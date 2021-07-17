#include <iostream>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <vector>
#include <thread>

#include "config.h"
#include "utimer.cpp"

using namespace std;

vector<vector<int>> run_in_parallel_omp(vector<vector<int>> init_board, int num_row_threads=2, int num_col_threads=2, int num_iters=100, int radius=1)
    {
        auto start = std::chrono::system_clock::now();
        pair<int, int> size(init_board.size(), init_board[0].size());
        vector<vector<int>> board = init_board;
        # pragma omp parallel for num_threads(num_row_threads)
        for(int iter=0; iter<num_iters; iter++){
            # pragma omp parallel for num_threads(num_col_threads)
            for(int i=0; i<size.first; i++){
                for(int j=0; j<size.second; j++){
                    pair<int, int> center(i, j);
                    board[i][j] = rules_func(
                        extract_matrix(init_board, center, radius)
                    );
                }
            }
            init_board = board;
        }
        auto elapsed = std::chrono::system_clock::now() - start;
        auto musec = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
        cout<<"Parallel task took " << musec << " to finish.\n";
        return(init_board);
    }

vector<vector<int>> run_sequentially(vector<vector<int>> init_board, int num_iters=100, int radius=1)
    {
        auto start = std::chrono::system_clock::now();
        pair<int, int> size(init_board.size(), init_board[0].size());
        vector<vector<int>> board = init_board;
        for(int iter=0; iter<num_iters; iter++){
            for(int i=0; i<size.first; i++){
                for(int j=0; j<size.second; j++){
                    pair<int, int> center(i, j);
                    board[i][j] = rules_func(
                        extract_matrix(init_board, center, radius)
                    );
                }
            }
            init_board = board;
        }
        auto elapsed = std::chrono::system_clock::now() - start;
        auto musec = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
        cout<<"Sequential task took " << musec << " to finish.\n";
        return(init_board);
    }

auto compute_chunk(vector<vector<int>> &old_board, vector<vector<int>> &new_board, \
    pair<int, int> top_left, pair<int, int> bottom_right, int radius){
    for(int i=top_left.first; i<bottom_right.first; i++){
        for(int j=top_left.second; j<bottom_right.second; j++){
                pair<int, int> center(i, j);
                new_board[i][j] = rules_func(extract_matrix(old_board, center, radius));
        }
    }
    return;
}

auto test_func(int x){
    cout<<x;
}

vector<vector<int>> run_in_parallel(vector<vector<int>> init_board, int num_row_threads=2, \
    int num_col_threads=2, int num_iters=100, int radius=1){
    
    auto start = std::chrono::system_clock::now();
    pair<int, int> size(init_board.size(), init_board[0].size());
    vector<vector<int>> even_board = init_board;
    vector<vector<int>> odd_board = init_board;

    int w_chunk_size = size.first/num_row_threads;
    int h_chunk_size = size.second/num_col_threads;

    vector<vector<pair<int, int>>> top_lefts(num_row_threads, vector<pair<int, int>>(num_col_threads));
    vector<vector<pair<int, int>>> bottom_rights(num_row_threads, vector<pair<int, int>>(num_col_threads));

    for(int i=0; i<num_row_threads; i++){
        for(int j=0; j<num_col_threads; j++){
            pair<int, int> top_left(w_chunk_size * i, h_chunk_size * j);

            int br_x, br_y;  // br stands for bottom right
            if(i == num_row_threads -1) br_x = size.first;
            else br_x = w_chunk_size * (i+1);
            if(j == num_col_threads -1) br_y = size.second;
            else br_y = h_chunk_size * (j+1);

            pair<int, int> bottom_right(br_x, br_y);

            top_lefts[i][j] = top_left;
            bottom_rights[i][j] = bottom_right;
        }

    }
    
    // vector<vector<thread>> tids(num_row_threads, vector<thread>(num_col_threads));
    vector<thread> tids;

    for(int iter=0; iter<num_iters; iter++){
        for(int i=0; i<num_row_threads; i++){
            for(int j=0; j<num_col_threads; j++){
                if(iter % 2 == 0)
                    tids.push_back(thread(compute_chunk, &even_board, &odd_board, top_lefts[i][j], bottom_rights[i][j], 1));
                else
                    tids.push_back(thread(compute_chunk, &odd_board, &even_board, top_lefts[i][j], bottom_rights[i][j], 1));
            }
        }
        for(thread& t: tids) t.join();
    }

    if(num_iters%2 == 0) return even_board;
    else return odd_board;
}

int main() {

    int board_w, board_h;

    cout<< "Let me know how many rows and columns the board must have:\n> ";
    cin >> board_w >> board_h;
    pair<int, int> board_size(board_w, board_h);

    cout<< "Do you want me to generate a random board for you? (Y|n) ";
    char rand_board_yn;
    cin >> rand_board_yn;

    vector<vector<int>> board(board_w, vector<int>(board_h));
    if(rand_board_yn == 'n' || rand_board_yn == 'N') {
        cout<< "OKay! Now enter the initial state of the board:\n";
        board = read_matrix(board);
    } else {
        fill_matrix_randomly(board);
        cout<<"Done!\n";
    }

    // write_matrix(board);

    // vector<vector<int>> seq_result = run_sequentially(board, 5);
    vector<vector<int>> par_result = run_in_parallel(board, 2, 2, 5);

    // cout<<"--------------\n";
    // write_matrix(seq_result);

    cout<<"--------------\n";
    write_matrix(par_result);

    // cout << "Parallel and sequential outputs are " << (check_matrix_equality(seq_result, par_result) ? "" : "NOT ") << "equal.\n";

    return(0);
}
