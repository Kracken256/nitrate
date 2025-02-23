/*
  __ _ _ __ __ _ _ __   __ _ _ __ ___  ___
 / _` | '__/ _` | '_ \ / _` | '__/ __|/ _ \ Argument Parser for Modern C++
| (_| | | | (_| | |_) | (_| | |  \__ \  __/ http://github.com/p-ranav/argparse
 \__,_|_|  \__, | .__/ \__,_|_|  |___/\___|
           |___/|_|

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
SPDX-License-Identifier: MIT
Copyright (c) 2019-2022 Pranav Srinivas Kumar <pranav.srinivas.kumar@gmail.com>
and other contributors.

Permission is hereby  granted, free of charge, to any  person obtaining a copy
of this software and associated  documentation files (the "Software"), to deal
in the Software  without restriction, including without  limitation the rights
to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#pragma once

#include <cerrno>

#ifndef ARGPARSE_MODULE_USE_STD_MODULE
#include <algorithm>
#include <any>
#include <array>
#include <charconv>
#include <cstdlib>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <list>
#include <map>
#include <numeric>
#include <optional>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>
#endif

#ifndef ARGPARSE_CUSTOM_STRTOF
#define ARGPARSE_CUSTOM_STRTOF strtof
#endif

#ifndef ARGPARSE_CUSTOM_STRTOD
#define ARGPARSE_CUSTOM_STRTOD strtod
#endif

#ifndef ARGPARSE_CUSTOM_STRTOLD
#define ARGPARSE_CUSTOM_STRTOLD strtold
#endif

namespace argparse {

  namespace details {  // namespace for helper methods

    template <typename T, typename = void>
    struct HasContainerTraits : std::false_type {};

    template <>
    struct HasContainerTraits<std::string> : std::false_type {};

    template <>
    struct HasContainerTraits<std::string_view> : std::false_type {};

    template <typename T>
    struct HasContainerTraits<T, std::void_t<typename T::value_type, decltype(std::declval<T>().begin()),
                                             decltype(std::declval<T>().end()), decltype(std::declval<T>().size())>>
        : std::true_type {};

    template <typename T>
    inline constexpr bool kIsContainer = HasContainerTraits<T>::value;

    template <typename T, typename = void>
    struct HasStreamableTraits : std::false_type {};

    template <typename T>
    struct HasStreamableTraits<T, std::void_t<decltype(std::declval<std::ostream &>() << std::declval<T>())>>
        : std::true_type {};

    template <typename T>
    inline constexpr bool kIsStreamable = HasStreamableTraits<T>::value;

    constexpr std::size_t kReprMaxContainerSize = 5;

    template <typename T>
    auto Repr(T const &val) -> std::string {
      if constexpr (std::is_same_v<T, bool>) {
        return val ? "true" : "false";
      } else if constexpr (std::is_convertible_v<T, std::string_view>) {
        return '"' + std::string{std::string_view{val}} + '"';
      } else if constexpr (kIsContainer<T>) {
        std::stringstream out;
        out << "{";
        const auto size = val.size();
        if (size > 1) {
          out << repr(*val.begin());
          std::for_each(std::next(val.begin()),
                        std::next(val.begin(), static_cast<typename T::iterator::difference_type>(
                                                   std::min<std::size_t>(size, kReprMaxContainerSize) - 1)),
                        [&out](const auto &v) { out << " " << repr(v); });
          if (size <= kReprMaxContainerSize) {
            out << " ";
          } else {
            out << "...";
          }
        }
        if (size > 0) {
          out << repr(*std::prev(val.end()));
        }
        out << "}";
        return out.str();
      } else if constexpr (kIsStreamable<T>) {
        std::stringstream out;
        out << val;
        return out.str();
      } else {
        return "<not representable>";
      }
    }

    namespace {

      template <typename T>
      constexpr bool kStandardSignedInteger = false;
      template <>
      constexpr bool kStandardSignedInteger<signed char> = true;
      template <>
      constexpr bool kStandardSignedInteger<short int> = true;
      template <>
      constexpr bool kStandardSignedInteger<int> = true;
      template <>
      constexpr bool kStandardSignedInteger<long int> = true;
      template <>
      constexpr bool kStandardSignedInteger<long long int> = true;

      template <typename T>
      constexpr bool kStandardUnsignedInteger = false;
      template <>
      constexpr bool kStandardUnsignedInteger<unsigned char> = true;
      template <>
      constexpr bool kStandardUnsignedInteger<unsigned short int> = true;
      template <>
      constexpr bool kStandardUnsignedInteger<unsigned int> = true;
      template <>
      constexpr bool kStandardUnsignedInteger<unsigned long int> = true;
      template <>
      constexpr bool kStandardUnsignedInteger<unsigned long long int> = true;

    }  // namespace

    constexpr int kRadix2 = 2;
    constexpr int kRadix8 = 8;
    constexpr int kRadix10 = 10;
    constexpr int kRadix16 = 16;

    template <typename T>
    constexpr bool kStandardInteger = kStandardSignedInteger<T> || kStandardUnsignedInteger<T>;

    template <class F, class Tuple, class Extra, std::size_t... I>
    constexpr auto ApplyPlusOneImpl(F &&f, Tuple &&t, Extra &&x,
                                    std::index_sequence<I...> /*unused*/) -> decltype(auto) {
      return std::invoke(std::forward<F>(f), std::get<I>(std::forward<Tuple>(t))..., std::forward<Extra>(x));
    }

    template <class F, class Tuple, class Extra>
    constexpr auto ApplyPlusOne(F &&f, Tuple &&t, Extra &&x) -> decltype(auto) {
      return details::ApplyPlusOneImpl(std::forward<F>(f), std::forward<Tuple>(t), std::forward<Extra>(x),
                                       std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<Tuple>>>{});
    }

    constexpr auto PointerRange(std::string_view s) noexcept { return std::tuple(s.data(), s.data() + s.size()); }

    template <class CharT, class Traits>
    constexpr auto StartsWith(std::basic_string_view<CharT, Traits> prefix,
                              std::basic_string_view<CharT, Traits> s) noexcept -> bool {
      return s.substr(0, prefix.size()) == prefix;
    }

    enum class chars_format {
      scientific = 0xf1,
      fixed = 0xf2,
      hex = 0xf4,
      binary = 0xf8,
      general = fixed | scientific
    };

    struct ConsumeBinaryPrefixResult {
      bool m_is_binary;
      std::string_view m_rest;
    };

    constexpr auto ConsumeBinaryPrefix(std::string_view s) -> ConsumeBinaryPrefixResult {
      if (StartsWith(std::string_view{"0b"}, s) || StartsWith(std::string_view{"0B"}, s)) {
        s.remove_prefix(2);
        return {true, s};
      }
      return {false, s};
    }

    struct ConsumeHexPrefixResult {
      bool m_is_hexadecimal;
      std::string_view m_rest;
    };

    using namespace std::literals;

    constexpr auto ConsumeHexPrefix(std::string_view s) -> ConsumeHexPrefixResult {
      if (StartsWith("0x"sv, s) || StartsWith("0X"sv, s)) {
        s.remove_prefix(2);
        return {true, s};
      }
      return {false, s};
    }

    template <class T, auto Param>
    inline auto DoFromChars(std::string_view s) -> T {
      T x{0};
      auto [first, last] = PointerRange(s);
      auto [ptr, ec] = std::from_chars(first, last, x, Param);
      if (ec == std::errc()) {
        if (ptr == last) {
          return x;
        }
        throw std::invalid_argument{"pattern '" + std::string(s) + "' does not match to the end"};
      }
      if (ec == std::errc::invalid_argument) {
        throw std::invalid_argument{"pattern '" + std::string(s) + "' not found"};
      }
      if (ec == std::errc::result_out_of_range) {
        throw std::range_error{"'" + std::string(s) + "' not representable"};
      }
      return x;  // unreachable
    }

    template <class T, auto Param = 0>
    struct ParseNumber {
      auto operator()(std::string_view s) -> T { return DoFromChars<T, Param>(s); }
    };

    template <class T>
    struct ParseNumber<T, kRadix2> {
      auto operator()(std::string_view s) -> T {
        if (auto [ok, rest] = ConsumeBinaryPrefix(s); ok) {
          return do_from_chars<T, kRadix2>(rest);
        }
        throw std::invalid_argument{"pattern not found"};
      }
    };

    template <class T>
    struct ParseNumber<T, kRadix16> {
      auto operator()(std::string_view s) -> T {
        if (StartsWith("0x"sv, s) || StartsWith("0X"sv, s)) {
          if (auto [ok, rest] = ConsumeHexPrefix(s); ok) {
            try {
              return do_from_chars<T, kRadix16>(rest);
            } catch (const std::invalid_argument &err) {
              throw std::invalid_argument("Failed to parse '" + std::string(s) + "' as hexadecimal: " + err.what());
            } catch (const std::range_error &err) {
              throw std::range_error("Failed to parse '" + std::string(s) + "' as hexadecimal: " + err.what());
            }
          }
        } else {
          // Allow passing hex numbers without prefix
          // Shape 'x' already has to be specified
          try {
            return do_from_chars<T, kRadix16>(s);
          } catch (const std::invalid_argument &err) {
            throw std::invalid_argument("Failed to parse '" + std::string(s) + "' as hexadecimal: " + err.what());
          } catch (const std::range_error &err) {
            throw std::range_error("Failed to parse '" + std::string(s) + "' as hexadecimal: " + err.what());
          }
        }

        throw std::invalid_argument{"pattern '" + std::string(s) + "' not identified as hexadecimal"};
      }
    };

    template <class T>
    struct ParseNumber<T> {
      auto operator()(std::string_view s) -> T {
        auto [ok, rest] = ConsumeHexPrefix(s);
        if (ok) {
          try {
            return do_from_chars<T, kRadix16>(rest);
          } catch (const std::invalid_argument &err) {
            throw std::invalid_argument("Failed to parse '" + std::string(s) + "' as hexadecimal: " + err.what());
          } catch (const std::range_error &err) {
            throw std::range_error("Failed to parse '" + std::string(s) + "' as hexadecimal: " + err.what());
          }
        }

        auto [ok_binary, rest_binary] = ConsumeBinaryPrefix(s);
        if (ok_binary) {
          try {
            return do_from_chars<T, kRadix2>(rest_binary);
          } catch (const std::invalid_argument &err) {
            throw std::invalid_argument("Failed to parse '" + std::string(s) + "' as binary: " + err.what());
          } catch (const std::range_error &err) {
            throw std::range_error("Failed to parse '" + std::string(s) + "' as binary: " + err.what());
          }
        }

        if (StartsWith("0"sv, s)) {
          try {
            return do_from_chars<T, kRadix8>(rest);
          } catch (const std::invalid_argument &err) {
            throw std::invalid_argument("Failed to parse '" + std::string(s) + "' as octal: " + err.what());
          } catch (const std::range_error &err) {
            throw std::range_error("Failed to parse '" + std::string(s) + "' as octal: " + err.what());
          }
        }

        try {
          return do_from_chars<T, kRadix10>(rest);
        } catch (const std::invalid_argument &err) {
          throw std::invalid_argument("Failed to parse '" + std::string(s) + "' as decimal integer: " + err.what());
        } catch (const std::range_error &err) {
          throw std::range_error("Failed to parse '" + std::string(s) + "' as decimal integer: " + err.what());
        }
      }
    };

    namespace {

      template <class T>
      inline const auto GENERIC_STRTOD = nullptr;
      template <>
      inline const auto GENERIC_STRTOD<float> = ARGPARSE_CUSTOM_STRTOF;
      template <>
      inline const auto GENERIC_STRTOD<double> = ARGPARSE_CUSTOM_STRTOD;
      template <>
      inline const auto GENERIC_STRTOD<long double> = ARGPARSE_CUSTOM_STRTOLD;

    }  // namespace

    template <class T>
    inline auto DoStrtod(std::string const &s) -> T {
      if (isspace(static_cast<unsigned char>(s[0])) || s[0] == '+') {
        throw std::invalid_argument{"pattern '" + s + "' not found"};
      }

      auto [first, last] = PointerRange(s);
      char *ptr;

      errno = 0;
      auto x = GENERIC_STRTOD<T>(first, &ptr);
      if (errno == 0) {
        if (ptr == last) {
          return x;
        }
        throw std::invalid_argument{"pattern '" + s + "' does not match to the end"};
      }
      if (errno == ERANGE) {
        throw std::range_error{"'" + s + "' not representable"};
      }
      return x;  // unreachable
    }

    template <class T>
    struct ParseNumber<T, chars_format::general> {
      auto operator()(std::string const &s) -> T {
        if (auto r = ConsumeHexPrefix(s); r.m_is_hexadecimal) {
          throw std::invalid_argument{"chars_format::general does not parse hexfloat"};
        }
        if (auto r = ConsumeBinaryPrefix(s); r.m_is_binary) {
          throw std::invalid_argument{"chars_format::general does not parse binfloat"};
        }

        try {
          return DoStrtod<T>(s);
        } catch (const std::invalid_argument &err) {
          throw std::invalid_argument("Failed to parse '" + s + "' as number: " + err.what());
        } catch (const std::range_error &err) {
          throw std::range_error("Failed to parse '" + s + "' as number: " + err.what());
        }
      }
    };

    template <class T>
    struct ParseNumber<T, chars_format::hex> {
      auto operator()(std::string const &s) -> T {
        if (auto r = ConsumeHexPrefix(s); !r.m_is_hexadecimal) {
          throw std::invalid_argument{"chars_format::hex parses hexfloat"};
        }
        if (auto r = ConsumeBinaryPrefix(s); r.m_is_binary) {
          throw std::invalid_argument{"chars_format::hex does not parse binfloat"};
        }

        try {
          return do_strtod<T>(s);
        } catch (const std::invalid_argument &err) {
          throw std::invalid_argument("Failed to parse '" + s + "' as hexadecimal: " + err.what());
        } catch (const std::range_error &err) {
          throw std::range_error("Failed to parse '" + s + "' as hexadecimal: " + err.what());
        }
      }
    };

    template <class T>
    struct ParseNumber<T, chars_format::binary> {
      auto operator()(std::string const &s) -> T {
        if (auto r = ConsumeHexPrefix(s); r.m_is_hexadecimal) {
          throw std::invalid_argument{"chars_format::binary does not parse hexfloat"};
        }
        if (auto r = ConsumeBinaryPrefix(s); !r.m_is_binary) {
          throw std::invalid_argument{"chars_format::binary parses binfloat"};
        }

        return do_strtod<T>(s);
      }
    };

    template <class T>
    struct ParseNumber<T, chars_format::scientific> {
      auto operator()(std::string const &s) -> T {
        if (auto r = ConsumeHexPrefix(s); r.m_is_hexadecimal) {
          throw std::invalid_argument{"chars_format::scientific does not parse hexfloat"};
        }
        if (auto r = ConsumeBinaryPrefix(s); r.m_is_binary) {
          throw std::invalid_argument{"chars_format::scientific does not parse binfloat"};
        }
        if (s.find_first_of("eE") == std::string::npos) {
          throw std::invalid_argument{"chars_format::scientific requires exponent part"};
        }

        try {
          return do_strtod<T>(s);
        } catch (const std::invalid_argument &err) {
          throw std::invalid_argument("Failed to parse '" + s + "' as scientific notation: " + err.what());
        } catch (const std::range_error &err) {
          throw std::range_error("Failed to parse '" + s + "' as scientific notation: " + err.what());
        }
      }
    };

    template <class T>
    struct ParseNumber<T, chars_format::fixed> {
      auto operator()(std::string const &s) -> T {
        if (auto r = ConsumeHexPrefix(s); r.m_is_hexadecimal) {
          throw std::invalid_argument{"chars_format::fixed does not parse hexfloat"};
        }
        if (auto r = ConsumeBinaryPrefix(s); r.m_is_binary) {
          throw std::invalid_argument{"chars_format::fixed does not parse binfloat"};
        }
        if (s.find_first_of("eE") != std::string::npos) {
          throw std::invalid_argument{"chars_format::fixed does not parse exponent part"};
        }

        try {
          return do_strtod<T>(s);
        } catch (const std::invalid_argument &err) {
          throw std::invalid_argument("Failed to parse '" + s + "' as fixed notation: " + err.what());
        } catch (const std::range_error &err) {
          throw std::range_error("Failed to parse '" + s + "' as fixed notation: " + err.what());
        }
      }
    };

    template <typename StrIt>
    auto Join(StrIt first, StrIt last, const std::string &separator) -> std::string {
      if (first == last) {
        return "";
      }
      std::stringstream value;
      value << *first;
      ++first;
      while (first != last) {
        value << separator << *first;
        ++first;
      }
      return value.str();
    }

    template <typename T>
    struct CanInvokeToString {
      template <typename U>
      static auto Test(int) -> decltype(std::to_string(std::declval<U>()), std::true_type{});

      template <typename U>
      static auto Test(...) -> std::false_type;

      static constexpr bool value  // NOLINT
          = decltype(Test<T>(0))::value;
    };

    template <typename T>
    struct IsChoiceTypeSupported {
      using CleanType = typename std::decay<T>::type;
      static const bool value =  // NOLINT
          std::is_integral<CleanType>::value || std::is_same<CleanType, std::string>::value ||
          std::is_same<CleanType, std::string_view>::value || std::is_same<CleanType, const char *>::value;
    };

    template <typename StringType>
    auto GetLevenshteinDistance(const StringType &s1, const StringType &s2) -> std::size_t {
      std::vector<std::vector<std::size_t>> dp(s1.size() + 1, std::vector<std::size_t>(s2.size() + 1, 0));

      for (std::size_t i = 0; i <= s1.size(); ++i) {
        for (std::size_t j = 0; j <= s2.size(); ++j) {
          if (i == 0) {
            dp[i][j] = j;
          } else if (j == 0) {
            dp[i][j] = i;
          } else if (s1[i - 1] == s2[j - 1]) {
            dp[i][j] = dp[i - 1][j - 1];
          } else {
            dp[i][j] = 1 + std::min<std::size_t>({dp[i - 1][j], dp[i][j - 1], dp[i - 1][j - 1]});
          }
        }
      }

      return dp[s1.size()][s2.size()];
    }

    template <typename ValueType>
    auto GetMostSimilarString(const std::map<std::string, ValueType> &map, const std::string &input) -> std::string {
      std::string most_similar{};
      std::size_t min_distance = (std::numeric_limits<std::size_t>::max)();

      for (const auto &entry : map) {
        std::size_t distance = GetLevenshteinDistance(entry.first, input);
        if (distance < min_distance) {
          min_distance = distance;
          most_similar = entry.first;
        }
      }

      return most_similar;
    }

  }  // namespace details

  enum class nargs_pattern { optional, any, at_least_one };

  enum class default_arguments : unsigned int {
    none = 0,
    help = 1,
    version = 2,
    all = help | version,
  };

  inline auto operator&(const default_arguments &a, const default_arguments &b) -> default_arguments {
    return static_cast<default_arguments>(static_cast<std::underlying_type<default_arguments>::type>(a) &
                                          static_cast<std::underlying_type<default_arguments>::type>(b));
  }

  class ArgumentParser;

  class Argument {
    friend class ArgumentParser;
    friend auto operator<<(std::ostream &stream, const ArgumentParser &parser) -> std::ostream &;

    template <std::size_t N, std::size_t... I>
    explicit Argument(std::string_view prefix_chars, std::array<std::string_view, N> &&a,
                      std::index_sequence<I...> /*unused*/)
        : m_accepts_optional_like_value(false),
          m_is_optional((IsOptional(a[I], prefix_chars) || ...)),
          m_is_required(false),
          m_is_repeatable(false),
          m_is_used(false),
          m_is_hidden(false),
          m_prefix_chars(prefix_chars) {
      ((void)m_names.emplace_back(a[I]), ...);
      std::sort(m_names.begin(), m_names.end(), [](const auto &lhs, const auto &rhs) {
        return lhs.size() == rhs.size() ? lhs < rhs : lhs.size() < rhs.size();
      });
    }

  public:
    template <std::size_t N>
    explicit Argument(std::string_view prefix_chars, std::array<std::string_view, N> &&a)
        : Argument(prefix_chars, std::move(a), std::make_index_sequence<N>{}) {}

    auto Help(std::string help_text) -> Argument & {
      m_help = std::move(help_text);
      return *this;
    }

    auto Metavar(std::string metavar) -> Argument & {
      m_metavar = std::move(metavar);
      return *this;
    }

    template <typename T>
    auto DefaultValue(T &&value) -> Argument & {
      m_num_args_range = NArgsRange{0, m_num_args_range.GetMax()};
      m_default_value_repr = details::Repr(value);

      if constexpr (std::is_convertible_v<T, std::string_view>) {
        m_default_value_str = std::string{std::string_view{value}};
      } else if constexpr (details::CanInvokeToString<T>::value) {
        m_default_value_str = std::to_string(value);
      }

      m_default_value = std::forward<T>(value);
      return *this;
    }

    auto DefaultValue(const char *value) -> Argument & { return DefaultValue(std::string(value)); }

    auto Required() -> Argument & {
      m_is_required = true;
      return *this;
    }

    auto ImplicitValue(std::any value) -> Argument & {
      m_implicit_value = std::move(value);
      m_num_args_range = NArgsRange{0, 0};
      return *this;
    }

    // This is shorthand for:
    //   program.add_argument("foo")
    //     .default_value(false)
    //     .implicit_value(true)
    auto Flag() -> Argument & {
      DefaultValue(false);
      ImplicitValue(true);
      return *this;
    }

    template <class F, class... Args>
    auto Action(F &&callable, Args &&...bound_args)
        -> std::enable_if_t<std::is_invocable_v<F, Args..., std::string const>, Argument &> {
      using action_type = std::conditional_t<std::is_void_v<std::invoke_result_t<F, Args..., std::string const>>,
                                             void_action, valued_action>;
      if constexpr (sizeof...(Args) == 0) {
        m_action.emplace<action_type>(std::forward<F>(callable));
      } else {
        m_action.emplace<action_type>(
            [f = std::forward<F>(callable), tup = std::make_tuple(std::forward<Args>(bound_args)...)](
                std::string const &opt) mutable { return details::ApplyPlusOne(f, tup, opt); });
      }
      return *this;
    }

    auto StoreInto(bool &var) -> auto & {
      if ((!m_default_value.has_value()) && (!m_implicit_value.has_value())) {
        Flag();
      }
      if (m_default_value.has_value()) {
        var = std::any_cast<bool>(m_default_value);
      }
      Action([&var](const auto & /*unused*/) { var = true; });
      return *this;
    }

    template <typename T, typename std::enable_if<std::is_integral<T>::value>::type * = nullptr>
    auto StoreInto(T &var) -> auto & {
      if (m_default_value.has_value()) {
        var = std::any_cast<T>(m_default_value);
      }
      Action([&var](const auto &s) { var = details::ParseNumber<T, details::kRadix10>()(s); });
      return *this;
    }

    auto StoreInto(double &var) -> auto & {
      if (m_default_value.has_value()) {
        var = std::any_cast<double>(m_default_value);
      }
      Action([&var](const auto &s) { var = details::ParseNumber<double, details::chars_format::general>()(s); });
      return *this;
    }

    auto StoreInto(std::string &var) -> auto & {
      if (m_default_value.has_value()) {
        var = std::any_cast<std::string>(m_default_value);
      }
      Action([&var](const std::string &s) { var = s; });
      return *this;
    }

    auto StoreInto(std::vector<std::string> &var) -> auto & {
      if (m_default_value.has_value()) {
        var = std::any_cast<std::vector<std::string>>(m_default_value);
      }
      Action([this, &var](const std::string &s) {
        if (!m_is_used) {
          var.clear();
        }
        m_is_used = true;
        var.push_back(s);
      });
      return *this;
    }

    auto StoreInto(std::vector<int> &var) -> auto & {
      if (m_default_value.has_value()) {
        var = std::any_cast<std::vector<int>>(m_default_value);
      }
      Action([this, &var](const std::string &s) {
        if (!m_is_used) {
          var.clear();
        }
        m_is_used = true;
        var.push_back(details::ParseNumber<int, details::kRadix10>()(s));
      });
      return *this;
    }

    auto StoreInto(std::set<std::string> &var) -> auto & {
      if (m_default_value.has_value()) {
        var = std::any_cast<std::set<std::string>>(m_default_value);
      }
      Action([this, &var](const std::string &s) {
        if (!m_is_used) {
          var.clear();
        }
        m_is_used = true;
        var.insert(s);
      });
      return *this;
    }

    auto StoreInto(std::set<int> &var) -> auto & {
      if (m_default_value.has_value()) {
        var = std::any_cast<std::set<int>>(m_default_value);
      }
      Action([this, &var](const std::string &s) {
        if (!m_is_used) {
          var.clear();
        }
        m_is_used = true;
        var.insert(details::ParseNumber<int, details::kRadix10>()(s));
      });
      return *this;
    }

    auto Append() -> auto & {
      m_is_repeatable = true;
      return *this;
    }

    // Cause the argument to be invisible in usage and help
    auto Hidden() -> auto & {
      m_is_hidden = true;
      return *this;
    }

    template <char Shape, typename T>
    auto Scan() -> std::enable_if_t<std::is_arithmetic_v<T>, Argument &> {
      static_assert(!(std::is_const_v<T> || std::is_volatile_v<T>), "T should not be cv-qualified");
      auto is_one_of = [](char c, auto... x) constexpr { return ((c == x) || ...); };

      if constexpr (is_one_of(Shape, 'd') && details::kStandardInteger<T>) {
        Action(details::ParseNumber<T, details::kRadix10>());
      } else if constexpr (is_one_of(Shape, 'i') && details::kStandardInteger<T>) {
        Action(details::ParseNumber<T>());
      } else if constexpr (is_one_of(Shape, 'u') && details::kStandardUnsignedInteger<T>) {
        Action(details::ParseNumber<T, details::kRadix10>());
      } else if constexpr (is_one_of(Shape, 'b') && details::kStandardUnsignedInteger<T>) {
        Action(details::ParseNumber<T, details::kRadix2>());
      } else if constexpr (is_one_of(Shape, 'o') && details::kStandardUnsignedInteger<T>) {
        Action(details::ParseNumber<T, details::kRadix8>());
      } else if constexpr (is_one_of(Shape, 'x', 'X') && details::kStandardUnsignedInteger<T>) {
        Action(details::ParseNumber<T, details::kRadix16>());
      } else if constexpr (is_one_of(Shape, 'a', 'A') && std::is_floating_point_v<T>) {
        Action(details::ParseNumber<T, details::chars_format::hex>());
      } else if constexpr (is_one_of(Shape, 'e', 'E') && std::is_floating_point_v<T>) {
        Action(details::ParseNumber<T, details::chars_format::scientific>());
      } else if constexpr (is_one_of(Shape, 'f', 'F') && std::is_floating_point_v<T>) {
        Action(details::ParseNumber<T, details::chars_format::fixed>());
      } else if constexpr (is_one_of(Shape, 'g', 'G') && std::is_floating_point_v<T>) {
        Action(details::ParseNumber<T, details::chars_format::general>());
      } else {
        static_assert(alignof(T) == 0, "No scan specification for T");
      }

      return *this;
    }

    auto Nargs(std::size_t num_args) -> Argument & {
      m_num_args_range = NArgsRange{num_args, num_args};
      return *this;
    }

    auto Nargs(std::size_t num_args_min, std::size_t num_args_max) -> Argument & {
      m_num_args_range = NArgsRange{num_args_min, num_args_max};
      return *this;
    }

    auto Nargs(nargs_pattern pattern) -> Argument & {
      switch (pattern) {
        case nargs_pattern::optional:
          m_num_args_range = NArgsRange{0, 1};
          break;
        case nargs_pattern::any:
          m_num_args_range = NArgsRange{0, (std::numeric_limits<std::size_t>::max)()};
          break;
        case nargs_pattern::at_least_one:
          m_num_args_range = NArgsRange{1, (std::numeric_limits<std::size_t>::max)()};
          break;
      }
      return *this;
    }

    auto Remaining() -> Argument & {
      m_accepts_optional_like_value = true;
      return Nargs(nargs_pattern::any);
    }

    template <typename T>
    void AddChoice(T &&choice) {
      static_assert(details::IsChoiceTypeSupported<T>::value, "Only string or integer type supported for choice");
      static_assert(std::is_convertible_v<T, std::string_view> || details::CanInvokeToString<T>::value,
                    "Choice is not convertible to string_type");
      if (!m_choices.has_value()) {
        m_choices = std::vector<std::string>{};
      }

      if constexpr (std::is_convertible_v<T, std::string_view>) {
        m_choices.value().push_back(std::string{std::string_view{std::forward<T>(choice)}});
      } else if constexpr (details::CanInvokeToString<T>::value) {
        m_choices.value().push_back(std::to_string(std::forward<T>(choice)));
      }
    }

    auto Choices() -> Argument & {
      if (!m_choices.has_value()) {
        throw std::runtime_error("Zero choices provided");
      }
      return *this;
    }

    template <typename T, typename... U>
    auto Choices(T &&first, U &&...rest) -> Argument & {
      AddChoice(std::forward<T>(first));
      Choices(std::forward<U>(rest)...);
      return *this;
    }

    void FindDefaultValueInChoicesOrThrow() const {
      const auto &choices = m_choices.value();

      if (m_default_value.has_value()) {
        if (std::find(choices.begin(), choices.end(), m_default_value_str) == choices.end()) {
          // provided arg not in list of allowed choices
          // report error

          std::string choices_as_csv = std::accumulate(
              choices.begin(), choices.end(), std::string(),
              [](const std::string &a, const std::string &b) { return a + (a.empty() ? "" : ", ") + b; });

          throw std::runtime_error(std::string{"Invalid default value "} + m_default_value_repr +
                                   " - allowed options: {" + choices_as_csv + "}");
        }
      }
    }

    template <typename Iterator>
    [[nodiscard]] auto IsValueInChoices(Iterator option_it) const -> bool {
      const auto &choices = m_choices.value();

      return (std::find(choices.begin(), choices.end(), *option_it) != choices.end());
    }

    template <typename Iterator>
    void ThrowInvalidArgumentsError(Iterator option_it) const {
      const auto &choices = m_choices.value();
      const std::string choices_as_csv = std::accumulate(choices.begin(), choices.end(), std::string(),
                                                         [](const std::string &option_a, const std::string &option_b) {
                                                           return option_a + (option_a.empty() ? "" : ", ") + option_b;
                                                         });

      throw std::runtime_error(std::string{"Invalid argument "} + details::Repr(*option_it) + " - allowed options: {" +
                               choices_as_csv + "}");
    }

    /* The dry_run parameter can be set to true to avoid running the actions,
     * and setting m_is_used. This may be used by a pre-processing step to do
     * a first iteration over arguments.
     */
    template <typename Iterator>
    auto Consume(Iterator start, Iterator end, std::string_view used_name = {}, bool dry_run = false) -> Iterator {
      if (!m_is_repeatable && m_is_used) {
        throw std::runtime_error(std::string("Duplicate argument ").append(used_name));
      }
      m_used_name = used_name;

      std::size_t passed_options = 0;

      if (m_choices.has_value()) {
        // Check each value in (start, end) and make sure
        // it is in the list of allowed choices/options
        const auto max_number_of_args = m_num_args_range.GetMax();
        const auto min_number_of_args = m_num_args_range.GetMin();
        for (auto it = start; it != end; ++it) {
          if (IsValueInChoices(it)) {
            passed_options += 1;
            continue;
          }

          if ((passed_options >= min_number_of_args) && (passed_options <= max_number_of_args)) {
            break;
          }

          ThrowInvalidArgumentsError(it);
        }
      }

      const auto num_args_max = (m_choices.has_value()) ? passed_options : m_num_args_range.GetMax();
      const auto num_args_min = m_num_args_range.GetMin();
      std::size_t dist = 0;
      if (num_args_max == 0) {
        if (!dry_run) {
          m_values.emplace_back(m_implicit_value);
          std::visit([](const auto &f) { f({}); }, m_action);
          m_is_used = true;
        }
        return start;
      }
      if ((dist = static_cast<std::size_t>(std::distance(start, end))) >= num_args_min) {
        if (num_args_max < dist) {
          end = std::next(start, static_cast<typename Iterator::difference_type>(num_args_max));
        }
        if (!m_accepts_optional_like_value) {
          end = std::find_if(start, end, std::bind(IsOptional, std::placeholders::_1, m_prefix_chars));
          dist = static_cast<std::size_t>(std::distance(start, end));
          if (dist < num_args_min) {
            throw std::runtime_error("Too few arguments for '" + std::string(m_used_name) + "'.");
          }
        }
        struct ActionApply {
          void operator()(valued_action &f) { std::transform(m_first, m_last, std::back_inserter(m_self.m_values), f); }

          void operator()(void_action &f) {
            std::for_each(m_first, m_last, f);
            if (!m_self.m_default_value.has_value()) {
              if (!m_self.m_accepts_optional_like_value) {
                m_self.m_values.resize(static_cast<std::size_t>(std::distance(m_first, m_last)));
              }
            }
          }

          Iterator m_first, m_last;
          Argument &m_self;
        };
        if (!dry_run) {
          std::visit(ActionApply{start, end, *this}, m_action);
          m_is_used = true;
        }
        return end;
      }
      if (m_default_value.has_value()) {
        if (!dry_run) {
          m_is_used = true;
        }
        return start;
      }
      throw std::runtime_error("Too few arguments for '" + std::string(m_used_name) + "'.");
    }

    /*
     * @throws std::runtime_error if argument values are not valid
     */
    void Validate() const {
      if (m_is_optional) {
        // TODO: check if an implicit value was programmed for this argument
        if (!m_is_used && !m_default_value.has_value() && m_is_required) {
          ThrowRequiredArgNotUsedError();
        }
        if (m_is_used && m_is_required && m_values.empty()) {
          ThrowRequiredArgNoValueProvidedError();
        }
      } else {
        if (!m_num_args_range.Contains(m_values.size()) && !m_default_value.has_value()) {
          ThrowNargsRangeValidationError();
        }
      }

      if (m_choices.has_value()) {
        // Make sure the default value (if provided)
        // is in the list of choices
        FindDefaultValueInChoicesOrThrow();
      }
    }

    [[nodiscard]] auto GetNamesCsv(char separator = ',') const -> std::string {
      return std::accumulate(m_names.begin(), m_names.end(), std::string{""},
                             [&](const std::string &result, const std::string &name) {
                               return result.empty() ? name : result + separator + name;
                             });
    }

    [[nodiscard]] auto GetUsageFull() const -> std::string {
      std::stringstream usage;

      usage << GetNamesCsv('/');
      const std::string metavar = !m_metavar.empty() ? m_metavar : "VAR";
      if (m_num_args_range.GetMax() > 0) {
        usage << " " << metavar;
        if (m_num_args_range.GetMax() > 1) {
          usage << "...";
        }
      }
      return usage.str();
    }

    [[nodiscard]] auto GetInlineUsage() const -> std::string {
      std::stringstream usage;
      // Find the longest variant to show in the usage string
      std::string longest_name = m_names.front();
      for (const auto &s : m_names) {
        if (s.size() > longest_name.size()) {
          longest_name = s;
        }
      }
      if (!m_is_required) {
        usage << "[";
      }
      usage << longest_name;
      const std::string metavar = !m_metavar.empty() ? m_metavar : "VAR";
      if (m_num_args_range.GetMax() > 0) {
        usage << " " << metavar;
        if (m_num_args_range.GetMax() > 1 && m_metavar.find("> <") == std::string::npos) {
          usage << "...";
        }
      }
      if (!m_is_required) {
        usage << "]";
      }
      if (m_is_repeatable) {
        usage << "...";
      }
      return usage.str();
    }

    [[nodiscard]] auto GetArgumentsLength() const -> std::size_t {
      std::size_t names_size = std::accumulate(std::begin(m_names), std::end(m_names), std::size_t(0),
                                               [](const auto &sum, const auto &s) { return sum + s.size(); });

      if (IsPositional(m_names.front(), m_prefix_chars)) {
        // A set metavar means this replaces the names
        if (!m_metavar.empty()) {
          // Indent and metavar
          return 2 + m_metavar.size();
        }

        // Indent and space-separated
        return 2 + names_size + (m_names.size() - 1);
      }
      // Is an option - include both names _and_ metavar
      // size = text + (", " between names)
      std::size_t size = names_size + 2 * (m_names.size() - 1);
      if (!m_metavar.empty() && m_num_args_range == NArgsRange{1, 1}) {
        size += m_metavar.size() + 1;
      }
      return size + 2;  // indent
    }

    friend auto operator<<(std::ostream &stream, const Argument &argument) -> std::ostream & {
      std::stringstream name_stream;
      name_stream << "  ";  // indent
      if (argument.IsPositional(argument.m_names.front(), argument.m_prefix_chars)) {
        if (!argument.m_metavar.empty()) {
          name_stream << argument.m_metavar;
        } else {
          name_stream << details::Join(argument.m_names.begin(), argument.m_names.end(), " ");
        }
      } else {
        name_stream << details::Join(argument.m_names.begin(), argument.m_names.end(), ", ");
        // If we have a metavar, and one narg - print the metavar
        if (!argument.m_metavar.empty() && argument.m_num_args_range == NArgsRange{1, 1}) {
          name_stream << " " << argument.m_metavar;
        } else if (!argument.m_metavar.empty() &&
                   argument.m_num_args_range.GetMin() == argument.m_num_args_range.GetMax() &&
                   argument.m_metavar.find("> <") != std::string::npos) {
          name_stream << " " << argument.m_metavar;
        }
      }

      // align multiline help message
      auto stream_width = stream.width();
      auto name_padding = std::string(name_stream.str().size(), ' ');
      auto pos = std::string::size_type{};
      auto prev = std::string::size_type{};
      auto first_line = true;
      auto hspace = "  ";  // minimal space between name and help message
      stream << name_stream.str();
      std::string_view help_view(argument.m_help);
      while ((pos = argument.m_help.find('\n', prev)) != std::string::npos) {
        auto line = help_view.substr(prev, pos - prev + 1);
        if (first_line) {
          stream << hspace << line;
          first_line = false;
        } else {
          stream.width(stream_width);
          stream << name_padding << hspace << line;
        }
        prev += pos - prev + 1;
      }
      if (first_line) {
        stream << hspace << argument.m_help;
      } else {
        auto leftover = help_view.substr(prev, argument.m_help.size() - prev);
        if (!leftover.empty()) {
          stream.width(stream_width);
          stream << name_padding << hspace << leftover;
        }
      }

      // print nargs spec
      if (!argument.m_help.empty()) {
        stream << " ";
      }
      stream << argument.m_num_args_range;

      bool add_space = false;
      if (argument.m_default_value.has_value() && argument.m_num_args_range != NArgsRange{0, 0}) {
        stream << "[default: " << argument.m_default_value_repr << "]";
        add_space = true;
      } else if (argument.m_is_required) {
        stream << "[required]";
        add_space = true;
      }
      if (argument.m_is_repeatable) {
        if (add_space) {
          stream << " ";
        }
        stream << "[may be repeated]";
      }
      stream << "\n";
      return stream;
    }

    template <typename T>
    auto operator!=(const T &rhs) const -> bool {
      return !(*this == rhs);
    }

    /*
     * Compare to an argument value of known type
     * @throws std::logic_error in case of incompatible types
     */
    template <typename T>
    auto operator==(const T &rhs) const -> bool {
      if constexpr (!details::kIsContainer<T>) {
        return Get<T>() == rhs;
      } else {
        using ValueType = typename T::value_type;
        auto lhs = Get<T>();
        return std::equal(std::begin(lhs), std::end(lhs), std::begin(rhs), std::end(rhs),
                          [](const auto &a, const auto &b) { return std::any_cast<const ValueType &>(a) == b; });
      }
    }

    /*
     * positional:
     *    _empty_
     *    '-'
     *    '-' decimal-literal
     *    !'-' anything
     */
    static auto IsPositional(std::string_view name, std::string_view prefix_chars) -> bool {
      auto first = Lookahead(name);

      if (first == kEof) {
        return true;
      }
      if (prefix_chars.find(static_cast<char>(first)) != std::string_view::npos) {
        name.remove_prefix(1);
        if (name.empty()) {
          return true;
        }
        return IsDecimalLiteral(name);
      }
      return true;
    }

  private:
    class NArgsRange {
      std::size_t m_min;
      std::size_t m_max;

    public:
      NArgsRange(std::size_t minimum, std::size_t maximum) : m_min(minimum), m_max(maximum) {
        if (minimum > maximum) {
          throw std::logic_error("Range of number of arguments is invalid");
        }
      }

      [[nodiscard]] auto Contains(std::size_t value) const -> bool { return value >= m_min && value <= m_max; }

      [[nodiscard]] auto IsExact() const -> bool { return m_min == m_max; }

      [[nodiscard]] auto IsRightBounded() const -> bool { return m_max < (std::numeric_limits<std::size_t>::max)(); }

      [[nodiscard]] auto GetMin() const -> std::size_t { return m_min; }

      [[nodiscard]] auto GetMax() const -> std::size_t { return m_max; }

      // Print help message
      friend auto operator<<(std::ostream &stream, const NArgsRange &range) -> std::ostream & {
        if (range.m_min == range.m_max) {
          if (range.m_min != 0 && range.m_min != 1) {
            stream << "[nargs: " << range.m_min << "] ";
          }
        } else {
          if (range.m_max == (std::numeric_limits<std::size_t>::max)()) {
            stream << "[nargs: " << range.m_min << " or more] ";
          } else {
            stream << "[nargs=" << range.m_min << ".." << range.m_max << "] ";
          }
        }
        return stream;
      }

      auto operator==(const NArgsRange &rhs) const -> bool { return rhs.m_min == m_min && rhs.m_max == m_max; }

      auto operator!=(const NArgsRange &rhs) const -> bool { return !(*this == rhs); }
    };

    void ThrowNargsRangeValidationError() const {
      std::stringstream stream;
      if (!m_used_name.empty()) {
        stream << m_used_name << ": ";
      } else {
        stream << m_names.front() << ": ";
      }
      if (m_num_args_range.IsExact()) {
        stream << m_num_args_range.GetMin();
      } else if (m_num_args_range.IsRightBounded()) {
        stream << m_num_args_range.GetMin() << " to " << m_num_args_range.GetMax();
      } else {
        stream << m_num_args_range.GetMin() << " or more";
      }
      stream << " argument(s) expected. " << m_values.size() << " provided.";
      throw std::runtime_error(stream.str());
    }

    void ThrowRequiredArgNotUsedError() const {
      std::stringstream stream;
      stream << m_names.front() << ": required.";
      throw std::runtime_error(stream.str());
    }

    void ThrowRequiredArgNoValueProvidedError() const {
      std::stringstream stream;
      stream << m_used_name << ": no value provided.";
      throw std::runtime_error(stream.str());
    }

    static constexpr int kEof = std::char_traits<char>::eof();

    static auto Lookahead(std::string_view s) -> int {
      if (s.empty()) {
        return kEof;
      }
      return static_cast<int>(static_cast<unsigned char>(s[0]));
    }

    /*
     * decimal-literal:
     *    '0'
     *    nonzero-digit digit-sequence_opt
     *    integer-part fractional-part
     *    fractional-part
     *    integer-part '.' exponent-part_opt
     *    integer-part exponent-part
     *
     * integer-part:
     *    digit-sequence
     *
     * fractional-part:
     *    '.' post-decimal-point
     *
     * post-decimal-point:
     *    digit-sequence exponent-part_opt
     *
     * exponent-part:
     *    'e' post-e
     *    'E' post-e
     *
     * post-e:
     *    sign_opt digit-sequence
     *
     * sign: one of
     *    '+' '-'
     */
    static auto IsDecimalLiteral(std::string_view s) -> bool {
      auto is_digit = [](auto c) constexpr {
        switch (c) {
          case '0':
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':
            return true;
          default:
            return false;
        }
      };

      // precondition: we have consumed or will consume at least one digit
      auto consume_digits = [=](std::string_view sd) {
        // NOLINTNEXTLINE(readability-qualified-auto)
        auto it = std::find_if_not(std::begin(sd), std::end(sd), is_digit);
        return sd.substr(static_cast<std::size_t>(it - std::begin(sd)));
      };

      switch (Lookahead(s)) {
        case '0': {
          s.remove_prefix(1);
          if (s.empty()) {
            return true;
          }
          goto integer_part;
        }
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': {
          s = consume_digits(s);
          if (s.empty()) {
            return true;
          }
          goto integer_part_consumed;
        }
        case '.': {
          s.remove_prefix(1);
          goto post_decimal_point;
        }
        default:
          return false;
      }

    integer_part:
      s = consume_digits(s);
    integer_part_consumed:
      switch (Lookahead(s)) {
        case '.': {
          s.remove_prefix(1);
          if (is_digit(Lookahead(s))) {
            goto post_decimal_point;
          } else {
            goto exponent_part_opt;
          }
        }
        case 'e':
        case 'E': {
          s.remove_prefix(1);
          goto post_e;
        }
        default:
          return false;
      }

    post_decimal_point:
      if (is_digit(Lookahead(s))) {
        s = consume_digits(s);
        goto exponent_part_opt;
      }
      return false;

    exponent_part_opt:
      switch (Lookahead(s)) {
        case kEof:
          return true;
        case 'e':
        case 'E': {
          s.remove_prefix(1);
          goto post_e;
        }
        default:
          return false;
      }

    post_e:
      switch (Lookahead(s)) {
        case '-':
        case '+':
          s.remove_prefix(1);
      }
      if (is_digit(Lookahead(s))) {
        s = consume_digits(s);
        return s.empty();
      }
      return false;
    }

    static auto IsOptional(std::string_view name, std::string_view prefix_chars) -> bool {
      return !IsPositional(name, prefix_chars);
    }

    /*
     * Get argument value given a type
     * @throws std::logic_error in case of incompatible types
     */
    template <typename T>
    [[nodiscard]] [[nodiscard]] auto Get() const -> T {
      if (!m_values.empty()) {
        if constexpr (details::kIsContainer<T>) {
          return any_cast_container<T>(m_values);
        } else {
          return std::any_cast<T>(m_values.front());
        }
      }
      if (m_default_value.has_value()) {
        return std::any_cast<T>(m_default_value);
      }
      if constexpr (details::kIsContainer<T>) {
        if (!m_accepts_optional_like_value) {
          return any_cast_container<T>(m_values);
        }
      }

      throw std::logic_error("No value provided for '" + m_names.back() + "'.");
    }

    /*
     * Get argument value given a type.
     * @pre The object has no default value.
     * @returns The stored value if any, std::nullopt otherwise.
     */
    template <typename T>
    auto Present() const -> std::optional<T> {
      if (m_default_value.has_value()) {
        throw std::logic_error("Argument with default value always presents");
      }
      if (m_values.empty()) {
        return std::nullopt;
      }
      if constexpr (details::kIsContainer<T>) {
        return any_cast_container<T>(m_values);
      }
      return std::any_cast<T>(m_values.front());
    }

    template <typename T>
    static auto AnyCastContainer(const std::vector<std::any> &operand) -> T {
      using ValueType = typename T::value_type;

      T result;
      std::transform(std::begin(operand), std::end(operand), std::back_inserter(result),
                     [](const auto &value) { return std::any_cast<ValueType>(value); });
      return result;
    }

    void SetUsageNewlineCounter(int i) { m_usage_newline_counter = i; }

    void SetGroupIdx(std::size_t i) { m_group_idx = i; }

    std::vector<std::string> m_names;
    std::string_view m_used_name;
    std::string m_help;
    std::string m_metavar;
    std::any m_default_value;
    std::string m_default_value_repr;
    std::optional<std::string> m_default_value_str;  // used for checking default_value against choices
    std::any m_implicit_value;
    std::optional<std::vector<std::string>> m_choices{std::nullopt};
    using valued_action = std::function<std::any(const std::string &)>;
    using void_action = std::function<void(const std::string &)>;
    std::variant<valued_action, void_action> m_action{std::in_place_type<valued_action>,
                                                      [](const std::string &value) { return value; }};
    std::vector<std::any> m_values;
    NArgsRange m_num_args_range{1, 1};
    // Bit field of bool values. Set default value in ctor.
    bool m_accepts_optional_like_value : 1;
    bool m_is_optional : 1;
    bool m_is_required : 1;
    bool m_is_repeatable : 1;
    bool m_is_used : 1;
    bool m_is_hidden : 1;             // if set, does not appear in usage or help
    std::string_view m_prefix_chars;  // ArgumentParser has the prefix_chars
    int m_usage_newline_counter = 0;
    std::size_t m_group_idx = 0;
  };

  class ArgumentParser {
  public:
    explicit ArgumentParser(std::string program_name = {}, std::string version = "1.0",
                            default_arguments add_args = default_arguments::all, bool exit_on_default_arguments = true,
                            std::ostream &os = std::cout)
        : m_program_name(std::move(program_name)),
          m_version(std::move(version)),
          m_exit_on_default_arguments(exit_on_default_arguments),
          m_parser_path(m_program_name) {
      if ((add_args & default_arguments::help) == default_arguments::help) {
        AddArgument("-h", "--help")
            .Action([&](const auto & /*unused*/) {
              os << Help().str();
              if (m_exit_on_default_arguments) {
                std::exit(0);
              }
            })
            .DefaultValue(false)
            .Help("shows help message and exits")
            .ImplicitValue(true)
            .Nargs(0);
      }
      if ((add_args & default_arguments::version) == default_arguments::version) {
        AddArgument("-v", "--version")
            .Action([&](const auto & /*unused*/) {
              os << m_version << std::endl;
              if (m_exit_on_default_arguments) {
                std::exit(0);
              }
            })
            .DefaultValue(false)
            .Help("prints version information and exits")
            .ImplicitValue(true)
            .Nargs(0);
      }
    }

    ~ArgumentParser() = default;

    // ArgumentParser is meant to be used in a single function.
    // Setup everything and parse arguments in one place.
    //
    // ArgumentParser internally uses std::string_views,
    // references, iterators, etc.
    // Many of these elements become invalidated after a copy or move.
    ArgumentParser(const ArgumentParser &other) = delete;
    auto operator=(const ArgumentParser &other) -> ArgumentParser & = delete;
    ArgumentParser(ArgumentParser &&) noexcept = delete;
    auto operator=(ArgumentParser &&) -> ArgumentParser & = delete;

    explicit operator bool() const {
      auto arg_used =
          std::any_of(m_argument_map.cbegin(), m_argument_map.cend(), [](auto &it) { return it.second->m_is_used; });
      auto subparser_used =
          std::any_of(m_subparser_used.cbegin(), m_subparser_used.cend(), [](auto &it) { return it.second; });

      return m_is_parsed && (arg_used || subparser_used);
    }

    // Parameter packing
    // Call add_argument with variadic number of string arguments
    template <typename... Targs>
    auto AddArgument(Targs... f_args) -> Argument & {
      using array_of_sv = std::array<std::string_view, sizeof...(Targs)>;
      auto argument =
          m_optional_arguments.emplace(std::cend(m_optional_arguments), m_prefix_chars, array_of_sv{f_args...});

      if (!argument->m_is_optional) {
        m_positional_arguments.splice(std::cend(m_positional_arguments), m_optional_arguments, argument);
      }
      argument->SetUsageNewlineCounter(m_usage_newline_counter);
      argument->SetGroupIdx(m_group_names.size());

      IndexArgument(argument);
      return *argument;
    }

    class MutuallyExclusiveGroup {
      friend class ArgumentParser;

    public:
      MutuallyExclusiveGroup() = delete;

      explicit MutuallyExclusiveGroup(ArgumentParser &parent, bool required = false)
          : m_parent(parent), m_required(required), m_elements({}) {}

      MutuallyExclusiveGroup(const MutuallyExclusiveGroup &other) = delete;
      auto operator=(const MutuallyExclusiveGroup &other) -> MutuallyExclusiveGroup & = delete;

      MutuallyExclusiveGroup(MutuallyExclusiveGroup &&other) noexcept
          : m_parent(other.m_parent), m_required(other.m_required), m_elements(std::move(other.m_elements)) {
        other.m_elements.clear();
      }

      template <typename... Targs>
      auto AddArgument(Targs... f_args) -> Argument & {
        auto &argument = m_parent.AddArgument(std::forward<Targs>(f_args)...);
        m_elements.push_back(&argument);
        argument.SetUsageNewlineCounter(m_parent.m_usage_newline_counter);
        argument.SetGroupIdx(m_parent.m_group_names.size());
        return argument;
      }

    private:
      ArgumentParser &m_parent;
      bool m_required{false};
      std::vector<Argument *> m_elements{};
    };

    auto AddMutuallyExclusiveGroup(bool required = false) -> MutuallyExclusiveGroup & {
      m_mutually_exclusive_groups.emplace_back(*this, required);
      return m_mutually_exclusive_groups.back();
    }

    // Parameter packed add_parents method
    // Accepts a variadic number of ArgumentParser objects
    template <typename... Targs>
    auto AddParents(const Targs &...f_args) -> ArgumentParser & {
      for (const ArgumentParser &parent_parser : {std::ref(f_args)...}) {
        for (const auto &argument : parent_parser.m_positional_arguments) {
          auto it = m_positional_arguments.insert(std::cend(m_positional_arguments), argument);
          IndexArgument(it);
        }
        for (const auto &argument : parent_parser.m_optional_arguments) {
          auto it = m_optional_arguments.insert(std::cend(m_optional_arguments), argument);
          IndexArgument(it);
        }
      }
      return *this;
    }

    // Ask for the next optional arguments to be displayed on a separate
    // line in usage() output. Only effective if set_usage_max_line_width() is
    // also used.
    auto AddUsageNewline() -> ArgumentParser & {
      ++m_usage_newline_counter;
      return *this;
    }

    // Ask for the next optional arguments to be displayed in a separate section
    // in usage() and help (<< *this) output.
    // For usage(), this is only effective if set_usage_max_line_width() is
    // also used.
    auto AddGroup(std::string group_name) -> ArgumentParser & {
      m_group_names.emplace_back(std::move(group_name));
      return *this;
    }

    auto AddDescription(std::string description) -> ArgumentParser & {
      m_description = std::move(description);
      return *this;
    }

    auto AddEpilog(std::string epilog) -> ArgumentParser & {
      m_epilog = std::move(epilog);
      return *this;
    }

    // Add a un-documented/hidden alias for an argument.
    // Ideally we'd want this to be a method of Argument, but Argument
    // does not own its owing ArgumentParser.
    auto AddHiddenAliasFor(Argument &arg, std::string_view alias) -> ArgumentParser & {
      for (auto it = m_optional_arguments.begin(); it != m_optional_arguments.end(); ++it) {
        if (&(*it) == &arg) {
          m_argument_map.insert_or_assign(std::string(alias), it);
          return *this;
        }
      }
      throw std::logic_error("Argument is not an optional argument of this parser");
    }

    /* Getter for arguments and subparsers.
     * @throws std::logic_error in case of an invalid argument or subparser name
     */
    template <typename T = Argument>
    auto At(std::string_view name) -> T & {
      if constexpr (std::is_same_v<T, Argument>) {
        return (*this)[name];
      } else {
        std::string str_name(name);
        auto subparser_it = m_subparser_map.find(str_name);
        if (subparser_it != m_subparser_map.end()) {
          return subparser_it->second->get();
        }
        throw std::logic_error("No such subparser: " + str_name);
      }
    }

    auto SetPrefixChars(std::string prefix_chars) -> ArgumentParser & {
      m_prefix_chars = std::move(prefix_chars);
      return *this;
    }

    auto SetAssignChars(std::string assign_chars) -> ArgumentParser & {
      m_assign_chars = std::move(assign_chars);
      return *this;
    }

    /* Call parse_args_internal - which does all the work
     * Then, validate the parsed arguments
     * This variant is used mainly for testing
     * @throws std::runtime_error in case of any invalid argument
     */
    void ParseArgs(const std::vector<std::string> &arguments) {
      ParseArgsInternal(arguments);
      // Check if all arguments are parsed
      for ([[maybe_unused]] const auto &[unused, argument] : m_argument_map) {
        argument->Validate();
      }

      // Check each mutually exclusive group and make sure
      // there are no constraint violations
      for (const auto &group : m_mutually_exclusive_groups) {
        auto mutex_argument_used{false};
        Argument *mutex_argument_it{nullptr};
        for (Argument *arg : group.m_elements) {
          if (!mutex_argument_used && arg->m_is_used) {
            mutex_argument_used = true;
            mutex_argument_it = arg;
          } else if (mutex_argument_used && arg->m_is_used) {
            // Violation
            throw std::runtime_error("Argument '" + arg->GetUsageFull() + "' not allowed with '" +
                                     mutex_argument_it->GetUsageFull() + "'");
          }
        }

        if (!mutex_argument_used && group.m_required) {
          // at least one argument from the group is
          // required
          std::string argument_names{};
          std::size_t i = 0;
          std::size_t size = group.m_elements.size();
          for (Argument *arg : group.m_elements) {
            if (i + 1 == size) {
              // last
              argument_names += std::string("'") + arg->GetUsageFull() + std::string("' ");
            } else {
              argument_names += std::string("'") + arg->GetUsageFull() + std::string("' or ");
            }
            i += 1;
          }
          throw std::runtime_error("One of the arguments " + argument_names + "is required");
        }
      }
    }

    /* Call parse_known_args_internal - which does all the work
     * Then, validate the parsed arguments
     * This variant is used mainly for testing
     * @throws std::runtime_error in case of any invalid argument
     */
    auto ParseKnownArgs(const std::vector<std::string> &arguments) -> std::vector<std::string> {
      auto unknown_arguments = ParseKnownArgsInternal(arguments);
      // Check if all arguments are parsed
      for ([[maybe_unused]] const auto &[unused, argument] : m_argument_map) {
        argument->Validate();
      }
      return unknown_arguments;
    }

    /* Main entry point for parsing command-line arguments using this
     * ArgumentParser
     * @throws std::runtime_error in case of any invalid argument
     */
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
    void ParseArgs(int argc, const char *const argv[]) { ParseArgs({argv, argv + argc}); }

    /* Main entry point for parsing command-line arguments using this
     * ArgumentParser
     * @throws std::runtime_error in case of any invalid argument
     */
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
    auto ParseKnownArgs(int argc, const char *const argv[]) { return ParseKnownArgs({argv, argv + argc}); }

    /* Getter for options with default values.
     * @throws std::logic_error if parse_args() has not been previously called
     * @throws std::logic_error if there is no such option
     * @throws std::logic_error if the option has no value
     * @throws std::bad_any_cast if the option is not of type T
     */
    template <typename T = std::string>
    [[nodiscard]] auto Get(std::string_view arg_name) const -> T {
      if (!m_is_parsed) {
        throw std::logic_error("Nothing parsed, no arguments are available.");
      }
      return (*this)[arg_name].Get<T>();
    }

    /* Getter for options without default values.
     * @pre The option has no default value.
     * @throws std::logic_error if there is no such option
     * @throws std::bad_any_cast if the option is not of type T
     */
    template <typename T = std::string>
    auto Present(std::string_view arg_name) const -> std::optional<T> {
      return (*this)[arg_name].Present<T>();
    }

    /* Getter that returns true for user-supplied options. Returns false if not
     * user-supplied, even with a default value.
     */
    [[nodiscard]] auto IsUsed(std::string_view arg_name) const { return (*this)[arg_name].m_is_used; }

    /* Getter that returns true if a subcommand is used.
     */
    [[nodiscard]] auto IsSubcommandUsed(std::string_view subcommand_name) const {
      return m_subparser_used.at(std::string(subcommand_name));
    }

    /* Getter that returns true if a subcommand is used.
     */
    [[nodiscard]] auto IsSubcommandUsed(const ArgumentParser &subparser) const {
      return IsSubcommandUsed(subparser.m_program_name);
    }

    /* Indexing operator. Return a reference to an Argument object
     * Used in conjunction with Argument.operator== e.g., parser["foo"] == true
     * @throws std::logic_error in case of an invalid argument name
     */
    auto operator[](std::string_view arg_name) const -> Argument & {
      std::string name(arg_name);
      auto it = m_argument_map.find(name);
      if (it != m_argument_map.end()) {
        return *(it->second);
      }
      if (!IsValidPrefixChar(arg_name.front())) {
        const auto legal_prefix_char = GetAnyValidPrefixChar();
        const auto prefix = std::string(1, legal_prefix_char);

        // "-" + arg_name
        name = prefix + name;
        it = m_argument_map.find(name);
        if (it != m_argument_map.end()) {
          return *(it->second);
        }
        // "--" + arg_name
        name = prefix + name;
        it = m_argument_map.find(name);
        if (it != m_argument_map.end()) {
          return *(it->second);
        }
      }
      throw std::logic_error("No such argument: " + std::string(arg_name));
    }

    // Print help message
    friend auto operator<<(std::ostream &stream, const ArgumentParser &parser) -> std::ostream & {
      stream.setf(std::ios_base::left);

      auto longest_arg_length = parser.GetLengthOfLongestArgument();

      stream << parser.Usage() << "\n\n";

      if (!parser.m_description.empty()) {
        stream << parser.m_description << "\n\n";
      }

      const bool has_visible_positional_args =
          std::find_if(parser.m_positional_arguments.begin(), parser.m_positional_arguments.end(),
                       [](const auto &argument) { return !argument.m_is_hidden; }) !=
          parser.m_positional_arguments.end();
      if (has_visible_positional_args) {
        stream << "Positional arguments:\n";
      }

      for (const auto &argument : parser.m_positional_arguments) {
        if (!argument.m_is_hidden) {
          stream.width(static_cast<std::streamsize>(longest_arg_length));
          stream << argument;
        }
      }

      if (!parser.m_optional_arguments.empty()) {
        stream << (!has_visible_positional_args ? "" : "\n") << "Optional arguments:\n";
      }

      for (const auto &argument : parser.m_optional_arguments) {
        if (argument.m_group_idx == 0 && !argument.m_is_hidden) {
          stream.width(static_cast<std::streamsize>(longest_arg_length));
          stream << argument;
        }
      }

      for (size_t i_group = 0; i_group < parser.m_group_names.size(); ++i_group) {
        stream << "\n" << parser.m_group_names[i_group] << " (detailed usage):\n";
        for (const auto &argument : parser.m_optional_arguments) {
          if (argument.m_group_idx == i_group + 1 && !argument.m_is_hidden) {
            stream.width(static_cast<std::streamsize>(longest_arg_length));
            stream << argument;
          }
        }
      }

      bool has_visible_subcommands = std::any_of(parser.m_subparser_map.begin(), parser.m_subparser_map.end(),
                                                 [](auto &p) { return !p.second->get().m_suppress; });

      if (has_visible_subcommands) {
        stream << (parser.m_positional_arguments.empty() ? (parser.m_optional_arguments.empty() ? "" : "\n") : "\n")
               << "Subcommands:\n";
        for (const auto &[command, subparser] : parser.m_subparser_map) {
          if (subparser->get().m_suppress) {
            continue;
          }

          stream << std::setw(2) << " ";
          stream << std::setw(static_cast<int>(longest_arg_length - 2)) << command;
          stream << " " << subparser->get().m_description << "\n";
        }
      }

      if (!parser.m_epilog.empty()) {
        stream << '\n';
        stream << parser.m_epilog << "\n\n";
      }

      return stream;
    }

    // Format help message
    [[nodiscard]] auto Help() const -> std::stringstream {
      std::stringstream out;
      out << *this;
      return out;
    }

    // Sets the maximum width for a line of the Usage message
    auto SetUsageMaxLineWidth(size_t w) -> ArgumentParser & {
      this->m_usage_max_line_width = w;
      return *this;
    }

    // Asks to display arguments of mutually exclusive group on separate lines
    // in the Usage message
    auto SetUsageBreakOnMutex() -> ArgumentParser & {
      this->m_usage_break_on_mutex = true;
      return *this;
    }

    // Format usage part of help only
    [[nodiscard]] auto Usage() const -> std::string {
      std::stringstream stream;

      std::string curline("Usage: ");
      curline += this->m_parser_path;
      const bool multiline_usage = this->m_usage_max_line_width < (std::numeric_limits<std::size_t>::max)();
      const size_t indent_size = curline.size();

      const auto deal_with_options_of_group = [&](std::size_t group_idx) {
        bool found_options = false;
        // Add any options inline here
        const MutuallyExclusiveGroup *cur_mutex = nullptr;
        int usage_newline_counter = -1;
        for (const auto &argument : this->m_optional_arguments) {
          if (argument.m_is_hidden) {
            continue;
          }
          if (multiline_usage) {
            if (argument.m_group_idx != group_idx) {
              continue;
            }
            if (usage_newline_counter != argument.m_usage_newline_counter) {
              if (usage_newline_counter >= 0) {
                if (curline.size() > indent_size) {
                  stream << curline << std::endl;
                  curline = std::string(indent_size, ' ');
                }
              }
              usage_newline_counter = argument.m_usage_newline_counter;
            }
          }
          found_options = true;
          const std::string arg_inline_usage = argument.GetInlineUsage();
          const MutuallyExclusiveGroup *arg_mutex = GetBelongingMutex(&argument);
          if ((cur_mutex != nullptr) && (arg_mutex == nullptr)) {
            curline += ']';
            if (this->m_usage_break_on_mutex) {
              stream << curline << std::endl;
              curline = std::string(indent_size, ' ');
            }
          } else if ((cur_mutex == nullptr) && (arg_mutex != nullptr)) {
            if ((this->m_usage_break_on_mutex && curline.size() > indent_size) ||
                curline.size() + 3 + arg_inline_usage.size() > this->m_usage_max_line_width) {
              stream << curline << std::endl;
              curline = std::string(indent_size, ' ');
            }
            curline += " [";
          } else if ((cur_mutex != nullptr) && (arg_mutex != nullptr)) {
            if (cur_mutex != arg_mutex) {
              curline += ']';
              if (this->m_usage_break_on_mutex ||
                  curline.size() + 3 + arg_inline_usage.size() > this->m_usage_max_line_width) {
                stream << curline << std::endl;
                curline = std::string(indent_size, ' ');
              }
              curline += " [";
            } else {
              curline += '|';
            }
          }
          cur_mutex = arg_mutex;
          if (curline.size() + 1 + arg_inline_usage.size() > this->m_usage_max_line_width) {
            stream << curline << std::endl;
            curline = std::string(indent_size, ' ');
            curline += " ";
          } else if (cur_mutex == nullptr) {
            curline += " ";
          }
          curline += arg_inline_usage;
        }
        if (cur_mutex != nullptr) {
          curline += ']';
        }
        return found_options;
      };

      const bool found_options = deal_with_options_of_group(0);

      if (found_options && multiline_usage && !this->m_positional_arguments.empty()) {
        stream << curline << std::endl;
        curline = std::string(indent_size, ' ');
      }
      // Put positional arguments after the optionals
      for (const auto &argument : this->m_positional_arguments) {
        if (argument.m_is_hidden) {
          continue;
        }
        const std::string pos_arg = !argument.m_metavar.empty() ? argument.m_metavar : argument.m_names.front();
        if (curline.size() + 1 + pos_arg.size() > this->m_usage_max_line_width) {
          stream << curline << std::endl;
          curline = std::string(indent_size, ' ');
        }
        curline += " ";
        if (argument.m_num_args_range.GetMin() == 0 && !argument.m_num_args_range.IsRightBounded()) {
          curline += "[";
          curline += pos_arg;
          curline += "]...";
        } else if (argument.m_num_args_range.GetMin() == 1 && !argument.m_num_args_range.IsRightBounded()) {
          curline += pos_arg;
          curline += "...";
        } else {
          curline += pos_arg;
        }
      }

      if (multiline_usage) {
        // Display options of other groups
        for (std::size_t i = 0; i < m_group_names.size(); ++i) {
          stream << curline << std::endl << std::endl;
          stream << m_group_names[i] << ":" << std::endl;
          curline = std::string(indent_size, ' ');
          deal_with_options_of_group(i + 1);
        }
      }

      stream << curline;

      // Put subcommands after positional arguments
      if (!m_subparser_map.empty()) {
        stream << " {";
        std::size_t i{0};
        for (const auto &[command, subparser] : m_subparser_map) {
          if (subparser->get().m_suppress) {
            continue;
          }

          if (i == 0) {
            stream << command;
          } else {
            stream << "," << command;
          }
          ++i;
        }
        stream << "}";
      }

      return stream.str();
    }

    // Printing the one and only help message
    // I've stuck with a simple message format, nothing fancy.
    [[deprecated("Use cout << program; instead.  See also help().")]] [[nodiscard]] auto PrintHelp() const
        -> std::string {
      auto out = Help();
      std::cout << out.rdbuf();
      return out.str();
    }

    void AddSubparser(ArgumentParser &parser) {
      parser.m_parser_path = m_program_name + " " + parser.m_program_name;
      auto it = m_subparsers.emplace(std::cend(m_subparsers), parser);
      m_subparser_map.insert_or_assign(parser.m_program_name, it);
      m_subparser_used.insert_or_assign(parser.m_program_name, false);
    }

    void SetSuppress(bool suppress) { m_suppress = suppress; }

  protected:
    auto GetBelongingMutex(const Argument *arg) const -> const MutuallyExclusiveGroup * {
      for (const auto &mutex : m_mutually_exclusive_groups) {
        if (std::find(mutex.m_elements.begin(), mutex.m_elements.end(), arg) != mutex.m_elements.end()) {
          return &mutex;
        }
      }
      return nullptr;
    }

    [[nodiscard]] auto IsValidPrefixChar(char c) const -> bool { return m_prefix_chars.find(c) != std::string::npos; }

    [[nodiscard]] auto GetAnyValidPrefixChar() const -> char { return m_prefix_chars[0]; }

    /*
     * Pre-process this argument list. Anything starting with "--", that
     * contains an =, where the prefix before the = has an entry in the
     * options table, should be split.
     */
    [[nodiscard]] auto PreprocessArguments(const std::vector<std::string> &raw_arguments) const
        -> std::vector<std::string> {
      std::vector<std::string> arguments{};
      for (const auto &arg : raw_arguments) {
        const auto argument_starts_with_prefix_chars = [this](const std::string &a) -> bool {
          if (!a.empty()) {
            const auto legal_prefix = [this](char c) -> bool { return m_prefix_chars.find(c) != std::string::npos; };

            // Windows-style
            // if '/' is a legal prefix char
            // then allow single '/' followed by argument name, followed by an
            // assign char, e.g., ':' e.g., 'test.exe /A:Foo'
            const auto windows_style = legal_prefix('/');

            if (windows_style) {
              if (legal_prefix(a[0])) {
                return true;
              }
            } else {
              // Slash '/' is not a legal prefix char
              // For all other characters, only support long arguments
              // i.e., the argument must start with 2 prefix chars, e.g,
              // '--foo' e,g, './test --foo=Bar -DARG=yes'
              if (a.size() > 1) {
                return (legal_prefix(a[0]) && legal_prefix(a[1]));
              }
            }
          }
          return false;
        };

        // Check that:
        // - We don't have an argument named exactly this
        // - The argument starts with a prefix char, e.g., "--"
        // - The argument contains an assign char, e.g., "="
        auto assign_char_pos = arg.find_first_of(m_assign_chars);

        if (m_argument_map.find(arg) == m_argument_map.end() && argument_starts_with_prefix_chars(arg) &&
            assign_char_pos != std::string::npos) {
          // Get the name of the potential option, and check it exists
          std::string opt_name = arg.substr(0, assign_char_pos);
          if (m_argument_map.find(opt_name) != m_argument_map.end()) {
            // This is the name of an option! Split it into two parts
            arguments.push_back(std::move(opt_name));
            arguments.push_back(arg.substr(assign_char_pos + 1));
            continue;
          }
        }
        // If we've fallen through to here, then it's a standard argument
        arguments.push_back(arg);
      }
      return arguments;
    }

    /*
     * @throws std::runtime_error in case of any invalid argument
     */
    void ParseArgsInternal(const std::vector<std::string> &raw_arguments) {
      auto arguments = PreprocessArguments(raw_arguments);
      if (m_program_name.empty() && !arguments.empty()) {
        m_program_name = arguments.front();
      }
      auto end = std::end(arguments);
      auto positional_argument_it = std::begin(m_positional_arguments);
      for (auto it = std::next(std::begin(arguments)); it != end;) {
        const auto &current_argument = *it;
        if (Argument::IsPositional(current_argument, m_prefix_chars)) {
          if (positional_argument_it == std::end(m_positional_arguments)) {
            // Check sub-parsers
            auto subparser_it = m_subparser_map.find(current_argument);
            if (subparser_it != m_subparser_map.end()) {
              // build list of remaining args
              const auto unprocessed_arguments = std::vector<std::string>(it, end);

              // invoke subparser
              m_is_parsed = true;
              m_subparser_used[current_argument] = true;
              return subparser_it->second->get().ParseArgs(unprocessed_arguments);
            }

            if (m_positional_arguments.empty()) {
              // Ask the user if they argument they provided was a typo
              // for some sub-parser,
              // e.g., user provided `git totes` instead of `git notes`
              if (!m_subparser_map.empty()) {
                throw std::runtime_error("Failed to parse '" + current_argument + "', did you mean '" +
                                         std::string{details::GetMostSimilarString(m_subparser_map, current_argument)} +
                                         "'");
              }

              // Ask the user if they meant to use a specific optional argument
              if (!m_optional_arguments.empty()) {
                for (const auto &opt : m_optional_arguments) {
                  if (!opt.m_implicit_value.has_value()) {
                    // not a flag, requires a value
                    if (!opt.m_is_used) {
                      throw std::runtime_error("Zero positional arguments expected, did you mean " +
                                               opt.GetUsageFull());
                    }
                  }
                }

                throw std::runtime_error("Zero positional arguments expected");
              } else {
                throw std::runtime_error("Zero positional arguments expected");
              }
            } else {
              throw std::runtime_error(
                  "Maximum number of positional arguments "
                  "exceeded, failed to parse '" +
                  current_argument + "'");
            }
          }
          auto argument = positional_argument_it++;

          // Deal with the situation of <positional_arg1>... <positional_arg2>
          if (argument->m_num_args_range.GetMin() == 1 &&
              argument->m_num_args_range.GetMax() == (std::numeric_limits<std::size_t>::max)() &&
              positional_argument_it != std::end(m_positional_arguments) &&
              std::next(positional_argument_it) == std::end(m_positional_arguments) &&
              positional_argument_it->m_num_args_range.GetMin() == 1 &&
              positional_argument_it->m_num_args_range.GetMax() == 1) {
            if (std::next(it) != end) {
              positional_argument_it->Consume(std::prev(end), end);
              end = std::prev(end);
            } else {
              throw std::runtime_error("Missing " + positional_argument_it->m_names.front());
            }
          }

          it = argument->Consume(it, end);
          continue;
        }

        auto arg_map_it = m_argument_map.find(current_argument);
        if (arg_map_it != m_argument_map.end()) {
          auto argument = arg_map_it->second;
          it = argument->Consume(std::next(it), end, arg_map_it->first);
        } else if (const auto &compound_arg = current_argument; compound_arg.size() > 1 &&
                                                                IsValidPrefixChar(compound_arg[0]) &&
                                                                !IsValidPrefixChar(compound_arg[1])) {
          ++it;
          for (std::size_t j = 1; j < compound_arg.size(); j++) {
            auto hypothetical_arg = std::string{'-', compound_arg[j]};
            auto arg_map_it2 = m_argument_map.find(hypothetical_arg);
            if (arg_map_it2 != m_argument_map.end()) {
              auto argument = arg_map_it2->second;
              it = argument->Consume(it, end, arg_map_it2->first);
            } else {
              throw std::runtime_error("Unknown argument: " + current_argument);
            }
          }
        } else {
          throw std::runtime_error("Unknown argument: " + current_argument);
        }
      }
      m_is_parsed = true;
    }

    /*
     * Like parse_args_internal but collects unused args into a vector<string>
     */
    auto ParseKnownArgsInternal(const std::vector<std::string> &raw_arguments) -> std::vector<std::string> {
      auto arguments = PreprocessArguments(raw_arguments);

      std::vector<std::string> unknown_arguments{};

      if (m_program_name.empty() && !arguments.empty()) {
        m_program_name = arguments.front();
      }
      auto end = std::end(arguments);
      auto positional_argument_it = std::begin(m_positional_arguments);
      for (auto it = std::next(std::begin(arguments)); it != end;) {
        const auto &current_argument = *it;
        if (Argument::IsPositional(current_argument, m_prefix_chars)) {
          if (positional_argument_it == std::end(m_positional_arguments)) {
            // Check sub-parsers
            auto subparser_it = m_subparser_map.find(current_argument);
            if (subparser_it != m_subparser_map.end()) {
              // build list of remaining args
              const auto unprocessed_arguments = std::vector<std::string>(it, end);

              // invoke subparser
              m_is_parsed = true;
              m_subparser_used[current_argument] = true;
              return subparser_it->second->get().ParseKnownArgsInternal(unprocessed_arguments);
            }

            // save current argument as unknown and go to next argument
            unknown_arguments.push_back(current_argument);
            ++it;
          } else {
            // current argument is the value of a positional argument
            // consume it
            auto argument = positional_argument_it++;
            it = argument->Consume(it, end);
          }
          continue;
        }

        auto arg_map_it = m_argument_map.find(current_argument);
        if (arg_map_it != m_argument_map.end()) {
          auto argument = arg_map_it->second;
          it = argument->Consume(std::next(it), end, arg_map_it->first);
        } else if (const auto &compound_arg = current_argument; compound_arg.size() > 1 &&
                                                                IsValidPrefixChar(compound_arg[0]) &&
                                                                !IsValidPrefixChar(compound_arg[1])) {
          ++it;
          for (std::size_t j = 1; j < compound_arg.size(); j++) {
            auto hypothetical_arg = std::string{'-', compound_arg[j]};
            auto arg_map_it2 = m_argument_map.find(hypothetical_arg);
            if (arg_map_it2 != m_argument_map.end()) {
              auto argument = arg_map_it2->second;
              it = argument->Consume(it, end, arg_map_it2->first);
            } else {
              unknown_arguments.push_back(current_argument);
              break;
            }
          }
        } else {
          // current argument is an optional-like argument that is unknown
          // save it and move to next argument
          unknown_arguments.push_back(current_argument);
          ++it;
        }
      }
      m_is_parsed = true;
      return unknown_arguments;
    }

    // Used by print_help.
    [[nodiscard]] auto GetLengthOfLongestArgument() const -> std::size_t {
      if (m_argument_map.empty()) {
        return 0;
      }
      std::size_t max_size = 0;
      for ([[maybe_unused]] const auto &[unused, argument] : m_argument_map) {
        max_size = std::max<std::size_t>(max_size, argument->GetArgumentsLength());
      }
      for ([[maybe_unused]] const auto &[command, unused] : m_subparser_map) {
        max_size = std::max<std::size_t>(max_size, command.size());
      }
      return max_size;
    }

    using argument_it = std::list<Argument>::iterator;
    using mutex_group_it = std::vector<MutuallyExclusiveGroup>::iterator;
    using argument_parser_it = std::list<std::reference_wrapper<ArgumentParser>>::iterator;

    void IndexArgument(argument_it it) {
      for (const auto &name : std::as_const(it->m_names)) {
        m_argument_map.insert_or_assign(name, it);
      }
    }

    std::string m_program_name;
    std::string m_version;
    std::string m_description;
    std::string m_epilog;
    bool m_exit_on_default_arguments = true;
    std::string m_prefix_chars{"-"};
    std::string m_assign_chars{"="};
    bool m_is_parsed = false;
    std::list<Argument> m_positional_arguments;
    std::list<Argument> m_optional_arguments;
    std::map<std::string, argument_it> m_argument_map;
    std::string m_parser_path;
    std::list<std::reference_wrapper<ArgumentParser>> m_subparsers;
    std::map<std::string, argument_parser_it> m_subparser_map;
    std::map<std::string, bool> m_subparser_used;
    std::vector<MutuallyExclusiveGroup> m_mutually_exclusive_groups;
    bool m_suppress = false;
    std::size_t m_usage_max_line_width = (std::numeric_limits<std::size_t>::max)();
    bool m_usage_break_on_mutex = false;
    int m_usage_newline_counter = 0;
    std::vector<std::string> m_group_names;
  };

}  // namespace argparse