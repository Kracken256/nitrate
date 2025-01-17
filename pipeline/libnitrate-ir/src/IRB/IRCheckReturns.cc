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

#define IRBUILDER_IMPL

#include <nitrate-core/Logger.hh>
#include <nitrate-ir/IR/Nodes.hh>
#include <nitrate-ir/IRB/Builder.hh>
#include <nitrate-ir/diagnostic/Report.hh>

using namespace ncc::ir;

bool NRBuilder::check_returns(FlowPtr<Seq> root, IReport *I) {
  bool failed = false;

  for_each<Function>(root, [&](auto x) {
    /* Skip function declarations */
    if (!x->getBody()) {
      return;
    }

    auto fn_ty_opt = x->getType();
    if (!fn_ty_opt) {
      I->report(TypeInference, IC::Error, "Failed to deduce function type",
                x->getLoc());
      failed = true;

      return;
    }

    const auto fn_ty = fn_ty_opt.value()->template as<FnTy>();
    const auto return_ty = fn_ty->getReturn();

    bool found_ret = false;

    for_each<Ret>(x->getBody().value(), [&](auto y) {
      found_ret = true;

      auto ret_expr_ty_opt = y->getExpr()->getType();
      if (!ret_expr_ty_opt) {
        I->report(TypeInference, IC::Error,
                  "Failed to deduce return expression type", y->getLoc());
        failed = true;

        return;
      }

      /// TODO: Implement return type coercion

      if (!return_ty->isSame(ret_expr_ty_opt.value().get())) {
        I->report(ReturnTypeMismatch, IC::Error,
                  {"Return value type '", ret_expr_ty_opt.value()->toString(),
                   "' does not match function return type '",
                   return_ty->toString(), "'"},
                  y->getLoc());
        failed = true;

        return;
      }
    });

    if (!found_ret) {
      I->report(MissingReturn, IC::Error, x->getName(), x->getLoc());
      failed = true;
    }
  });

  return !failed;
}
