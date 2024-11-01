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

#ifndef __QUIX_QXIR_REPORT_H__
#define __QUIX_QXIR_REPORT_H__

#include <quix-lexer/Token.h>
#include <quix-qxir/IR.h>

#include <cstdarg>
#include <functional>
#include <stdexcept>
#include <string_view>
#include <unordered_set>

namespace qparse {
  class Node;
}

namespace qxir::diag {
  class SyntaxError : public std::runtime_error {
  public:
    SyntaxError() : std::runtime_error("") {}
  };

  enum class IssueClass {
    Debug = 0,
    Info,
    Warn,
    Error,
    FatalError,
  };

  enum class IssueCode {
    Default = 0,

    SignalReceived,
    PTreeInvalid,
    DSPolyCyclicRef,
    DSNullPtr,
    DSBadType,
    DSMissingMod,

    Redefinition,
    UnknownFunction,
    TooManyArguments,
    UnknownArgument,

    UnknownType,
    UnresolvedIdentifier,
  };

  typedef std::function<void(std::string_view, IssueClass)> DiagnosticMessageHandler;

  struct DiagMessage {
    std::string m_msg;
    qlex_loc_t m_start, m_end;
    IssueClass m_type;
    IssueCode m_code;

    DiagMessage(std::string_view msg = "", IssueClass type = IssueClass::Debug,
                IssueCode code = IssueCode::Default, qlex_loc_t start = {0}, qlex_loc_t end = {0})
        : m_msg(msg), m_start(start), m_end(end), m_type(type), m_code(code) {}

    uint64_t hash() const;
  };

  class DiagnosticManager {
    struct Channel {
      std::vector<DiagMessage> vec;
      std::unordered_set<uint64_t> visited;
    };

    qmodule_t *m_qxir;
    std::unordered_map<qxir_audit_ticket_t, Channel> m_msgs;
    qxir_audit_ticket_t m_last_ticket;

    std::string mint_clang16_message(const DiagMessage &msg) const;
    std::string mint_plain_message(const DiagMessage &msg) const;
    std::string mint_modern_message(const DiagMessage &msg) const;
    size_t dump_diagnostic_vector(std::vector<DiagMessage> &vec, DiagnosticMessageHandler handler,
                                  qxir_diag_format_t style);

  public:
    DiagnosticManager() {
      m_qxir = nullptr;
      m_msgs[QXIR_AUDIT_CONV] = {};
      m_last_ticket = QXIR_AUDIT_CONV;
    }

    void push(qxir_audit_ticket_t ticket, DiagMessage &&msg);
    size_t render(qxir_audit_ticket_t ticket, DiagnosticMessageHandler handler,
                  qxir_diag_format_t style);

    void set_ctx(qmodule_t *qxir) { m_qxir = qxir; }

    size_t clear(qxir_audit_ticket_t t) {
      if (t == QXIR_AUDIT_ALL) {
        size_t n = 0;
        for (auto &[_, msgs] : m_msgs) {
          n += msgs.vec.size();
          msgs.vec.clear();
          msgs.visited.clear();
        }
        return n;
      } else if (t == QXIR_AUDIT_LAST) {
        size_t n = m_msgs[m_last_ticket].vec.size();
        m_msgs[m_last_ticket].vec.clear();
        m_msgs[m_last_ticket].visited.clear();
        return n;
      } else {
        if (!m_msgs.contains(t)) {
          return 0;
        }

        size_t n = m_msgs[t].vec.size();
        m_msgs[t].vec.clear();
        m_msgs[t].visited.clear();
        return n;
      }
    }

    size_t count(qxir_audit_ticket_t t) {
      if (t == QXIR_AUDIT_ALL) {
        size_t n = 0;
        for (const auto &[_, msgs] : m_msgs) {
          n += msgs.vec.size();
        }
        return n;
      } else if (t == QXIR_AUDIT_LAST) {
        return m_msgs[m_last_ticket].vec.size();
      } else {
        if (!m_msgs.contains(t)) {
          return 0;
        }

        return m_msgs[t].vec.size();
      }
    }
  };

  /**
   * @brief Report a syntax error
   */
  void badtree(const qparse::Node *node, std::string_view fmt, ...);

#define CONV_DEBUG(_msg)               \
  mod->getDiag().push(QXIR_AUDIT_CONV, \
                      diag::DiagMessage(_msg, diag::IssueClass::Debug, diag::IssueCode::Default));

#define DUPLICATE_VARIABLE(_varname)                                                 \
  mod->getDiag().push(                                                               \
      QXIR_AUDIT_CONV,                                                               \
      diag::DiagMessage("Variable named " + std::string(_varname) + " is redefined", \
                        diag::IssueClass::Error, diag::IssueCode::Redefinition,      \
                        cur->getLoc().first, cur->getLoc().second));

#define DUPLICATE_FUNCTION(_varname)                                                 \
  mod->getDiag().push(                                                               \
      QXIR_AUDIT_CONV,                                                               \
      diag::DiagMessage("Function named " + std::string(_varname) + " is redefined", \
                        diag::IssueClass::Error, diag::IssueCode::Redefinition,      \
                        cur->getLoc().first, cur->getLoc().second));

#define NO_MATCHING_FUNCTION(_funcname)                                                            \
  mod->getDiag().push(QXIR_AUDIT_CONV,                                                             \
                      diag::DiagMessage("No matching function named " + std::string(_funcname),    \
                                        diag::IssueClass::Error, diag::IssueCode::UnknownFunction, \
                                        cur->getLoc().first, cur->getLoc().second));

#define NO_MATCHING_PARAMETER(_funcname, _paramname)                                               \
  mod->getDiag().push(                                                                             \
      QXIR_AUDIT_CONV,                                                                             \
      diag::DiagMessage("Call to function " + std::string(_funcname) +                             \
                            " has no matching default parameter named " + std::string(_paramname), \
                        diag::IssueClass::Error, diag::IssueCode::UnknownArgument,                 \
                        cur->getLoc().first, cur->getLoc().second));

#define TOO_MANY_ARGUMENTS(_funcname)                                                        \
  mod->getDiag().push(                                                                       \
      QXIR_AUDIT_CONV,                                                                       \
      diag::DiagMessage("Too many arguments provided to function " + std::string(_funcname), \
                        diag::IssueClass::Error, diag::IssueCode::TooManyArguments,          \
                        cur->getLoc().first, cur->getLoc().second));

  void report(diag::IssueCode code, diag::IssueClass type = diag::IssueClass::Error,
              qlex_loc_t loc_start = {0}, qlex_loc_t loc_end = {0}, int channel = QXIR_AUDIT_CONV);

  void report(diag::IssueCode code, diag::IssueClass type = diag::IssueClass::Error,
              std::string_view subject = "", qlex_loc_t loc_start = {0}, qlex_loc_t loc_end = {0},
              int channel = QXIR_AUDIT_CONV);

};  // namespace qxir::diag

#endif  // __QUIX_QXIR_REPORT_H__
