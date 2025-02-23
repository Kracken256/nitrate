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

#ifndef __NITRATE_CORE_STRING_FACTORY_H__
#define __NITRATE_CORE_STRING_FACTORY_H__

#include <cstdint>
#include <nitrate-core/Assert.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-core/Testing.hh>
#include <string>
#include <string_view>

namespace ncc {
  class StringMemory;
  class String;

  class CStringView final : public std::string_view {
  public:
    constexpr CStringView() : std::string_view("") {}
    constexpr CStringView(const char *begin, size_t len) : std::string_view(begin, len - 1) {
      qcore_assert(begin[len - 1] == '\0');
    }

    ~CStringView() = default;

    [[nodiscard]]
    auto c_str() const  // NOLINT
        -> const char * {
      return data();
    }

    [[nodiscard]] constexpr operator const char *() const { return c_str(); }
  };

  class NCC_EXPORT StringMemory {
    NCC_TESTING_ACCESSIBLE();

    friend class String;

    static auto FromString(std::string_view str) -> uint64_t;
    static auto FromString(std::string &&str) -> uint64_t;

  public:
    StringMemory() = delete;
    static void Reset();
  };

  class NCC_EXPORT __attribute__((packed)) String {
    NCC_TESTING_ACCESSIBLE();

    uint64_t m_id : 40;

  public:
    constexpr explicit String() : m_id(0) {}

    constexpr NCC_FORCE_INLINE String(std::string_view str) : m_id(str.empty() ? 0 : StringMemory::FromString(str)) {}

    constexpr NCC_FORCE_INLINE String(std::string &&str)
        : m_id(str.empty() ? 0 : StringMemory::FromString(std::move(str))) {}

    constexpr NCC_FORCE_INLINE String(const std::string &str) : m_id(str.empty() ? 0 : StringMemory::FromString(str)) {}

    constexpr NCC_FORCE_INLINE String(const char *str)
        : m_id(str[0] == 0 ? 0 : StringMemory::FromString(std::string_view(str))) {}

    [[nodiscard]] auto Get() const -> CStringView;

    auto operator==(const String &o) const -> bool;
    auto operator<(const String &o) const -> bool;
    auto operator<=(const String &o) const -> bool;
    auto operator>(const String &o) const -> bool;
    auto operator>=(const String &o) const -> bool;

    constexpr auto operator*() const { return Get(); }
    auto operator->() const { return Get().c_str(); }
    constexpr operator CStringView() const { return Get(); }

    [[nodiscard]] size_t size() const {  // NOLINT
      return Get().size();
    }

    [[nodiscard]] bool empty() const {  // NOLINT
      return Get().empty();
    }

    [[nodiscard]] auto c_str() const {  // NOLINT
      return Get().c_str();
    }

    [[nodiscard]] auto data() const {  // NOLINT
      return Get().data();
    }

    [[nodiscard]] auto at(size_t i) const {  // NOLINT
      return Get().at(i);
    }

    [[nodiscard]] auto ends_with(std::string_view end) const {  // NOLINT
      return Get().ends_with(end);
    }

    [[nodiscard]] auto starts_with(std::string_view start) const {  // NOLINT
      return Get().starts_with(start);
    }

    [[nodiscard]] constexpr auto GetId() const { return m_id; }
  };

  using string = String;

  static inline auto operator<<(std::ostream &os, const String &str) -> std::ostream & { return os << str.Get(); }
}  // namespace ncc

namespace std {
  template <>
  struct hash<ncc::String> {
    auto operator()(const ncc::String &str) const -> size_t { return std::hash<std::string_view>{}(str.Get()); }
  };
}  // namespace std

#endif
