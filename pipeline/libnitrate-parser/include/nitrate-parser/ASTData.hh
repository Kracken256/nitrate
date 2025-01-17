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

#ifndef __NITRATE_AST_ASTDATA_H__
#define __NITRATE_AST_ASTDATA_H__

#include <memory>
#include <nitrate-core/Allocate.hh>
#include <nitrate-core/FlowPtr.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-core/NullableFlowPtr.hh>
#include <nitrate-core/String.hh>
#include <nitrate-parser/ASTCommon.hh>
#include <nitrate-parser/ASTVisitor.hh>
#include <variant>
#include <vector>

namespace ncc::parse {
  extern thread_local std::unique_ptr<ncc::IMemory> npar_allocator;

  template <class T>
  struct Arena {
    using value_type = T;

    Arena() = default;

    template <class U>
    constexpr Arena(const Arena<U> &) {}

    [[nodiscard]] T *allocate(std::size_t n) {
      return static_cast<T *>(npar_allocator->alloc(sizeof(T) * n));
    }

    void deallocate(T *p, std::size_t n) {
      (void)n;
      (void)p;
    }
  };

  template <class T, class U>
  bool operator==(const Arena<T> &, const Arena<U> &) {
    return true;
  }
  template <class T, class U>
  bool operator!=(const Arena<T> &, const Arena<U> &) {
    return false;
  }
};  // namespace ncc::parse

namespace ncc::parse {
  using ExpressionList = std::vector<FlowPtr<Expr>, Arena<FlowPtr<Expr>>>;
  using TupleTyItems = std::vector<FlowPtr<Type>, Arena<FlowPtr<Type>>>;
  using CallArg = std::pair<string, FlowPtr<Expr>>;
  using CallArgs = std::vector<CallArg, Arena<CallArg>>;
  using FStringItem = std::variant<string, FlowPtr<Expr>>;
  using FStringItems = std::vector<FStringItem, Arena<FStringItem>>;

  using TemplateParameter =
      std::tuple<string, FlowPtr<Type>, NullableFlowPtr<Expr>>;
  using TemplateParameters =
      std::vector<TemplateParameter, Arena<TemplateParameter>>;

  using BlockItems = std::vector<FlowPtr<Stmt>, Arena<FlowPtr<Stmt>>>;
  using ScopeDeps = std::vector<string, Arena<string>>;

  using SwitchCases = std::vector<FlowPtr<CaseStmt>, Arena<FlowPtr<CaseStmt>>>;
  using EnumItem = std::pair<string, NullableFlowPtr<Expr>>;
  using EnumDefItems = std::vector<EnumItem, Arena<EnumItem>>;

  class StructField {
    string m_name;
    NullableFlowPtr<Expr> m_value;
    FlowPtr<Type> m_type;
    Vis m_vis;
    bool m_is_static;

  public:
    StructField(Vis vis, bool is_static, string name, FlowPtr<Type> type,
                NullableFlowPtr<Expr> value)
        : m_name(std::move(name)),
          m_value(std::move(value)),
          m_type(type),
          m_vis(vis),
          m_is_static(is_static) {}

    auto get_vis() const { return m_vis; }
    auto is_static() const { return m_is_static; }
    auto get_name() const { return m_name.get(); }
    auto get_type() const { return m_type; }
    auto get_value() const { return m_value; }
  };

  struct StructFunction {
    Vis vis;
    FlowPtr<Stmt> func;

    StructFunction(Vis vis, FlowPtr<Stmt> func) : vis(vis), func(func) {}
  };

  using StructDefFields = std::vector<StructField, Arena<StructField>>;
  using StructDefMethods = std::vector<StructFunction, Arena<StructFunction>>;
  using StructDefStaticMethods =
      std::vector<StructFunction, Arena<StructFunction>>;
  using StructDefNames = std::vector<string, Arena<string>>;

  using FuncParam = std::tuple<string, FlowPtr<Type>, NullableFlowPtr<Expr>>;
  using FuncParams = std::vector<FuncParam, Arena<FuncParam>>;

  using FnCaptures =
      std::vector<std::pair<string, bool>, Arena<std::pair<string, bool>>>;
}  // namespace ncc::parse

#endif
