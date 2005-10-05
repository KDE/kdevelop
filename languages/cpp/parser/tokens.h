/* This file is part of KDevelop
    Copyright (C) 2002-2005 Roberto Raggi <roberto@kdevelop.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef TOKENS_H
#define TOKENS_H

enum TOKEN_KIND
{
    Token_EOF = 0,

    Token_K_DCOP = 1000,
    Token_Q_OBJECT,
    Token___attribute__,
    Token___typeof,
    Token_and,
    Token_and_eq,
    Token_arrow,
    Token_asm,
    Token_assign,
    Token_auto,
    Token_bitand,
    Token_bitor,
    Token_bool,
    Token_break,
    Token_case,
    Token_catch,
    Token_char,
    Token_char_literal,
    Token_class,
    Token_comment,
    Token_compl,
    Token_concat,
    Token_const,
    Token_const_cast,
    Token_continue,
    Token_decr,
    Token_default,
    Token_delete,
    Token_do,
    Token_double,
    Token_dynamic_cast,
    Token_ellipsis,
    Token_else,
    Token_emit,
    Token_enum,
    Token_eq,
    Token_explicit,
    Token_export,
    Token_extern,
    Token_false,
    Token_float,
    Token_for,
    Token_friend,
    Token_geq,
    Token_goto,
    Token_identifier,
    Token_if,
    Token_incr,
    Token_inline,
    Token_int,
    Token_k_dcop,
    Token_k_dcop_signals,
    Token_leq,
    Token_long,
    Token_mutable,
    Token_namespace,
    Token_new,
    Token_not,
    Token_not_eq,
    Token_number_literal,
    Token_operator,
    Token_or,
    Token_or_eq,
    Token_preproc,
    Token_private,
    Token_protected,
    Token_ptrmem,
    Token_public,
    Token_register,
    Token_reinterpret_cast,
    Token_return,
    Token_scope,
    Token_shift,
    Token_short,
    Token_signals,
    Token_signed,
    Token_sizeof,
    Token_slots,
    Token_static,
    Token_static_cast,
    Token_string_literal,
    Token_struct,
    Token_switch,
    Token_template,
    Token_this,
    Token_throw,
    Token_true,
    Token_try,
    Token_typedef,
    Token_typeid,
    Token_typename,
    Token_union,
    Token_unsigned,
    Token_using,
    Token_virtual,
    Token_void,
    Token_volatile,
    Token_wchar_t,
    Token_while,
    Token_whitespaces,
    Token_xor,
    Token_xor_eq,

    TOKEN_KIND_COUNT
};

char const *token_name(int token);

#endif

// kate: space-indent on; indent-width 2; replace-tabs on;
