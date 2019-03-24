#include "sudoku.hpp"
#include <algorithm>

namespace com_masaers {

  class trivial_solver {
  protected:
    std::deque<int> agenda_m;
    std::deque<int>& agenda() { return agenda_m; }
    void analyze_board(const sudoku_board& board) {
      agenda_m.clear();
      for (int pos = 0; pos < sudoku_board::L.NN; ++pos) {
        if (board.solved(pos)) {
          agenda_m.emplace_back(pos);
        }
      }
    }
    bool propagate_solutions(sudoku_board& board) {
      bool result = true;
      while (result && ! agenda_m.empty()) {
        result = board.propagate_solution(agenda_m.front(), back_inserter(agenda_m));
        agenda_m.pop_front();
      }
      agenda_m.clear();
      return result;
    }
    bool apply_mask(sudoku_board& board, int pos, const typename sudoku_board::cell_type& mask) {
      return board.apply_mask(pos, mask, back_inserter(agenda_m))
      &&     propagate_solutions(board);
    }
  public:
    sudoku_board operator()(sudoku_board board) {
      analyze_board(board);
      propagate_solutions(board);
      return board;
    }
  }; // trivial_solver


  class depth_first_solver : trivial_solver {
    std::vector<sudoku_board> solutions_m;
  public:
    const sudoku_board& operator()(const sudoku_board& board) {
      solutions_m.clear();
      depth_first(board, 1);
      if (solutions_m.empty()) {
        return board;
      } else {
        return solutions_m.front();
      }
    }
    const std::vector<sudoku_board>& operator()(const sudoku_board& board, std::size_t solutions) {
      solutions_m.clear();
      depth_first(board, solutions);
      return solutions_m;
    }
    // bool recursive(const sudoku_board& board) {
    //   using namespace std;
    //   if (board.solved()) {
    //     return true;
    //   } else {
    //     for (int pos = 0; pos < sudoku_board::L.NN; ++pos) {
    //       if (! board.solved(pos)) {
    //         for (int value = 0; value < sudoku_board::L.N; ++value) {
    //           sudoku_board b = board;
    //           if (apply_mask(b, pos, sudoku_board::make_mask(value)) && recursive(b)) {
    //             board = b;
    //             return true;
    //           }
    //         }
    //         break;
    //       }
    //     }
    //     return false;
    //   }
    // }
    void depth_first(const sudoku_board& board, std::size_t solutions) {
      using namespace std;
      vector<sudoku_board> frontier{{ board }};
      while (! frontier.empty() && solutions_m.size() < solutions) {
        if (frontier.back().solved()) {
          solutions_m.emplace_back(frontier.back());
          frontier.pop_back();
        } else {
          sudoku_board b = frontier.back();
          frontier.pop_back();
          for (int pos = 0; pos < sudoku_board::L.NN; ++pos) {
            if (! b.solved(pos)) {
              for (int value = 0; value < sudoku_board::L.N; ++value) {
                if (b[pos][value]) {
                  frontier.emplace_back(b);
                  if (! apply_mask(frontier.back(), pos, sudoku_board::make_mask(value))) {
                    frontier.pop_back();
                  }
                }
              }
              break;
            }
          }
        }
      }
    }
    // void breath_first(const sudoku_board& board) {
    //   using namespace std;
    //   deque<sudoku_board> agenda{{ board }};
    //   while (! agenda.empty() && ! agenda.front().solved()) {
    //     for (int pos = 0; pos < sudoku_board::L.NN; ++pos) {
    //       if (! agenda.front().solved(pos)) {
    //         for (int value = 0; value < sudoku_board::L.N; ++value) {
    //           if (agenda.front()[pos][value]) {
    //             agenda.emplace_back(agenda.front());
    //             if (! apply_mask(agenda.back(), pos, sudoku_board::make_mask(value))) {
    //               agenda.pop_back();
    //             }
    //           }
    //         }
    //         break;
    //       }
    //     }
    //     agenda.pop_front();
    //   }
    //   if (! agenda.empty()) {
    //     board = agenda.front();
    //   }
    // }
  }; // depth_first_solver

}