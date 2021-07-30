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

pair<vector<vector<int>>, double> run_sequentially(vector<vector<int>> init_board, int num_iters=100, int radius=1)
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
        double duration = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
        
        return make_pair(init_board, duration);
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

pair<vector<vector<int>>, double> run_in_parallel(vector<vector<int>> init_board, int num_row_threads=2, int num_col_threads=2, \
    int num_iters=100, int radius=1){
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

    auto elapsed = std::chrono::system_clock::now() - start;
    double duration = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();

    if(num_iters%2 == 0) return make_pair(even_board, duration);
    else return make_pair(odd_board, duration);
}


pair<vector<vector<int>>, double> run_in_parallel_ff(vector<vector<int>> init_board, int num_row_threads=2, \
    int num_col_threads=2, int num_iters=100, int radius=1){

    // auto start = std::chrono::system_clock::now();

    pair<int, int> size(init_board.size(), init_board[0].size());
    vector<vector<int>> even_board = init_board;
    vector<vector<int>> odd_board = init_board;

    ffTime(START_TIME);

    ParallelFor pf(num_row_threads * num_col_threads);

    for(int iter=0; iter < num_iters; iter++){
        pf.parallel_for(0, size.first, [&](int i){
            for(int j=0; j<size.second; j++){
                if(iter%2 == 0){
                    odd_board[i][j] = rules_func(extract_matrix(even_board, make_pair(i, j), radius));
                } else{
                    even_board[i][j] = rules_func(extract_matrix(odd_board, make_pair(i, j), radius));
                }
            }
        }, num_row_threads * num_col_threads);
    }

    // auto elapsed = std::chrono::system_clock::now() - start;
    // double duration = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();

    ffTime(STOP_TIME);

    double duration = ffTime(GET_TIME) * 1000;  // Couldn't find anything in FastFlow to report time in usec.

    return (num_iters%2 == 0 ? make_pair(even_board, duration) : make_pair(odd_board, duration));
}
