#include <iostream>
#include <bitset>
#include <deque>
#include <vector>
#include <functional>
#include <unordered_map>
#include <unordered_set>

static const int M = 3;
static const int N = M*M;
inline const int frow(int field, int cell) {
  return ((field / M) * M) + (cell / M);
}
inline const int fcol(int field, int cell) {
  return ((field % M) * M) + (cell % M);
}
inline const int field(int row, int col) {
  return ((row / M) * M) + (col / M);
}

typedef std::bitset<N> cell_type;

std::ostream& operator<<(std::ostream& os, const cell_type& cell) {
  static const char* padding = "         ";
  os << (padding + (cell.count()));
  for (int k = 0; k < N; ++k) {
    if (cell[k]) {
      os << (k + 1);
    }
  }
  return os;
}

struct null_iterator {
  const null_iterator& operator*() const { return *this; }
  template<typename T>
  const null_iterator& operator=(T&&) const { return *this; }
  const null_iterator& operator++() const { return *this; }
  const null_iterator& operator++(int) const { return *this; }
};

class pseudoku {
public:
  template<typename solved_it_T>
  void read(std::istream& is, solved_it_T&& solved_it) {
    unknown_m = 0;
    int number;
    for (int row = 0; row < N; ++row) {
      for (int col = 0; col < N; ++col) {
	is >> number;
        cell_type& cell = (*this)(row, col);
	cell.reset();
	if (number == 0) {
          cell = ~cell;
	  ++unknown_m;
	} else {
          cell.set(number - 1);
          *solved_it = std::make_pair(row, col);
          ++solved_it;
	}
      }
    }
  }
  void print(std::ostream& os) const {
    for (int i = 0; i < N; ++i) {
      for (int j = 0; j < N; ++j) {
        if (j != 0) { os << ' '; }
        os << (*this)(i, j);
      }
      os << std::endl;
    }
  }
  const cell_type get_known_buddies(const int row, const int col) {
    using namespace std;
    cell_type result;
    for (int c = 0; c < N; ++c) {
      if (c != col && solved(row, c)) {
        result |= (*this)(row, c);
      }
      if (c != row && solved(c, col)) {
        result |= (*this)(c, col);
      }
      const int f = field(row, col);
      if (frow(f, c) != row && fcol(f, c) != col &&
          solved(frow(f, c), fcol(f, c))) {
        result |= (*this)(frow(field(row, col), c), fcol(field(row, col), c));
      }
    }
    return result;
  }
  template<typename solved_it_T>
  void apply_mask(int row,
                  int col,
                  const cell_type& mask,
                  solved_it_T&& solved_it) {
    cell_type& cell = (*this)(row, col);
    if (! solved(cell)) {
      cell &= mask;
      if (solved(cell)) {
        *solved_it = std::make_pair(row, col);
        ++solved_it;
        --unknown_m;
      }
    }
  }
  ///
  /// Only applies the mask if it solves the cell.
  ///
  template<typename solved_it_T>
  void try_mask(int row,
                int col,
                const cell_type& mask,
                solved_it_T&& solved_it) {
    cell_type& cell = (*this)(row, col);
    if (! solved(cell)) {
      const cell_type new_cell = cell & mask;
      if (solved(new_cell)) {
        cell = new_cell;
        *solved_it = std::make_pair(row, col);
        ++solved_it;
        --unknown_m;
      }
    }
  }
  cell_type& operator()(int i, int j) { return board_m[i][j]; }
  const cell_type& operator()(int i, int j) const { return board_m[i][j]; }
  cell_type& operator()(const std::pair<int, int>& ij) {
    return board_m[ij.first][ij.second];
  }
  const cell_type& operator()(const std::pair<int, int>& ij) const {
    return board_m[ij.first][ij.second];
  }
  bool solved() const { return unknown_m == 0; }
  bool solved(const cell_type& cell) const { return cell.count() == 1; }
  bool solved(const int i, const int j) const { return solved((*this)(i, j)); }
  const int unknown() const { return unknown_m; }
  template<typename solved_it_T>
  int propagate_solution(const int row, const int col, solved_it_T&& solved_it) {
    using namespace std;
    cell_type mask = ~(*this)(row, col);
    for (int c = 0; c < N; ++c) {
      if (c != col) {
        apply_mask(row, c, mask, solved_it);
      }
      if (c != row) {
        apply_mask(c, col, mask, solved_it);
      }
      const int f = field(row, col);
      if (frow(f, c) != row && fcol(f, c) != col) {
        apply_mask(frow(f, c), fcol(f, c), mask, solved_it);
      }
    }
    return 3 * (N-1);
  }
  bool valid() const {
    bool result = true;
    for (int i = 0; result && i < N; ++i) {
      cell_type row, col, field;
      for (int j = 0; j < N; ++j) {
        row |= (*this)(i, j);
        col |= (*this)(j, i);
        field |= (*this)(frow(i, j), fcol(i, j));
      }
      result = result && row.all() && col.all() && field.all();
    }
    return result;
  }
protected:
  cell_type board_m[N][N];
  int unknown_m;
}; // pseudoku


class pseudoku_solver {
public:
  typedef std::pair<int, int> coord_type;
  typedef std::deque<coord_type> agenda_type;
  agenda_type& agenda() { return agenda_m; }
  int clear_agenda(pseudoku& board) {
    int result = 0;
    while (! empty()) {
      const int row = top_row();
      const int col = top_col();
      pop();
      board.propagate_solution(row, col, back_inserter(agenda_m));
      ++result;
    }
    return result;
  }
  void process_row(pseudoku& board, const int row) {
    using namespace std;
    auto solved_it = back_inserter(agenda_m);
    // Analyze row
    cell_type backward[N];
    cell_type field_out[M];
    cell_type field_in[M];
    unordered_map<cell_type, unordered_set<int> > uniq_sets;
    for (int col = 0; col < N; ++col) {
      if (col != 0) {
        backward[N-(col+1)] = backward[N-col] | board(row, N-col);
      }
      const int f = field(row, col) % M;
      const cell_type& cell = board(row, col);
      field_in[f] |= cell;
      for (int foff = 1; foff < M; ++foff) {
        field_out[(f + foff) % M] |= cell;
      }
      if(cell.count() > 1) {
        uniq_sets[cell].insert(col);
      }
    }
    // Find unique sets and eliminate their members from other cells
    // in the same row
    for (auto it = begin(uniq_sets); it != end(uniq_sets); ++it) {
      if (it->first.count() == it->second.size()) {
        for (auto col = 0; col < N; ++col) {
          if (it->second.find(col) == it->second.end()) {
            board.apply_mask(row, col, ~it->first, solved_it);
          }
        }
      }
    }
    // Find cells that have row-unique numbers and commit to them
    cell_type forward;
    for (int col = 0; col < N; ++col) {
      const cell_type mask = ~(backward[col] | forward);
      board.try_mask(row, col, mask, solved_it);
      forward |= board(row, col);
    }
    // Find rows in fields that have row-unique numbers and
    // eliminate the numbers from rest of field.
    for (int fc = 0; fc < M; ++fc) {
      const cell_type mask = field_in[fc] & ~field_out[fc];
      if (mask.any()) {
        const int f = field(row, fc * M);
        for (int c = 0; c < N; ++c) {
          if (frow(f, c) != row) {
            board.apply_mask(frow(f, c), fcol(f, c), ~mask, solved_it);
          }
        }
      }
    }
  }
  void process_column(pseudoku& board, const int col) {
    using namespace std;
    auto solved_it = back_inserter(agenda_m);
    // Analyze column
    cell_type backward[N];
    cell_type field_out[M];
    cell_type field_in[M];
    unordered_map<cell_type, unordered_set<int> > uniq_sets;
    backward[N-1] = cell_type();
    for (int row = 0; row < N; ++row) {
      if (row != 0) {
        backward[N-(row+1)] = backward[N-row] | board(N-row, col);
      }
      const int f = field(row, col) / M;
      const cell_type& cell = board(row, col);
      field_in[f] |= cell;
      for (int foff = 1; foff < M; ++foff) {
        field_out[(f + foff) % M] |= cell;
      }
      if(cell.count() > 1) {
        uniq_sets[cell].insert(row);
      }
    }
    // Find unique sets and eliminate their members from other cells
    // in the same column
    for (auto it = begin(uniq_sets); it != end(uniq_sets); ++it) {
      if (it->first.count() == it->second.size()) {
        for (auto row = 0; row < N; ++row) {
          if (it->second.find(row) == it->second.end()) {
            board.apply_mask(row, col, ~it->first, solved_it);
          }
        }
      }
    }
    // Find cells that have column-unique numbers and commit to them
    cell_type forward;
    for (int row = 0; row < N; ++row) {
      board.try_mask(row, col, ~backward[row] & ~forward, solved_it);
      forward |= board(row, col);
    }
    // Find columns in fields that have column-unique numbers and
    // eliminate the numbers from rest of field.
    for (int fc = 0; fc < M; ++fc) {
      const cell_type mask = field_in[fc] & ~field_out[fc];
      if (mask.any()) {
        const int f = field(fc * M, col);
        for (int c = 0; c < N; ++c) {
          if (fcol(f, c) != col) {
            board.apply_mask(frow(f, c), fcol(f, c), ~mask, solved_it);
          }
        }
      }
    }
  }
  void process_field(pseudoku& board, const int field) {
    using namespace std;
    auto solved_it = back_inserter(agenda_m);
    // Analyze field
    cell_type backward[N];
    backward[N-1] = cell_type();
    for (int f = N-2; f >= 0; --f) {
      backward[f] = backward[f+1] | board(frow(field, f+1), fcol(field, f+1));
    }
    // Find cells that have field-unique numbers and commit to them
    cell_type forward;
    for (int f = 0; f < N; ++f) {
      const cell_type mask = ~backward[f] & ~forward;
      board.try_mask(frow(field, f), fcol(field, f), mask, solved_it);
      forward |= board(frow(field, f), fcol(field, f));
    }
  }
  void operator()(pseudoku& s) {
    clear_agenda(s);
    while (true) {
      for (int row = 0; row < N; ++row) {
        process_row(s, row);
      }
      for (int col = 0; col < N; ++col) {
        process_column(s, col);
      }
      for (int field = 0; field < N; ++field) {
        process_field(s, field);
      }
      if (clear_agenda(s) == 0) {
        break;
      }
    }
  }
  const int top_row() const { return agenda_m.front().first; }
  const int top_col() const { return agenda_m.front().second; }
  void pop() { agenda_m.pop_front(); }
  bool empty() const { return agenda_m.empty(); }
protected:
  agenda_type agenda_m;
}; // pseudoku_solver


template<typename solved_it_T>
int brute_force_obvious(pseudoku& s, solved_it_T&& solved_it) {
  int result = N*N;
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      if (s.solved(i, j)) {
        result += 1;
      } else {
        s.apply_mask(i, j, ~s.get_known_buddies(i, j), solved_it);
        result += 1 + (3 * (N-1));
      }
    }
  }
  return result;
}

int silly(pseudoku& s) {
  std::cout << "strategy: silly" << std::endl;
  int result = 0;
  while (! s.solved()) {
    int prev_unknown = s.unknown();
    result += brute_force_obvious(s, null_iterator());
    if (prev_unknown == s.unknown()) {
      break;
    }
  }
  return result;
}

int silly_fast(pseudoku& s) {
  using namespace std;
  cout << "strategy: silly_fast" << endl;
  int result = 0;
  deque<pair<int, int>> todo;
  result += brute_force_obvious(s, back_inserter(todo));
  while (! todo.empty()) {
    const int i = todo.front().first;
    const int j = todo.front().second;
    todo.pop_front();
    result += s.propagate_solution(i, j, back_inserter(todo));
  }
  return result;
}


int main(const int argc, const char** argv) {
  using namespace std;

  pseudoku board;
  pseudoku_solver solve;
  board.read(cin, back_inserter(solve.agenda()));
  if (! board.valid()) {
    cerr << "Provided board not valid." << endl;
    return EXIT_FAILURE;
  }
  board.print(cout);
  cout << endl;
  
  solve(board);
  
  if (! board.solved()) {
    cout << "Failed to find a solution :-(" << endl;
  }
  cout << "Board is " << (board.valid() ? "" : "in") << "valid." << endl;
  board.print(cout);
  cout << endl;
  
  return EXIT_SUCCESS;
}
