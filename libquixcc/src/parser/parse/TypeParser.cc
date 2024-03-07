////////////////////////////////////////////////////////////////////////////////////
///                                                                              ///
///    ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░    ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░   ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░          ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░          ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░          ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░   ///
///    ░▒▓██████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░    ///
///      ░▒▓█▓▒░                                                                 ///
///       ░▒▓██▓▒░                                                               ///
///                                                                              ///
///     * QUIX LANG COMPILER - The official compiler for the Quix language.      ///
///     * Copyright (c) 2024, Wesley C. Jones. All rights reserved.              ///
///     * License terms may be found in the LICENSE file.                        ///
///                                                                              ///
////////////////////////////////////////////////////////////////////////////////////

#define QUIXCC_INTERNAL

#include <parse/Parser.h>
#include <LibMacro.h>
#include <error/Message.h>

using namespace libquixcc;

static std::map<std::string, std::shared_ptr<TypeNode>> primitive_types = {
    {"u8", U8TypeNode::create()},
    {"u16", U16TypeNode::create()},
    {"u32", U32TypeNode::create()},
    {"u64", U64TypeNode::create()},
    {"i8", I8TypeNode::create()},
    {"i16", I16TypeNode::create()},
    {"i32", I32TypeNode::create()},
    {"i64", I64TypeNode::create()},
    {"f32", F32TypeNode::create()},
    {"f64", F64TypeNode::create()},
    {"bool", BoolTypeNode::create()},
    {"char", CharTypeNode::create()},
    {"string", StringTypeNode::create()},
    {"void", VoidTypeNode::create()}};

bool libquixcc::parse_type(quixcc_job_t &job, std::shared_ptr<libquixcc::Scanner> scanner, std::shared_ptr<libquixcc::TypeNode> &node)
{
    Token tok = scanner->next();
    if (tok.type() == TokenType::Keyword)
    {
        switch (std::get<Keyword>(tok.val()))
        {
        case Keyword::Void:
            node = VoidTypeNode::create();
            return true;

        default:
            return false;
        }
    }
    else if (tok.type() == TokenType::Identifier)
    {
        if (primitive_types.contains(std::get<std::string>(tok.val())))
        {
            node = primitive_types[std::get<std::string>(tok.val())];
            return true;
        }
        else
        {
            node = std::make_shared<UserTypeNode>(std::get<std::string>(tok.val()));
            return true;
        }
    }
    else if (tok.type() == TokenType::Punctor && std::get<Punctor>(tok.val()) == Punctor::OpenBracket)
    {
        // Array type
        // syntax [type; size]
        std::shared_ptr<TypeNode> type;
        if (!parse_type(job, scanner, type))
        {
            PARMSG(tok, libquixcc::Err::ERROR, feedback[TYPE_EXPECTED_TYPE]);
            return false;
        }

        tok = scanner->next();
        if (tok.type() != TokenType::Punctor || std::get<Punctor>(tok.val()) != Punctor::Semicolon)
        {
            PARMSG(tok, libquixcc::Err::ERROR, feedback[TYPE_EXPECTED_SEMICOLON]);
            return false;
        }

        std::shared_ptr<ConstExprNode> size;
        if (!parse_const_expr(job, scanner, Token(TokenType::Punctor, Punctor::CloseBracket), size))
        {
            PARMSG(tok, libquixcc::Err::ERROR, feedback[TYPE_EXPECTED_CONST_EXPR]);
            return false;
        }

        tok = scanner->next();
        if (tok.type() != TokenType::Punctor || std::get<Punctor>(tok.val()) != Punctor::CloseBracket)
        {
            PARMSG(tok, libquixcc::Err::ERROR, feedback[TYPE_EXPECTED_CLOSE_BRACKET]);
            return false;
        }

        node = std::make_shared<ArrayTypeNode>(type, size);
        return true;
    }
    else
    {
        PARMSG(tok, libquixcc::Err::ERROR, feedback[TYPE_EXPECTED_IDENTIFIER_OR_BRACKET]);
        return false;
    }
}
