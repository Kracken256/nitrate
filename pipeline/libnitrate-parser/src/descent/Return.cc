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

FlowPtr<Stmt> Parser::recurse_return() {
  if (next_if(PuncSemi)) {
    return make<ReturnStmt>(std::nullopt)();
  } else {
    auto return_value = recurse_expr({
        Token(Punc, PuncSemi),
    });

    if (!next_if(PuncSemi)) [[unlikely]] {
      log << SyntaxError << current()
          << "Expected ';' after the return statement.";
    }

    return make<ReturnStmt>(return_value)();
  }
}

FlowPtr<Stmt> Parser::recurse_retif() {
  auto return_if = recurse_expr({
      Token(Punc, PuncComa),
  });

  if (next_if(PuncComa)) [[likely]] {
    auto return_value = recurse_expr({
        Token(Punc, PuncSemi),
    });

    if (!next_if(PuncSemi)) [[unlikely]] {
      log << SyntaxError << current() << "Expected ';' after the retif value.";
    }

    return make<ReturnIfStmt>(return_if, return_value)();
  } else {
    log << SyntaxError << current()
        << "Expected ',' after the retif condition.";
    return mock_stmt(QAST_RETIF);
  }
}
