////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///     .-----------------.    .----------------.     .----------------.     ///
///    | .--------------. |   | .--------------. |   | .--------------. |    ///
///    | | ____  _____  | |   | |     ____     | |   | |    ______    | |    ///
///    | ||_   _|_   _| | |   | |   .'    `.   | |   | |   / ____ `.  | |    ///
///    | |  |   \ | |   | |   | |  /  .--.  \  | |   | |   `'  __) |  | |    ///
///    | |  | |\ \| |   | |   | |  | |    | |  | |   | |   _  |__ '.  | |    ///
///    | | _| |_\   |_  | |   | |  \  `--'  /  | |   | |  | \____) |  | |    ///
///    | ||_____|\____| | |   | |   `.____.'   | |   | |   \______.'  | |    ///
///    | |              | |   | |              | |   | |              | |    ///
///    | '--------------' |   | '--------------' |   | '--------------' |    ///
///     '----------------'     '----------------'     '----------------'     ///
///                                                                          ///
///   * NITRATE TOOLCHAIN - The official toolchain for the Nitrate language. ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The Nitrate Toolchain is free software; you can redistribute it or     ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The Nitrate Toolcain is distributed in the hope that it will be        ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the Nitrate Toolchain; if not, see                  ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#ifndef __NITRATE_PARSE_H__
#define __NITRATE_PARSE_H__

#include <nitrate-core/Macro.h>
#include <nitrate-lexer/Token.h>
#include <nitrate-parser/Node.h>
#include <nitrate-parser/Parser.h>

#include <core/ParseReport.hh>
#include <core/ParserStruct.hh>
#include <set>
#include <unordered_set>

namespace npar {
  Stmt *recurse_pub(npar_t &S, qlex_t &rd);
  Stmt *recurse_sec(npar_t &S, qlex_t &rd);
  Stmt *recurse_pro(npar_t &S, qlex_t &rd);
  std::vector<Stmt *> recurse_variable(npar_t &S, qlex_t &rd, VarDeclType type);
  Stmt *recurse_enum(npar_t &S, qlex_t &rd);
  Stmt *recurse_struct(npar_t &S, qlex_t &rd, CompositeType type);
  Stmt *recurse_scope(npar_t &S, qlex_t &rd);
  Stmt *recurse_function(npar_t &S, qlex_t &rd);
  Type *recurse_type(npar_t &S, qlex_t &rd);
  Stmt *recurse_typedef(npar_t &S, qlex_t &rd);
  Stmt *recurse_return(npar_t &S, qlex_t &rd);
  Stmt *recurse_retif(npar_t &S, qlex_t &rd);
  Stmt *recurse_if(npar_t &S, qlex_t &rd);
  Stmt *recurse_while(npar_t &S, qlex_t &rd);
  Stmt *recurse_for(npar_t &S, qlex_t &rd);
  Stmt *recurse_foreach(npar_t &S, qlex_t &rd);
  Stmt *recurse_switch(npar_t &S, qlex_t &rd);
  Stmt *recurse_inline_asm(npar_t &S, qlex_t &rd);
  Stmt *recurse_composite_field(npar_t &S, qlex_t &rd);

  bool recurse_attributes(npar_t &S, qlex_t &rd, std::set<Expr *> &attributes);

  Stmt *recurse_block(npar_t &S, qlex_t &rd, bool expect_braces = true,
                      bool single_stmt = false);

  Expr *recurse_expr(npar_t &S, qlex_t &rd, std::set<qlex_tok_t> terminators,
                     size_t depth = 0);

#define next() qlex_next(&rd)
#define peek() qlex_peek(&rd)
#define current() qlex_current(&rd)

  template <auto tok>
  static std::optional<qlex_tok_t> next_if_(qlex_t &rd) {
    let t = peek();
    if constexpr (std::is_same_v<decltype(tok), qlex_ty_t>) {
      if (t.is(tok)) {
        next();
        return t;
      }
    } else {
      if (t.is<tok>()) {
        next();
        return t;
      }
    }

    return std::nullopt;
  }

#define next_if(tok) next_if_<tok>(rd)

};  // namespace npar

#endif  // __NITRATE_PARSE_H__