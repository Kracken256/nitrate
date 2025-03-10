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

#ifndef __NITRATE_AST_STMT_H__
#define __NITRATE_AST_STMT_H__

#include <nitrate-parser/ASTBase.hh>
#include <nitrate-parser/ASTData.hh>
#include <span>

namespace ncc::parse {
  enum class BlockMode : uint8_t {
    Unknown = 0,
    Safe = 1,
    Unsafe = 2,
  };

  class Block final : public Expr {
    std::span<FlowPtr<Expr>> m_items;
    BlockMode m_safety;

  public:
    constexpr Block(auto items, auto safety) : Expr(QAST_BLOCK), m_items(items), m_safety(safety) {}

    [[nodiscard]] constexpr auto GetStatements() const { return m_items; }
    [[nodiscard]] constexpr auto GetSafety() const { return m_safety; }

    constexpr void SetStatements(auto items) { m_items = items; }
    constexpr void SetSafety(auto safety) { m_safety = safety; }
  };

  enum class VariableType : uint8_t { Const, Var, Let };

  class Variable final : public Expr {
    std::span<FlowPtr<Expr>> m_attributes;
    FlowPtr<Type> m_type;
    NullableFlowPtr<Expr> m_value;
    VariableType m_decl_type;
    string m_name;

  public:
    constexpr Variable(auto name, auto type, auto value, auto decl_type, auto attributes)
        : Expr(QAST_VAR),
          m_attributes(attributes),
          m_type(std::move(type)),
          m_value(std::move(value)),
          m_decl_type(decl_type),
          m_name(name) {}

    [[nodiscard]] constexpr auto GetName() const { return m_name; }
    [[nodiscard]] constexpr auto GetType() const { return m_type; }
    [[nodiscard]] constexpr auto GetInitializer() const { return m_value; }
    [[nodiscard]] constexpr auto GetVariableKind() const { return m_decl_type; }
    [[nodiscard]] constexpr auto GetAttributes() const { return m_attributes; }

    constexpr void SetName(auto name) { m_name = name; }
    constexpr void SetType(auto type) { m_type = std::move(type); }
    constexpr void SetInitializer(auto value) { m_value = std::move(value); }
    constexpr void SetVariableKind(auto decl_type) { m_decl_type = decl_type; }
    constexpr void SetAttributes(auto attributes) { m_attributes = attributes; }
  };

  class Assembly final : public Expr {
    std::span<FlowPtr<Expr>> m_args;
    string m_code;

  public:
    constexpr Assembly(auto code, auto args) : Expr(QAST_INLINE_ASM), m_args(args), m_code(code) {}

    [[nodiscard]] constexpr auto GetCode() const { return m_code; }
    [[nodiscard]] constexpr auto GetArguments() const { return m_args; }

    constexpr void SetCode(auto code) { m_code = code; }
    constexpr void SetArguments(auto args) { m_args = args; }
  };

  class If final : public Expr {
    FlowPtr<Expr> m_cond, m_then;
    NullableFlowPtr<Expr> m_else;

  public:
    constexpr If(auto cond, auto then, auto ele)
        : Expr(QAST_IF), m_cond(std::move(cond)), m_then(std::move(then)), m_else(std::move(ele)) {}

    [[nodiscard]] constexpr auto GetCond() const { return m_cond; }
    [[nodiscard]] constexpr auto GetThen() const { return m_then; }
    [[nodiscard]] constexpr auto GetElse() const { return m_else; }

    constexpr void SetCond(auto cond) { m_cond = std::move(cond); }
    constexpr void SetThen(auto then) { m_then = std::move(then); }
    constexpr void SetElse(auto ele) { m_else = std::move(ele); }
  };

  class While final : public Expr {
    FlowPtr<Expr> m_cond, m_body;

  public:
    constexpr While(auto cond, auto body) : Expr(QAST_WHILE), m_cond(std::move(cond)), m_body(std::move(body)) {}

    [[nodiscard]] constexpr auto GetCond() const { return m_cond; }
    [[nodiscard]] constexpr auto GetBody() const { return m_body; }

    constexpr void SetCond(auto cond) { m_cond = std::move(cond); }
    constexpr void SetBody(auto body) { m_body = std::move(body); }
  };

  class For final : public Expr {
    NullableFlowPtr<Expr> m_init, m_cond, m_step;
    FlowPtr<Expr> m_body;

  public:
    constexpr For(auto init, auto cond, auto step, auto body)
        : Expr(QAST_FOR),
          m_init(std::move(init)),
          m_cond(std::move(cond)),
          m_step(std::move(step)),
          m_body(std::move(body)) {}

    [[nodiscard]] constexpr auto GetInit() const { return m_init; }
    [[nodiscard]] constexpr auto GetCond() const { return m_cond; }
    [[nodiscard]] constexpr auto GetStep() const { return m_step; }
    [[nodiscard]] constexpr auto GetBody() const { return m_body; }

    constexpr void SetInit(auto init) { m_init = std::move(init); }
    constexpr void SetCond(auto cond) { m_cond = std::move(cond); }
    constexpr void SetStep(auto step) { m_step = std::move(step); }
    constexpr void SetBody(auto body) { m_body = std::move(body); }
  };

  class Foreach final : public Expr {
    FlowPtr<Expr> m_expr, m_body;
    string m_idx_ident, m_val_ident;

  public:
    constexpr Foreach(auto idx_ident, auto val_ident, auto expr, auto body)
        : Expr(QAST_FOREACH),
          m_expr(std::move(expr)),
          m_body(std::move(body)),
          m_idx_ident(idx_ident),
          m_val_ident(val_ident) {}

    [[nodiscard]] constexpr auto GetIndex() const { return m_idx_ident; }
    [[nodiscard]] constexpr auto GetValue() const { return m_val_ident; }
    [[nodiscard]] constexpr auto GetExpr() const { return m_expr; }
    [[nodiscard]] constexpr auto GetBody() const { return m_body; }

    constexpr void SetIndex(auto idx_ident) { m_idx_ident = idx_ident; }
    constexpr void SetValue(auto val_ident) { m_val_ident = val_ident; }
    constexpr void SetExpr(auto expr) { m_expr = std::move(expr); }
    constexpr void SetBody(auto body) { m_body = std::move(body); }
  };

  class Break final : public Expr {
  public:
    constexpr Break() : Expr(QAST_BREAK){};
  };

  class Continue final : public Expr {
  public:
    constexpr Continue() : Expr(QAST_CONTINUE){};
  };

  class Return final : public Expr {
    NullableFlowPtr<Expr> m_value;

  public:
    constexpr Return(auto value) : Expr(QAST_RETURN), m_value(std::move(value)) {}

    [[nodiscard]] constexpr auto GetValue() const { return m_value; }

    constexpr void SetValue(auto value) { m_value = std::move(value); }
  };

  class ReturnIf final : public Expr {
    FlowPtr<Expr> m_cond;
    NullableFlowPtr<Expr> m_value;

  public:
    constexpr ReturnIf(auto cond, auto value) : Expr(QAST_RETIF), m_cond(std::move(cond)), m_value(std::move(value)) {}

    [[nodiscard]] constexpr auto GetCond() const { return m_cond; }
    [[nodiscard]] constexpr auto GetValue() const { return m_value; }

    constexpr void SetCond(auto cond) { m_cond = std::move(cond); }
    constexpr void SetValue(auto value) { m_value = std::move(value); }
  };

  class Case final : public Expr {
    FlowPtr<Expr> m_cond, m_body;

  public:
    constexpr Case(auto cond, auto body) : Expr(QAST_CASE), m_cond(std::move(cond)), m_body(std::move(body)) {}

    [[nodiscard]] constexpr auto GetCond() const { return m_cond; }
    [[nodiscard]] constexpr auto GetBody() const { return m_body; }

    constexpr void SetCond(auto cond) { m_cond = std::move(cond); }
    constexpr void SetBody(auto body) { m_body = std::move(body); }
  };

  class Switch final : public Expr {
    std::span<FlowPtr<Case>> m_cases;
    FlowPtr<Expr> m_cond;
    NullableFlowPtr<Expr> m_default;

  public:
    constexpr Switch(auto cond, auto cases, auto def)
        : Expr(QAST_SWITCH), m_cases(cases), m_cond(std::move(cond)), m_default(std::move(def)) {}

    [[nodiscard]] constexpr auto GetCond() const { return m_cond; }
    [[nodiscard]] constexpr auto GetCases() const { return m_cases; }
    [[nodiscard]] constexpr auto GetDefault() const { return m_default; }

    constexpr void SetCond(auto cond) { m_cond = std::move(cond); }
    constexpr void SetCases(auto cases) { m_cases = cases; }
    constexpr void SetDefault(auto def) { m_default = std::move(def); }
  };

  class Export final : public Expr {
    std::span<FlowPtr<Expr>> m_attrs;
    FlowPtr<Expr> m_body;
    string m_abi_name;
    Vis m_vis;

  public:
    constexpr Export(auto content, auto abi_name, auto vis, auto attrs)
        : Expr(QAST_EXPORT), m_attrs(attrs), m_body(std::move(content)), m_abi_name(abi_name), m_vis(vis) {}

    [[nodiscard]] constexpr auto GetAbiName() const { return m_abi_name; }
    [[nodiscard]] constexpr auto GetBody() const { return m_body; }
    [[nodiscard]] constexpr auto GetVis() const { return m_vis; }
    [[nodiscard]] constexpr auto GetAttributes() const { return m_attrs; }

    constexpr void SetAbiName(auto abi_name) { m_abi_name = abi_name; }
    constexpr void SetBody(auto content) { m_body = std::move(content); }
    constexpr void SetVis(auto vis) { m_vis = vis; }
    constexpr void SetAttributes(auto attrs) { m_attrs = attrs; }
  };

  class Scope final : public Expr {
    std::span<string> m_deps;
    FlowPtr<Expr> m_body;
    string m_name;

  public:
    constexpr Scope(auto name, auto body, auto deps)
        : Expr(QAST_SCOPE), m_deps(deps), m_body(std::move(body)), m_name(name) {}

    [[nodiscard]] constexpr auto GetName() const { return m_name; }
    [[nodiscard]] constexpr auto GetBody() const { return m_body; }
    [[nodiscard]] constexpr auto GetDeps() const { return m_deps; }

    constexpr void SetName(auto name) { m_name = name; }
    constexpr void SetBody(auto body) { m_body = std::move(body); }
    constexpr void SetDeps(auto deps) { m_deps = deps; }
  };

  class Typedef final : public Expr {
    FlowPtr<Type> m_type;
    string m_name;

  public:
    constexpr Typedef(auto name, auto type) : Expr(QAST_TYPEDEF), m_type(std::move(type)), m_name(name) {}

    [[nodiscard]] constexpr auto GetName() const { return m_name; }
    [[nodiscard]] constexpr auto GetType() const { return m_type; }

    constexpr void SetName(auto name) { m_name = name; }
    constexpr void SetType(auto type) { m_type = std::move(type); }
  };

  class Enum final : public Expr {
    std::span<std::pair<string, NullableFlowPtr<Expr>>> m_items;
    NullableFlowPtr<Type> m_type;
    string m_name;

  public:
    constexpr Enum(auto name, auto type, auto items)
        : Expr(QAST_ENUM), m_items(items), m_type(std::move(type)), m_name(name) {}

    [[nodiscard]] constexpr auto GetName() const { return m_name; }
    [[nodiscard]] constexpr auto GetFields() const { return m_items; }
    [[nodiscard]] constexpr auto GetType() const { return m_type; }

    constexpr void SetName(auto name) { m_name = name; }
    constexpr void SetFields(auto items) { m_items = items; }
    constexpr void SetType(auto type) { m_type = std::move(type); }
  };

  class Function final : public Expr {
    std::optional<std::span<TemplateParameter>> m_template_parameters;
    std::span<FlowPtr<Expr>> m_attributes;
    std::span<std::pair<string, bool>> m_captures;
    std::span<FuncParam> m_params;
    FlowPtr<Type> m_return;
    NullableFlowPtr<Expr> m_precond, m_postcond;
    NullableFlowPtr<Expr> m_body;
    string m_name;
    Purity m_purity;
    bool m_variadic;

  public:
    constexpr Function(auto attributes, auto purity, auto captures, auto name, auto params, auto fn_params,
                       auto variadic, auto return_type, auto precond, auto postcond, auto body)
        : Expr(QAST_FUNCTION),
          m_attributes(attributes),
          m_captures(captures),
          m_params(fn_params),
          m_return(std::move(return_type)),
          m_precond(std::move(precond)),
          m_postcond(std::move(postcond)),
          m_body(std::move(body)),
          m_name(name),
          m_purity(purity),
          m_variadic(variadic) {
      if (params.has_value()) {
        m_template_parameters = params.value();
      }
    }

    [[nodiscard]] constexpr auto GetName() const { return m_name; }
    [[nodiscard]] constexpr auto GetAttributes() const { return m_attributes; }
    [[nodiscard]] constexpr auto GetPurity() const { return m_purity; }
    [[nodiscard]] constexpr auto GetCaptures() const { return m_captures; }
    [[nodiscard]] constexpr auto GetTemplateParams() const { return m_template_parameters; }
    [[nodiscard]] constexpr auto GetParams() const { return m_params; }
    [[nodiscard]] constexpr auto GetReturn() const { return m_return; }
    [[nodiscard]] constexpr auto GetPrecond() const { return m_precond; }
    [[nodiscard]] constexpr auto GetPostcond() const { return m_postcond; }
    [[nodiscard]] constexpr auto GetBody() const { return m_body; }
    [[nodiscard]] constexpr auto IsVariadic() const { return m_variadic; }
    [[nodiscard]] constexpr auto IsDeclaration() const -> bool { return !m_body.has_value(); }
    [[nodiscard]] constexpr auto IsDefinition() const -> bool { return m_body.has_value(); }
    [[nodiscard]] constexpr auto HasContract() const -> bool { return m_precond.has_value() || m_postcond.has_value(); }

    constexpr void SetName(auto name) { m_name = name; }
    constexpr void SetAttributes(auto attributes) { m_attributes = attributes; }
    constexpr void SetPurity(auto purity) { m_purity = purity; }
    constexpr void SetCaptures(auto captures) { m_captures = captures; }
    constexpr void SetTemplateParams(auto params) { m_template_parameters = params; }
    constexpr void SetParams(auto params) { m_params = params; }
    constexpr void SetReturn(auto return_type) { m_return = std::move(return_type); }
    constexpr void SetPrecond(auto precond) { m_precond = std::move(precond); }
    constexpr void SetPostcond(auto postcond) { m_postcond = std::move(postcond); }
    constexpr void SetBody(auto body) { m_body = std::move(body); }
    constexpr void SetVariadic(auto variadic) { m_variadic = variadic; }
  };

  enum class CompositeType : uint8_t { Region, Struct, Group, Class, Union };

  class Struct final : public Expr {
    std::optional<std::span<TemplateParameter>> m_template_parameters;
    std::span<FlowPtr<Expr>> m_attributes;
    std::span<string> m_names;
    std::span<StructField> m_fields;
    std::span<StructFunction> m_methods, m_static_methods;
    CompositeType m_comp_type;
    string m_name;

  public:
    constexpr Struct(auto comp_type, auto attributes, auto name, auto params, auto names, auto fields, auto methods,
                     auto static_methods)
        : Expr(QAST_STRUCT),
          m_attributes(attributes),
          m_names(names),
          m_fields(fields),
          m_methods(methods),
          m_static_methods(static_methods),
          m_comp_type(comp_type),
          m_name(name) {
      if (params.has_value()) {
        m_template_parameters = params.value();
      }
    }

    [[nodiscard]] constexpr auto GetName() const { return m_name; }
    [[nodiscard]] constexpr auto GetCompositeType() const { return m_comp_type; }
    [[nodiscard]] constexpr auto GetAttributes() const { return m_attributes; }
    [[nodiscard]] constexpr auto GetTemplateParams() const { return m_template_parameters; }
    [[nodiscard]] constexpr auto GetNames() const { return m_names; }
    [[nodiscard]] constexpr auto GetFields() const { return m_fields; }
    [[nodiscard]] constexpr auto GetMethods() const { return m_methods; }
    [[nodiscard]] constexpr auto GetStaticMethods() const { return m_static_methods; }

    constexpr void SetName(auto name) { m_name = name; }
    constexpr void SetCompositeType(auto comp_type) { m_comp_type = comp_type; }
    constexpr void SetAttributes(auto attributes) { m_attributes = attributes; }
    constexpr void SetTemplateParams(auto params) { m_template_parameters = params; }
    constexpr void SetNames(auto names) { m_names = names; }
    constexpr void SetFields(auto fields) { m_fields = fields; }
    constexpr void SetMethods(auto methods) { m_methods = methods; }
    constexpr void SetStaticMethods(auto static_methods) { m_static_methods = static_methods; }
  };
}  // namespace ncc::parse

#endif
