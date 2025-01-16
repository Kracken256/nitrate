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

#include <nitrate-lexer/Lexer.hh>
#include <nitrate-seq/Sequencer.hh>
#include <sys/List.hh>

extern "C" {
#include <lua/lauxlib.h>
}

using namespace ncc::lex;

int ncc::seq::SysNext(lua_State* L) {
  Token tok = get_engine()->Next();

  lua_newtable(L);

  lua_pushstring(L, "ty");
  lua_pushstring(L, qlex_ty_str(tok.GetKind()));
  lua_settable(L, -3);

  lua_pushstring(L, "v");
  switch (tok.GetKind()) {
    case EofF:
    case KeyW: {
      lua_pushstring(L, kw_repr(tok.GetKeyword()));
      break;
    }
    case Oper: {
      lua_pushstring(L, op_repr(tok.GetOperator()));
      break;
    }
    case Punc: {
      lua_pushstring(L, punct_repr(tok.GetPunctor()));
      break;
    }
    case IntL:
    case NumL:
    case Text:
    case Char:
    case Name:
    case MacB:
    case Macr:
    case Note: {
      lua_pushstring(L, std::string(tok.GetString()).c_str());
      break;
    }
  }

  lua_settable(L, -3);

  return 1;
}