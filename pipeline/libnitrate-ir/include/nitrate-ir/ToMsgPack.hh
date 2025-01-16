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

#ifndef __NITRATE_IR_ENCODE_TOMSGPACK_H__
#define __NITRATE_IR_ENCODE_TOMSGPACK_H__

#include <nitrate-ir/IRWriter.hh>
#include <ostream>

namespace ncc::ir {
  class NCC_EXPORT IRMsgPackWriter : public IRWriter {
    std::ostream& m_os;

    void StrImpl(std::string_view str);
    void UintImpl(uint64_t val);
    void DoubleImpl(double val);
    void BoolImpl(bool val);
    void NullImpl();
    void BeginObjImpl(size_t pair_count);
    void EndObjImpl();
    void BeginArrImpl(size_t size);
    void EndArrImpl();

  public:
    IRMsgPackWriter(std::ostream& os, WriterSourceProvider rd = std::nullopt)
        : IRWriter(
              std::bind(&IRMsgPackWriter::StrImpl, this, std::placeholders::_1),
              std::bind(&IRMsgPackWriter::UintImpl, this,
                        std::placeholders::_1),
              std::bind(&IRMsgPackWriter::DoubleImpl, this,
                        std::placeholders::_1),
              std::bind(&IRMsgPackWriter::BoolImpl, this,
                        std::placeholders::_1),
              std::bind(&IRMsgPackWriter::NullImpl, this),
              std::bind(&IRMsgPackWriter::BeginObjImpl, this,
                        std::placeholders::_1),
              std::bind(&IRMsgPackWriter::EndObjImpl, this),
              std::bind(&IRMsgPackWriter::BeginArrImpl, this,
                        std::placeholders::_1),
              std::bind(&IRMsgPackWriter::EndArrImpl, this), rd),
          m_os(os) {}
    virtual ~IRMsgPackWriter() = default;
  };
}  // namespace ncc::ir

#endif