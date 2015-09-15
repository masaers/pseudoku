#ifndef COM_MASAERS_PSEUDOKU_HPP
#define COM_MASAERS_PSEUDOKU_HPP
#include "sudoku.hpp"
#include <unordered_map>
#include <unordered_set>

namespace com_masaers {
  /**
     My sudoku solver
   */
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
} // namespace com_masaers


#endif
