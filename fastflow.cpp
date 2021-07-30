#include <iostream>
#include <fstream>

#include "config.h"
#include "run.h"

using namespace std;

void get_shell_params(int argc, char *argv[], int &board_w, int &board_h, int &num_row_threads, \
    int &num_iters, int &num_repetitions, int &num_states, bool &fill_randomly, bool &verbose){
    for(int i=0; i<argc; i++){
        if(strcmp(argv[i], "-w") == 0) board_w = atoi(argv[i+1]);
        if(strcmp(argv[i], "-h") == 0) board_h = atoi(argv[i+1]);
        if(strcmp(argv[i], "-t") == 0) num_row_threads = atoi(argv[i+1]);
        if(strcmp(argv[i], "-i") == 0) num_iters = atoi(argv[i+1]);
        if(strcmp(argv[i], "-I") == 0) num_repetitions = atoi(argv[i+1]);
        if(strcmp(argv[i], "-s") == 0) num_states = atoi(argv[i+1]);
        if(strcmp(argv[i], "-v") == 0) verbose = true;

        if(strcmp(argv[i], "--rand") == 0) fill_randomly = true;
    }
}

int main(int argc, char *argv[]){
    int board_w, board_h, num_row_threads, num_iters, num_states;
    int num_reps=1;
    bool fill_rand = false, verbose=false;

    get_shell_params(argc, argv, board_w, board_h, num_row_threads, \
        num_iters, num_reps, num_states, fill_rand, verbose);

    vector<vector<int>> board(board_w, vector<int>(board_h));

    fill_rand ? fill_matrix_randomly(board, num_states) : read_matrix(board);

    double par_time = 0;

    for(int rep=0; rep<num_reps; rep++){
        auto par_result = run_in_parallel_ff(board, num_row_threads, num_iters, 1, verbose);
        par_time += par_result.second;
    }

    cout<<"Time: "<<par_time/num_reps<<endl;
}