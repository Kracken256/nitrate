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

#include <algorithm>
#include <fstream>
#include <nitrate-core/Assert.hh>
#include <nitrate-core/Logger.hh>

using namespace ncc;

NCC_EXPORT thread_local LoggerContext ncc::Log;

NCC_EXPORT auto ncc::Formatter(std::string_view msg, Sev sev) -> std::string {
  std::array<std::string_view, Sev_MaxValue + 1> ansi_prefixes = {
      "\x1b[1mtrace:\x1b[0m ",         "\x1b[1mdebug:\x1b[0m ",      "\x1b[37;1minfo:\x1b[0m ",
      "\x1b[37;1mnotice:\x1b[0m ",     "\x1b[35;1mwarning:\x1b[0m ", "\x1b[31;1merror:\x1b[0m ",
      "\x1b[31;1;4mcritical:\x1b[0m ", "\x1b[31;1;4malert:\x1b[0m ", "\x1b[31;1;4memergency:\x1b[0m "};

  std::stringstream ss;
  ss << ansi_prefixes[sev] << "\x1b[37;1m" << msg << "\x1b[0m";

  return ss.str();
}

NCC_EXPORT ECUnique::ECUnique(std::source_location loc) {
  std::stringstream ss;
  ss << loc.file_name() << ":" << loc.line() << ":" << loc.column();
  m_ec = std::hash<std::string>{}(ss.str());
}

void ECBase::GetJsonRepresentation(std::ostream &os) const {
  os << R"({"flagname":")" << FlagName() << R"(","nice_name":")" << NiceName() << R"(","details":")" << Details()
     << R"(","tags":[)";
  for (auto it = Tags().begin(); it != Tags().end(); ++it) {
    os << "\"" << *it << "\"";
    if (it + 1 != Tags().end()) {
      os << ",";
    }
  }
  os << "],\"fixes\":[";
  for (auto it = Fixes().begin(); it != Fixes().end(); ++it) {
    os << "\"" << *it << "\"";
    if (it + 1 != Fixes().end()) {
      os << ",";
    }
  }
  os << "],\"examples\":[";
  for (auto it = Examples().begin(); it != Examples().end(); ++it) {
    os << "\"" << *it << "\"";
    if (it + 1 != Examples().end()) {
      os << ",";
    }
  }
  os << "],\"dev_notes\":[";
  for (auto it = DevNotes().begin(); it != DevNotes().end(); ++it) {
    os << "\"" << *it << "\"";
    if (it + 1 != DevNotes().end()) {
      os << ",";
    }
  }
  os << "],\"user_notes\":[";
  for (auto it = UserNotes().begin(); it != UserNotes().end(); ++it) {
    os << "\"" << *it << "\"";
    if (it + 1 != UserNotes().end()) {
      os << ",";
    }
  }
  os << "]}";
}

static std::optional<std::string> GetRealPath(std::string_view in_path) {
  std::string path((in_path));

  if (auto index = path.find("$NCC_CONF"); index != std::string::npos) {
    const char *ncc_conf = std::getenv("NCC_CONF");  // NOLINT(concurrency-mt-unsafe)
    if (ncc_conf == nullptr) {
      return std::nullopt;
    }

    path.replace(index, 9, ncc_conf);
  }

  return path;
}

void ECBase::Finalize() {
  m_ec = GetIdentity().Get();

  /* Try to load information about the error from disk */
  if (auto prepath_opt = GetDetailsPath()) {
    if (auto path_opt = GetRealPath(*prepath_opt)) {
      if (std::ifstream ifs(*path_opt); ifs.is_open()) {
        std::string json((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

        if (auto details_opt = ParseJsonECConfig(json)) {
          m_details = std::move(*details_opt);
        }
      }
    }
  }

  /* Generate JSON representation */
  std::ostringstream oss;
  GetJsonRepresentation(oss);
  m_json = oss.str();
}

auto LoggerContext::Subscribe(LogCallback cb) -> size_t {
  m_subscribers.push_back(std::move(cb));
  return m_subscribers.size() - 1;
}

void LoggerContext::Unsubscribe(size_t idx) {
  if (idx < m_subscribers.size()) {
    m_subscribers.erase(m_subscribers.begin() + idx);
  }
}

void LoggerContext::UnsubscribeAll() { m_subscribers.clear(); }

auto LoggerContext::AddFilter(LogFilterFunc filter) -> size_t {
  m_filters.push_back(filter);
  return m_filters.size() - 1;
}

void LoggerContext::RemoveFilter(size_t idx) {
  if (idx < m_filters.size()) {
    m_filters.erase(m_filters.begin() + idx);
  }
}

void LoggerContext::ClearFilters() { m_filters.clear(); }

void LoggerContext::Publish(const std::string &msg, Sev sev, const ECBase &ec) const {
  if (m_enabled) {
    bool emit = m_filters.empty() ||
                std::all_of(m_filters.begin(), m_filters.end(), [&](const auto &f) { return f(msg, sev, ec); });

    if (emit) {
      for (const auto &sub : m_subscribers) {
        sub(msg, sev, ec);
      }
    }
  }
}
