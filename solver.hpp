#include "sudoku.hpp"
#include <algorithm>
#include <vector>

namespace com_masaers {

  template<typename Layout>
  class trivial_solver {
  public:
    sudoku_board<Layout> operator()(sudoku_board<Layout> board);
  protected:
    void analyze_board(const sudoku_board<Layout>& board);
    bool propagate_solutions(sudoku_board<Layout>& board);
    bool apply_mask(sudoku_board<Layout>& board, int pos, const typename sudoku_board<Layout>::cell_type& mask);
  private:
    std::deque<int> agenda_m;
    std::deque<int>& agenda();
  }; // trivial_solver


  template<typename Layout>
  class depth_first_solver : trivial_solver<Layout> {
  public:
    const sudoku_board<Layout>& operator()(const sudoku_board<Layout>& board);
    const std::vector<sudoku_board<Layout> >& operator()(const sudoku_board<Layout>& board, std::size_t solutions);
  protected:
    void depth_first(const sudoku_board<Layout>& board, std::size_t solutions);
  private:
    std::vector<sudoku_board<Layout> > solutions_m;
  }; // depth_first_solver

} // namespace com_masaers


template<typename Layout>
inline com_masaers::sudoku_board<Layout> com_masaers::trivial_solver<Layout>::operator()(sudoku_board<Layout> board) {
  analyze_board(board);
  propagate_solutions(board);
  return board;
}

template<typename Layout>
inline std::deque<int>& com_masaers::trivial_solver<Layout>::agenda() {
  return agenda_m;
}

template<typename Layout>
inline void com_masaers::trivial_solver<Layout>::analyze_board(const sudoku_board<Layout>& board) {
  agenda_m.clear();
  for (int pos = 0; pos < Layout::NN; ++pos) {
    if (board.solved(pos)) {
      agenda_m.emplace_back(pos);
    }
  }
}

template<typename Layout>
inline bool com_masaers::trivial_solver<Layout>::propagate_solutions(sudoku_board<Layout>& board) {
  bool result = true;
  while (result && ! agenda_m.empty()) {
    result = board.propagate_solution(agenda_m.front(), back_inserter(agenda_m));
    agenda_m.pop_front();
  }
  agenda_m.clear();
  return result;
}

template<typename Layout>
inline bool com_masaers::trivial_solver<Layout>::apply_mask(sudoku_board<Layout>& board, int pos, const typename sudoku_board<Layout>::cell_type& mask) {
  return board.apply_mask(pos, mask, back_inserter(agenda_m))
  &&     propagate_solutions(board);
}

template<typename Layout>
inline const com_masaers::sudoku_board<Layout>& com_masaers::depth_first_solver<Layout>::operator()(const sudoku_board<Layout>& board) {
  solutions_m.clear();
  depth_first(board, 1);
  if (solutions_m.empty()) {
    return board;
  } else {
    return solutions_m.front();
  }
}

template<typename Layout>
inline const std::vector<com_masaers::sudoku_board<Layout> >& com_masaers::depth_first_solver<Layout>::operator()(const sudoku_board<Layout>& board, std::size_t solutions) {
  solutions_m.clear();
  depth_first(board, solutions);
  return solutions_m;
}

template<typename Layout>
void com_masaers::depth_first_solver<Layout>::depth_first(const sudoku_board<Layout>& board, std::size_t solutions) {
  std::vector<sudoku_board<Layout> > frontier{{ board }};
  while (! frontier.empty() && solutions_m.size() < solutions) {
    if (frontier.back().solved()) {
      solutions_m.emplace_back(frontier.back());
      frontier.pop_back();
    } else {
      sudoku_board<Layout> b = frontier.back();
      frontier.pop_back();
      for (int pos = 0; pos < Layout::NN; ++pos) {
        if (! b.solved(pos)) {
          for (int value = 0; value < Layout::N; ++value) {
            if (b[pos][value]) {
              frontier.emplace_back(b);
              if (! this->apply_mask(frontier.back(), pos, sudoku_board<Layout>::make_mask(value))) {
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
