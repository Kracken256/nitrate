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

#include <descent/Recurse.hh>

using namespace ncc;
using namespace ncc::lex;
using namespace ncc::parse;

NullableFlowPtr<Stmt> Parser::recurse_for_init_expr() {
  if (next_if(PuncSemi)) {
    return std::nullopt;
  } else if (next_if(Let)) {
    if (auto vars = recurse_variable(VarDeclType::Let); vars.size() == 1) {
      return vars[0];
    } else {
      log << SyntaxError << current()
          << "Expected exactly one variable in for loop";
    }
  } else if (next_if(Var)) {
    if (auto vars = recurse_variable(VarDeclType::Var); vars.size() == 1) {
      return vars[0];
    } else {
      log << SyntaxError << current()
          << "Expected exactly one variable in for loop";
    }
  } else if (next_if(Const)) {
    if (auto vars = recurse_variable(VarDeclType::Const); vars.size() == 1) {
      return vars[0];
    } else {
      log << SyntaxError << current()
          << "Expected exactly one variable in for loop";
    }
  } else {
    return make<ExprStmt>(recurse_expr({
        Token(Punc, PuncSemi),
    }))();
  }

  return std::nullopt;
}

NullableFlowPtr<Expr> Parser::recurse_for_condition() {
  if (next_if(PuncSemi)) {
    return std::nullopt;
  } else {
    auto condition = recurse_expr({
        Token(Punc, PuncSemi),
    });

    if (!next_if(PuncSemi)) {
      log << SyntaxError << current()
          << "Expected semicolon after condition expression";
    }

    return condition;
  }
}

NullableFlowPtr<Expr> Parser::recurse_for_step_expr(bool has_paren) {
  if (has_paren) {
    if (peek().is<PuncRPar>()) {
      return std::nullopt;
    } else {
      return recurse_expr({
          Token(Punc, PuncRPar),
      });
    }
  } else {
    if (peek().is<OpArrow>() || peek().is<PuncLCur>()) {
      return std::nullopt;
    } else {
      return recurse_expr({
          Token(Punc, PuncLCur),
          Token(Oper, OpArrow),
      });
    }
  }
}

FlowPtr<Stmt> Parser::recurse_for_body() {
  if (next_if(OpArrow)) {
    return recurse_block(false, true, SafetyMode::Unknown);
  } else {
    return recurse_block(true, false, SafetyMode::Unknown);
  }
}

FlowPtr<Stmt> Parser::recurse_for() {
  bool for_with_paren = next_if(PuncLPar).has_value();
  auto for_init = recurse_for_init_expr();
  auto for_cond = recurse_for_condition();
  auto for_step = recurse_for_step_expr(for_with_paren);

  if (for_with_paren && !next_if(PuncRPar)) {
    log << SyntaxError << current()
        << "Expected closing parenthesis in for statement";
  }

  auto for_body = recurse_for_body();

  return make<ForStmt>(for_init, for_cond, for_step, for_body)();
}
