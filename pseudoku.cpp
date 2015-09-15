#include "sudoku.hpp"
#include "pseudoku.hpp"
#include <iostream>
#include <functional>
#include <algorithm>








int main(const int argc, const char** argv) {
  using namespace std;
  using namespace com_masaers;
  
  sudoku_board board;
  pseudoku_solver solve;
  trialanderror_solver trialanderror;
  int tried_cells = 0;

  board.read(cin, back_inserter(solve.agenda()));
  if (! board.valid()) {
    cout << "Provided board not valid." << endl;
  }
  cout << board << endl;

  // solve.clear_agenda(board);
  // cout << "Board is " << (board.valid() ? "" : "in") << "valid." << endl;
  // cout << board << endl;
  // while (board.valid() && ! board.solved()) {
  //   brute_force(board);
  //   ++brute_forced_cells;
  // }
  // cout << "Board is " << (board.valid() ? "" : "in") << "valid." << endl;
  // cout << board << endl;

  while (board.valid() && ! board.solved()) {
    solve(board);
    cout << "Board is " << (board.valid() ? "" : "in") << "valid." << endl;
    cout << board << endl;
    if (! board.solved()) {
      trialanderror(board);
      ++tried_cells;
      cout << "Board is " << (board.valid() ? "" : "in") << "valid." << endl;
      cout << board << endl;
    }
  }
  
  if (board.valid() && board.solved()) {
    cout << "Found a solution!";
    if (tried_cells > 0) {
      cout << " (" << tried_cells
           << " cell" << (tried_cells > 1 ? "s" : "")
           << " had to be solved by trial and error)";
    }
    cout << endl;
  }
  return board.valid() ? EXIT_SUCCESS : EXIT_FAILURE;
}
