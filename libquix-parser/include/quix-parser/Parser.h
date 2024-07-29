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

#ifndef __QUIX_PARSER_PARSER_H__
#define __QUIX_PARSER_PARSER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <quix-core/Arena.h>
#include <quix-lexer/Lexer.h>
#include <quix-parser/Config.h>
#include <quix-parser/Node.h>
#include <stdbool.h>

struct qparse_impl_t;
typedef struct qparse_impl_t qparse_impl_t;

typedef struct qparse_t {
  qparse_impl_t *impl; /* Parser implementation struct */
  qlex_t *lexer;       /* Polymporphic lexer */
  qparse_conf_t *conf; /* Parser configuration */
} qparse_t;

/**
 * @brief Create a new parser instance from non-owning references to a lexer and
 * parser configuration.
 *
 * @param lexer The lexer to use for parsing.
 * @param conf The parser configuration to use for parsing.
 *
 * @return A new parser instance.
 *
 * @note If `!lexer` or `!conf`, NULL is returned.
 *
 * @note The parser instance is owned by the caller and must be freed with
 * `qparse_free`.
 *
 * @note The lexer and configuration are not owned by the parser and must be
 * freed separately after the parser is freed.
 *
 * @note The parser instance isn't thread safe internally, but the functions
 * provided to operate on the parser instance are thread safe themselves. This means that there is
 * no internal locking in the parser context.
 *
 * @note This function is thread safe.
 */
qparse_t *qparse_new(qlex_t *lexer, qparse_conf_t *conf);

/**
 * @brief Free a parser instance.
 *
 * @param parser The parser instance to free (may be NULL).
 *
 * @note If `!parser`, this function is a no-op.
 *
 * @note This function will not free the lexer or configuration associated with
 * the it. The caller is responsible for freeing the lexer and configuration
 * separately.
 * @note This function is thread safe.
 */
void qparse_free(qparse_t *parser);

/**
 * @brief Serialize a parse tree to a string.
 *
 * @param _node The root node of the parse tree.
 * @param minify Whether to minify the output.
 * @param indent The number of spaces to indent the output.
 * @param arena The arena to allocate memory from.
 * @param outlen The length of the output string.
 *
 * @return The serialized parse tree as a string.
 *
 * @note This function is thread safe.
 */
char *qparse_ast_repr(const qparse_node_t *_node, bool minify, size_t indent, qcore_arena_t *arena,
                      size_t *outlen);

/**
 * @brief Serialize a parse tree to a binary representation.
 *
 * @param node The root node of the parse tree.
 * @param compress Whether to compress the output.
 * @param arena The arena to allocate memory from.
 * @param out The output buffer.
 * @param outlen The length of the output buffer.
 *
 * @note This function is thread safe.
 */
void qparse_ast_brepr(const qparse_node_t *node, bool compress, qcore_arena_t *arena, uint8_t **out,
                      size_t *outlen);

/**
 * @brief Parse QUIX code into a parse tree.
 *
 * @param parser The parser instance to use for parsing.
 * @param arena The arena to allocate memory from.
 * @param out The output parse tree.
 *
 * @return Returns true if no non-fatal parsing errors occurred, false otherwise. A value of true,
 * however, does not guarantee that the parse tree is valid.
 *
 * @note If `!parser`, `!arena`, or `!out`, false is returned.
 *
 * @note The `out` node is allocated from the arena and is therefore
 * bound to the arena's lifetime.
 *
 * @note This function is thread safe.
 */
bool qparse_do(qparse_t *parser, qcore_arena_t *arena, qparse_node_t **out);

/**
 * @brief Check if the parse tree is valid.
 *
 * @param parser The parser instance to use for parsing.
 * @param base The base node of the parse tree to check.
 *
 * @return Returns true if the parse tree is valid, false otherwise. A value of true, however, does
 * not indicate that the parse tree is free of semantic errors. This just ensures it doesn't contain
 * NULL nodes and other illegal constructs (basic checks).
 *
 * @note If `!parser`, `!base` false is returned.
 *
 * @note This function is thread safe.
 */
bool qparse_check(qparse_t *parser, const qparse_node_t *base);

/**
 * @brief A callback function to facilitate the communication reports generated
 * by the parser.
 *
 * @param msg The message to report.
 * @param len The length of the message.
 * @param data The user data to pass to the callback.
 *
 * @note This function is thread safe.
 */
typedef void (*qparse_dump_cb)(const char *msg, size_t len, uintptr_t data);

/**
 * @brief Dump the parser's reports to a callback function.
 *
 * @param parser The parser instance to dump reports from.
 * @param cb The callback function to pass reports to.
 * @param data An arbitrary pointer to pass to every callback function.
 *
 * @note If `!parser` or `!cb`, this function is a no-op.
 *
 * @note This function is thread safe.
 */
void qparse_dumps(qparse_t *parser, bool no_ansi, qparse_dump_cb cb, uintptr_t data);

#ifdef __cplusplus
}
#endif

#endif  // __QUIX_PARSER_PARSER_H__