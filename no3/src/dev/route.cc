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

#include <argparse.h>
#include <glog/logging.h>
#include <lsp/nitrated.h>
#include <nitrate-core/Error.h>
#include <nitrate-core/Lib.h>
#include <nitrate-emit/Code.h>
#include <nitrate-emit/Lib.h>
#include <nitrate-ir/Lib.h>
#include <nitrate-lexer/Lib.h>
#include <nitrate-parser/Lib.h>
#include <nitrate-seq/Lib.h>

#include <clean/Cleanup.hh>
#include <core/ANSI.hh>
#include <core/Config.hh>
#include <core/Logger.hh>
#include <fstream>
#include <iostream>
#include <nitrate-core/Classes.hh>
#include <nitrate-emit/Classes.hh>
#include <nitrate-ir/Classes.hh>
#include <nitrate-parser/Classes.hh>
#include <nitrate-seq/Classes.hh>
#include <string_view>
#include <unordered_map>

#include "nitrate-ir/IR.h"
#include "nitrate-parser/Parser.h"

using namespace argparse;
using namespace no3;

namespace no3::benchmark {

  enum class Benchmark {
    LEXER,
    PARSER,
    Q_IR,
    DELTA_IR,
    LLVM_IR,
    LLVM_CODEGEN,
    C11_CODEGEN,
    PIPELINE
  };

  static int do_benchmark(Benchmark bench_type) {
    int R = -1;

    switch (bench_type) {
      case Benchmark::LEXER: {
        /// TODO: Implement benchmark
        break;
      }

      case Benchmark::PARSER: {
        /// TODO: Implement benchmark
        break;
      }

      case Benchmark::Q_IR: {
        /// TODO: Implement benchmark
        break;
      }

      case Benchmark::DELTA_IR: {
        /// TODO: Implement benchmark
        break;
      }

      case Benchmark::LLVM_IR: {
        /// TODO: Implement benchmark
        break;
      }

      case Benchmark::LLVM_CODEGEN: {
        /// TODO: Implement benchmark
        break;
      }

      case Benchmark::C11_CODEGEN: {
        /// TODO: Implement benchmark
        break;
      }

      case Benchmark::PIPELINE: {
        /// TODO: Implement benchmark
        break;
      }
    }

    return R;
  }
}  // namespace no3::benchmark

static int do_parse(std::string source, std::string output) {
  qcore_env env;

  auto file = std::make_shared<std::fstream>(source, std::ios::in);
  if (!file->is_open()) {
    LOG(ERROR) << "Failed to open source file: " << source;
    return 1;
  }

  qprep lexer(file, "in", env.get());
  qparser parser(lexer.get(), env.get());

  qparse_node_t *tree = nullptr;

  bool ok = qparse_do(parser.get(), &tree);

  qparse_dumps(
      parser.get(), !ansi::IsUsingColors(),
      [](const char *msg, size_t len, uintptr_t) { std::cerr.write(msg, len); },
      0);

  if (!ok) {
    LOG(ERROR) << "Failed to parse source file: " << source;
    return 1;
  }

  { /* Write output */
    std::ostream *out = nullptr;
    std::shared_ptr<std::ostream> out_ptr;

    if (output.empty()) {
      out = &std::cout;
    } else {
      out_ptr = std::make_shared<std::ofstream>(output);
      out = out_ptr.get();
    }

    size_t len = 0;
    char *serialized = qparse_repr(tree, false, 2, &len);
    std::string repr(serialized, len);
    free(serialized);

    *out << repr << std::endl;
  }

  return 0;
}

static int do_nr(std::string source, std::string output, std::string opts,
                 bool verbose) {
  if (!opts.empty()) {
    LOG(ERROR) << "Options are not implemented yet";
  }

  qcore_env env;

  auto file = std::make_shared<std::fstream>(source, std::ios::in);
  if (!file->is_open()) {
    LOG(ERROR) << "Failed to open source file: " << source;
    return 1;
  }

  qprep lexer(file, "in", env.get());
  qparser parser(lexer.get(), env.get());

  qparse_node_t *tree = nullptr;

  bool ok = qparse_do(parser.get(), &tree);

  qparse_dumps(
      parser.get(), !ansi::IsUsingColors(),
      [](const char *msg, size_t len, uintptr_t) { std::cerr.write(msg, len); },
      0);

  if (!ok) {
    LOG(ERROR) << "Failed to parse source file: " << source;
    return 1;
  }

  qmodule mod;
  ok = nr_lower(&mod.get(), tree, "module", true);

  nr_diag_read(
      mod.get(), ansi::IsUsingColors() ? NR_DIAG_COLOR : NR_DIAG_NOCOLOR,
      [](const uint8_t *msg, size_t len, nr_level_t lvl, uintptr_t verbose) {
        if (verbose || lvl != NR_LEVEL_DEBUG) {
          std::cerr.write((const char *)msg, len);
        }
      },
      verbose);

  if (!ok) {
    LOG(ERROR) << "Failed to lower source file: " << source;
    return 1;
  }

  { /* Write output */
    FILE *out = output.empty() ? stdout : fopen(output.c_str(), "wb");

    if (!out) {
      LOG(ERROR) << "Failed to open output file: " << output;
      return 1;
    }

    ok = nr_write(mod.get(), nullptr, NR_SERIAL_CODE, out, nullptr, 0);
    if (out != stdout) {
      fclose(out);
    }

    if (!ok) {
      LOG(ERROR) << "Failed to write output file: " << output;
      return 1;
    }
  }

  return 0;
}

static int do_codegen(std::string source, std::string output, std::string opts,
                      std::string target, bool verbose) {
  if (!opts.empty()) {
    LOG(ERROR) << "Options are not implemented yet";
  }

  qcore_env env;

  auto file = std::make_shared<std::fstream>(source, std::ios::in);
  if (!file->is_open()) {
    LOG(ERROR) << "Failed to open source file: " << source;
    return 1;
  }

  qprep lexer(file, "in", env.get());
  qparser parser(lexer.get(), env.get());

  qparse_node_t *tree = nullptr;

  bool ok = qparse_do(parser.get(), &tree);

  qparse_dumps(
      parser.get(), !ansi::IsUsingColors(),
      [](const char *msg, size_t len, uintptr_t) { std::cerr.write(msg, len); },
      0);

  if (!ok) {
    LOG(ERROR) << "Failed to parse source file: " << source;
    return 1;
  }

  qmodule mod;
  ok = nr_lower(&mod.get(), tree, "module", true);

  nr_diag_read(
      mod.get(), ansi::IsUsingColors() ? NR_DIAG_COLOR : NR_DIAG_NOCOLOR,
      [](const uint8_t *msg, size_t len, nr_level_t lvl, uintptr_t verbose) {
        if (!verbose && lvl == NR_LEVEL_DEBUG) {
          return;
        }

        std::cerr.write((const char *)msg, len);
      },
      verbose);

  if (!ok) {
    LOG(ERROR) << "Failed to lower source file: " << source;
    return 1;
  }

  FILE *out = output.empty() ? stdout : fopen(output.c_str(), "wb");

  qcode_conf codegen_conf;
  ok = qcode_ir(mod.get(), codegen_conf.get(), stderr, out);

  if (out != stdout) {
    fclose(out);
  }

  if (!ok) {
    LOG(ERROR) << "Failed to generate code for source file: " << source;
    return 1;
  }

  return 0;
}

static int do_dev_test() {
  /// TODO: Implement testing
  LOG(ERROR) << "The integrated test suite is not implemented yet";
  return 1;
}

namespace no3::router {
  int run_dev_mode(
      const ArgumentParser &parser,
      const std::unordered_map<std::string_view,
                               std::unique_ptr<ArgumentParser>> &subparsers) {
    if (parser.is_subcommand_used("bench")) {
      using namespace no3::benchmark;

      auto &bench_parser = *subparsers.at("bench");
      core::SetDebugMode(bench_parser["--verbose"] == true);

      if (bench_parser["--list"] == true) {
        std::cout << "Available benchmarks:" << std::endl;
        std::cout << "  lexer" << std::endl;
        std::cout << "  parser" << std::endl;
        std::cout << "  nitrate-ir" << std::endl;
        std::cout << "  delta-ir" << std::endl;
        std::cout << "  llvm-ir" << std::endl;
        std::cout << "  llvm-codegen" << std::endl;
        std::cout << "  c11-codegen" << std::endl;
        std::cout << "  pipeline" << std::endl;
        return 0;
      }

      if (!bench_parser.is_used("--name")) {
        LOG(ERROR) << "No benchmark specified" << std::endl;
        LOG(ERROR) << bench_parser;
        return 1;
      }

      std::string bench_name = bench_parser.get<std::string>("--name");

      static const std::unordered_map<std::string, Benchmark> name_map = {
          {"lexer", Benchmark::LEXER},
          {"parser", Benchmark::PARSER},
          {"nitrate-ir", Benchmark::Q_IR},
          {"delta-ir", Benchmark::DELTA_IR},
          {"llvm-ir", Benchmark::LLVM_IR},
          {"llvm-codegen", Benchmark::LLVM_CODEGEN},
          {"c11-codegen", Benchmark::C11_CODEGEN},
          {"pipeline", Benchmark::PIPELINE}};

      if (!name_map.contains(bench_name)) {
        LOG(ERROR) << "Unknown benchmark specified" << std::endl;
        LOG(ERROR) << bench_parser;
        return 1;
      }

      return do_benchmark(name_map.at(bench_name));
    } else if (parser.is_subcommand_used("test")) {
      auto &test_parser = *subparsers.at("test");
      core::SetDebugMode(test_parser["--verbose"] == true);

      return do_dev_test();
    } else if (parser.is_subcommand_used("parse")) {
      auto &parse_parser = *subparsers.at("parse");
      core::SetDebugMode(parse_parser["--verbose"] == true);

      std::string source = parse_parser.get<std::string>("source");
      std::string output = parse_parser.get<std::string>("--output");

      return do_parse(source, output);
    } else if (parser.is_subcommand_used("nr")) {
      auto &nr_parser = *subparsers.at("nr");

      core::SetDebugMode(nr_parser["--verbose"] == true);

      std::string source = nr_parser.get<std::string>("source");
      std::string output = nr_parser.get<std::string>("--output");
      std::string opts = nr_parser.get<std::string>("--opts");

      return do_nr(source, output, opts, nr_parser["--verbose"] == true);
    } else if (parser.is_subcommand_used("codegen")) {
      auto &nr_parser = *subparsers.at("codegen");

      core::SetDebugMode(nr_parser["--verbose"] == true);

      std::string source = nr_parser.get<std::string>("source");
      std::string output = nr_parser.get<std::string>("--output");
      std::string opts = nr_parser.get<std::string>("--opts");
      std::string target = nr_parser.get<std::string>("--target");

      return do_codegen(source, output, opts, target,
                        nr_parser["--verbose"] == true);
    } else if (parser.is_used("--demangle")) {
      std::string mangled_name = parser.get<std::string>("--demangle");
      if (mangled_name.starts_with("@")) {
        mangled_name.erase(0);
      }

      nr::SymbolEncoding codec;
      auto demangled_name = codec.demangle_name(mangled_name);
      if (!demangled_name) {
        LOG(ERROR) << "Failed to demangle symbol" << std::endl;
        return 1;
      }

      std::cout << demangled_name.value() << std::endl;
      return 0;
    } else {
      LOG(ERROR) << "Unknown subcommand for dev" << std::endl;
      LOG(ERROR) << parser;

      return 1;
    }
  }
}  // namespace no3::router