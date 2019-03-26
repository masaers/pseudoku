#include "sudoku.hpp"
#include "pseudoku.hpp"
#include "solver.hpp"
#include "timer.hpp"
#include <iostream>
#include <fstream>
#include <functional>
#include <algorithm>

bool process_board(com_masaers::sudoku_board& board, com_masaers::timer& solve_time, const std::size_t max_solutions) {
  using namespace std;
  using namespace com_masaers;
  static trivial_solver trivial;
  static depth_first_solver depth_first;
  bool result = false;
  timer local_time;
  if (board.valid()) {
    cout << board << endl;
    local_time.start();
    board = trivial(board);
    local_time.stop();
    if (board.valid() && board.solved()) {
      cout << board << endl;
      cout << "Found trivial solution!" << endl;
      result = true;
    } else {
      cout << "Looking for at most " << max_solutions << " solution(s)..." << endl;
      local_time.start();
      auto boards = depth_first(board, max_solutions);
      local_time.stop();
      if (! boards.empty()) {
        cout << boards.front() << endl;
        cout << "Found " << boards.size() << " solution(s)!" << endl;
        result = true;
      }
    }
  } else {
    cout << "Provided board not valid." << endl;
  }
  cout << "Time spent solving this problem: " << local_time << "." << endl;
  solve_time += local_time;
  return result;
}


int main(const int argc, const char** argv) {
  using namespace std;
  using namespace com_masaers;

  timer solve_time;
  timer program_time;
  program_time.start();
  bool exit_status = true;
  std::size_t max_solutions = 1;

  sudoku_board board;

  if (argc > 1) {
    for (int i = 1; i < argc; ++i) {
      std::ifstream file(argv[i]);
      board.read(file);
      cout << "file: " << argv[i] << endl;
      exit_status = exit_status && process_board(board, solve_time, max_solutions);
    }
  } else {
    board.read(cin);
    exit_status = exit_status && process_board(board, solve_time, max_solutions);
  }

  cout << "Time spent solving: " << solve_time << "." << endl;    
  program_time.stop();
  cout << "Total runtime: " << program_time << endl;

  return exit_status ? EXIT_SUCCESS : EXIT_FAILURE;
}
