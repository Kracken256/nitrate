////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///  ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
///  ░▒▓██████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
///    ░▒▓█▓▒░                                                               ///
///     ░▒▓██▓▒░                                                             ///
///                                                                          ///
///   * QUIX LANG COMPILER - The official compiler for the Quix language.    ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The QUIX Compiler Suite is free software; you can redistribute it or   ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The QUIX Compiler Suite is distributed in the hope that it will be     ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the QUIX Compiler Suite; if not, see                ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#include <cstddef>
#define __QUIX_IMPL__

#include <quix-core/Error.h>
#include <quix-lexer/Lexer.h>
#include <string.h>

#include <boost/bimap.hpp>
#include <boost/unordered_map.hpp>
#include <cctype>
#include <cmath>
#include <csetjmp>
#include <cstdio>
#include <quix-lexer/Base.hh>
#include <stack>
#include <utility>

#include "LibMacro.h"
#include "quix-lexer/Token.h"

///============================================================================///

LIB_EXPORT qlex_t *qlex_direct(const char *src, size_t len, const char *filename) {
  try {
    if (!filename) {
      filename = "<unknown>";
    }

    FILE *file = fmemopen((void *)src, len, "r");
    if (!file) {
      return nullptr;
    }

    qlex_t *obj = qlex_new(file, filename);
    if (!obj) {
      fclose(file);
      return nullptr;
    }

    obj->m_is_owned = true;

    return obj;
  } catch (std::bad_alloc &) {
    return nullptr;
  } catch (...) {
    return nullptr;
  }
}

LIB_EXPORT void qlex_free(qlex_t *obj) {
  try {
    delete obj;
  } catch (...) {
    qcore_panic("qlex_free: failed to free lexer");
  }
}

LIB_EXPORT void qlex_set_flags(qlex_t *obj, qlex_flags_t flags) { obj->m_flags = flags; }
LIB_EXPORT qlex_flags_t qlex_get_flags(qlex_t *obj) { return obj->m_flags; }

LIB_EXPORT void qlex_collect(qlex_t *obj, const qlex_tok_t *tok) { obj->collect_impl(tok); }

LIB_EXPORT void qlex_push(qlex_t *obj, qlex_tok_t tok) { obj->push_impl(&tok); }
LIB_EXPORT const char *qlex_filename(qlex_t *obj) { return obj->m_filename; }

LIB_EXPORT qlex_size qlex_line(qlex_t *obj, qlex_loc_t loc) {
  try {
    auto r = obj->loc2rowcol(loc);
    if (!r) {
      return UINT32_MAX;
    }

    return r->first;
  } catch (...) {
    qcore_panic("qlex_line: failed to get line number");
  }
}

LIB_EXPORT qlex_size qlex_col(qlex_t *obj, qlex_loc_t loc) {
  try {
    auto r = obj->loc2rowcol(loc);
    if (!r) {
      return UINT32_MAX;
    }

    return r->second;
  } catch (...) {
    qcore_panic("qlex_col: failed to get column number");
  }
}

LIB_EXPORT qlex_loc_t qlex_offset(qlex_t *obj, qlex_loc_t base, qlex_size offset) {
  try {
    long curpos;
    std::optional<qlex_size> seek_base_pos;
    uint8_t *buf = nullptr;
    size_t bufsz;

    if (!(seek_base_pos = obj->loc2offset(base))) {
      return base;
    }

    if ((curpos = ftell(obj->m_file)) == -1) {
      return base;
    }

    if (fseek(obj->m_file, *seek_base_pos + offset, SEEK_SET) != 0) {
      return base;
    }

    bufsz = offset;

    if ((buf = (uint8_t *)malloc(bufsz + 1)) == nullptr) {
      fseek(obj->m_file, curpos, SEEK_SET);
      return base;
    }

    if (fread(buf, 1, bufsz, obj->m_file) != bufsz) {
      free(buf);
      fseek(obj->m_file, curpos, SEEK_SET);
      return base;
    }

    buf[bufsz] = '\0';
    fseek(obj->m_file, curpos, SEEK_SET);

    //===== AUTOMATA TO CALCULATE THE NEW ROW AND COLUMN =====//
    uint32_t row, col;

    if ((row = qlex_line(obj, base)) == UINT32_MAX) {
      free(buf);
      return base;
    }

    if ((col = qlex_col(obj, base)) == UINT32_MAX) {
      free(buf);
      return base;
    }

    for (size_t i = 0; i < bufsz; i++) {
      if (buf[i] == '\n') {
        row++;
        col = 1;
      } else {
        col++;
      }
    }

    free(buf);

    return obj->save_loc(row, col, *seek_base_pos + offset);
  } catch (...) {
    qcore_panic("qlex_offset: failed to calculate offset");
  }
}

LIB_EXPORT qlex_size qlex_span(qlex_t *obj, qlex_loc_t start, qlex_loc_t end) {
  try {
    std::optional<qlex_size> begoff, endoff;

    if (!(begoff = obj->loc2offset(start))) {
      return UINT32_MAX;
    }

    if (!(endoff = obj->loc2offset(end))) {
      return UINT32_MAX;
    }

    if (*endoff < *begoff) {
      return 0;
    }

    return *endoff - *begoff;
  } catch (...) {
    qcore_panic("qlex_span: failed to calculate span");
  }
}

LIB_EXPORT qlex_size qlex_spanx(qlex_t *obj, qlex_loc_t start, qlex_loc_t end,
                                void (*callback)(const char *, qlex_size, uintptr_t),
                                uintptr_t userdata) {
  try {
    std::optional<qlex_size> begoff, endoff;

    if (!(begoff = obj->loc2offset(start))) {
      return UINT32_MAX;
    }

    if (!(endoff = obj->loc2offset(end))) {
      return UINT32_MAX;
    }

    if (*endoff < *begoff) {
      return 0;
    }

    qlex_size span = *endoff - *begoff;

    long curpos;
    uint8_t *buf = nullptr;
    size_t bufsz;

    if ((curpos = ftell(obj->m_file)) == -1) {
      return UINT32_MAX;
    }

    if (fseek(obj->m_file, *begoff, SEEK_SET) != 0) {
      return UINT32_MAX;
    }

    bufsz = span;

    if ((buf = (uint8_t *)malloc(bufsz + 1)) == nullptr) {
      fseek(obj->m_file, curpos, SEEK_SET);
      return UINT32_MAX;
    }

    if (fread(buf, 1, bufsz, obj->m_file) != bufsz) {
      free(buf);
      fseek(obj->m_file, curpos, SEEK_SET);
      return UINT32_MAX;
    }

    buf[bufsz] = '\0';
    fseek(obj->m_file, curpos, SEEK_SET);

    callback((const char *)buf, bufsz, userdata);

    free(buf);
    return span;
  } catch (...) {
    qcore_panic("qlex_spanx: failed to calculate span");
  }
}

LIB_EXPORT char *qlex_snippet(qlex_t *obj, qlex_tok_t tok, qlex_size *offset) {
  try {
#define SNIPPET_SIZE 100

    qlex_size tok_beg_offset;
    char snippet_buf[SNIPPET_SIZE];
    qlex_size tok_size = qlex_tok_size(obj, &tok);
    size_t curpos, seek_base_pos, read;

    { /* Convert the location to an offset into the source */
      auto src_offset_opt = obj->loc2offset(tok.start);
      if (!src_offset_opt) {
        return nullptr; /* Return early if translation failed */
      }

      tok_beg_offset = *src_offset_opt - tok_size - 1;
    }

    { /* Calculate offsets and seek to the correct position */
      curpos = ftell(obj->m_file);
      seek_base_pos = tok_beg_offset < SNIPPET_SIZE / 2 ? 0 : tok_beg_offset - SNIPPET_SIZE / 2;

      if (fseek(obj->m_file, seek_base_pos, SEEK_SET) != 0) {
        fseek(obj->m_file, curpos, SEEK_SET);
        return nullptr;
      }
    }

    { /* Read the snippet and calculate token offset */
      read = fread(snippet_buf, 1, SNIPPET_SIZE, obj->m_file);

      if (tok_beg_offset < SNIPPET_SIZE / 2) {
        *offset = tok_beg_offset;
      } else {
        *offset = SNIPPET_SIZE / 2;
      }
    }

    // Extract the line that contains the token
    qlex_size lstart = 0;

    for (size_t i = 0; i < read; i++) {
      if (snippet_buf[i] == '\n') {
        lstart = i + 1;
      } else if (i == *offset) { /* Danger ?? */
        qlex_size count = (i - lstart) + tok_size;
        char *output = (char *)malloc(count + 1);
        memcpy(output, snippet_buf + lstart, count);
        output[count] = '\0';
        *offset -= lstart;
        fseek(obj->m_file, curpos, SEEK_SET);
        return output;
      }
    }

    fseek(obj->m_file, curpos, SEEK_SET);
    return nullptr;
  } catch (std::bad_alloc &) {
    return nullptr;
  } catch (...) {
    qcore_panic("qlex_snippet: failed to get snippet");
  }
}

LIB_EXPORT qlex_tok_t qlex_next(qlex_t *self) {
  try {
    return self->next();
  } catch (...) {
    qcore_panic("qlex_next: failed to get next token");
  }
}

LIB_EXPORT qlex_tok_t qlex_peek(qlex_t *lexer) {
  try {
    return lexer->peek();
  } catch (...) {
    qcore_panic("qlex_peek: failed to peek next token");
  }
}

///============================================================================///

std::optional<qlex_size> qlex_t::loc2offset(qlex_loc_t loc) {
  if (m_tag_to_off.find(loc.tag) == m_tag_to_off.end()) [[unlikely]] {
    return std::nullopt;
  }

  return m_tag_to_off[loc.tag];
}

std::optional<std::pair<qlex_size, qlex_size>> qlex_t::loc2rowcol(qlex_loc_t loc) {
  if (m_tag_to_loc.find(loc.tag) == m_tag_to_loc.end()) [[unlikely]] {
    return std::nullopt;
  }

  clever_me_t it = m_tag_to_loc[loc.tag];

  if (!it.rc_fmt) [[unlikely]] {
    return std::nullopt;
  }

  qlex_size row = it.row;
  qlex_size col = it.col;

  return std::make_pair(row, col);
}

qlex_loc_t qlex_t::save_loc(qlex_size row, qlex_size col, qlex_size offset) {
  clever_me_t bits;
  static_assert(sizeof(bits) == sizeof(qlex_size));

  if (row <= 2097152 || col <= 1024) {
    bits.rc_fmt = 1;
    bits.col = col;
    bits.row = row;

  } else {
    bits.rc_fmt = 0;
  }

  qlex_size tag = m_locctr++;
  m_tag_to_loc[tag] = bits;
  m_tag_to_off[tag] = offset;

  return {tag};
}

qlex_loc_t qlex_t::cur_loc() { return save_loc(m_row, m_col, m_offset); }

///============================================================================///

void qlex_t::push_impl(const qlex_tok_t *tok) {
  /// TODO: Implement this function
  qcore_implement(__func__);
  (void)tok;
}

void qlex_t::collect_impl(const qlex_tok_t *tok) {
  switch (tok->ty) {
    case qEofF:
    case qErro:
    case qKeyW:
    case qOper:
    case qPunc:
      break;
    case qName:
    case qIntL:
    case qNumL:
    case qText:
    case qChar:
    case qMacB:
    case qMacr:
    case qNote:
      release_string(tok->v.str_idx);
      break;
  }
}

///============================================================================///

std::string_view qlex_t::get_string(qlex_size idx) {
#ifdef MEMORY_OVER_SPEED
  if (auto it = m_strings->first.left.find(idx); it != m_strings->first.left.end()) [[likely]] {
    return it->second;
  }
#else
  if (auto it = m_strings->first.find(idx); it != m_strings->first.end()) [[likely]] {
    return it->second;
  }
#endif

  return "";
}

qlex_size qlex_t::put_string(std::string_view str) {
#ifdef MEMORY_OVER_SPEED
  if (auto it = m_strings->first.right.find(str); it != m_strings->first.right.end()) {
    return it->second;
  }

  m_strings->first.insert({m_strings->second, std::string(str)});
  return m_strings->second++;
#else
  if (str.empty()) [[unlikely]] {
    return UINT32_MAX;
  }

  (*m_strings).first[m_strings->second] = std::string(str);
  return m_strings->second++;
#endif
}

void qlex_t::release_string(qlex_size idx) {
#ifdef MEMORY_OVER_SPEED

#else
  if (auto it = m_strings->first.find(idx); it != m_strings->first.end()) [[likely]] {
    m_strings->first.erase(it);
  }
#endif
}

void qlex_t::replace_interner(StringInterner new_interner) { m_strings = new_interner; }

///============================================================================///

static thread_local std::stack<__jmp_buf_tag> getc_jmpbuf;

char qlex_t::getc() {
  /* Refill the buffer if necessary */
  if (m_getc_pos == GETC_BUFFER_SIZE) [[unlikely]] {
    size_t read = fread(m_getc_buf.data(), 1, GETC_BUFFER_SIZE, m_file);

    if (read == 0) [[unlikely]] {
      longjmp(&getc_jmpbuf.top(), 1);
    }

    memset(m_getc_buf.data() + read, '\n', GETC_BUFFER_SIZE - read);
    m_getc_pos = 0;
  }

  char c = m_getc_buf[m_getc_pos++];

  /* Update the row and column */
  if (c == '\n') {
    m_row++;
    m_col = 1;
  } else {
    m_col++;
  }

  m_offset++;

  return c;
}

qlex_tok_t qlex_t::next() {
  qlex_tok_t t = peek();
  m_cur.ty = qErro;
  return t;
}

qlex_tok_t qlex_t::peek() {
  if (m_cur.ty != qErro) {
    return m_cur;
  }

  do {
    m_cur = step_buffer();
  } while (m_flags & QLEX_NO_COMMENTS && m_cur.ty == qNote);

  return m_cur;
}

void qlex_t::refill_buffer() {
  std::fill(m_tok_buf.begin(), m_tok_buf.end(), qlex_tok_t::eof({}, {}));

  getc_jmpbuf.push({});

  if (setjmp(&getc_jmpbuf.top()) == 0) {
    for (size_t i = 0; i < TOKEN_BUF_SIZE; i++) {
      m_tok_buf[i] = next_impl();
    }
  } else [[unlikely]] {
    // We have reached the end of the file
  }

  getc_jmpbuf.pop();
}

qlex_tok_t qlex_t::step_buffer() {
  if (m_tok_pos == TOKEN_BUF_SIZE) {
    refill_buffer();
    m_tok_pos = 0;
  }

  return m_tok_buf[m_tok_pos++];
}