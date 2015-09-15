#include <iostream>
#include <bitset>
#include <deque>
#include <vector>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <stdexcept>

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
        cerr << "Failed to verify field of position " << i << endl;
        cerr << "\tfield_of_pos(i)=" << field_of_pos(i)
                  << "\tfieldcell_of_pos(i)=" << fieldcell_of_pos(i)
                  << endl;
      }
      if (pos_of_colcell(col_of_pos(i), colcell_of_pos(i)) != i) {
        cerr << "Column problem!" << endl;
      }
      if (pos_of_rowcell(row_of_pos(i), rowcell_of_pos(i)) != i) {
        cerr << "Row problem!" << endl;
      }
    }
    return result;
  }
  int dependents_m[NN][3*(N-1)];
}; // sudoku_layout




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
        // os << board[board.L.pos_of_rowcell(i, j)];
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
        throw runtime_error("Applied mask to invalidate cell");
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




class pseudoku_solver {
public:
  typedef std::deque<int> agenda_type;
  agenda_type& agenda() { return agenda_m; }
  void clear_agenda(sudoku_board& board) {
    while (! agenda_m.empty()) {
      board.propagate_solution(agenda_m.front(), back_inserter(agenda_m));
      agenda_m.pop_front();
    }
  }
  void process_row(sudoku_board& board, const int row) {
    using namespace std;
    typedef typename sudoku_board::cell_type cell_type;
    auto solved_it = back_inserter(agenda_m);
    // Analyze row
    cell_type backward[board.L.N];
    cell_type field_out[board.L.FIELDS_PER_ROW];
    cell_type field_in[board.L.FIELDS_PER_ROW];
    unordered_map<cell_type, unordered_set<int> > uniq_sets;
    for (int c = 0; c < board.L.N; ++c) {
      const int pos = board.L.pos_of_rowcell(row, c);
      const cell_type& cell = board[pos];
      if (c != 0) {
        backward[board.L.N-1 - c] = backward[board.L.N - c] | board[board.L.pos_of_rowcell(row, board.L.N - c)];
      }
      const int f = board.L.field_of_pos(pos) % board.L.FIELDS_PER_ROW;
      field_in[f] |= cell;
      for (int i = 1; i < board.L.FIELDS_PER_ROW; ++i) {
        field_out[(f + i) % board.L.FIELDS_PER_ROW] |= cell;
      }
      if (cell.count() > 1) {
        uniq_sets[cell].insert(c);
      }
    }
    // Find unique sets and eliminate their members from other cells
    // in the same row
    for (auto it = begin(uniq_sets); it != end(uniq_sets); ++it) {
      if (it->first.count() == it->second.size()) {
        for (auto c = 0; c < board.L.N; ++c) {
          if (it->second.find(c) == it->second.end()) {
            board.apply_mask(board.L.pos_of_rowcell(row, c), ~it->first, solved_it);
          }
        }
      }
    }
    // Find cells that have row-unique numbers and commit to them
    cell_type forward;
    for (int c = 0; c < board.L.N; ++c) {
      const int pos = board.L.pos_of_rowcell(row, c);
      board.try_mask(pos, ~(backward[c] | forward), solved_it);
      forward |= board[pos];
    }
    // Find rows in fields that have row-unique numbers and
    // eliminate the numbers from rest of field.
    for (int fc = 0; fc < board.L.FIELDS_PER_ROW; ++fc) {
      const cell_type mask = field_in[fc] & ~field_out[fc];
      if (mask.any()) {
        const int f = board.L.field_of_pos(board.L.pos_of_rowcell(row, fc * board.L.FIELDS_PER_ROW));
        for (int c = 0; c < board.L.N; ++c) {
          const int pos = board.L.pos_of_fieldcell(f, c);
          if (board.L.row_of_pos(pos) != row) {
            board.apply_mask(pos, ~mask, solved_it);
          }
        }
      }
    }
  }
  void process_column(sudoku_board& board, const int col) {
    using namespace std;
    typedef typename sudoku_board::cell_type cell_type;
    auto solved_it = back_inserter(agenda_m);
    // Analyze column
    cell_type backward[board.L.N];
    cell_type field_out[board.L.FIELDS_PER_COL];
    cell_type field_in[board.L.FIELDS_PER_COL];
    unordered_map<cell_type, unordered_set<int> > uniq_sets;
    for (int c = 0; c < board.L.N; ++c) {
      const int pos = board.L.pos_of_colcell(col, c);
      const cell_type& cell = board[pos];
      if (c != 0) {
        backward[board.L.N-1 - c] = backward[board.L.N - c] | board[board.L.pos_of_colcell(col, board.L.N - c)];
      }
      const int f = board.L.field_of_pos(pos) / board.L.FIELDS_PER_COL;
      field_in[f] |= cell;
      for (int foff = 1; foff < board.L.FIELDS_PER_COL; ++foff) {
        field_out[(f + foff) % board.L.FIELDS_PER_COL] |= cell;
      }
      if(cell.count() > 1) {
        uniq_sets[cell].insert(c);
      }
    }
    // Find unique sets and eliminate their members from other cells
    // in the same column
    for (auto it = begin(uniq_sets); it != end(uniq_sets); ++it) {
      if (it->first.count() == it->second.size()) {
        for (auto c = 0; c < board.L.N; ++c) {
          if (it->second.find(c) == it->second.end()) {
            board.apply_mask(board.L.pos_of_colcell(col, c), ~it->first, solved_it);
          }
        }
      }
    }
    // Find cells that have column-unique numbers and commit to them
    cell_type forward;
    for (int c = 0; c < board.L.N; ++c) {
      const int pos = board.L.pos_of_colcell(col, c);
      board.try_mask(pos, ~(backward[c] | forward), solved_it);
      forward |= board[pos];
    }
    // Find columns in fields that have column-unique numbers and
    // eliminate the numbers from rest of field.
    for (int fc = 0; fc < board.L.FIELDS_PER_COL; ++fc) {
      const cell_type mask = field_in[fc] & ~field_out[fc];
      if (mask.any()) {
        const int f = board.L.field_of_pos(board.L.pos_of_colcell(col, fc * board.L.FIELDS_PER_COL));
        for (int c = 0; c < board.L.N; ++c) {
          const int pos = board.L.pos_of_fieldcell(f, c);
          if (board.L.col_of_pos(pos) != col) {
            board.apply_mask(pos, ~mask, solved_it);
          }
        }
      }
    }
  }
  void process_field(sudoku_board& board, const int field) {
    using namespace std;
    typedef typename sudoku_board::cell_type cell_type;
    auto solved_it = back_inserter(agenda_m);
    // Analyze field
    cell_type backward[board.L.N];
    for (int c = board.L.N-1; c > 0; --c) {
      const auto& cell = board[board.L.pos_of_fieldcell(field, c)];
      backward[c-1] = backward[c] | cell;
    }
    // Find cells that have field-unique numbers and commit to them
    cell_type forward;
    for (int c = 0; c < board.L.N; ++c) {
      const cell_type mask = ~backward[c] & ~forward;
      const int pos = board.L.pos_of_fieldcell(field, c);
      board.try_mask(pos, mask, solved_it);
      forward |= board[pos];
    }
  }
  void operator()(sudoku_board& board) {
    clear_agenda(board);
    while (true) {
      const int unknown = board.unknown();
      for (int rcf = 0; rcf < board.L.N; ++rcf) {
        process_row(board, rcf);
        process_column(board, rcf);
        process_field(board, rcf);
      }
      clear_agenda(board);
      if (board.unknown() == unknown) {
        break;
      }
    }
  }
protected:
  agenda_type agenda_m;
}; // pseudoku_solver



class brut_force_solver {
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
}; // brut_force_solver



int main(const int argc, const char** argv) {
  using namespace std;
  
  sudoku_board board;
  pseudoku_solver solve;
  brut_force_solver brute_force;
  int brute_forced_cells = 0;

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
      brute_force(board);
      ++brute_forced_cells;
      cout << "Board is " << (board.valid() ? "" : "in") << "valid." << endl;
      cout << board << endl;
    }
  }
  
  if (board.valid() && board.solved()) {
    cout << "Found a solution!";
    if (brute_forced_cells > 0) {
      cout << " (" << brute_forced_cells
           << " cell" << (brute_forced_cells > 1 ? "s" : "")
           << " had to be brute-forced)";
    }
    cout << endl;
  }
  return board.valid() ? EXIT_SUCCESS : EXIT_FAILURE;
}
