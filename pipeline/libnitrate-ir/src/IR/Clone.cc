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

#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-ir/IR.hh>
#include <nitrate-ir/IR/Nodes.hh>

using namespace ncc;
using namespace ncc::ir;

class CloneVisitor : public IRVisitor<void> {
  std::optional<Expr *> R;

  void for_each(const auto &container, auto callback) {
    for (const auto &item : container) {
      callback(item);
    }
  }

public:
  CloneVisitor() {}
  virtual ~CloneVisitor() = default;

  Expr *GetClone() { return R.value(); }

  void visit(FlowPtr<Expr> n) override { R = create<Expr>(n->getKind()); }

  void visit(FlowPtr<BinExpr> n) override {
    auto lhs = n->getLHS()->clone();
    auto rhs = n->getRHS()->clone();
    auto op = n->getOp();

    R = create<BinExpr>(lhs, rhs, op);
  }

  void visit(FlowPtr<Unary> n) override {
    auto expr = n->getExpr()->clone();
    auto op = n->getOp();
    auto postfix = n->isPostfix();

    R = create<Unary>(expr, op, postfix);
  }

  void visit(FlowPtr<U1Ty>) override { R = getU1Ty(); }
  void visit(FlowPtr<U8Ty>) override { R = getU8Ty(); }
  void visit(FlowPtr<U16Ty>) override { R = getU16Ty(); }
  void visit(FlowPtr<U32Ty>) override { R = getU32Ty(); }
  void visit(FlowPtr<U64Ty>) override { R = getU64Ty(); }
  void visit(FlowPtr<U128Ty>) override { R = getU128Ty(); }
  void visit(FlowPtr<I8Ty>) override { R = getI8Ty(); }
  void visit(FlowPtr<I16Ty>) override { R = getI16Ty(); }
  void visit(FlowPtr<I32Ty>) override { R = getI32Ty(); }
  void visit(FlowPtr<I64Ty>) override { R = getI64Ty(); }
  void visit(FlowPtr<I128Ty>) override { R = getI128Ty(); }
  void visit(FlowPtr<F16Ty>) override { R = getF16Ty(); }
  void visit(FlowPtr<F32Ty>) override { R = getF32Ty(); }
  void visit(FlowPtr<F64Ty>) override { R = getF64Ty(); }
  void visit(FlowPtr<F128Ty>) override { R = getF128Ty(); }
  void visit(FlowPtr<VoidTy>) override { R = getVoidTy(); }
  void visit(FlowPtr<OpaqueTy> n) override { R = getOpaqueTy(n->getName()); }

  void visit(FlowPtr<StructTy> n) override {
    std::vector<FlowPtr<Type>> fields;
    fields.reserve(n->getFields().size());

    for_each(n->getFields(), [&](auto item) {
      fields.push_back(item->template clone<Type>());
    });

    R = getStructTy(fields);
  }

  void visit(FlowPtr<UnionTy> n) override {
    std::vector<FlowPtr<Type>> fields;
    fields.reserve(n->getFields().size());

    for_each(n->getFields(), [&](auto item) {
      fields.push_back(item->template clone<Type>());
    });

    R = getUnionTy(fields);
  }

  void visit(FlowPtr<PtrTy> n) override {
    R = getPtrTy(n->getPointee()->clone<Type>(), n->getNativeSize());
  }

  void visit(FlowPtr<ConstTy> n) override {
    R = getConstTy(n->getItem()->clone<Type>());
  }

  void visit(FlowPtr<ArrayTy> n) override {
    R = getArrayTy(n->getElement()->clone<Type>(), n->getCount());
  }

  void visit(FlowPtr<FnTy> n) override {
    std::vector<FlowPtr<Type>> params;
    params.reserve(n->getParams().size());
    for_each(n->getParams(), [&](auto item) {
      params.push_back(item->template clone<Type>());
    });

    R = getFnTy(params, n->getReturn()->clone<Type>(), n->isVariadic(),
                n->getNativeSize());
  }

  void visit(FlowPtr<Int> n) override {
    R = create<Int>(n->getValue(), n->getSize());
  }

  void visit(FlowPtr<Float> n) override {
    R = create<Float>(n->getValue(), n->getSize());
  }

  void visit(FlowPtr<List> n) override {
    IR_Vertex_ListItems<void> items;
    items.reserve(n->size());

    std::for_each(n->begin(), n->end(),
                  [&](auto item) { items.push_back(item->clone()); });

    R = create<List>(items, n->isHomogenous());
  }

  void visit(FlowPtr<Call> n) override {
    IR_Vertex_CallArgs<void> args;
    args.reserve(n->getArgs().size());

    for_each(n->getArgs(), [&](auto item) { args.push_back(item->clone()); });

    auto old_ref = n->getTarget();  // Resolve later

    R = create<Call>(old_ref, args);
  }

  void visit(FlowPtr<Seq> n) override {
    IR_Vertex_SeqItems<void> items;
    items.reserve(n->size());

    for_each(n->getItems(), [&](auto item) { items.push_back(item->clone()); });

    R = create<Seq>(items);
  }

  void visit(FlowPtr<Index> n) override {
    auto base = n->getExpr()->clone();
    auto index = n->getIndex()->clone();

    R = create<Index>(base, index);
  }

  void visit(FlowPtr<Ident> n) override {
    auto name = n->getName();
    auto old_ref = n->getWhat();  // Resolve later

    R = create<Ident>(name, old_ref);
  }

  void visit(FlowPtr<Extern> n) override {
    auto value = n->getValue()->clone();
    auto abi_name = n->getAbiName();

    R = create<Extern>(value, abi_name);
  }

  void visit(FlowPtr<Local> n) override {
    auto name = n->getName();
    auto value = n->getValue()->clone();
    auto abi_name = n->getAbiName();
    auto readonly = n->isReadonly();
    auto storage_class = n->getStorageClass();

    R = create<Local>(name, value, abi_name, readonly, storage_class);
  }

  void visit(FlowPtr<Ret> n) override {
    auto expr = n->getExpr()->clone();

    R = create<Ret>(expr);
  }

  void visit(FlowPtr<Brk>) override { R = create<Brk>(); }
  void visit(FlowPtr<Cont>) override { R = create<Cont>(); }

  void visit(FlowPtr<If> n) override {
    auto cond = n->getCond()->clone();
    auto then = n->getThen()->clone();
    auto ele = n->getElse()->clone();

    R = create<If>(cond, then, ele);
  }

  void visit(FlowPtr<While> n) override {
    auto cond = n->getCond()->clone();
    auto body = n->getBody()->clone<Seq>();

    R = create<While>(cond, body);
  }

  void visit(FlowPtr<For> n) override {
    auto init = n->getInit()->clone();
    auto cond = n->getCond()->clone();
    auto step = n->getStep()->clone();
    auto body = n->getBody()->clone();

    R = create<For>(init, cond, step, body);
  }

  void visit(FlowPtr<Case> n) override {
    auto cond = n->getCond()->clone();
    auto body = n->getBody()->clone();

    R = create<Case>(cond, body);
  }

  void visit(FlowPtr<Switch> n) override {
    IR_Vertex_SwitchCases<void> cases;
    cases.reserve(n->getCases().size());

    for_each(n->getCases(),
             [&](auto item) { cases.push_back(item->template clone<Case>()); });

    auto cond = n->getCond()->clone();
    auto default_ = n->getDefault().has_value()
                        ? n->getDefault().value()->clone()
                        : nullptr;

    R = create<Switch>(cond, cases, default_);
  }

  void visit(FlowPtr<Function> n) override {
    IR_Vertex_Params<void> params;
    params.reserve(n->getParams().size());

    for_each(n->getParams(), [&](auto item) {
      params.push_back({item.first->template clone<Type>(), item.second});
    });

    auto body =
        n->getBody().has_value() ? n->getBody().value()->clone<Seq>() : nullptr;
    auto return_type = n->getReturn()->clone<Type>();
    auto name = n->getName();
    auto abi_name = n->getAbiName();
    auto is_variadic = n->isVariadic();

    R = create<Function>(name, params, return_type, body, is_variadic,
                         abi_name);
  }

  void visit(FlowPtr<Asm>) override {
    qcore_panic("Cannot clone Asm node because it is not implemented");
  }

  void visit(FlowPtr<Tmp> n) override {
    if (std::holds_alternative<string>(n->getData())) {
      R = create<Tmp>(n->getTmpType(), std::get<string>(n->getData()));
    } else if (std::holds_alternative<IR_Vertex_CallArgsTmpNodeCradle<void>>(
                   n->getData())) {
      auto data = std::get<IR_Vertex_CallArgsTmpNodeCradle<void>>(n->getData());
      auto base = data.base->clone();
      IR_Vertex_CallArguments<void> args;
      args.reserve(data.args.size());

      for_each(data.args, [&](auto item) {
        args.push_back({item.first, item.second->clone()});
      });

      R = create<Tmp>(n->getTmpType(),
                      IR_Vertex_CallArgsTmpNodeCradle<void>{base, args});
    } else {
      qcore_panic("Unknown Tmp node data type");
    }
  }
};

///===========================================================================///

NCC_EXPORT Expr *detail::Expr_getCloneImpl(Expr *self) {
  static thread_local struct {
    std::unordered_map<Expr *, Expr *> in_out;
    size_t depth = 0;
  } state; /* The state behaves like a recursive argument */

  {
    state.depth++;

    CloneVisitor V;
    self->accept(V);

    FlowPtr<Expr> E = V.GetClone();
    E->setLoc(self->getLoc());

    state.depth--;

    if (state.depth == 0) {
      // Resolve internal cyclic references

      for_each(E, [](auto ty, auto n) {
        switch (ty) {
          case IR_eIDENT: {
            auto ident = n->template as<Ident>();

            if (auto what = ident->getWhat()) {
              if (auto it = state.in_out.find(what.value().get());
                  it != state.in_out.end()) {
                ident->setWhat(it->second);
              }
            }
          }

          case IR_eCALL: {
            auto call = n->template as<Call>();

            if (auto target = call->getTarget()) {
              if (auto it = state.in_out.find(target.value().get());
                  it != state.in_out.end()) {
                call->setTarget(it->second);
              }
            }
          }

          default: {
            break;
          }
        }
      });

      state.in_out.clear();
    }

    return E.get();
  }
}
