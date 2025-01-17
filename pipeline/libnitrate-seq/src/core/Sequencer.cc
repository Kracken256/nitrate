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

#include <cstddef>
#include <memory>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-lexer/Lexer.hh>
#include <nitrate-seq/Sequencer.hh>
#include <sys/List.hh>

extern "C" {
#include <lua/lauxlib.h>
#include <lua/lua.h>
#include <lua/lualib.h>
}

#define MAX_RECURSION_DEPTH 10000

using namespace ncc::lex;
using namespace ncc::seq;

Sequencer::PImpl::PImpl() {
  L = luaL_newstate();
  m_random.seed(0);
}

Sequencer::PImpl::~PImpl() {
  if (L) {
    lua_close(L);
  }
}

static inline std::string_view ltrim(std::string_view s) {
  s.remove_prefix(std::min(s.find_first_not_of(" \t\n\r"), s.size()));
  return s;
}

static inline std::string_view rtrim(std::string_view s) {
  s.remove_suffix(
      std::min(s.size() - s.find_last_not_of(" \t\n\r") - 1, s.size()));
  return s;
}

bool Sequencer::ApplyDynamicTransforms(Token last) {
  /**
   * @brief We do it this way because the callback could potentially modify the
   * `m_defer` vector, which would invalidate the iterator.
   * The callback is able to instruct us to keep it around for the next token or
   * to remove it.
   *
   * The callback is able to consume and emit token sequences, which may result
   * in this `ApplyDynamicTransforms` function being called recursively.
   *
   * So like, if any of the callbacks says to emit a token, we should emit a
   * token. Otherwise, we should not emit a token.
   */

  std::vector<DeferCallback> saved;
  saved.reserve(m_core->m_defer.size());
  bool emit_token = m_core->m_defer.empty();

  while (!m_core->m_defer.empty()) {
    auto cb = m_core->m_defer.back();
    m_core->m_defer.pop_back();

    auto op = cb(this, last);
    if (op != UninstallHandler) {
      saved.push_back(cb);
    }
    if (op == EmitToken) {
      emit_token = true;
    }
  }

  m_core->m_defer = saved;

  return emit_token;
}

void Sequencer::RecursiveExpand(std::string_view code) {
  std::istringstream ss(std::string(code.data(), code.size()));
  std::vector<Token> tokens;

  {
    Sequencer clone(ss, m_env, false);
    clone.m_core = m_core;
    clone.m_core->m_depth = m_core->m_depth + 1;

    Token tok;
    while ((tok = (clone.Next())).get_type() != EofF) {
      tokens.push_back(tok);
    }
  }

  for (auto it = tokens.rbegin(); it != tokens.rend(); it++) {
    m_core->m_buffer.push_front(*it);
  }
}

bool Sequencer::ExecuteLua(const char *code) {
  auto top = lua_gettop(m_core->L);
  auto rc = luaL_dostring(m_core->L, code);

  if (rc) {
    qcore_logf(QCORE_ERROR, "Lua error: %s\n", lua_tostring(m_core->L, -1));
    return false;
  }

  if (lua_isstring(m_core->L, -1)) {
    RecursiveExpand(lua_tostring(m_core->L, -1));
  } else if (lua_isnumber(m_core->L, -1)) {
    RecursiveExpand(std::to_string(lua_tonumber(m_core->L, -1)));
  } else if (lua_isboolean(m_core->L, -1)) {
    RecursiveExpand(lua_toboolean(m_core->L, -1) ? "true" : "false");
  } else if (lua_gettop(m_core->L) != top && !lua_isnil(m_core->L, -1)) {
    return false;
  }

  return true;
}

class RecursiveGuard {
  size_t &m_depth;

public:
  RecursiveGuard(size_t &depth) : m_depth(depth) { m_depth++; }
  ~RecursiveGuard() { m_depth--; }

  bool should_stop() { return m_depth >= MAX_RECURSION_DEPTH; }
};

Token Sequencer::GetNext() {
func_entry:  // do tail call optimization manually

  Token x;

  try {
    RecursiveGuard guard(m_core->m_depth);
    if (guard.should_stop()) {
      qcore_logf(QCORE_FATAL, "Maximum macro recursion depth reached\n");
      throw StopException();
    }

    if (!m_core->m_buffer.empty()) {
      x = m_core->m_buffer.front();
      m_core->m_buffer.pop_front();
    } else {
      x = m_scanner->Next();
    }

    switch (x.get_type()) {
      case EofF:
        return x;
      case KeyW:
      case IntL:
      case Text:
      case Char:
      case NumL:
      case Oper:
      case Punc:
      case Note: {
        goto emit_token;
      }

      case Name: { /* Handle the expansion of defines */
        auto key = "def." + std::string(x.as_string());

        if (auto value = m_env->get(key.c_str())) {
          RecursiveExpand(value.value());
          goto func_entry;
        } else {
          goto emit_token;
        }
      }

      case MacB: {
        auto block = ltrim(x.as_string());
        if (!block.starts_with("fn ")) {
          if (!ExecuteLua(std::string(block).c_str())) {
            qcore_logf(QCORE_ERROR, "Failed to expand macro block: %s\n",
                       block.data());
            x = Token::EndOfFile();
            SetFailBit();

            goto emit_token;
          }
        } else {
          block = ltrim(block.substr(3));
          auto pos = block.find_first_of("(");
          if (pos == std::string_view::npos) {
            qcore_logf(QCORE_ERROR, "Invalid macro function definition: %s\n",
                       block.data());
            x = Token::EndOfFile();
            SetFailBit();

            goto emit_token;
          }

          auto name = rtrim(block.substr(0, pos));
          auto code =
              "function " + std::string(name) + std::string(block.substr(pos));

          { /* Remove the opening brace */
            pos = code.find_first_of("{");
            if (pos == std::string::npos) {
              qcore_logf(QCORE_ERROR, "Invalid macro function definition: %s\n",
                         block.data());
              x = Token::EndOfFile();
              SetFailBit();

              goto emit_token;
            }
            code.erase(pos, 1);
          }

          { /* Remove the closing brace */
            pos = code.find_last_of("}");
            if (pos == std::string::npos) {
              qcore_logf(QCORE_ERROR, "Invalid macro function definition: %s\n",
                         block.data());
              x = Token::EndOfFile();
              SetFailBit();

              goto emit_token;
            }
            code.erase(pos, 1);
            code.insert(pos, "end");
          }

          if (!ExecuteLua(code.c_str())) {
            qcore_logf(QCORE_ERROR, "Failed to expand macro function: %s\n",
                       name.data());
            x = Token::EndOfFile();
            SetFailBit();

            goto emit_token;
          }
        }

        goto func_entry;
      }

      case Macr: {
        auto body = x.as_string().get();
        auto pos = body.find_first_of("(");

        if (pos != std::string_view::npos) {
          if (!ExecuteLua(("return " + std::string(body)).c_str())) {
            qcore_logf(QCORE_ERROR, "Failed to expand macro function: %s\n",
                       body.data());
            x = Token::EndOfFile();
            SetFailBit();

            goto emit_token;
          }

          goto func_entry;
        } else {
          if (!ExecuteLua(("return " + std::string(body) + "()").c_str())) {
            qcore_logf(QCORE_ERROR, "Failed to expand macro function: %s\n",
                       body.data());
            x = Token::EndOfFile();
            SetFailBit();

            goto emit_token;
          }

          goto func_entry;
        }
      }
    }

  emit_token:
    if (!ApplyDynamicTransforms(x)) {
      goto func_entry;
    }
  } catch (StopException &) {
    x = Token::EndOfFile();
  }

  return x;
}

void Sequencer::LoadLuaLibs() {
  static constexpr luaL_Reg the_libs[] = {
      {LUA_GNAME, luaopen_base},        {LUA_TABLIBNAME, luaopen_table},
      {LUA_STRLIBNAME, luaopen_string}, {LUA_MATHLIBNAME, luaopen_math},
      {LUA_UTF8LIBNAME, luaopen_utf8},  {NULL, NULL},
  };

  for (auto lib = the_libs; lib->func; lib++) {
    luaL_requiref(m_core->L, lib->name, lib->func, 1);
    lua_pop(m_core->L, 1); /* remove lib */
  }
}

void Sequencer::BindLuaAPI() {
  lua_newtable(m_core->L);

  for (auto Func : SysFunctions) {
    lua_pushinteger(m_core->L, (lua_Integer)(uintptr_t)this);
    lua_pushcclosure(m_core->L, Func.getFunc(), 1);
    lua_setfield(m_core->L, -2, Func.getName().data());
  }

  lua_setglobal(m_core->L, "n");
}

Sequencer::Sequencer(std::istream &file, std::shared_ptr<ncc::Environment> env,
                     bool is_root)
    : ncc::lex::IScanner(env) {
  m_scanner = std::make_unique<Tokenizer>(file, env);

  if (is_root) {
    m_core = std::make_shared<PImpl>();

    LoadLuaLibs();
    BindLuaAPI();
    RecursiveExpand(CodePrefix);
  }
}

std::optional<std::vector<std::string>> Sequencer::GetSourceWindow(
    Point start, Point end, char fillchar) {
  return m_scanner->GetSourceWindow(start, end, fillchar);
}
