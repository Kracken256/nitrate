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

string Parser::recurse_scope_name() {
  if (auto tok = next_if(Name)) {
    return tok->as_string();
  } else {
    return "";
  }
}

std::optional<ScopeDeps> Parser::recurse_scope_deps() {
  ScopeDeps dependencies;

  if (!next_if(PuncColn)) {
    return dependencies;
  }

  if (next_if(PuncLBrk)) [[likely]] {
    while (true) {
      if (next_if(EofF)) [[unlikely]] {
        log << SyntaxError << current()
            << "Unexpected EOF in scope dependencies";
        break;
      }

      if (next_if(PuncRBrk)) {
        return dependencies;
      }

      if (auto tok = next_if(Name)) {
        auto dependency_name = tok->as_string();
        dependencies.push_back(dependency_name);
      } else {
        log << SyntaxError << next() << "Expected dependency name";
      }

      next_if(PuncComa);
    }
  } else {
    log << SyntaxError << current()
        << "Expected '[' at start of scope dependencies";
  }

  return std::nullopt;
}

FlowPtr<Stmt> Parser::recurse_scope_block() {
  if (next_if(PuncSemi)) {
    return make<Block>(BlockItems(), SafetyMode::Unknown)();
  } else if (next_if(OpArrow)) {
    return recurse_block(false, true, SafetyMode::Unknown);
  } else {
    return recurse_block(true, false, SafetyMode::Unknown);
  }
}

FlowPtr<Stmt> Parser::recurse_scope() {
  auto scope_name = recurse_scope_name();

  if (auto dependencies = recurse_scope_deps()) [[likely]] {
    auto scope_block = recurse_scope_block();

    return make<ScopeStmt>(scope_name, scope_block, dependencies.value())();
  } else {
    log << SyntaxError << current() << "Expected scope dependencies";
  }

  return mock_stmt(QAST_SCOPE);
}
