/*
* Copyright (c) 1998,1999 Tal Davidson. All rights reserved.
*
* ASResource.cpp   (10 September 1999)
* by Tal Davidson (davidsont@bigfoot.com)
* This file is a part of "Artistic Style" - an indentater and reformatter
* of C++, C, and Java source files.
*
* The "Artistic Style" project, including all files needed to compile it,
* is free software; you can redistribute it and/or use it and/or modify it
* under the terms of EITHER the "Artistic License" OR
* the GNU General Public License as published by the Free Software Foundation;
* either version 2 of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* You should have received a copy of EITHER the "Artistic License" or
* the GNU General Public License along with this program.
*/

#include "compiler_defines.h"
#include "astyle.h"

#include <string>


#ifdef USES_NAMESPACE
using namespace std;

namespace astyle
{
#endif

const string ASResource::IF = string("if");
const string ASResource::ELSE = string ("else");
const string ASResource::FOR = string("for");
const string ASResource::DO = string("do");
const string ASResource::WHILE = string("while");
const string ASResource::SWITCH = string ("switch");
const string ASResource::CASE = string ("case");
const string ASResource::DEFAULT = string("default");
const string ASResource::CLASS = string("class");
const string ASResource::STRUCT = string("struct");
const string ASResource::UNION = string("union");
const string ASResource::INTERFACE = string("interface");
const string ASResource::NAMESPACE = string("namespace");
const string ASResource::EXTERN = string("extern");
const string ASResource::PUBLIC = string("public");
const string ASResource::PROTECTED = string("protected");
const string ASResource::PRIVATE = string("private");
const string ASResource::STATIC = string("static");
const string ASResource::SYNCHRONIZED = string("synchronized");
const string ASResource::OPERATOR = string("operator");
const string ASResource::TEMPLATE = string("template");
const string ASResource::TRY = string("try");
const string ASResource::CATCH = string("catch");
const string ASResource::FINALLY = string("finally");
const string ASResource::THROWS = string("throws");
const string ASResource::CONST = string("const");

const string ASResource::BAR_DEFINE = string("#define");
const string ASResource::BAR_INCLUDE = string("#include");
const string ASResource::BAR_IF = string("#if");
const string ASResource::BAR_EL = string("#el");
const string ASResource::BAR_ENDIF = string("#endif");

const string ASResource::OPEN_BRACKET = string("{");
const string ASResource::CLOSE_BRACKET = string("}");
const string ASResource::OPEN_LINE_COMMENT = string("//");
const string ASResource::OPEN_COMMENT = string("/*");
const  string ASResource::CLOSE_COMMENT = string("*/");

const string ASResource::ASSIGN = string("=");
const string ASResource::PLUS_ASSIGN = string("+=");
const string ASResource::MINUS_ASSIGN = string("-=");
const string ASResource::MULT_ASSIGN = string("*=");
const string ASResource::DIV_ASSIGN = string("/=");
const string ASResource::MOD_ASSIGN = string("%=");
const string ASResource::OR_ASSIGN = string("|=");
const string ASResource::AND_ASSIGN = string("&=");
const string ASResource::XOR_ASSIGN = string("^=");
const string ASResource::GR_GR_ASSIGN = string(">>=");
const string ASResource::LS_LS_ASSIGN = string("<<=");
const string ASResource::GR_GR_GR_ASSIGN = string(">>>=");
const string ASResource::LS_LS_LS_ASSIGN = string("<<<=");
const string ASResource::RETURN = string("return");

const string ASResource::EQUAL = string("==");
const string ASResource::PLUS_PLUS = string("++");
const string ASResource::MINUS_MINUS = string("--");
const string ASResource::NOT_EQUAL = string("!=");
const string ASResource::GR_EQUAL = string(">=");
const string ASResource::GR_GR = string(">>");
const string ASResource::GR_GR_GR = string(">>>");
const string ASResource::LS_EQUAL = string("<=");
const string ASResource::LS_LS = string("<<");
const string ASResource::LS_LS_LS = string("<<<");
const string ASResource::ARROW = string("->");
const string ASResource::AND = string("&&");
const string ASResource::OR = string("||");
const string ASResource::COLON_COLON = string("::");
const string ASResource::PAREN_PAREN = string("()");
const string ASResource::BLPAREN_BLPAREN = string("[]");

const string ASResource::PLUS = string("+");
const string ASResource::MINUS = string("-");
const string ASResource::MULT = string("*");
const string ASResource::DIV = string("/");
const string ASResource::MOD = string("%");
const string ASResource::GR = string(">");
const string ASResource::LS = string("<");
const string ASResource::NOT = string("!");
const string ASResource::BIT_OR = string("|");
const string ASResource::BIT_AND = string("&");
const string ASResource::BIT_NOT = string("~");
const string ASResource::BIT_XOR = string("^");
const string ASResource::QUESTION = string("?");
const string ASResource::COLON = string(":");
const string ASResource::COMMA = string(",");
const string ASResource::SEMICOLON = string(";");



#ifdef USES_NAMESPACE
}
#endif


