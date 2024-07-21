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

#ifndef __QUIXCC_CORE_QUIXJOB_H__
#define __QUIXCC_CORE_QUIXJOB_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <quixcc/Library.h>
#include <quixcc/llvm/LLVMWrapper.h>
#include <quixcc/preprocessor/Preprocessor.h>

#include <atomic>
#include <boost/unordered_map.hpp>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace libquixcc::quixcc {
  typedef struct quixcc_uuid_t {
    uint8_t data[16];
  } quixcc_uuid_t;

  typedef char *(*quixcc_macro_fn_t)(uint32_t argc, const char **argv);

  constexpr uint64_t JOB_MAGIC = 0x32b287410bbef790;

} // namespace libquixcc::quixcc

namespace libquixcc::quixcc {
  extern thread_local uint8_t g_target_word_size;
  extern std::atomic<uint64_t> g_num_of_contexts;
  extern std::mutex g_library_lock;
} // namespace libquixcc::quixcc

struct quixcc_cc_job_t {
  volatile uint64_t m_magic;
  libquixcc::LLVMContext m_inner;
  std::stack<std::string> m_filename;
  std::unordered_set<std::string> m_owned_strings;
  std::vector<std::pair<std::string, std::string>> m_argset;
  std::unordered_map<std::string, libquixcc::quixcc::quixcc_macro_fn_t> m_macros;
  std::set<std::unique_ptr<void, std::function<void(void *)>>> m_dlhandles;
  libquixcc::QSysCallRegistry m_qsyscalls;
  std::mutex m_lock;
  std::string m_triple;
  std::string m_cpu;
  libquixcc::quixcc::quixcc_uuid_t m_id;
  std::vector<std::string> m_options;
  quixcc_status_t m_result;
  std::unique_ptr<libquixcc::Scanner> m_scanner;
  std::optional<std::pair<uint32_t, uint32_t>> version;
  FILE *m_in;
  FILE *m_out;
  uint8_t m_priority;
  uint8_t m_wordsize;
  bool m_debug;
  bool m_tainted;
  bool m_running;

  quixcc_cc_job_t &operator=(quixcc_cc_job_t &&) = delete;
  quixcc_cc_job_t &operator=(const quixcc_cc_job_t &) = delete;
  quixcc_cc_job_t(const quixcc_cc_job_t &) = delete;

  quixcc_cc_job_t() {
    m_magic = libquixcc::quixcc::JOB_MAGIC;
    m_in = nullptr;
    m_out = nullptr;
    m_priority = 0;
    m_wordsize = 0;
    m_debug = false;
    m_tainted = false;
    m_running = false;
  }

  bool has(std::string_view name, std::string_view value = "") const {
    for (const auto &arg : m_argset) {
      if (arg.first == name && (value.empty() || arg.second == value))
        return true;
    }
    return false;
  }

  std::string_view get(std::string_view name) const {
    for (const auto &arg : m_argset) {
      if (arg.first == name)
        return arg.second;
    }
    return "";
  }
};

#endif // __QUIXCC_CORE_QUIXJOB_H__