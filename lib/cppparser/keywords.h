// 
// Keywords file is included in lookup.cpp
// [erbsland] replacement for old hash table
// 
#define INSERT( x, y ) keywords.insert( std::pair<HashedString, Type>( x, y ) )
// KDE Keywords
INSERT( "K_DCOP", Token_K_DCOP );
INSERT( "k_dcop", Token_k_dcop );
INSERT( "k_dcop_signals", Token_k_dcop_signals );

// Qt Keywords
INSERT( "Q_OBJECT", Token_Q_OBJECT );
INSERT( "signals", Token_signals );
INSERT( "slots", Token_slots );
INSERT( "emit", Token_emit );
INSERT( "foreach", Token_foreach );

// C++ Keywords
INSERT( "__int64", Token_int );
INSERT( "__asm__", Token_asm );
INSERT( "and", Token_and );
INSERT( "and_eq", Token_and_eq );
INSERT( "asm", Token_asm );
INSERT( "auto", Token_auto );
INSERT( "bitand", Token_bitand );
INSERT( "bitor", Token_bitor );
INSERT( "bool", Token_bool );
INSERT( "break", Token_break );
INSERT( "case", Token_case );
INSERT( "catch", Token_catch );
INSERT( "char", Token_char );
INSERT( "class", Token_class );
INSERT( "compl", Token_compl );
INSERT( "const", Token_const );
INSERT( "const_cast", Token_const_cast );
INSERT( "continue", Token_continue );
INSERT( "default", Token_default );
INSERT( "delete", Token_delete );
INSERT( "do", Token_do );
INSERT( "double", Token_double );
INSERT( "dynamic_cast", Token_dynamic_cast );
INSERT( "else", Token_else );
INSERT( "enum", Token_enum );
INSERT( "explicit", Token_explicit );
INSERT( "export", Token_export );
INSERT( "extern", Token_extern );
INSERT( "float", Token_float );
INSERT( "for", Token_for );
INSERT( "friend", Token_friend );
INSERT( "goto", Token_goto );
INSERT( "if", Token_if );
INSERT( "inline", Token_inline );
INSERT( "int", Token_int );
INSERT( "long", Token_long );
INSERT( "mutable", Token_mutable );
INSERT( "namespace", Token_namespace );
INSERT( "new", Token_new );
INSERT( "not", Token_not );
INSERT( "not_eq", Token_not_eq );
INSERT( "operator", Token_operator );
INSERT( "or", Token_or );
INSERT( "or_eq", Token_or_eq );
INSERT( "private", Token_private );
INSERT( "protected", Token_protected );
INSERT( "public", Token_public );
INSERT( "register", Token_register );
INSERT( "reinterpret_cast", Token_reinterpret_cast );
INSERT( "return", Token_return );
INSERT( "short", Token_short );
INSERT( "signed", Token_signed );
INSERT( "sizeof", Token_sizeof );
INSERT( "static", Token_static );
INSERT( "static_cast", Token_static_cast );
INSERT( "struct", Token_struct );
INSERT( "switch", Token_switch );
INSERT( "template", Token_template );
INSERT( "this", Token_this );
INSERT( "throw", Token_throw );
INSERT( "try", Token_try );
INSERT( "typedef", Token_typedef );
INSERT( "typeid", Token_typeid );
INSERT( "typename", Token_typename );
INSERT( "union", Token_union );
INSERT( "unsigned", Token_unsigned );
INSERT( "using", Token_using );
INSERT( "virtual", Token_virtual );
INSERT( "void", Token_void );
INSERT( "volatile", Token_volatile );
INSERT( "while", Token_while );
INSERT( "xor", Token_xor );
INSERT( "xor_eq", Token_xor_eq );

//
// End of file
//
