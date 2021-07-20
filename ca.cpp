#include <iostream>
#include <fstream>

#include "config.h"
#include "run.h"

using namespace std;
using namespace ff;

int interactive_main()
{
    int board_w, board_h;

    cout << "Let me know how many rows and columns the board must have:\n> ";
    cin >> board_w >> board_h;
    pair<int, int> board_size(board_w, board_h);

    cout << "Do you want me to generate a random board for you? (Y|n) ";
    char rand_board_yn;
    cin >> rand_board_yn;

    vector<vector<int>> board(board_w, vector<int>(board_h));
    if (rand_board_yn == 'n' || rand_board_yn == 'N')
    {
        cout << "OKay! Now enter the initial state of the board:\n";
        read_matrix(board);
    }
    else
    {
        fill_matrix_randomly(board);
        cout << "Matrix filled with random values!\n";
    }

    int num_row_threads, num_col_threads, num_iters;
    cout << "How many threads do you need on rows and columns respectively? \n> ";
    cin >> num_row_threads >> num_col_threads;

    cout << "How many iteration should it run for?\n> ";
    cin >> num_iters;

    char verbose;
    cout << "Wanna see final state of the board? (y|n) ";
    cin >> verbose;

    auto seq_result = run_sequentially(board, num_iters, 1);
    auto par_result = run_in_parallel(board, num_row_threads, num_col_threads, num_iters);
    auto ff_par_result = run_in_parallel_ff(board, num_row_threads, num_col_threads, num_iters);

    cout << "\nParallel and sequential outputs are " << (check_matrix_equality(seq_result.first, par_result.first) ? "" : "NOT ") << "equal.\n";
    cout << "\nParallel outputs using native c++ threads and FastFlow are " << (check_matrix_equality(par_result.first, ff_par_result.first) ? "" : "NOT ") << "equal.\n";

    if (verbose == 'y' || verbose == 'Y')
    {
        cout << "Sequential Result:\n\n";
        write_matrix(seq_result.first);
        cout << "\n";

        cout << "Parallel Result:\n\n";
        write_matrix(par_result.first);
        cout << "\n";

        cout << "Parallel Result Using FastFlow:\n\n";
        write_matrix(ff_par_result.first);
        cout << "\n";
    }

    cout << "Time taken for each task:\n";
    cout << "\n> Sequential:\t\t" << seq_result.second;
    cout << "\n> Parallel:\t\t" << par_result.second;
    cout << "\n> FastFlow:\t\t" << ff_par_result.second << "\n";
    
    return (0);
}

void get_shell_params(int argc, char *argv[], int &board_w, int &board_h, int &num_row_threads, \
    int &num_col_threads, int &num_iters, int &num_repetitions, bool &fill_randomly){
    for(int i=0; i<argc; i++){
        if(strcmp(argv[i], "-w") == 0) board_w = atoi(argv[i+1]);
        if(strcmp(argv[i], "-h") == 0) board_h = atoi(argv[i+1]);
        if(strcmp(argv[i], "-r") == 0) num_row_threads = atoi(argv[i+1]);
        if(strcmp(argv[i], "-c") == 0) num_col_threads = atoi(argv[i+1]);
        if(strcmp(argv[i], "-i") == 0) num_iters = atoi(argv[i+1]);
        if(strcmp(argv[i], "-I") == 0) num_repetitions = atoi(argv[i+1]);

        if(strcmp(argv[i], "--rand") == 0) fill_randomly = true;
    }
}


int cli_main(int argc, char *argv[]){
    int board_w, board_h, num_row_threads, num_col_threads, num_iters;
    int num_reps=1;
    bool fill_rand = false;

    get_shell_params(argc, argv, board_w, board_h, num_row_threads, \
        num_col_threads, num_iters, num_reps, fill_rand);

    vector<vector<int>> board(board_w, vector<int>(board_h));

    fill_rand ? fill_matrix_randomly(board) : read_matrix(board);

    long long seq_time, par_time, ff_par_time, par_1_time, ff_par_1_time;

    seq_time = 0;
    par_time = 0;
    ff_par_time = 0;
    par_1_time = 0;
    ff_par_1_time = 0;


    for(int rep=0; rep<num_reps; rep++){
        auto seq_result = run_sequentially(board, num_iters, 1);
        auto par_result = run_in_parallel(board, num_row_threads, num_col_threads, num_iters);
        auto ff_par_result = run_in_parallel_ff(board, num_row_threads, num_col_threads, num_iters);

        // We need two extra results to compute scalability.
        auto par_1_result = run_in_parallel(board, 1, 1, num_iters);
        auto ff_par_1_result = run_in_parallel_ff(board, 1, 1, num_iters);


        seq_time += seq_result.second;
        par_time += par_result.second;
        ff_par_time += ff_par_result.second;
        par_1_time += par_1_result.second;
        ff_par_1_time += ff_par_1_result.second;
    }

    // Calculating the average
    seq_time /= num_reps;
    par_time /= num_reps;
    ff_par_time /= num_reps;
    par_1_time /= num_reps;
    ff_par_1_time /= num_reps;

    float sp_par = seq_time / par_time;
    float sp_ff = seq_time / ff_par_time;

    float sc_par = par_1_time / par_time;
    float sc_ff = ff_par_1_time / ff_par_time;

    float eff_par = sp_par / (num_row_threads * num_col_threads);
    float eff_ff = sp_ff / (num_row_threads * num_col_threads);

    long csv_row_setup[] = {board_w, board_h, num_row_threads, num_col_threads, num_iters, seq_time, par_time, ff_par_time};
    float csv_row_results[] = {sp_par, sp_ff, sc_par, sc_ff, eff_par, eff_ff};
    int row_setup_len = sizeof(csv_row_setup)/sizeof(csv_row_setup[0]);
    int row_result_len = sizeof(csv_row_results)/sizeof(csv_row_results[0]);
    
    char delim[] = ", ";

    for(int i=0; i<row_setup_len; i++){
        cout<<csv_row_setup[i] << delim;
    }
    for(int i=0; i<row_result_len; i++){
        cout<<csv_row_results[i];
        if(i+1 == row_result_len) cout<<"\n";
        else cout<<delim;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    if (argc == 1)
        return interactive_main();
    else{
        return cli_main(argc, argv);
    }
}
