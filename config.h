#pragma once
#include <chrono>
#include <thread>

#include "utils.h"

using namespace std;

int rules_func(vector<vector<int>> mat) {
    pair<int, int> center(mat.size()/2, mat[0].size()/2);
    
    int n_ones = count_in_matrix(mat, 1);

    int center_element = mat[center.first][center.second];
    if(center_element == 1){
        if(n_ones < 3) return 0;  //Any live cell with fewer than two live neighbours dies, as if by underpopulation.
        else if(n_ones >= 3 && n_ones < 5) return 1;  //Any live cell with two or three live neighbours lives on to the next generation.
        else return 0;  //Any live cell with more than three live neighbours dies, as if by overpopulation.
    }
    else if(n_ones == 3) return 1;  //Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.
    else return center_element;
}