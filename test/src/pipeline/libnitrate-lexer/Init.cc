#include <gtest/gtest.h>

#include <nitrate-lexer/Init.hh>

using namespace ncc;
using namespace ncc::lex;

TEST(Lexer, Init_Init) {
  EXPECT_FALSE(CoreLibrary.IsInitialized());
  EXPECT_FALSE(LexerLibrary.IsInitialized());

  if (auto lib_rc = LexerLibrary.GetRC()) {
    EXPECT_TRUE(CoreLibrary.IsInitialized());
    EXPECT_TRUE(LexerLibrary.IsInitialized());
  }

  EXPECT_FALSE(CoreLibrary.IsInitialized());
  EXPECT_FALSE(LexerLibrary.IsInitialized());
}

TEST(Lexer, Init_Version) { EXPECT_FALSE(LexerLibrary.GetVersion().empty()); }
