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

auto Parser::PImpl::RecurseForInitExpr() -> NullableFlowPtr<Stmt> {
  if (NextIf<PuncSemi>()) {
    return std::nullopt;
  }

  if (NextIf<Let>()) {
    if (auto vars = RecurseVariable(VariableType::Let); vars.size() == 1) {
      return vars[0];
    }

    Log << SyntaxError << Current() << "Expected exactly one variable in for loop";

  } else if (NextIf<Var>()) {
    if (auto vars = RecurseVariable(VariableType::Var); vars.size() == 1) {
      return vars[0];
    }

    Log << SyntaxError << Current() << "Expected exactly one variable in for loop";
  } else if (NextIf<Const>()) {
    if (auto vars = RecurseVariable(VariableType::Const); vars.size() == 1) {
      return vars[0];
    }

    Log << SyntaxError << Current() << "Expected exactly one variable in for loop";

  } else {
    return CreateNode<ExprStmt>(RecurseExpr({
        Token(Punc, PuncSemi),
    }))();
  }

  return std::nullopt;
}

auto Parser::PImpl::RecurseForCondition() -> NullableFlowPtr<Expr> {
  if (NextIf<PuncSemi>()) {
    return std::nullopt;
  }

  auto condition = RecurseExpr({
      Token(Punc, PuncSemi),
  });

  if (!NextIf<PuncSemi>()) {
    Log << SyntaxError << Current() << "Expected semicolon after condition expression";
  }

  return condition;
}

auto Parser::PImpl::RecurseForStepExpr(bool has_paren) -> NullableFlowPtr<Expr> {
  if (has_paren) {
    if (Peek().Is<PuncRPar>()) {
      return std::nullopt;
    }

    return RecurseExpr({
        Token(Punc, PuncRPar),
    });
  }
  if (Peek().Is<OpArrow>() || Peek().Is<PuncLCur>()) {
    return std::nullopt;
  }

  return RecurseExpr({
      Token(Punc, PuncLCur),
      Token(Oper, OpArrow),
  });
}

auto Parser::PImpl::RecurseForBody() -> FlowPtr<Stmt> {
  if (NextIf<OpArrow>()) {
    return RecurseBlock(false, true, SafetyMode::Unknown);
  }

  return RecurseBlock(true, false, SafetyMode::Unknown);
}

auto Parser::PImpl::RecurseFor() -> FlowPtr<Stmt> {
  bool for_with_paren = NextIf<PuncLPar>().has_value();
  auto for_init = RecurseForInitExpr();
  auto for_cond = RecurseForCondition();
  auto for_step = RecurseForStepExpr(for_with_paren);

  if (for_with_paren && !NextIf<PuncRPar>()) {
    Log << SyntaxError << Current() << "Expected closing parenthesis in for statement";
  }

  auto for_body = RecurseForBody();

  return CreateNode<For>(for_init, for_cond, for_step, for_body)();
}
