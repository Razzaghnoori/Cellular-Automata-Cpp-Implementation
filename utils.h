#pragma once

#include <iostream>
#include <vector>

#include <stdlib.h>

using namespace std;

int remainder(int x, int y){
    if(x < 0) x += (abs(x / y) + 1) * y;
    return(x%y);
}

void write_matrix(vector<vector<int>> mat) {
    int m = mat.size();
    int n = mat[0].size();
    for(int i=0; i<m; i++){
        for(int j=0; j<n; j++){
            cout << mat[i][j]<<" ";
        }
        cout<<"\n";
    }
}

vector<vector<int>> read_matrix(vector<vector<int>> &board){
    int n_rows = board.size();
    int n_cols = board[0].size();

    for(int i=0; i<n_rows; i++){
        for(int j=0; j<n_cols; j++){
            cin >> board[i][j];
        }
    }
    return(board);
}

vector<vector<int>> extract_matrix(vector<vector<int>> mat, pair<int, int> top_left, \
    int n_rows, int n_cols) {
        // Slices matrix starting from the top_left taking n_rows rows and n_cols columns.
        int mat_n_rows = mat.size();
        int mat_n_cols = mat[0].size();

        vector<vector<int>> out(n_rows, vector<int>(n_cols));

        for(int i=0; i < n_rows; i++){
            for(int j=0; j<n_cols; j++){
                // Using reminder operator to make the matrix a toroidal one
                // C++'s default remainder doesn't do the job though. Looking for a python-style implementation
                out[i][j] = mat[remainder(top_left.first + i, mat_n_rows)][remainder(top_left.second + j, mat_n_cols)];
            }
        }
        return(out);
}

vector<vector<int>> extract_matrix(vector<vector<int>> mat, pair<int, int> center, int radius) {
    pair<int, int> mat_size(mat.size(), mat[0].size());

    pair<int, int> top_left(remainder(center.first - radius, mat_size.first),
    remainder(center.second - radius, mat_size.second));

    int w_n_h = 2 * radius + 1;
    return extract_matrix(mat, top_left, w_n_h, w_n_h);
}

int count_in_matrix(vector<vector<int>> mat, int state){
    int count = 0;
    for(int i=0; i<mat.size(); i++){
        for(int j=0; j<mat[i].size(); j++){
            if(mat[i][j] == 1) count++;
        }
    }
    return(count);
}

bool check_matrix_equality(vector<vector<int>> A, vector<vector<int>> B){
    if(A.size() != B.size() || A[0].size() != B[0].size()) return false;

    for(int i=0; i<A.size(); i++){
        for(int j=0; j<A[0].size(); j++){
            if(A[i][j] != B[i][j]) return false;
        }
    }
    return true;
}

void fill_matrix_randomly(vector<vector<int>> &A){
    for(int i=0; i< A.size(); i++)
        for(int j=0; j<A[0].size(); j++)
            A[i][j] = rand()%2;
}