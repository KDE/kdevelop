
   !(#klass=class SEMICOLON)
-> program ;;

   CLASS type=TYPE (INHERITS base_type=TYPE | 0) LBRACE !(#feature=feature SEMICOLON) RBRACE
-> class ;;

   name=ID COLON type=TYPE
-> formal ;;

   ?[: LA(2).kind == Token_LPAREN :] name=ID LPAREN (#formal=formal @ COMMA | 0) RPAREN
                                     COLON type=TYPE LBRACE expression=expression RBRACE
 | ?[: LA(2).kind == Token_COLON :] name=ID COLON type=TYPE (LEFT_ARROW expression=expression | 0)
-> feature ;;

   ?[: LA(2).kind == Token_LEFT_ARROW :] name=ID LEFT_ARROW expression=expression                  -- assignment
 | ?[: LA(2).kind == Token_LPAREN :]     name=ID LPAREN (#argument=expression @ COMMA | 0) RPAREN  -- dispatch
 | variable=ID
 | integer_literal=INTEGER
 | string_literal=STRING
 | true_literal=TRUE
 | false_literal=FALSE
 | NEW new_type=TYPE
 | LPAREN expression=expression RPAREN
 | if_expression=if_expression
 | while_expression=while_expression
 | block_expression=block_expression
 | let_expression=let_expression
 | case_expression=case_expression
-> primary_expression ;;

   op=TILDE  expression=primary_expression -- ^tilde_expression
 | op=NOT    expression=primary_expression -- ^not_expression
 | op=ISVOID expression=primary_expression -- ^isvoid_expression
 |           expression=primary_expression
-> unary_expression ;;

   base_expression=unary_expression !(AT at_type=TYPE DOT name=ID LPAREN (#arguments=expression @ COMMA | 0) RPAREN
                                     |                DOT name=ID LPAREN (#arguments=expression @ COMMA | 0) RPAREN)
-> postfix_expression ;;

   #expression=postfix_expression @ (op=STAR | op=DIVIDE)
-> multiplicative_expression ;;

   #expression=multiplicative_expression @ (op=PLUS | op=MINUS)
-> additive_expression ;;

   #expression=additive_expression @ (op=EQUAL | op=LESS_EQUAL | op=LESS)
-> relational_expression ;;

   IF condition=expression THEN true_expression=expression ELSE false_expression=expression FI
-> if_expression ;;

   WHILE condition=expression LOOP loop_expression=expression POOL
-> while_expression ;;

   LBRACE !(#expression=expression SEMICOLON) RBRACE
-> block_expression ;;

   LET #declaration=let_declaration @ COMMA IN body_expression=expression
-> let_expression ;;

   CASE expression=expression OF !(#condition=case_condition SEMICOLON) ESAC
-> case_expression ;;

   expression=relational_expression
-> expression ;;

   name=ID COLON type=TYPE (LEFT_ARROW expression=expression | 0)
-> let_declaration ;;

   name=ID COLON type=TYPE RIGHT_ARROW expression=expression
-> case_condition ;;
