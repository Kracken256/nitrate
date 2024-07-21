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

#ifndef __QUIXCC_IR_Q_NODES_FUNCTION_H__
#define __QUIXCC_IR_Q_NODES_FUNCTION_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <quixcc/IR/Q/Expr.h>
#include <quixcc/IR/Q/QIR.h>
#include <quixcc/IR/Q/Type.h>

namespace libquixcc::ir::q {
  class Block : public Value {
protected:
    bool print_impl(std::ostream &os, PState &state) const override;
    boost::uuids::uuid hash_impl() const override;
    bool verify_impl() const override;

    Block(std::vector<Value *> stmts) : stmts(stmts) { ntype = (int)QType::Block; }

public:
    static Block *create(std::vector<Value *> stmts);

    std::vector<Value *> stmts;
  };

  class Segment : public Expr {
protected:
    bool print_impl(std::ostream &os, PState &state) const override;
    boost::uuids::uuid hash_impl() const override;
    bool verify_impl() const override;

    Segment(std::vector<std::pair<std::string, Type *>> params, Type *return_type, Block *block,
            bool is_variadic, bool is_pure, bool is_thread_safe, bool is_no_throw,
            bool is_no_return, bool is_foriegn)
        : params(params), return_type(return_type), block(block), is_variadic(is_variadic),
          is_pure(is_pure), is_thread_safe(is_thread_safe), is_no_throw(is_no_throw),
          is_no_return(is_no_return), is_foriegn(is_foriegn) {
      ntype = (int)QType::Segment;
    }

public:
    static Segment *create(std::vector<std::pair<std::string, Type *>> params, Type *return_type,
                           Block *block, bool is_variadic, bool is_pure, bool is_thread_safe,
                           bool is_no_throw, bool is_no_return, bool is_foriegn);
    Type *infer() const override;

    std::vector<std::pair<std::string, Type *>> params;
    Type *return_type;
    Block *block;
    bool is_variadic;
    bool is_pure;
    bool is_thread_safe;
    bool is_no_throw;
    bool is_no_return;
    bool is_foriegn;
  };
} // namespace libquixcc::ir::q

#endif // __QUIXCC_IR_Q_NODES_FUNCTION_H__