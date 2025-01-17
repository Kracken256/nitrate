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

#ifndef __NITRATE_IR_GRAPH_TYPE_H__
#define __NITRATE_IR_GRAPH_TYPE_H__

#include <nitrate-ir/IR/Base.hh>

namespace ncc::ir {
  template <class A>
  class IR_Vertex_U1Ty final : public IR_Vertex_Type<A> {
    friend A;

  public:
    constexpr IR_Vertex_U1Ty() : IR_Vertex_Type<A>(IR_tU1) {}
  };

  template <class A>
  class IR_Vertex_U8Ty final : public IR_Vertex_Type<A> {
    friend A;

  public:
    constexpr IR_Vertex_U8Ty() : IR_Vertex_Type<A>(IR_tU8) {}
  };

  template <class A>
  class IR_Vertex_U16Ty final : public IR_Vertex_Type<A> {
    friend A;

  public:
    constexpr IR_Vertex_U16Ty() : IR_Vertex_Type<A>(IR_tU16) {}
  };

  template <class A>
  class IR_Vertex_U32Ty final : public IR_Vertex_Type<A> {
    friend A;

  public:
    constexpr IR_Vertex_U32Ty() : IR_Vertex_Type<A>(IR_tU32) {}
  };

  template <class A>
  class IR_Vertex_U64Ty final : public IR_Vertex_Type<A> {
    friend A;

  public:
    constexpr IR_Vertex_U64Ty() : IR_Vertex_Type<A>(IR_tU64) {}
  };

  template <class A>
  class IR_Vertex_U128Ty final : public IR_Vertex_Type<A> {
    friend A;

  public:
    constexpr IR_Vertex_U128Ty() : IR_Vertex_Type<A>(IR_tU128) {}
  };

  template <class A>
  class IR_Vertex_I8Ty final : public IR_Vertex_Type<A> {
    friend A;

  public:
    constexpr IR_Vertex_I8Ty() : IR_Vertex_Type<A>(IR_tI8) {}
  };

  template <class A>
  class IR_Vertex_I16Ty final : public IR_Vertex_Type<A> {
    friend A;

  public:
    constexpr IR_Vertex_I16Ty() : IR_Vertex_Type<A>(IR_tI16){};
  };

  template <class A>
  class IR_Vertex_I32Ty final : public IR_Vertex_Type<A> {
    friend A;

  public:
    constexpr IR_Vertex_I32Ty() : IR_Vertex_Type<A>(IR_tI32) {}
  };

  template <class A>
  class IR_Vertex_I64Ty final : public IR_Vertex_Type<A> {
    friend A;

  public:
    constexpr IR_Vertex_I64Ty() : IR_Vertex_Type<A>(IR_tI64) {}
  };

  template <class A>
  class IR_Vertex_I128Ty final : public IR_Vertex_Type<A> {
    friend A;

  public:
    constexpr IR_Vertex_I128Ty() : IR_Vertex_Type<A>(IR_tI128) {}
  };

  template <class A>
  class IR_Vertex_F16Ty final : public IR_Vertex_Type<A> {
    friend A;

  public:
    constexpr IR_Vertex_F16Ty() : IR_Vertex_Type<A>(IR_tF16_TY) {}
  };

  template <class A>
  class IR_Vertex_F32Ty final : public IR_Vertex_Type<A> {
    friend A;

  public:
    constexpr IR_Vertex_F32Ty() : IR_Vertex_Type<A>(IR_tF32_TY) {}
  };

  template <class A>
  class IR_Vertex_F64Ty final : public IR_Vertex_Type<A> {
    friend A;

  public:
    constexpr IR_Vertex_F64Ty() : IR_Vertex_Type<A>(IR_tF64_TY) {}
  };

  template <class A>
  class IR_Vertex_F128Ty final : public IR_Vertex_Type<A> {
    friend A;

  public:
    constexpr IR_Vertex_F128Ty() : IR_Vertex_Type<A>(IR_tF128_TY) {}
  };

  template <class A>
  class IR_Vertex_VoidTy final : public IR_Vertex_Type<A> {
    friend A;

  public:
    constexpr IR_Vertex_VoidTy() : IR_Vertex_Type<A>(IR_tVOID) {}
  };

  template <class A>
  class IR_Vertex_PtrTy final : public IR_Vertex_Type<A> {
    friend A;

    FlowPtr<IR_Vertex_Type<A>> m_pointee;
    uint8_t m_native_size;

  public:
    constexpr IR_Vertex_PtrTy(auto pointee, size_t native_size = 8)
        : IR_Vertex_Type<A>(IR_tPTR),
          m_pointee(pointee),
          m_native_size(native_size) {}

    constexpr auto getPointee() const { return m_pointee; }
    constexpr auto getNativeSize() const { return m_native_size; }
  };

  template <class A>
  class IR_Vertex_ConstTy final : public IR_Vertex_Type<A> {
    friend A;

    FlowPtr<IR_Vertex_Type<A>> m_item;

  public:
    constexpr IR_Vertex_ConstTy(auto item)
        : IR_Vertex_Type<A>(IR_tCONST), m_item(item) {}

    constexpr auto getItem() const { return m_item; }
  };

  template <class A>
  class IR_Vertex_OpaqueTy final : public IR_Vertex_Type<A> {
    friend A;

    string m_name;

  public:
    constexpr IR_Vertex_OpaqueTy(auto name)
        : IR_Vertex_Type<A>(IR_tOPAQUE), m_name(name) {}

    constexpr auto getName() const { return m_name.get(); }
  };

  template <class A>
  class IR_Vertex_StructTy final : public IR_Vertex_Type<A> {
    friend A;

    std::span<FlowPtr<IR_Vertex_Type<A>>> m_fields;

  public:
    constexpr IR_Vertex_StructTy(auto fields)
        : IR_Vertex_Type<A>(IR_tSTRUCT), m_fields(fields) {}

    constexpr auto getFields() const { return m_fields; }
  };

  template <class A>
  class IR_Vertex_UnionTy final : public IR_Vertex_Type<A> {
    friend A;

    std::span<FlowPtr<IR_Vertex_Type<A>>> m_fields;

  public:
    constexpr IR_Vertex_UnionTy(auto fields)
        : IR_Vertex_Type<A>(IR_tUNION), m_fields(fields) {}

    constexpr auto getFields() const { return m_fields; }
  };

  template <class A>
  class IR_Vertex_ArrayTy final : public IR_Vertex_Type<A> {
    friend A;

    FlowPtr<IR_Vertex_Type<A>> m_element;
    size_t m_size;

  public:
    constexpr IR_Vertex_ArrayTy(auto element, auto size)
        : IR_Vertex_Type<A>(IR_tARRAY), m_element(element), m_size(size) {}

    constexpr auto getElement() const { return m_element; }
    constexpr auto getCount() const { return m_size; }
  };

  template <class A>
  class IR_Vertex_FnTy final : public IR_Vertex_Type<A> {
    friend A;

    std::span<FlowPtr<IR_Vertex_Type<A>>> m_params;
    FlowPtr<IR_Vertex_Type<A>> m_return;
    uint8_t m_native_size;
    bool m_variadic;

  public:
    constexpr IR_Vertex_FnTy(auto params, auto ret, auto variadic,
                             size_t native_size = 8)
        : IR_Vertex_Type<A>(IR_tFUNC),
          m_params(params),
          m_return(ret),
          m_native_size(native_size),
          m_variadic(variadic) {}

    constexpr auto getParams() const { return m_params; }
    constexpr auto getReturn() const { return m_return; }
    constexpr auto isVariadic() const { return m_variadic; }
    constexpr auto getNativeSize() const { return m_native_size; }
  };

  template <class A>
  class IR_Vertex_Tmp final : public IR_Vertex_Type<A> {
    friend A;

    TmpType m_type;
    TmpNodeCradle<A> m_data;

  public:
    IR_Vertex_Tmp(TmpType type, TmpNodeCradle<A> data = {})
        : IR_Vertex_Type<A>(IR_tTMP), m_type(type), m_data(data) {}

    auto getTmpType() { return m_type; }
    auto getData() const { return m_data; }
  };

  static inline U1Ty* getU1Ty() {
    static U1Ty u1;
    return &u1;
  }

  static inline U8Ty* getU8Ty() {
    static U8Ty u8;
    return &u8;
  }

  static inline U16Ty* getU16Ty() {
    static U16Ty u16;
    return &u16;
  }

  static inline U32Ty* getU32Ty() {
    static U32Ty u32;
    return &u32;
  }

  static inline U64Ty* getU64Ty() {
    static U64Ty u64;
    return &u64;
  }

  static inline U128Ty* getU128Ty() {
    static U128Ty u128;
    return &u128;
  }

  static inline I8Ty* getI8Ty() {
    static I8Ty i8;
    return &i8;
  }

  static inline I16Ty* getI16Ty() {
    static I16Ty i16;
    return &i16;
  }

  static inline I32Ty* getI32Ty() {
    static I32Ty i32;
    return &i32;
  }

  static inline I64Ty* getI64Ty() {
    static I64Ty i64;
    return &i64;
  }

  static inline I128Ty* getI128Ty() {
    static I128Ty i128;
    return &i128;
  }

  static inline F16Ty* getF16Ty() {
    static F16Ty f16;
    return &f16;
  }

  static inline F32Ty* getF32Ty() {
    static F32Ty f32;
    return &f32;
  }

  static inline F64Ty* getF64Ty() {
    static F64Ty f64;
    return &f64;
  }

  static inline F128Ty* getF128Ty() {
    static F128Ty f128;
    return &f128;
  }

  static inline VoidTy* getVoidTy() {
    static VoidTy void_ty;
    return &void_ty;
  }

  PtrTy* getPtrTy(FlowPtr<Type> pointee, uint8_t native_size = 8);
  ConstTy* getConstTy(FlowPtr<Type> item);
  OpaqueTy* getOpaqueTy(string name);
  StructTy* getStructTy(std::span<FlowPtr<Type>> fields);
  UnionTy* getUnionTy(std::span<FlowPtr<Type>> fields);
  ArrayTy* getArrayTy(FlowPtr<Type> element, size_t size);
  FnTy* getFnTy(std::span<FlowPtr<Type>> params, FlowPtr<Type> ret,
                bool variadic, size_t native_size = 8);
}  // namespace ncc::ir

#endif
