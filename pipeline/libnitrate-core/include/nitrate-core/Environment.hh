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

#ifndef __NITRATE_CORE_ENV_H__
#define __NITRATE_CORE_ENV_H__

#include <mutex>
#include <nitrate-core/Allocate.hh>
#include <nitrate-core/String.hh>
#include <optional>
#include <unordered_map>

namespace ncc::core {
  class IEnvironment {
  public:
    virtual ~IEnvironment() = default;

    virtual bool contains(std::string_view key) = 0;

    virtual std::optional<std::string_view> get(std::string_view key) = 0;

    virtual void set(std::string_view key,
                     std::optional<std::string_view> value,
                     bool privset = false) = 0;
  };

  class Environment : public IEnvironment {
    std::unordered_map<std::string_view, std::string_view> m_data;
    std::mutex m_mutex;

    void setup_default_env();

  public:
    Environment();
    virtual ~Environment() = default;

    bool contains(std::string_view key);

    std::optional<std::string_view> get(std::string_view key);

    /* String interning is done internally */
    void set(std::string_view key, std::optional<std::string_view> value,
             bool privset = false);
  };

}  // namespace ncc::core

#endif  // __NITRATE_CORE_ENV_H__