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
    int first_row, int last_row, int radius=1){
    pair<int, int> size(old_board.size(), old_board[0].size());
    for(int i=first_row; i<last_row; i++){
        for(int j=0; j<size.second; j++){
            pair<int, int> center(i, j);
            new_board[i][j] = rules_func(extract_matrix(old_board, center, radius));
        }
    }
    return;
}

pair<vector<vector<int>>, double> run_in_parallel(vector<vector<int>> init_board, int num_threads=2, \
    int num_iters=100, int radius=1){
    pair<int, int> size(init_board.size(), init_board[0].size());
    vector<vector<int>> even_board = init_board;
    vector<vector<int>> odd_board = init_board;

    auto start = std::chrono::system_clock::now();
    int chunk_size = size.first/num_threads;
    pair<int, int> ranges[num_threads];
    for(int i=0; i<num_threads; i++)
        ranges[i] = ((i+1)==num_threads ? \
            make_pair(chunk_size * i, size.first) : \
            make_pair(chunk_size * i, chunk_size * (i+1)));

    for(int iter=0; iter<num_iters; iter++){
        vector<thread> tids;
        for(int i=0; i<num_threads; i++){
            if(iter % 2 == 0){
                auto chunk_func = [&even_board, &odd_board](int first_row, int last_row) {
                    return compute_chunk(even_board, odd_board, first_row, last_row);
                };
                tids.push_back(thread(chunk_func, ranges[i].first, ranges[i].second));
            } else {
                auto chunk_func = [&odd_board, &even_board](int first_row, int last_row) {
                    return compute_chunk(even_board, odd_board, first_row, last_row);
                };
                tids.push_back(thread(chunk_func, ranges[i].first, ranges[i].second));
            }
            
        }
        for(thread& t: tids) t.join();
    }

    auto elapsed = std::chrono::system_clock::now() - start;
    double duration = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();

    if(num_iters%2 == 0) return make_pair(even_board, duration);
    else return make_pair(odd_board, duration);
}


pair<vector<vector<int>>, double> run_in_parallel_ff(vector<vector<int>> init_board, int num_threads=2, \
    int num_iters=100, int radius=1, bool verbose=false){

    pair<int, int> size(init_board.size(), init_board[0].size());
    vector<vector<int>> even_board = init_board;
    vector<vector<int>> odd_board = init_board;

    auto init_start = std::chrono::system_clock::now();

    ParallelFor pf;

    ffTime(START_TIME);
    for(int iter=0; iter < num_iters; iter++){
        pf.parallel_for(0, size.first, [&](int i){
            for(int j=0; j<size.second; j++){
                if(iter%2 == 0){
                    odd_board[i][j] = rules_func(extract_matrix(even_board, make_pair(i, j), radius));
                } else{
                    even_board[i][j] = rules_func(extract_matrix(odd_board, make_pair(i, j), radius));
                }
            }
        }, num_threads);
    }

    // auto elapsed = std::chrono::system_clock::now() - start;
    // double duration = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();

    ffTime(STOP_TIME);


    auto init_elapsed = chrono::system_clock::now() - init_start;
    double init_duration = chrono::duration_cast<std::chrono::microseconds>(init_elapsed).count();

    if(verbose == true) cout <<"\nFastFlow Parallel For instantiation took: "<<init_duration <<" microseconds\n";


    double duration = ffTime(GET_TIME) * 1000;  // Couldn't find anything in FastFlow to report time in usec.

    return (num_iters%2 == 0 ? make_pair(even_board, duration) : make_pair(odd_board, duration));
}
