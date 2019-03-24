#include "sudoku.hpp"
#include "pseudoku.hpp"
#include "solver.hpp"
#include <iostream>
#include <functional>
#include <algorithm>


int main(const int argc, const char** argv) {
  using namespace std;
  using namespace com_masaers;

  int exit_status = EXIT_FAILURE;
  std::size_t max_solutions = 10000;

  sudoku_board board;
  trivial_solver trivial;
  depth_first_solver depth_first;

  board.read(cin);
  if (! board.valid()) {
    cout << "Provided board not valid." << endl;
  }
  cout << board << endl;
  board = trivial(board);
  cout << board << endl;
  if (board.valid() && board.solved()) {
    cout << "Found trivial solution!" << endl;
    exit_status = EXIT_SUCCESS;
  } else {
    cout << "Looking for at most " << max_solutions << " solutions..." << endl;
    auto boards = depth_first(board, max_solutions);
    if (! boards.empty()) {
      cout << boards.front() << endl;
      cout << "Found " << boards.size() << " solutions!" << endl;
      exit_status = EXIT_SUCCESS;
    }
  }

  return exit_status;
}
