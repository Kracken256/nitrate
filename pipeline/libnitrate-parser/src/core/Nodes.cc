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

#include <cstring>
#include <descent/Recurse.hh>
#include <nitrate-core/Init.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-parser/AST.hh>
#include <nitrate-parser/ASTData.hh>
#include <nitrate-parser/ASTWriter.hh>
#include <nitrate-parser/Algorithm.hh>
#include <sstream>

using namespace ncc;
using namespace ncc::parse;

NCC_EXPORT thread_local std::unique_ptr<ncc::IMemory> parse::MainAllocator = std::make_unique<ncc::DynamicArena>();

NCC_EXPORT ASTExtension parse::ExtensionDataStore;

auto ASTExtension::Add(lex::LocationID begin, lex::LocationID end) -> ASTExtensionKey {
  bool sync = EnableSync;

  if (sync) {
    m_mutex.lock();
  }

  m_pairs.push_back({begin, end});

  auto r = ASTExtensionKey(m_pairs.size() - 1);

  if (sync) {
    m_mutex.unlock();
  }

  return r;
}

auto ASTExtension::Get(ASTExtensionKey loc) -> const ASTExtensionPackage & {
  bool sync = EnableSync;

  if (sync) {
    m_mutex.lock();
  }

  const auto &r = m_pairs.at(loc.Key());

  if (sync) {
    m_mutex.unlock();
  }

  return r;
}

void ASTExtension::Set(ASTExtensionKey id, ASTExtensionPackage &&data) {
  bool sync = EnableSync;

  if (sync) {
    m_mutex.lock();
  }

  m_pairs.at(id.Key()) = std::move(data);

  if (sync) {
    m_mutex.unlock();
  }
}

NCC_EXPORT auto parse::operator<<(std::ostream &os, const ASTExtensionKey &idx) -> std::ostream & {
  os << "${L:" << idx.Key() << "}";
  return os;
}

///=============================================================================

std::string Base::DebugString(WriterSourceProvider rd) const {
  std::stringstream ss;
  AstWriter writer(ss, rd, true);
  this->Accept(writer);

  return ss.str();
}

void Base::DebugString(std::ostream &os, WriterSourceProvider rd) const {
  AstWriter writer(os, rd, true);
  this->Accept(writer);
}

void Base::Serialize(std::ostream &os) const {
  AstWriter writer(os);
  this->Accept(writer);
}

std::string Base::Serialize() const {
  std::stringstream ss;
  AstWriter writer(ss);
  this->Accept(writer);

  return ss.str();
}

auto Base::IsEq(FlowPtr<Base> o) const -> bool {
  if (this == o.get()) {
    return true;
  }

  if (GetKind() != o->GetKind()) {
    return false;
  }

  std::stringstream ss1;
  std::stringstream ss2;
  AstWriter writer1(ss1);
  AstWriter writer2(ss2);

  this->Accept(writer1);
  o.Accept(writer2);

  return ss1.str() == ss2.str();
}

auto Base::Hash64() const -> uint64_t {
  std::stringstream ss;
  AstWriter writer(ss);
  this->Accept(writer);

  return std::hash<std::string>{}(ss.str());
}

auto Base::RecursiveChildCount() -> size_t {
  size_t count = 0;

  for_each(this, [&](auto, auto) { count++; });

  return count;
}

void Base::SetComments(std::span<const string> comments) {
  auto old = ExtensionDataStore.Get(m_data);
  old.AddComments(comments);
  ExtensionDataStore.Set(m_data, std::move(old));
}

///=============================================================================

NCC_EXPORT auto Type::IsPtrTo(const Type *type) const -> bool {
  if (!IsPointer()) {
    return false;
  }

  auto item = As<PtrTy>()->GetItem();
  while (item->Is<RefTy>()) {
    item = item->As<RefTy>()->GetItem();
  }

  return item->Is(type->GetKind());
}

auto Parser::PImpl::MockStmt(std::optional<ASTNodeKind>) -> FlowPtr<Stmt> {
  auto node = CreateNode<Stmt>(QAST_BASE)();
  node->SetOffset(m_rd.Current().GetStart());

  return node;
}

auto Parser::PImpl::MockExpr(std::optional<ASTNodeKind>) -> FlowPtr<Expr> {
  auto node = CreateNode<Expr>(QAST_BASE)();
  node->SetOffset(m_rd.Current().GetStart());

  return node;
}

auto Parser::PImpl::MockType() -> FlowPtr<Type> {
  auto node = CreateNode<Type>(QAST_BASE)();
  node->SetOffset(m_rd.Current().GetStart());

  return node;
}
