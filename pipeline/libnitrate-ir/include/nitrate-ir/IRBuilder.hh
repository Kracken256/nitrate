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

#ifndef __NITRATE_QXIR_IRBUILDER_H__
#define __NITRATE_QXIR_IRBUILDER_H__

#include <unordered_map>
#ifndef __cplusplus
#error "This header is C++ only."
#endif

#include <nitrate-core/Error.h>
#include <nitrate-core/Memory.h>
#include <nitrate-ir/TypeDecl.h>
#include <nitrate-lexer/Token.h>

#include <boost/multiprecision/cpp_int.hpp>
#include <boost/uuid/uuid.hpp>
#include <cassert>
#include <cmath>
#include <experimental/source_location>
#include <nitrate-core/Classes.hh>
#include <nitrate-ir/IRGraph.hh>
#include <nitrate-ir/Module.hh>
#include <nitrate-ir/Report.hh>
#include <optional>
#include <string_view>

namespace nr {
  enum class Purity {
    Impure = 0,
    Pure = 1,
    Quasipure = 2,
    Retropure = 3,
  };

  enum class Vis {
    Sec = 0,
    Pub = 1,
    Pro = 2,
  };

  enum class StorageClass {
    /* Automatic storeage duration */
    LLVM_StackAlloa,

    /* Static storage duration */
    LLVM_Static,

    /* Thread-local storage duration */
    LLVM_ThreadLocal,
  };

  enum class ABIStringStyle {
    CStr, /* Only supported variant */
  };

  using bigfloat_t = long double;
  using boost::multiprecision::uint128_t;

  class __attribute__((visibility("default"))) NRBuilder {
    /** Implicit copying is not allowed */
    NRBuilder(const NRBuilder &) = delete;
    NRBuilder &operator=(const NRBuilder &) = delete;

    ///**************************************************************************///
    // Builder external complexes
    ///**************************************************************************///

    /**
     * @brief The lexer doubles as the source location manager,
     * therefore we need it.
     */
    qlex_t *m_lex;
    TargetInfo m_target_info;

    ///**************************************************************************///
    // Builder state variables
    ///**************************************************************************///

    enum class SelfState {
      Constructed,  // start => [Finished, FailEarly, Destroyed]
      Finished,     // => [Verified, FailEarly, Destroyed]
      Verified,     // => [Emitted, FailEarly, Destroyed]
      Emitted,      // => [FailEarly, Destroyed]
      FailEarly,    // exit => [Destroyed]
      Destroyed,    // exit => []
    };

    SelfState m_state;
    std::optional<qmodule_t *> m_result;
    Seq *m_root;

    Seq *m_current_scope;
    std::optional<Fn *> m_current_function;
    std::optional<Expr *> m_current_expr;

    std::unordered_map<std::string_view, std::string> m_interned_strings;

    ///**************************************************************************///
    // Builder helper methods
    ///**************************************************************************///

    std::string_view intern(std::string_view in) noexcept;

    std::optional<Local *> lookup_global(std::string_view global) noexcept;
    std::optional<Local *> lookup_local(std::string_view local) noexcept;
    std::optional<Fn *> lookup_function(std::string_view function) noexcept;

    static bool check_acyclic(Seq *root, diag::IDiagnosticEngine *L) noexcept;
    static bool check_duplicates(Seq *root, diag::IDiagnosticEngine *L) noexcept;
    static bool check_symbols_exist(Seq *root, diag::IDiagnosticEngine *L) noexcept;
    static bool check_function_calls(Seq *root, diag::IDiagnosticEngine *L) noexcept;
    static bool check_returns(Seq *root, diag::IDiagnosticEngine *L) noexcept;
    static bool check_scopes(Seq *root, diag::IDiagnosticEngine *L) noexcept;
    static bool check_mutability(Seq *root, diag::IDiagnosticEngine *L) noexcept;
    static bool check_control_flow(Seq *root, diag::IDiagnosticEngine *L) noexcept;
    static bool check_types(Seq *root, diag::IDiagnosticEngine *L) noexcept;
    static bool check_safety_claims(Seq *root, diag::IDiagnosticEngine *L) noexcept;

#if defined(NDEBUG)
#define SOURCE_LOCATION_PARAM
#define SOURCE_LOCATION_PARAM_ONCE

    void contract_enforce_(bool cond, std::string_view cond_str,
                           std::experimental::source_location caller =
                               std::experimental::source_location::current()) const noexcept;
#define contract_enforce(cond) contract_enforce_(cond, #cond)
#else
#define SOURCE_LOCATION_PARAM \
  , std::experimental::source_location caller_info = std::experimental::source_location::current()
#define SOURCE_LOCATION_PARAM_ONCE \
  std::experimental::source_location caller_info = std::experimental::source_location::current()

    void contract_enforce_(bool cond, std::string_view cond_str SOURCE_LOCATION_PARAM,
                           std::experimental::source_location caller =
                               std::experimental::source_location::current()) const noexcept;
#define contract_enforce(cond) contract_enforce_(cond, #cond, caller_info)

#endif

#define DEBUG_INFO 1, 1

  public:
    NRBuilder(qlex_t &lexer_instance, TargetInfo target_info SOURCE_LOCATION_PARAM) noexcept;
    ~NRBuilder() noexcept;

    /* Moving the module is permitted */
    NRBuilder &operator=(NRBuilder &&) noexcept;
    NRBuilder(NRBuilder &&) noexcept;

    /** @warning: This is a slow and resource heavy operation for
     * most programs. */
    NRBuilder deep_clone(SOURCE_LOCATION_PARAM_ONCE) const noexcept;

    /** @brief Get an approximate figure of how much memory the
     * builder is currently using. The returned value is a lower bound. */
    size_t approx_memory_usage(SOURCE_LOCATION_PARAM_ONCE) noexcept;

    /** @brief Count *ALL* nodes currently in the builder. This includes
     * temporary nodes. */
    size_t node_count(SOURCE_LOCATION_PARAM_ONCE) noexcept;

    /**
     * @brief Finialize the module build
     * @note After the builder is finalized, it can't be updated anymore.
     * @note This function is idempotent, without any overhead from additional calls.
     */
    void finish(SOURCE_LOCATION_PARAM_ONCE) noexcept;

    /**
     * @brief Run basic checks on the module:
     * @param sink The diagnostic engine to use, never nullptr.
     * @return True if the module is usable, false otherwise.
     *
     * Usability means that the module is in a state where all data-structure invariants are
     * intact, such that it can be used for further processing as-if it were fully correct.
     *
     * An example of something is is semantically erroronous, but still "usable" is an
     * out-of-bounds array access. `verify()` may report an error to the diagnostic sink
     * regarding the out-of-bounds access, but it may return true because the module's
     * data-structure invariants are verified as correct.
     *
     *  - Check for cyclic references in the internal data-structure;
     *  - Ensure that all symbols are resolved;
     *  - Check for duplicate identifiers;
     *  - Ensure that all types are resolved;
     *  - Check range and type of initial values;
     *  - Type check entire module;
     *  - Function calls have the correct number of arguments;
     *  - Ensure that return statements are present;
     *  - Ensure that all scopes are obeyed:
     *      Variables exist by the time they are accessed;
     *      Functions exist by the time they are called;
     *  - Verify mutability rules are obeyed;
     *  - Verify usage and presence of control flow nodes;
     *  - Do complex safety checks to verify proper usage of `safe` and `unsafe`.
     *
     * @note This function calls `finish()`.
     */
    bool verify(diag::IDiagnosticEngine *sink SOURCE_LOCATION_PARAM) noexcept;

    /**
     * @brief Return the build module.
     * @note `verify()` must be called first.
     */
    qmodule_t *get_module(SOURCE_LOCATION_PARAM_ONCE) noexcept;

    ///**************************************************************************///
    // Builder navigation

    NRBuilder &insertAfter(Expr *last SOURCE_LOCATION_PARAM) noexcept;
    NRBuilder &insertAfterVariable(std::string_view name SOURCE_LOCATION_PARAM) noexcept;
    NRBuilder &insertAfterFunction(std::string_view name SOURCE_LOCATION_PARAM) noexcept;

    NRBuilder &insertBefore(Expr *last SOURCE_LOCATION_PARAM) noexcept;
    NRBuilder &insertBeforeVariable(std::string_view name SOURCE_LOCATION_PARAM) noexcept;
    NRBuilder &insertBeforeFunction(std::string_view name SOURCE_LOCATION_PARAM) noexcept;

    ///**************************************************************************///
    // Create linkable symbols

    Fn *createFunctionDefintion(std::string_view name, std::span<Type *> params, Type *ret_ty,
                                bool is_variadic = false, Vis visibility = Vis::Sec,
                                Purity purity = Purity::Impure, bool thread_safe = false,
                                bool is_noexcept = false,
                                bool foreign = true SOURCE_LOCATION_PARAM) noexcept;

    Fn *createFunctionDeclaration(std::string_view name, std::span<Type *> params, Type *ret_ty,
                                  bool is_variadic = false, Vis visibility = Vis::Sec,
                                  Purity purity = Purity::Impure, bool thread_safe = false,
                                  bool is_noexcept = false,
                                  bool foreign = true SOURCE_LOCATION_PARAM) noexcept;

    Fn *createAnonymousFunction(std::span<Type *> params, Type *ret_ty, bool is_variadic = false,
                                Purity purity = Purity::Impure, bool thread_safe = false,
                                bool is_noexcept = false SOURCE_LOCATION_PARAM) noexcept;

    /* This is the only intended way to overload operaters */
    Fn *createOperatorOverload(Op op, std::span<Type *> params, Type *ret_ty,
                               Purity purity = Purity::Impure, bool thread_safe = false,
                               bool is_noexcept = false SOURCE_LOCATION_PARAM) noexcept;

    Fn *createTemplateFunction(std::string_view name, std::span<std::string_view> template_params,
                               std::span<Type *> params, Type *ret_ty, bool is_variadic = false,
                               Vis visibility = Vis::Sec, Purity purity = Purity::Impure,
                               bool thread_safe = false,
                               bool is_noexcept = false SOURCE_LOCATION_PARAM) noexcept;

    /* Works for both local and global variables */
    Local *createVariable(std::string_view name, Type *ty, Vis visibility = Vis::Sec,
                          StorageClass storage = StorageClass::LLVM_StackAlloa,
                          bool is_readonly = false SOURCE_LOCATION_PARAM) noexcept;

    /// TODO:

    ///**************************************************************************///
    // Create literals

    Int *createBool(bool value SOURCE_LOCATION_PARAM) noexcept;

    Int *createFixedInteger(uint128_t value, IntSize width SOURCE_LOCATION_PARAM) noexcept;

    Float *createFixedFloat(bigfloat_t value, FloatSize width SOURCE_LOCATION_PARAM) noexcept;

    List *createStringDataArray(
        std::string_view value,
        ABIStringStyle style = ABIStringStyle::CStr SOURCE_LOCATION_PARAM) noexcept;

    List *createList(std::span<Expr *> items,

                     /* Require assert(typeof(result)==typeof(array<result.element, result.size>))
                      * ? Reason: It has to do with type inference and implicit conversions of the
                      * elements in the list.
                      */
                     bool cast_homogenous SOURCE_LOCATION_PARAM) noexcept;

    ///**************************************************************************///
    // Create types

    U1Ty *getU1Ty(SOURCE_LOCATION_PARAM_ONCE) noexcept;
    U8Ty *getU8Ty(SOURCE_LOCATION_PARAM_ONCE) noexcept;
    U16Ty *getU16Ty(SOURCE_LOCATION_PARAM_ONCE) noexcept;
    U32Ty *getU32Ty(SOURCE_LOCATION_PARAM_ONCE) noexcept;
    U64Ty *getU64Ty(SOURCE_LOCATION_PARAM_ONCE) noexcept;
    U128Ty *getU128Ty(SOURCE_LOCATION_PARAM_ONCE) noexcept;
    I8Ty *getI8Ty(SOURCE_LOCATION_PARAM_ONCE) noexcept;
    I16Ty *getI16Ty(SOURCE_LOCATION_PARAM_ONCE) noexcept;
    I32Ty *getI32Ty(SOURCE_LOCATION_PARAM_ONCE) noexcept;
    I64Ty *getI64Ty(SOURCE_LOCATION_PARAM_ONCE) noexcept;
    I128Ty *getI128Ty(SOURCE_LOCATION_PARAM_ONCE) noexcept;
    F16Ty *getF16Ty(SOURCE_LOCATION_PARAM_ONCE) noexcept;
    F32Ty *getF32Ty(SOURCE_LOCATION_PARAM_ONCE) noexcept;
    F64Ty *getF64Ty(SOURCE_LOCATION_PARAM_ONCE) noexcept;
    F128Ty *getF128Ty(SOURCE_LOCATION_PARAM_ONCE) noexcept;
    VoidTy *getVoidTy(SOURCE_LOCATION_PARAM_ONCE) noexcept;

    /* Type inference unknowns; Converted to proper type upon resolution */
    OpaqueTy *getUnknownTy(SOURCE_LOCATION_PARAM_ONCE) noexcept;

    PtrTy *getPtrTy(Type *pointee SOURCE_LOCATION_PARAM) noexcept;

    OpaqueTy *getOpaqueTy(std::string_view name SOURCE_LOCATION_PARAM) noexcept;

    StructTy *getStructTy(std::span<Type *> fields SOURCE_LOCATION_PARAM) noexcept;

    UnionTy *getUnionTy(std::span<Type *> fields SOURCE_LOCATION_PARAM) noexcept;

    ArrayTy *getArrayTy(Type *element_ty, Expr *constant_size SOURCE_LOCATION_PARAM) noexcept;

    ArrayTy *getArrayTy(Type *element_ty, size_t count SOURCE_LOCATION_PARAM) noexcept;

    FnTy *getFnTy(std::span<Type *> params, Type *ret_ty, bool is_variadic = false,
                  Purity purity = Purity::Impure, bool thread_safe = false,
                  bool is_noexcept = false, bool foreign = true SOURCE_LOCATION_PARAM) noexcept;

    /**
     * Each entry in `params` shall correspond to the name of an opaque type in the supplied
     * struct. The fields of the struct type are searched recursively to resolve all such fields
     * upon instaniation. The opaque type mentioned above must begin with some reserved prefix to
     * ensure the space of names doesn't conflict with other normal opaque types.
     */
    StructTy *createStructTemplateDefintion(std::string_view name,
                                            std::span<std::string_view> template_params,
                                            StructTy *ty SOURCE_LOCATION_PARAM) noexcept;

    /**
     * Each entry in `params` shall correspond to the name of an opaque type in the supplied
     * union. The fields of the union type are searched recursively to resolve all such fields
     * upon instaniation. The opaque type mentioned above must begin with some reserved prefix to
     * ensure the space of names doesn't conflict with other normal opaque types.
     */
    UnionTy *createUnionTemplateDefintion(std::string_view name,
                                          std::span<std::string_view> template_params,
                                          UnionTy *ty SOURCE_LOCATION_PARAM) noexcept;

    ///**************************************************************************///

#undef SOURCE_LOCATION_PARAM
#undef SOURCE_LOCATION_PARAM_ONCE

#if defined(IRBUILDER_IMPL)

#if defined(NDEBUG)
#define SOURCE_LOCATION_PARAM
#define SOURCE_LOCATION_PARAM_ONCE
#define CALLER_INFO 0
#define ignore_caller_info()
#else
#define SOURCE_LOCATION_PARAM , std::experimental::source_location caller_info
#define SOURCE_LOCATION_PARAM_ONCE std::experimental::source_location caller_info
#define CALLER_INFO caller_info
#define CALLEE_KNOWN
#define ignore_caller_info() (void)caller_info;
#define compiler_trace(x) x
#endif

#else
#undef DEBUG_INFO
#endif
  };
}  // namespace nr

#endif