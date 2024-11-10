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
///   * QUIX LANG COMPILER - The official compiler for the Quix language.    ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The QUIX Compiler Suite is free software; you can redistribute it or   ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The QUIX Compiler Suite is distributed in the hope that it will be     ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the QUIX Compiler Suite; if not, see                ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#include <core/LibMacro.h>
#include <quix-core/Error.h>

#include <cstddef>
#include <quix-ir/Format.hh>
#include <quix-ir/IRGraph.hh>
#include <sstream>
#include <unordered_set>

using namespace qxir;

///=============================================================================

constexpr bool is_alnum(char ch) {
  return (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}
constexpr std::array<char, 256> ns_valid_chars = []() {
  std::array<char, 256> valid_chars = {};
  valid_chars.fill(false);

  for (size_t i = 0; i < 256; i++) {
    if (is_alnum(i) || i == '_' || i == '$') {
      valid_chars[i] = true;
    }
  }

  return valid_chars;
}();

static void encode_ns_size_value(std::string_view input, std::ostream &ss) {
  int state = 0;
  std::string buf;

  for (size_t i = 0; i < input.size(); i++) {
    switch (state) {
      case 0: {
        if (input[i] == ':') {
          state = 1;
        } else {
          buf.push_back(input[i]);
        }
        break;
      }
      case 1: {
        if (input[i] != ':') {
          ss << buf.size() << buf;
          buf.clear();
          buf.push_back(input[i]);
          state = 0;
        }
        break;
      }
    }
  }

  if (!buf.empty()) {
    ss << buf.size() << buf;
  }
}

static bool decode_ns_size_value(std::string_view &input, std::ostream &ss) {
  int state = 0;
  std::string buf;
  bool first = true;

  for (size_t i = 0; i < input.size(); i++) {
    switch (state) {
      case 0: {
        if (std::isdigit(input[i])) {
          buf.push_back(input[i]);
        } else if (i != 0 && buf.empty()) {
          input.remove_prefix(i);
          return true;
        } else {
          state = 1;
          i--;
        }
        break;
      }
      case 1: {
        if (buf.empty()) {
          return false;
        }

        int size = std::stoi(buf);
        buf.clear();

        if (i + size > input.size()) {
          return false;
        }

        std::string_view part = input.substr(i, size);

        if (!std::all_of(part.begin(), part.end(), [](char ch) { return ns_valid_chars[ch]; })) {
          return false;
        }

        if (first) {
          ss << part;
          first = false;
        } else {
          ss << "::" << part;
        }

        i += size - 1;
        state = 0;
        break;
      }
    }
  }

  return true;
}

static void mangle_type(Type *n, std::ostream &ss) {
  /**
   * @brief Name mangling for QUIX is inspired by the Itanium C++ ABI.
   * @ref https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling
   *
   *  <builtin-type> ::= v	# void
   *                 ::= w	# wchar_t
   *                 ::= b	# bool
   *                 ::= c	# char
   *                 ::= a	# signed char
   *                 ::= h	# unsigned char
   *                 ::= s	# short
   *                 ::= t	# unsigned short
   *                 ::= i	# int
   *                 ::= j	# unsigned int
   *                 ::= l	# long
   *                 ::= m	# unsigned long
   *                 ::= x	# long long, __int64
   *                 ::= y	# unsigned long long, __int64
   *                 ::= n	# __int128
   *                 ::= o	# unsigned __int128
   *                 ::= f	# float
   *                 ::= d	# double
   *                 ::= e	# long double, __float80
   *                 ::= g	# __float128
   *                 ::= z	# ellipsis
   *                 ::= Dd # IEEE 754r decimal floating point (64 bits)
   *                 ::= De # IEEE 754r decimal floating point (128 bits)
   *                 ::= Df # IEEE 754r decimal floating point (32 bits)
   *                 ::= Dh # IEEE 754r half-precision floating point (16 bits)
   *                 ::= DF <number> _ # ISO/IEC TS 18661 binary floating point type _FloatN (N
   * bits), C++23 std::floatN_t
   *                 ::= DF <number> x # IEEE extended precision formats, C23 _FloatNx (N bits)
   *                 ::= DF16b # C++23 std::bfloat16_t
   *                 ::= DB <number> _        # C23 signed _BitInt(N)
   *                 ::= DB <instantiation-dependent expression> _ # C23 signed _BitInt(N)
   *                 ::= DU <number> _        # C23 unsigned _BitInt(N)
   *                 ::= DU <instantiation-dependent expression> _ # C23 unsigned _BitInt(N)
   *                 ::= Di # char32_t
   *                 ::= Ds # char16_t
   *                 ::= Du # char8_t
   *                 ::= Da # auto
   *                 ::= Dc # decltype(auto)
   *                 ::= Dn # std::nullptr_t (i.e., decltype(nullptr))
   *                 ::= [DS] DA  # N1169 fixed-point [_Sat] T _Accum
   *                 ::= [DS] DR  # N1169 fixed-point [_Sat] T _Fract
   *                 ::= u <source-name> [<template-args>] # vendor extended type
   *
   *  <fixed-point-size>
   *                 ::= s # short
   *                 ::= t # unsigned short
   *                 ::= i # plain
   *                 ::= j # unsigned
   *                 ::= l # long
   *                 ::= m # unsigned long
   */

  switch (n->getKind()) {
    case QIR_NODE_U1_TY: {
      ss << 'b';
      break;
    }

    case QIR_NODE_U8_TY: {
      ss << 'h';
      break;
    }

    case QIR_NODE_U16_TY: {
      ss << 't';
      break;
    }

    case QIR_NODE_U32_TY: {
      ss << 'j';
      break;
    }

    case QIR_NODE_U64_TY: {
      ss << 'm';
      break;
    }

    case QIR_NODE_U128_TY: {
      ss << 'o';
      break;
    }

    case QIR_NODE_I8_TY: {
      ss << 'a';
      break;
    }

    case QIR_NODE_I16_TY: {
      ss << 's';
      break;
    }

    case QIR_NODE_I32_TY: {
      ss << 'i';
      break;
    }

    case QIR_NODE_I64_TY: {
      ss << 'l';
      break;
    }

    case QIR_NODE_I128_TY: {
      ss << 'n';
      break;
    }

    case QIR_NODE_F16_TY: {
      ss << "Dh";
      break;
    }

    case QIR_NODE_F32_TY: {
      ss << "Df";
      break;
    }

    case QIR_NODE_F64_TY: {
      ss << "Dd";
      break;
    }

    case QIR_NODE_F128_TY: {
      ss << "De";
      break;
    }

    case QIR_NODE_VOID_TY: {
      ss << 'v';
      break;
    }

    case QIR_NODE_PTR_TY: {
      ss << 'P';
      mangle_type(n->as<PtrTy>()->getPointee(), ss);
      break;
    }

    case QIR_NODE_OPAQUE_TY: {
      ss << 'N';
      encode_ns_size_value(n->as<OpaqueTy>()->getName(), ss);
      ss << 'E';
      break;
    }

    case QIR_NODE_STRUCT_TY: {
      /**
       * @brief Unlike C++, QUIX encodes field types into the name.
       * Making any changes to a struct will break ABI compatibility
       * at link time avoiding runtime UB.
       */

      ss << 'c';
      for (auto *field : n->as<StructTy>()->getFields()) {
        mangle_type(field, ss);
      }
      ss << 'E';
      break;
    }

    case QIR_NODE_UNION_TY: {
      /**
       * @brief Unlike C++, QUIX encodes field types into the name.
       * Making any changes to a union will break ABI compatibility
       * at link time avoiding runtime UB.
       */

      ss << 'u';
      for (auto *field : n->as<StructTy>()->getFields()) {
        mangle_type(field, ss);
      }
      ss << 'E';
      break;
    }

    case QIR_NODE_ARRAY_TY: {
      ss << 'A';
      ss << n->as<ArrayTy>()->getCount();
      ss << '_';
      mangle_type(n->as<ArrayTy>()->getElement(), ss);
      break;
    }

    case QIR_NODE_FN_TY: {
      /**
       * @brief Unlike C++, QUIX also encodes the parameter types
       * into the name. This is to avoid runtime UB when calling
       * functions with the wrong number of arguments.
       * These bugs will be caught at link time.
       */

      ss << 'F';
      auto *fn = n->as<FnTy>();
      mangle_type(fn->getReturn(), ss);
      for (auto *param : fn->getParams()) {
        mangle_type(param, ss);
      }
      if (fn->getAttrs().count(FnAttr::Variadic)) {
        ss << '_';
      }
      ss << 'E';
      break;
    }

    default: {
      qcore_panicf("Unknown type kind: %d", (int)n->getKind());
    }
  }
}

static bool demangle_type(std::string_view &name, std::ostream &ss) {
  static std::unordered_map<char, std::string_view> basic_types = {
      {'b', "u1"}, {'h', "u8"},  {'t', "u16"}, {'j', "u32"}, {'m', "u64"},  {'o', "u128"},
      {'a', "i8"}, {'s', "i16"}, {'i', "i32"}, {'l', "i64"}, {'n', "i128"}, {'v', "void"},
  };

  if (name.empty()) {
    return false;
  }

  auto it = basic_types.find(name[0]);
  if (it != basic_types.end()) {
    ss << it->second;
    name.remove_prefix(1);
    return true;
  }

  switch (name[0]) {
    case 'D': {
      if (name.size() < 2) {
        return false;
      }

      switch (name[1]) {
        case 'h': {
          ss << "f16";
          name.remove_prefix(2);
          return true;
        }

        case 'f': {
          ss << "f32";
          name.remove_prefix(2);
          return true;
        }

        case 'd': {
          ss << "f64";
          name.remove_prefix(2);
          return true;
        }

        case 'e': {
          ss << "f128";
          name.remove_prefix(2);
          return true;
        }

        default: {
          return false;
        }
      }
    }

    case 'P': {
      ss << '*';
      name.remove_prefix(1);
      return demangle_type(name, ss);
    }

    case 'N': {
      ss << "opaque ";
      name.remove_prefix(1);
      if (!decode_ns_size_value(name, ss)) {
        return false;
      }

      return true;
    }

    case 'c': {
      ss << "(";
      name.remove_prefix(1);
      bool first = true;
      while (!name.empty() && name[0] != 'E') {
        if (first) {
          first = false;
        } else {
          ss << ", ";
        }

        if (!demangle_type(name, ss)) {
          return false;
        }
      }
      ss << ")";
      name.remove_prefix(1);
      return true;
    }

    case 'u': {
      ss << "union {";
      name.remove_prefix(1);
      bool first = true;
      while (!name.empty() && name[0] != 'E') {
        if (first) {
          first = false;
        } else {
          ss << ", ";
        }

        if (!demangle_type(name, ss)) {
          return false;
        }
      }
      ss << "}";
      name.remove_prefix(1);
      return true;
    }

    case 'A': {
      ss << "[";
      name.remove_prefix(1);
      size_t count = 0;
      while (!name.empty() && name[0] != '_') {
        if (!std::isdigit(name[0])) {
          return false;
        }

        count = count * 10 + (name[0] - '0');
        name.remove_prefix(1);
      }
      name.remove_prefix(1);
      if (!demangle_type(name, ss)) {
        return false;
      }
      ss << "; " << count << "]";

      return true;
    }

    case 'F': {
      ss << "fn (";
      name.remove_prefix(1);
      std::stringstream return_ty, params;
      if (!demangle_type(name, return_ty)) {
        return false;
      }

      bool first = true;
      size_t i = 0;
      while (!name.empty() && name[0] != 'E') {
        if (first) {
          first = false;
        } else {
          params << ", ";
        }

        params << "_" << i++ << ": ";
        if (!demangle_type(name, params)) {
          return false;
        }

        if (!name.empty() && name[0] == '_') {
          params << ", ...";
          name.remove_prefix(1);
          break;
        }
      }
      name.remove_prefix(1);

      ss << params.str() << "): " << return_ty.str();
      return true;
    }

    default: {
      return false;
    }
  }
}

static std::string mangle_c_abi(std::string_view name, Type *) {
  std::string s = std::string(name);
  std::replace(s.begin(), s.end(), ':', '_');
  return s;
}

static std::string mangle_quix_abi(std::string_view name, Type *type) {
  std::stringstream ss;

  ss << "_Q";  // QUIX ABI prefix
  encode_ns_size_value(name, ss);
  mangle_type(type, ss);
  ss << "_0";  // ABI version 0

  return ss.str();
}

static std::string demangle_c_abi(std::string_view name) {
  /**
   * @brief There isn't really anything to do here.
   * The C ABI is weak and doesn't encode type information.
   */

  return std::string(name);
}

static void escape_string(std::ostream &ss, std::string_view input) {
  ss << '"';

  for (char ch : input) {
    switch (ch) {
      case '"':
        ss << "\\\"";
        break;
      case '\\':
        ss << "\\\\";
        break;
      case '\b':
        ss << "\\b";
        break;
      case '\f':
        ss << "\\f";
        break;
      case '\n':
        ss << "\\n";
        break;
      case '\r':
        ss << "\\r";
        break;
      case '\t':
        ss << "\\t";
        break;
      case '\0':
        ss << "\\0";
        break;
      default:
        if (ch >= 32 && ch < 127) {
          ss << ch;
        } else {
          char hex[5];
          snprintf(hex, sizeof(hex), "\\x%02x", (int)(uint8_t)ch);
          ss << hex;
        }
        break;
    }
  }

  ss << '"';
}

static std::optional<std::string> demangle_quix_abi(std::string_view name) {
  if (name.size() < 2) {
    printf("Failed to decode QUIX ABI prefix\n");
    return std::nullopt;
  }

  name.remove_prefix(2);  // Remove QUIX ABI prefix

  if (!name.ends_with("_0")) {  // Version check
    printf("Failed to decode ABI version\n");
    return std::nullopt;
  }

  name.remove_suffix(2);  // Remove ABI version

  std::stringstream ss;

  ss << "{\"name\":";

  { /* Write the symbol name */
    std::stringstream name_ss;
    if (!decode_ns_size_value(name, name_ss)) {
      printf("Failed to decode namespace size value\n");
      return std::nullopt;
    }

    escape_string(ss, name_ss.str());
  }

  ss << ",\"type\":";

  { /* Write the type */
    std::stringstream type_ss;
    if (!demangle_type(name, type_ss)) {
      printf("Failed to decode type\n");
      return std::nullopt;
    }

    escape_string(ss, type_ss.str());
  }

  ss << "}";

  return ss.str();
}

CPP_EXPORT std::optional<std::string> qxir::SymbolEncoding::mangle_name(const qxir::Expr *symbol,
                                                                        AbiTag abi) const noexcept {
  static std::unordered_set<qxir_ty_t> valid = {
      QIR_NODE_FN,
      QIR_NODE_LOCAL,
  };

  qxir_ty_t kind = symbol->getKind();
  if (!valid.contains(kind)) {
    return std::nullopt;
  }

  std::string_view name = symbol->getName();
  Type *type = const_cast<Expr *>(symbol)->getType().value_or(nullptr);
  if (!type) {
    return std::nullopt;
  }

  switch (abi) {
    case AbiTag::C: {
      return mangle_c_abi(name, type);
    }

    case AbiTag::QUIX: {
      return mangle_quix_abi(name, type);
    }

    case AbiTag::Internal: {
      return mangle_quix_abi(name, type);
    }
  }
}

CPP_EXPORT std::optional<std::string> qxir::SymbolEncoding::demangle_name(
    std::string_view symbol) const noexcept {
  if (symbol.empty()) {
    return std::nullopt;
  }

  if (symbol.starts_with("_Q")) {
    return demangle_quix_abi(symbol);
  } else {
    return demangle_c_abi(symbol);
  }
}