/*
    SPDX-FileCopyrightText: 2004 Roberto Raggi <roberto@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TOKENS_H
#define TOKENS_H

enum Type
{
    Token_eof = 0,
    Token_identifier = 1000,
    Token_number_literal,
    Token_string_literal,
    Token_whitespaces
};

#endif

