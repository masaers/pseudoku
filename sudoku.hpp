#ifndef COM_MASAERS_SUDOKU_HPP
#define COM_MASAERS_SUDOKU_HPP
#include <bitset>
#include <deque>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <tuple>

namespace com_masaers {
  // Example:
  // HROWS = 2, HCOLS = 3
  // ... means that each house (3x3 square in classic sudoku) has
  // 2 rows and 3 columns. As each house is 2x3, the whole board 
  // must be (2x3=) 6 houses big. This means that the board must 
  // have 3 x 2 houses to get a total of (2x3=) 6 rows and
  // (3x2=) 6 columns.
  //
  // Each cell has a unique identifying position, laid out as follows:
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
  // The houses are numbered in a similar way:
  //
  //  0       | 1
  //          |
  // ---------+---------
  //  2       | 3
  //          |
  // ---------+---------
  //  4       | 5
  //          |
  //
  // The rows and columns are numbered the usual way.
  //
  // Furtherore, the cells in each house have unique "room" identifiers
  // (much like columns in a row or rows in a column) that are
  // laid out in row-major order, like so:
  //
  //  0   1   2
  //  3   4   5
  //
  // Every unique position on a board can thus be defined using one of 
  // three ways:
  // 1. its unique identifier,
  // 2. a row-column pair, or
  // 3. a house-room pair.
  
  /**
     The abstract layout of all sudoku boards of a specific size.
   */
  template<int HROWS, int HCOLS = HROWS>
  class sudoku_layout {
  public:
    static constexpr int N = HCOLS * HROWS;
    static constexpr int NN = N * N;
    static constexpr int HOUSES_PER_ROW = HROWS;
    static constexpr int HOUSES_PER_COL = HCOLS;
    typedef std::bitset<N> cell_type;
    /// Gets the absolute position of the Cth column from the Rth row.
    static constexpr int pos_of_rowcol(const int row, const int col) {
      return (row * N) + col;
    }
    // Gets the absolute position of the Rth room in the Hth house.
    static constexpr int pos_of_houseroom(const int house, const int room) {
      return ((house / HROWS) * HROWS * N) // house row offset
        +    ((house % HROWS) * HCOLS)     // house col offset
        +    ((room / HCOLS) * N)          //  room row offset
        +    (room % HCOLS)                //  room col offset
        ;
    }
    // Gets the row of the Pth abolute position.
    static constexpr int row_of_pos(const int pos) {
      return pos / N;
    }
    // Gets the column of the Pth absolute position.
    static constexpr int col_of_pos(const int pos) {
      return pos % N;
    }
    // Gets the house of the Pth absolute position.
    static constexpr int house_of_pos(const int pos) {
      return ((row_of_pos(pos) / HROWS) * HROWS) + (col_of_pos(pos) / HCOLS);
    }
    // Gets the room of the Pth absolute position.
    static constexpr int room_of_pos(const int pos) {
      return ((row_of_pos(pos) % HROWS) * HCOLS) + (col_of_pos(pos) % HCOLS);
    }
    // Returns an iterator to the first dependent of an absolute position.
    static constexpr const int* first_dep(const int pos) {
      return &dependents_m[pos][0];
    }
    // Returns an iterator to the last dependent of an absolute position.
    static constexpr const int* last_dep(const int pos) {
      return &dependents_m[pos][3*(N-1)];
    }
    // Returns an iterator to the first cell in the same row.
    static constexpr const int* first_row_dep(const int pos) {
      return first_dep(pos);
    }
    // Returns an iterator to the last cell in the same column.
    static constexpr const int* first_col_dep(const int pos) {
      return &dependents_m[pos][N-1];
    }
    // Returns an iterator to the first cell in the same house.
    static constexpr const int* first_house_dep(const int pos) {
      return &dependents_m[pos][2*(N-1)];
    }
    // Returns an iterator to the last cell in the same row.
    static constexpr const int* last_row_dep(const int pos) {
      return first_col_dep(pos);
    }
    // Returns an iterator to the first cell in the same column.
    static constexpr const int* last_col_dep(const int pos) {
      return first_house_dep(pos);
    }
    // Returns an iterator to the last cell in the same house.
    static constexpr const int* last_house_dep(const int pos) {
      return last_dep(pos);
    }
  protected:
    static std::array<std::array<int, 3*(N-1)>, NN> create_dependents() {
      std::array<std::array<int, 3*(N-1)>, NN> result;
      for (int pos = 0; pos < NN; ++pos) {
        int dep = 0;
        for (int col = 0; col < N; ++col) {
          if (col != col_of_pos(pos)) {
            result[pos][dep++] = pos_of_rowcol(row_of_pos(pos), col);
          }
        }
        for (int row = 0; row < N; ++row) {
          if (row != row_of_pos(pos)) {
            result[pos][dep++] = pos_of_rowcol(row, col_of_pos(pos));
          }
        }
        for (int room = 0; room < N; ++room) {
          if (room != room_of_pos(pos)) {
            result[pos][dep++] = pos_of_houseroom(house_of_pos(pos), room);
          }
        }
      }
      return result;
    }
    static const std::array<std::array<int, 3*(N-1)>, NN> dependents_m;
  }; // sudoku_layout

  template<int HROWS, int HCOLS>
  const std::array<std::array<int, 3*(sudoku_layout<HROWS, HCOLS>::N-1)>, sudoku_layout<HROWS, HCOLS>::NN>
  sudoku_layout<HROWS, HCOLS>::dependents_m = sudoku_layout<HROWS, HCOLS>::create_dependents();


  /**
     Represents an instance of a sudoku board.
   */
  template<typename Layout = sudoku_layout<3> >
  class sudoku_board {
  public:
    typedef typename Layout::cell_type cell_type;
  protected:
    cell_type cells_m[Layout::NN];
    int unknown_m;
  public:
    bool operator==(const sudoku_board& x) const {
      bool result = unknown_m == x.unknown_m;
      if (result) {
        for (int i = 0; result && i < Layout::NN; ++i) {
          result = result && cells_m[i] == x.cells_m[i];
        }
      }
      return result;
    }
    inline bool operator!=(const sudoku_board& x) const { return ! operator==(x); }
    template<typename solved_it_T>
    void read(std::istream& is, solved_it_T&& solved_it) {
      unknown_m = 0;
      int number;
      for (int pos = 0; pos < Layout::NN; ++pos) {
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
    void read(std::istream& is) {
      unknown_m = 0;
      int number;
      for (int pos = 0; pos < Layout::NN; ++pos) {
        is >> number;
        cell_type& cell = (*this)[pos];
        cell.reset();
        if (number == 0) {
          cell = ~cell;
          ++unknown_m;
        } else {
          cell.set(number - 1);
        }
      }
    }
    friend std::ostream& operator<<(std::ostream& os, const sudoku_board& board) {
      for (int i = 0; i < Layout::N; ++i) {
        for (int j = 0; j < Layout::N; ++j) {
          if (j != 0) { os << ' '; }
          const auto& cell = board[Layout::pos_of_rowcol(i, j)];
          for (int k = 0; k < Layout::N; ++k) {
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
      for (auto it = Layout::first_dep(pos); it != Layout::last_dep(pos); ++it) {
        result |= (*this)[*it];
      }
      return result;
    }
    template<typename solved_it_T>
    bool apply_mask(const int pos, cell_type mask, solved_it_T&& solved_it) {
      cell_type& cell = (*this)[pos];
      mask &= cell;
      if (mask != cell && mask.count() == 1) {
        *solved_it = pos;
        ++solved_it;
        --unknown_m;
      }
      cell = mask;
      return ! cell.none();
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
    static inline cell_type make_mask(int value) {
      cell_type result;
      result.flip(value);
      return result;
    }
    bool solved() const { return unknown_m == 0; }
    bool solved(const cell_type& cell) const { return cell.count() == 1; }
    bool solved(const int pos) const { return solved((*this)[pos]); }
    const int unknown() const { return unknown_m; }
    template<typename solved_it_T>
    bool propagate_solution(const int pos, solved_it_T&& solved_it) {
      using namespace std;
      bool result = true;
      cell_type mask = ~(*this)[pos];
      for (auto it = Layout::first_dep(pos); result && it != Layout::last_dep(pos); ++it) {
        result = result && apply_mask(*it, mask, solved_it);
      }
      return result;
    }
    cell_type& operator[](const int pos) { return cells_m[pos]; }
    const cell_type& operator[](const int pos) const { return cells_m[pos]; }
    bool valid() const {
      bool result = true;
      for (int i = 0; result && i < Layout::N; ++i) {
        cell_type row, col, house;
        for (int j = 0; result && j < Layout::N; ++j) {
          const auto& row_cell = (*this)[Layout::pos_of_rowcol(i, j)];
          if (row_cell.count() == 1) {
            if ((row & row_cell).any()) {
              result = false;
            } else {
              row |= row_cell;
            }
          }
          const auto& col_cell = (*this)[Layout::pos_of_rowcol(j, i)];
          if (col_cell.count() == 1) {
            if ((col & col_cell).any()) {
              result = false;
            } else {
              col |= col_cell;
            }
          }
          const auto& room_cell = (*this)[Layout::pos_of_houseroom(i, j)];
          if (room_cell.count() == 1) {
            if ((house & room_cell).any()) {
              result = false;
            } else {
              house |= room_cell;
            }
          }
        }
      }
      return result;
    }
  }; // sudoku_board
} // namespace com_masaers

#endif
