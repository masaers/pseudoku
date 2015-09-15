#ifndef COM_MASAERS_SUDOKU_HPP
#define COM_MASAERS_SUDOKU_HPP
#include <bitset>
#include <deque>
#include <vector>
#include <stdexcept>
#include <iostream>

namespace com_masaers {
  // FROWS = 2, FCOLS = 3
  // Each filed has 3 columns and 2 rows,
  // meaning that the board is 6 x 6 cells big.
  // Each cell has a unique identifying position, laied out as follows:
  // 
  //  0  1  2 |  3  4  5
  //  6  7  8 |  9 10 11
  // ---------+---------
  // 12 13 14 | 15 16 17
  // 18 19 20 | 21 22 23
  // ---------+---------
  // 24 25 26 | 27 28 29
  // 30 31 32 | 33 34 35
  //
  // The fields are numbered in a similar way:
  // 0 1
  // 2 3
  // 4 5
  //
  
  
  /**
     The abstract layout of all sudoku boards of a specific size.
   */
  template<int FROWS, int FCOLS>
  class sudoku_layout {
  public:
    static const int N = FCOLS * FROWS;
    static const int NN = N * N;
    static const int FIELDS_PER_ROW = FROWS;
    static const int FIELDS_PER_COL = FCOLS;
    typedef std::bitset<N> cell_type;
    static constexpr int pos_of_rowcell(const int row, const int cell) {
      return (row * N) + cell;
    }
    static constexpr int pos_of_colcell(const int col, const int cell) {
      return col + (cell * N);
    }
    static constexpr int pos_of_fieldcell(const int field, const int cell) {
      return ((field / FROWS) * FROWS * N) // field row offset
        +    ((field % FROWS) * FCOLS)     // field col offset
        +    ((cell / FCOLS) * N)          //  cell row offset
        +    (cell % FCOLS)                //  cell col offset
        ;
    }
    static constexpr int row_of_pos(const int pos) {
      return pos / N;
    }
    static constexpr int col_of_pos(const int pos) {
      return pos % N;
    }
    static constexpr int field_of_pos(const int pos) {
      return ((row_of_pos(pos) / FROWS) * FROWS) + (col_of_pos(pos) / FCOLS);
    }
    static constexpr int rowcell_of_pos(const int pos) {
      return col_of_pos(pos);
    }
    static constexpr int colcell_of_pos(const int pos) {
      return row_of_pos(pos);
    }
    static constexpr int fieldcell_of_pos(const int pos) {
      return ((row_of_pos(pos) % FROWS) * FCOLS)
        +    (col_of_pos(pos) % FCOLS)
        ;
    }
    constexpr const int* first_dep(const int pos) {
      return &dependents_m[pos][0];
    }
    constexpr const int* last_dep(const int pos) {
      return &dependents_m[pos][3*(N-1)];
    }
    constexpr const int* first_row_dep(const int pos) {
      return first_dep(pos);
    }
    constexpr const int* first_col_dep(const int pos) {
      return &dependents_m[pos][N-1];
    }
    constexpr const int* first_field_dep(const int pos) {
      return &dependents_m[pos][2*(N-1)];
    }
    constexpr const int* last_row_dep(const int pos) {
      return first_col_dep(pos);
    }
    constexpr const int* last_col_dep(const int pos) {
      return first_field_dep(pos);
    }
    constexpr const int* last_field_dep(const int pos) {
      return last_dep(pos);
    }
    sudoku_layout() {
      for (int pos = 0; pos < NN; ++pos) {
        int dep = 0;
        for (int cell = 0; cell < N; ++cell) {
          if (cell != rowcell_of_pos(pos)) {
            dependents_m[pos][dep++] = pos_of_rowcell(row_of_pos(pos), cell);
          }
        }
        for (int cell = 0; cell < N; ++cell) {
          if (cell != colcell_of_pos(pos)) {
            dependents_m[pos][dep++] = pos_of_colcell(col_of_pos(pos), cell);
          }
        }
        for (int cell = 0; cell < N; ++cell) {
          if (cell != fieldcell_of_pos(pos)) {
            dependents_m[pos][dep++] = pos_of_fieldcell(field_of_pos(pos), cell);
          }
        }
      }
      test_consistency();
    }
  protected:
    static bool test_consistency() {
      using namespace std;
      bool result = true;
      for (int i = 0; i < NN; ++i) {
        if (pos_of_fieldcell(field_of_pos(i), fieldcell_of_pos(i)) != i) {
          throw runtime_error("Failed to verify field of position!");
        }
        if (pos_of_colcell(col_of_pos(i), colcell_of_pos(i)) != i) {
          throw runtime_error("Column problem!");
        }
        if (pos_of_rowcell(row_of_pos(i), rowcell_of_pos(i)) != i) {
          throw runtime_error("Row problem!");
        }
      }
      return result;
    }
    int dependents_m[NN][3*(N-1)];
  }; // sudoku_layout


  
  /**
     Represents an instance of a 3x3 sudoku board.
   */
  class sudoku_board {
  protected:
    typedef sudoku_layout<3, 3> layout_type;
  public:
    typedef typename layout_type::cell_type cell_type;
  protected:
    cell_type cells_m[layout_type::NN];
    int unknown_m;
    int invalid_m;
  public:
    static const layout_type L;
    template<typename solved_it_T>
    void read(std::istream& is, solved_it_T&& solved_it) {
      unknown_m = 0;
      int number;
      for (int pos = 0; pos < L.NN; ++pos) {
        is >> number;
        cell_type& cell = (*this)[pos];
        cell.reset();
        if (number == 0) {
          cell = ~cell;
          ++unknown_m;
        } else {
          cell.set(number - 1);
          *solved_it = pos;
          ++solved_it;
        }
      }
    }
    friend std::ostream& operator<<(std::ostream& os, const sudoku_board& board) {
      for (int i = 0; i < L.N; ++i) {
        for (int j = 0; j < L.N; ++j) {
          if (j != 0) { os << ' '; }
          const auto& cell = board[board.L.pos_of_rowcell(i, j)];
          for (int k = 0; k < L.N; ++k) {
            if (cell[k]) {
              os << (k + 1);
            } else {
              os << '.';
            }
          }
        }
        os << std::endl;
      }
      return os;
    }
    const cell_type get_known_buddies(const int pos) {
      using namespace std;
      cell_type result;
      for (auto it = L.first_dep(pos); it != L.last_dep(pos); ++it) {
        result |= (*this)[*it];
      }
      return result;
    }
    template<typename solved_it_T>
    void apply_mask(const int pos,
                    const cell_type& mask,
                    solved_it_T&& solved_it) {
      cell_type& cell = (*this)[pos];
      if (! solved(cell)) {
        cell &= mask;
        if (solved(cell)) {
          *solved_it = pos;
          ++solved_it;
          --unknown_m;
        }
        if (cell.none()) {
          throw std::runtime_error("Applied mask to invalidate cell");
        }
      }
    }
    ///
    /// Only applies the mask if it solves the cell.
    ///
    template<typename solved_it_T>
    void try_mask(const int pos,
                  const cell_type& mask,
                  solved_it_T&& solved_it) {
      cell_type& cell = (*this)[pos];
      if (! solved(cell)) {
        const cell_type new_cell = cell & mask;
        if (solved(new_cell)) {
          cell = new_cell;
          *solved_it = pos;
          ++solved_it;
          --unknown_m;
        }
      }
    }
    bool solved() const { return unknown_m == 0; }
    bool solved(const cell_type& cell) const { return cell.count() == 1; }
    bool solved(const int pos) const { return solved((*this)[pos]); }
    const int unknown() const { return unknown_m; }
    template<typename solved_it_T>
    void propagate_solution(const int pos, solved_it_T&& solved_it) {
      using namespace std;
      cell_type mask = ~(*this)[pos];
      for (auto it = L.first_dep(pos); it != L.last_dep(pos); ++it) {
        apply_mask(*it, mask, solved_it);
      }
    }
    cell_type& operator[](const int pos) { return cells_m[pos]; }
    const cell_type& operator[](const int pos) const { return cells_m[pos]; }
    bool valid() const {
      bool result = true;
      for (int i = 0; result && i < L.N; ++i) {
        cell_type row, col, field;
        for (int j = 0; j < L.N; ++j) {
          row   |= (*this)[L.pos_of_rowcell(i, j)];
          col   |= (*this)[L.pos_of_colcell(i, j)];
          field |= (*this)[L.pos_of_fieldcell(i, j)];
        }
        result = result && row.all() && col.all() && field.all();
      }
      return result;
    }
  }; // sudoku_board

  const sudoku_layout<3, 3> sudoku_board::L = sudoku_layout<3, 3>();



  /**
     Trial-and-error solver that tries different solution until
     something sticks.
   */
  class trialanderror_solver {
  public:
    typedef std::deque<int> agenda_type;
    agenda_type& agenda() { return agenda_m; }
    void clear_agenda(sudoku_board& board) {
      while (! agenda_m.empty()) {
        board.propagate_solution(agenda_m.front(), back_inserter(agenda_m));
        agenda_m.pop_front();
      }
    }
    const int choose_pos(const sudoku_board& board) const {
      int result = -1;
      std::size_t count = -1;
      for (int pos = 0; pos < board.L.NN; ++pos) {
        const auto& cell = board[pos];
        if (cell.count() > 1 && cell.count() < count) {
          result = pos;
          count = cell.count();
        }
      }
      return result;
    }
    void operator()(sudoku_board& board) {
      using namespace std;
      typedef typename sudoku_board::cell_type cell_type;
      clear_agenda(board);
      const int pos = choose_pos(board);
      const cell_type& cell = board[pos];
      for (int n = 0; n < board.L.N; ++n) {
        if (cell.test(n)) {
          sudoku_board b(board);
          cell_type mask = ~cell_type();
          mask.flip(n);
          b.apply_mask(pos, mask, back_inserter(agenda_m));
          clear_agenda(b);
          if (b.valid()) {
            board = b;
            break;
          }
        }
      }
    }
  private:
    agenda_type agenda_m;
  }; // trialanderror_solver

  
} // namespace com_masaers

#endif
