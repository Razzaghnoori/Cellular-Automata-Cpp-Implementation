#include <iostream>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <vector>
#include <thread>
#include <ff/ff.hpp>
#include <ff/parallel_for.hpp>

#include "config.h"
#include "utimer.cpp"

using namespace std;
using namespace ff;


vector<vector<int>> run_in_parallel_omp(vector<vector<int>> init_board, int num_row_threads=2, int num_col_threads=2, int num_iters=100, int radius=1)
    {
        // Didn't work. Output different from that of the sequential one.
        // Deprecated. 
        
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
        utimer timer("Sequential task");
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

vector<vector<int>> run_in_parallel(vector<vector<int>> init_board, int num_row_threads=2, int num_col_threads=2, int num_iters=100, int radius=1){
    utimer timer("Parallel task");
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
    
    for(int iter=0; iter<num_iters; iter++){
        vector<thread> tids;
        for(int i=0; i<num_row_threads; i++){
            for(int j=0; j<num_col_threads; j++){
                if(iter % 2 == 0){
                    auto chunk_func = [&even_board, &odd_board](pair<int, int> tl, pair<int, int> br) {
                        return compute_chunk(even_board, odd_board, tl, br, 1);
                    };
                    tids.push_back(thread(chunk_func, top_lefts[i][j], bottom_rights[i][j]));
                }
                else{
                    auto chunk_func = [&even_board, &odd_board](pair<int, int> tl, pair<int, int> br) {
                        return compute_chunk(odd_board, even_board, tl, br, 1);
                    };

                    tids.push_back(thread(chunk_func, top_lefts[i][j], bottom_rights[i][j]));
                }
            }
        }
        for(thread& t: tids) t.join();
    }

    if(num_iters%2 == 0) return even_board;
    else return odd_board;
}

vector<vector<int>> run_in_parallel_ff(vector<vector<int>> init_board, int num_row_threads=2, int num_col_threads=2, int num_iters=100, int radius=1){
    utimer timer("Parallel task using FastFlow");
    pair<int, int> size(init_board.size(), init_board[0].size());
    vector<vector<int>> even_board = init_board;
    vector<vector<int>> odd_board = init_board;

    ParallelFor outter_pf;
    // ParallelFor inner_pf;


    for(int iter=0; iter < num_iters; iter++){
        outter_pf.parallel_for(0, size.first, [&](int i){
#if 0
            outter_pf.parallel_for(0, size.second, [&](int j){
                if(iter%2 == 0){
                    odd_board[i][j] = rules_func(extract_matrix(even_board, make_pair(i, j), radius));
                } else{
                    even_board[i][j] = rules_func(extract_matrix(odd_board, make_pair(i, j), radius));
                }
            }, num_col_threads);
#else
            for(int j=0; j<size.second; j++){
                if(iter%2 == 0){
                    odd_board[i][j] = rules_func(extract_matrix(even_board, make_pair(i, j), radius));
                } else{
                    even_board[i][j] = rules_func(extract_matrix(odd_board, make_pair(i, j), radius));
                }
            }
#endif
        }, num_row_threads * num_col_threads);
    }

    return (num_iters%2 == 0 ? even_board : odd_board);
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
        cout<<"Matrix filled with random values!\n";
    }

    int num_row_threads, num_col_threads, num_iters;
    cout << "How many threads do you need on rows and columns respectively? \n> ";
    cin >> num_row_threads >> num_col_threads;

    cout << "Finally, how many iteration should it run for?\n> ";
    cin >> num_iters;

    vector<vector<int>> seq_result = run_sequentially(board, num_iters);
    vector<vector<int>> par_result = run_in_parallel(board, num_row_threads, num_col_threads, num_iters);
    vector<vector<int>> ff_par_result = run_in_parallel_ff(board, num_row_threads, num_col_threads, num_iters);


    // cout<<"--------------\n";
    // write_matrix(seq_result);

    // cout<<"--------------\n";
    // write_matrix(par_result);

    cout << "Parallel and sequential outputs are " << (check_matrix_equality(seq_result, par_result) ? "" : "NOT ") << "equal.\n";
    cout << "Parallel outputs using native c++ threads and FastFlow are " << (check_matrix_equality(par_result, ff_par_result) ? "" : "NOT ") << "equal.\n";

    return(0);
}
