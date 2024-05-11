////////////////////////////////////////////////////////////////////////////////////
///                                                                              ///
///    ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░    ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░   ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░          ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░          ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░          ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░   ///
///    ░▒▓██████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░    ///
///      ░▒▓█▓▒░                                                                 ///
///       ░▒▓██▓▒░                                                               ///
///                                                                              ///
///     * QUIX LANG COMPILER - The official compiler for the Quix language.      ///
///     * Copyright (C) 2024 Wesley C. Jones                                     ///
///                                                                              ///
///     The QUIX Compiler Suite is free software; you can redistribute it and/or ///
///     modify it under the terms of the GNU Lesser General Public               ///
///     License as published by the Free Software Foundation; either             ///
///     version 2.1 of the License, or (at your option) any later version.       ///
///                                                                              ///
///     The QUIX Compiler Suite is distributed in the hope that it will be       ///
///     useful, but WITHOUT ANY WARRANTY; without even the implied warranty of   ///
///     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU        ///
///     Lesser General Public License for more details.                          ///
///                                                                              ///
///     You should have received a copy of the GNU Lesser General Public         ///
///     License along with the QUIX Compiler Suite; if not, see                  ///
///     <https://www.gnu.org/licenses/>.                                         ///
///                                                                              ///
////////////////////////////////////////////////////////////////////////////////////

#ifndef __QUIXCC_IR_QIR_H__
#define __QUIXCC_IR_QIR_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <parse/nodes/AllNodes.h>
#include <IR/IRModule.h>
#include <IR/Type.h>

namespace libquixcc
{
    namespace ir
    {
        namespace q
        {
            enum class NodeType
            {
                Root,

                /* Types */
                I1,
                I8,
                I16,
                I32,
                I64,
                I128,
                U8,
                U16,
                U32,
                U64,
                U128,
                F32,
                F64,
                Void,
                Ptr,
                Array,
                Vector,
                FType,
                Region,
                Group,
                Union,
                Opaque,

                /* Functions */
                FunctionBlock,
                Function,
                Asm,

                /* OO */
                RegionDef,
                GroupDef,
                UnionDef,

                /* Casting */
                SCast,
                UCast,
                PtrICast,
                IPtrCast,
                Bitcast,

                /* Control Flow */
                Call,
                CallIndirect,
                IfElse,
                While,
                For,
                Loop,
                Break,
                Continue,
                Ret,
                Throw,
                TryCatchFinally,
                Case,
                Switch,

                Ident,

                /* Arithmetic */
                Add,
                Sub,
                Mul,
                Div,
                Mod,
                BitAnd,
                BitOr,
                BitXor,
                BitNot,
                Shl,
                Shr,
                Rotl,
                Rotr,

                /* Comparison */
                Eq,
                Ne,
                Lt,
                Gt,
                Le,
                Ge,

                /* Logical */
                And,
                Or,
                Not,
                Xor,

                /* Variables */
                Local,
                Global,
                Number,
                String,
            };

            class RootNode : public libquixcc::ir::Value<Q>
            {
            protected:
                Result<bool> print_impl(std::ostream &os, PState &state) const override;
                boost::uuids::uuid hash_impl() const override;
                bool verify_impl() const override;

                RootNode(std::vector<const Value<Q> *> children) : children(children) {}

            public:
                static const RootNode *create(std::vector<const Value<Q> *> children = {});

                std::vector<const Value<Q> *> children;
            };

            class QModule : public libquixcc::ir::IRModule<IR::Q, const RootNode *>
            {
            protected:
                Result<bool> print_impl(std::ostream &os, PState &state) const override;
                std::string_view ir_dialect_name_impl() const override;
                unsigned ir_dialect_version_impl() const override;
                std::string_view ir_dialect_family_impl() const override;
                std::string_view ir_dialect_description_impl() const override;
                bool verify_impl() const override;

            public:
                QModule(const std::string_view &name) : IRModule<IR::Q, const RootNode *>(name) {}
                ~QModule() = default;

                bool from_ast(std::shared_ptr<BlockNode> ast);
            };
        }
    }
}

#endif // __QUIXCC_IR_QIR_H__