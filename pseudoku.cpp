#include <iostream>
#include <bitset>
#include <deque>
#include <vector>
#include <functional>
#include <unordered_map>
#include <unordered_set>

static const int M = 3;
static const int N = M*M;

typedef std::bitset<N> cell_type;

struct null_iterator {
  const null_iterator& operator*() const { return *this; }
  template<typename T>
  const null_iterator& operator=(T&&) const { return *this; }
  const null_iterator& operator++() const { return *this; }
  const null_iterator& operator++(int) const { return *this; }
};

class pseudoku {
public:
  void read(std::istream& is) {
    unknown_m = 0;
    int number;
    for (int i = 0; i < N; ++i) {
      for (int j = 0; j < N; ++j) {
	is >> number;
	(*this)(i, j).reset();
	if (number == 0) {
	  for (int k = 0; k < N; ++k) {
	    (*this)(i, j).set(k);
	  }
	  ++unknown_m;
	} else {
	  (*this)(i, j).set(number - 1);
	}
      }
    }
  }
  static std::ostream& print_cell(std::ostream& os, const cell_type& cell) {
    static const char* padding = "          ";
    os << (padding + (cell.count()));
    for (int k = 0; k < N; ++k) {
      if (cell[k]) {
        os << (k + 1);
      }
    }
    return os;
  }
  void print(std::ostream& os) const {
    for (int i = 0; i < N; ++i) {
      for (int j = 0; j < N; ++j) {
        print_cell(os, (*this)(i, j));
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
    // bool result = false;
    bool solved_before = solved(row, col);
    (*this)(row, col) &= mask;
    if (! solved_before && solved(row, col)) {
      *solved_it = std::make_pair(row, col);
      ++solved_it;
      --unknown_m;
      // result = true;
    }
    // return result;
  }
  ///
  /// Only applies the mask if it solves the cell.
  ///
  template<typename solved_it_T>
  void try_mask(int row,
                int col,
                const cell_type& mask,
                solved_it_T&& solved_it) {
    if (! solved(row, col)) {
      cell_type& cell = (*this)(row, col);
      const cell_type new_cell = cell & mask;
      if (new_cell.count() == 1) {
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
  bool solved(int i, int j) const { return (*this)(i, j).count() == 1; }
  const int unknown() const { return unknown_m; }
  int frow(int field, int cell) const { return ((field / M) * M) + (cell / M); }
  int fcol(int field, int cell) const { return ((field % M) * M) + (cell % M); }
  int field(int row, int col) const { return ((row / M) * M) + (col / M); }
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
};

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

int smarter(pseudoku& s) {
  using namespace std;
  cout << "strategy: smarter" << endl;
  int result = 0;
  deque<pair<int, int>> todo;

  result += brute_force_obvious(s, back_inserter(todo));
  while (! todo.empty()) {
    const int i = todo.front().first;
    const int j = todo.front().second;
    todo.pop_front();
    result += s.propagate_solution(i, j, back_inserter(todo));
  }
  while (true) {
    auto solved_it = back_inserter(todo);
    for (int row = 0; row < N; ++row) {
      // Analyze row
      cell_type backward[N];
      cell_type field_out[M];
      cell_type field_in[M];
      unordered_map<cell_type, unordered_set<int> > uniq_sets;
      for (int col = 0; col < N; ++col) {
        if (col != 0) {
          backward[N-(col+1)] = backward[N-col] | s(row, N-col);
          ++result;
        }
        const int f = s.field(row, col) % M;
        const cell_type& cell = s(row, col);
        ++result;
        field_in[f] |= cell;
        for (int foff = 1; foff < M; ++foff) {
          field_out[(f + foff) % M] |= cell;
        }
        if(cell.count() > 1) {
          uniq_sets[cell].insert(col);
        }
      }
      // Find unique sets and eliminate their members from other cells
      // in the same column
      for (auto it = begin(uniq_sets); it != end(uniq_sets); ++it) {
        if (it->first.count() == it->second.size()) {
          for (auto col = 0; col < N; ++col) {
            if (it->second.find(col) == it->second.end()) {
              s.apply_mask(row, col, ~it->first, solved_it);
              ++result;
            }
          }
        }
      }
      // Find cells that have row-unique numbers and commit to them
      cell_type forward;
      for (int col = 0; col < N; ++col) {
        const cell_type mask = ~(backward[col] | forward);
        s.try_mask(row, col, mask, solved_it);
        forward |= s(row, col);
        result += 2;
      }
      // Find rows in fields that have row-unique numbers and
      // eliminate the numbers from rest of field.
      for (int fc = 0; fc < M; ++fc) {
        const cell_type mask = field_in[fc] & ~field_out[fc];
        if (mask.any()) {
          const int f = s.field(row, fc * M);
          for (int c = 0; c < N; ++c) {
            if (s.frow(f, c) != row) {
              s.apply_mask(s.frow(f, c), s.fcol(f, c), ~mask, solved_it);
            }
          }
        }
      }
    } // rows
    for (int col = 0; col < N; ++col) {
      // Analyze column
      cell_type backward[N];
      cell_type field_out[M];
      cell_type field_in[M];
      unordered_map<cell_type, unordered_set<int> > uniq_sets;
      backward[N-1] = cell_type();
      for (int row = 0; row < N; ++row) {
        if (row != 0) {
          backward[N-(row+1)] = backward[N-row] | s(N-row, col);
          ++result;
        }
        const int f = s.field(row, col) / M;
        const cell_type& cell = s(row, col);
        ++result;
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
              s.apply_mask(row, col, ~it->first, solved_it);
              ++result;
            }
          }
        }
      }
      // Find cells that have column-unique numbers and commit to them
      cell_type forward;
      for (int row = 0; row < N; ++row) {
        s.try_mask(row, col, ~backward[row] & ~forward, solved_it);
        forward |= s(row, col);
      }
      result += N;
      // Find columns in fields that have column-unique numbers and
      // eliminate the numbers from rest of field.
      for (int fc = 0; fc < M; ++fc) {
        const cell_type mask = field_in[fc] & ~field_out[fc];
        if (mask.any()) {
          const int f = s.field(fc * M, col);
          for (int c = 0; c < N; ++c) {
            if (s.fcol(f, c) != col) {
              s.apply_mask(s.frow(f, c), s.fcol(f, c), ~mask, solved_it);
            }
          }
        }
      }
    } // columns
    for (int field = 0; field < N; ++field) {
      // Analyze field
      cell_type backward[N];
      backward[N-1] = cell_type();
      for (int f = N-2; f >= 0; --f) {
        backward[f] = backward[f+1] | s(s.frow(field, f+1), s.fcol(field, f+1));
      }
      result += N;
      // Find cells that have field-unique numbers and commit to them
      cell_type forward;
      for (int f = 0; f < N; ++f) {
        s.try_mask(s.frow(field, f), s.fcol(field, f),
                   ~backward[f] & ~forward,
                   solved_it);
        forward |= s(s.frow(field, f), s.fcol(field, f));
      }
    } // fields
    if (todo.empty()) {
      // Failed to solve any cells, give up.
      break;
    }
    while (! todo.empty()) {
      const int i = todo.front().first;
      const int j = todo.front().second;
      todo.pop_front();
      result += s.propagate_solution(i, j, back_inserter(todo));
    }
  }
  return result;
}

int main(const int argc, const char** argv) {
  using namespace std;
  
  pseudoku start;
  start.read(cin);
  start.print(cout);
  cout << endl;
  if (! start.valid()) {
    cout << "Initial board is invalid!" << endl;
    return 1;
  }
  vector<function<int(pseudoku&)>> strategies({silly, silly_fast, smarter});
  for (const auto strategy : strategies) {
    pseudoku s(start);
    int inspected = strategy(s);
    cout << "inspected " << inspected << " cells." << endl;
    if (! s.solved()) {
      cout << "Filed to find a solution :-(" << endl;
    }
    cout << "Board is " << (s.valid() ? "" : "in") << "valid." << endl;
    s.print(cout);
    cout << endl;
  }
  return 0;
}
