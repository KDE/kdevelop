// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#ifndef ruby_H_INCLUDED
#define ruby_H_INCLUDED

#include "ruby_ast.h"
#include "kdev-pg-memory-pool.h"
#include "kdev-pg-allocator.h"
#include "kdev-pg-token-stream.h"

namespace ruby
  {

  class parser
    {
    public:
      typedef kdev_pg_token_stream token_stream_type;
      typedef kdev_pg_token_stream::token_type token_type;
      kdev_pg_token_stream *token_stream;
      int yytoken;

      inline token_type LA(std::size_t k = 1) const
        {
          return token_stream->token(token_stream->index() - 1 + k - 1);
        }
      inline int yylex()
      {
        return (yytoken = token_stream->next_token());
      }
      inline void rewind(std::size_t index)
      {
        token_stream->rewind(index);
        yylex();
      }

      // token stream
      void set_token_stream(kdev_pg_token_stream *s)
      {
        token_stream = s;
      }

      // error handling
      void yy_expected_symbol(int kind, char const *name);
      void yy_expected_token(int kind, std::size_t token, char const *name);

      bool yy_block_errors;
      inline bool block_errors(bool block)
      {
        bool previous = yy_block_errors;
        yy_block_errors = block;
        return previous;
      }

      // memory pool
      typedef kdev_pg_memory_pool memory_pool_type;

      kdev_pg_memory_pool *memory_pool;
      void set_memory_pool(kdev_pg_memory_pool *p)
      {
        memory_pool = p;
      }
      template <class T>
      inline T *create()
      {
        T *node = new (memory_pool->allocate(sizeof(T))) T();
        node->kind = T::KIND;
        return node;
      }

      enum token_type_enum
      {
        Token_ALIAS = 1000,
        Token_AND = 1001,
        Token_ASCII_VALUE = 1002,
        Token_ASSIGN = 1003,
        Token_ASSIGN_WITH_NO_LEADING_SPACE = 1004,
        Token_ASSOC = 1005,
        Token_BAND = 1006,
        Token_BAND_ASSIGN = 1007,
        Token_BEGIN = 1008,
        Token_BEGIN_UPCASE = 1009,
        Token_BINARY = 1010,
        Token_BLOCK_ARG_PREFIX = 1011,
        Token_BNOT = 1012,
        Token_BOR = 1013,
        Token_BOR_ASSIGN = 1014,
        Token_BREAK = 1015,
        Token_BXOR = 1016,
        Token_BXOR_ASSIGN = 1017,
        Token_CASE = 1018,
        Token_CASE_EQUAL = 1019,
        Token_CLASS = 1020,
        Token_CLASS_VARIABLE = 1021,
        Token_COLON = 1022,
        Token_COLON_WITH_NO_FOLLOWING_SPACE = 1023,
        Token_COMMA = 1024,
        Token_COMMAND_OUTPUT = 1025,
        Token_COMMAND_OUTPUT_BEFORE_EXPRESSION_SUBSTITUTION = 1026,
        Token_COMPARE = 1027,
        Token_CONSTANT = 1028,
        Token_DEF = 1029,
        Token_DEFINED = 1030,
        Token_DIV = 1031,
        Token_DIV_ASSIGN = 1032,
        Token_DO = 1033,
        Token_DOT = 1034,
        Token_DOUBLE_QUOTED_STRING = 1035,
        Token_DO_IN_CONDITION = 1036,
        Token_ELSE = 1037,
        Token_ELSIF = 1038,
        Token_END = 1039,
        Token_END_UPCASE = 1040,
        Token_ENSURE = 1041,
        Token_EOF = 1042,
        Token_EQUAL = 1043,
        Token_EXCLUSIVE_RANGE = 1044,
        Token_FALSE = 1045,
        Token_FILE = 1046,
        Token_FLOAT = 1047,
        Token_FOR = 1048,
        Token_FUNCTION = 1049,
        Token_GLOBAL_VARIABLE = 1050,
        Token_GREATER_OR_EQUAL = 1051,
        Token_GREATER_THAN = 1052,
        Token_HERE_DOC_BEGIN = 1053,
        Token_HEX = 1054,
        Token_IDENTIFIER = 1055,
        Token_IF = 1056,
        Token_IF_MODIFIER = 1057,
        Token_IN = 1058,
        Token_INCLUSIVE_RANGE = 1059,
        Token_INSTANCE_VARIABLE = 1060,
        Token_INTEGER = 1061,
        Token_KEYWORD_NOT = 1062,
        Token_LBRACK = 1063,
        Token_LBRACK_ARRAY_ACCESS = 1064,
        Token_LCURLY = 1065,
        Token_LCURLY_BLOCK = 1066,
        Token_LCURLY_HASH = 1067,
        Token_LEADING_TWO_COLON = 1068,
        Token_LEFT_SHIFT = 1069,
        Token_LEFT_SHIFT_ASSIGN = 1070,
        Token_LESS_OR_EQUAL = 1071,
        Token_LESS_THAN = 1072,
        Token_LINE = 1073,
        Token_LINE_BREAK = 1074,
        Token_LOGICAL_AND = 1075,
        Token_LOGICAL_AND_ASSIGN = 1076,
        Token_LOGICAL_OR = 1077,
        Token_LOGICAL_OR_ASSIGN = 1078,
        Token_LPAREN = 1079,
        Token_MATCH = 1080,
        Token_MINUS = 1081,
        Token_MINUS_ASSIGN = 1082,
        Token_MOD = 1083,
        Token_MODULE = 1084,
        Token_MOD_ASSIGN = 1085,
        Token_NEXT = 1086,
        Token_NIL = 1087,
        Token_NOT = 1088,
        Token_NOT_EQUAL = 1089,
        Token_NOT_MATCH = 1090,
        Token_OCTAL = 1091,
        Token_OR = 1092,
        Token_PLUS = 1093,
        Token_PLUS_ASSIGN = 1094,
        Token_POWER = 1095,
        Token_POWER_ASSIGN = 1096,
        Token_QUESTION = 1097,
        Token_RBRACK = 1098,
        Token_RCURLY = 1099,
        Token_RCURLY_BLOCK = 1100,
        Token_REDO = 1101,
        Token_REGEX = 1102,
        Token_REGEX_BEFORE_EXPRESSION_SUBSTITUTION = 1103,
        Token_RESCUE = 1104,
        Token_RESCUE_MODIFIER = 1105,
        Token_REST_ARG_PREFIX = 1106,
        Token_RETRY = 1107,
        Token_RETURN = 1108,
        Token_RIGHT_SHIFT = 1109,
        Token_RIGHT_SHIFT_ASSIGN = 1110,
        Token_RPAREN = 1111,
        Token_SELF = 1112,
        Token_SEMI = 1113,
        Token_SINGLE_QUOTE = 1114,
        Token_SINGLE_QUOTED_STRING = 1115,
        Token_STAR = 1116,
        Token_STAR_ASSIGN = 1117,
        Token_STRING_AFTER_EXPRESSION_SUBSTITUTION = 1118,
        Token_STRING_BEFORE_EXPRESSION_SUBSTITUTION = 1119,
        Token_STRING_BETWEEN_EXPRESSION_SUBSTITUTION = 1120,
        Token_STUB_A = 1121,
        Token_STUB_B = 1122,
        Token_STUB_C = 1123,
        Token_SUPER = 1124,
        Token_THEN = 1125,
        Token_TRUE = 1126,
        Token_TWO_COLON = 1127,
        Token_UNARY_MINUS = 1128,
        Token_UNARY_PLUS = 1129,
        Token_UNARY_PLUS_MINUS_METHOD_NAME = 1130,
        Token_UNDEF = 1131,
        Token_UNLESS = 1132,
        Token_UNLESS_MODIFIER = 1133,
        Token_UNTIL = 1134,
        Token_UNTIL_MODIFIER = 1135,
        Token_WHEN = 1136,
        Token_WHILE = 1137,
        Token_WHILE_MODIFIER = 1138,
        Token_W_ARRAY = 1139,
        Token_YIELD = 1140,
        token_type_size
      }; // token_type_enum

      // user defined declarations:
    public:

      /**
       * Transform the raw input into tokens.
       * When this method returns, the parser's token stream has been filled
       * and any parse_*() method can be called.
       */
      void tokenize( char *contents );

      enum problem_type {
        error,
        warning,
        info
      };

      void report_problem( parser::problem_type type, const char* message );
      void report_problem( parser::problem_type type, std::string message );

    private:

      struct parser_state
        {
          // ltCounter stores the amount of currently open type arguments rules,
          // all of which are beginning with a less than ("<") character.
          // This way, also RSHIFT (">>") can be used to close type arguments rules,
          // in addition to GREATER_THAN (">").
          int ltCounter;
        };
      parser_state _M_state;

      //state modifiers
      bool seen_star;
      bool seen_star_or_band;
      bool seen_rparen;

      bool expect_array_or_block_arguments;

      Lexer *m_lexer;


    public:
      // The copy_current_state() and restore_state() methods are only declared
      // if you are using try blocks in your grammar, and have to be
      // implemented by yourself, and you also have to define a
      // "struct parser_state" inside a %parserclass directive.

      // This method should create a new parser_state object and return it,
      // or return 0 if no state variables need to be saved.
      parser_state *copy_current_state();

      // This method is only called for parser_state objects != 0
      // and should restore the parser state given as argument.
      void restore_state(parser_state *state);
      parser()
      {
        memory_pool = 0;
        token_stream = 0;
        yytoken = Token_EOF;
        yy_block_errors = false;
      }

      virtual ~parser()
      {}

      bool parse_additiveExpression(additiveExpression_ast **yynode);
      bool parse_aliasParameter(aliasParameter_ast **yynode);
      bool parse_andExpression(andExpression_ast **yynode);
      bool parse_andorExpression(andorExpression_ast **yynode);
      bool parse_arrayAccess(arrayAccess_ast **yynode);
      bool parse_arrayExpression(arrayExpression_ast **yynode);
      bool parse_arrayReferenceArgument(arrayReferenceArgument_ast **yynode);
      bool parse_assignmentExpression(assignmentExpression_ast **yynode);
      bool parse_blockContent(blockContent_ast **yynode);
      bool parse_blockMethodDefinitionArgument(blockMethodDefinitionArgument_ast **yynode);
      bool parse_blockMethodInvocationArgument(blockMethodInvocationArgument_ast **yynode);
      bool parse_block_var(block_var_ast **yynode);
      bool parse_block_vars(block_vars_ast **yynode);
      bool parse_bodyStatement(bodyStatement_ast **yynode);
      bool parse_caseExpression(caseExpression_ast **yynode);
      bool parse_classDefinition(classDefinition_ast **yynode);
      bool parse_className(className_ast **yynode);
      bool parse_codeBlock(codeBlock_ast **yynode);
      bool parse_colonAccess(colonAccess_ast **yynode);
      bool parse_command(command_ast **yynode);
      bool parse_commandOutput(commandOutput_ast **yynode);
      bool parse_compoundStatement(compoundStatement_ast **yynode);
      bool parse_doOrTerminalOrColon(doOrTerminalOrColon_ast **yynode);
      bool parse_dotAccess(dotAccess_ast **yynode);
      bool parse_elementReference(elementReference_ast **yynode);
      bool parse_equalityExpression(equalityExpression_ast **yynode);
      bool parse_exceptionHandlingExpression(exceptionHandlingExpression_ast **yynode);
      bool parse_exceptionList(exceptionList_ast **yynode);
      bool parse_expression(expression_ast **yynode);
      bool parse_expressionSubstitution(expressionSubstitution_ast **yynode);
      bool parse_forExpression(forExpression_ast **yynode);
      bool parse_hashExpression(hashExpression_ast **yynode);
      bool parse_ifExpression(ifExpression_ast **yynode);
      bool parse_keyValuePair(keyValuePair_ast **yynode);
      bool parse_keyword(keyword_ast **yynode);
      bool parse_keywordAlias(keywordAlias_ast **yynode);
      bool parse_keywordAnd(keywordAnd_ast **yynode);
      bool parse_keywordAsMethodName(keywordAsMethodName_ast **yynode);
      bool parse_keywordBeginUpcase(keywordBeginUpcase_ast **yynode);
      bool parse_keywordBreak(keywordBreak_ast **yynode);
      bool parse_keywordClass(keywordClass_ast **yynode);
      bool parse_keywordDef(keywordDef_ast **yynode);
      bool parse_keywordDefined(keywordDefined_ast **yynode);
      bool parse_keywordDo(keywordDo_ast **yynode);
      bool parse_keywordEndUpcase(keywordEndUpcase_ast **yynode);
      bool parse_keywordFor(keywordFor_ast **yynode);
      bool parse_keywordIn(keywordIn_ast **yynode);
      bool parse_keywordModule(keywordModule_ast **yynode);
      bool parse_keywordNot(keywordNot_ast **yynode);
      bool parse_keywordOr(keywordOr_ast **yynode);
      bool parse_keywordUndef(keywordUndef_ast **yynode);
      bool parse_keywordUntil(keywordUntil_ast **yynode);
      bool parse_keywordWhen(keywordWhen_ast **yynode);
      bool parse_keywordWhile(keywordWhile_ast **yynode);
      bool parse_literal(literal_ast **yynode);
      bool parse_logicalAndExpression(logicalAndExpression_ast **yynode);
      bool parse_logicalOrExpression(logicalOrExpression_ast **yynode);
      bool parse_methodCall(methodCall_ast **yynode);
      bool parse_methodDefinition(methodDefinition_ast **yynode);
      bool parse_methodDefinitionArgument(methodDefinitionArgument_ast **yynode);
      bool parse_methodDefinitionArgumentWithoutParen(methodDefinitionArgumentWithoutParen_ast **yynode);
      bool parse_methodInvocationArgumentWithParen(methodInvocationArgumentWithParen_ast **yynode);
      bool parse_methodInvocationArgumentWithoutParen(methodInvocationArgumentWithoutParen_ast **yynode);
      bool parse_methodName(methodName_ast **yynode);
      bool parse_methodNameSupplement(methodNameSupplement_ast **yynode);
      bool parse_mlhs_item(mlhs_item_ast **yynode);
      bool parse_moduleDefinition(moduleDefinition_ast **yynode);
      bool parse_moduleName(moduleName_ast **yynode);
      bool parse_mrhs(mrhs_ast **yynode);
      bool parse_multiplicativeExpression(multiplicativeExpression_ast **yynode);
      bool parse_normalMethodDefinitionArgument(normalMethodDefinitionArgument_ast **yynode);
      bool parse_normalMethodInvocationArgument(normalMethodInvocationArgument_ast **yynode);
      bool parse_notExpression(notExpression_ast **yynode);
      bool parse_numeric(numeric_ast **yynode);
      bool parse_operatorAsMethodname(operatorAsMethodname_ast **yynode);
      bool parse_operatorAssign(operatorAssign_ast **yynode);
      bool parse_operatorBAnd(operatorBAnd_ast **yynode);
      bool parse_operatorBAndAssign(operatorBAndAssign_ast **yynode);
      bool parse_operatorBNot(operatorBNot_ast **yynode);
      bool parse_operatorBOr(operatorBOr_ast **yynode);
      bool parse_operatorBXor(operatorBXor_ast **yynode);
      bool parse_operatorBXorAssign(operatorBXorAssign_ast **yynode);
      bool parse_operatorBorAssign(operatorBorAssign_ast **yynode);
      bool parse_operatorCaseEqual(operatorCaseEqual_ast **yynode);
      bool parse_operatorColon(operatorColon_ast **yynode);
      bool parse_operatorCompare(operatorCompare_ast **yynode);
      bool parse_operatorDiv(operatorDiv_ast **yynode);
      bool parse_operatorDivAssign(operatorDivAssign_ast **yynode);
      bool parse_operatorEqual(operatorEqual_ast **yynode);
      bool parse_operatorExclusiveRange(operatorExclusiveRange_ast **yynode);
      bool parse_operatorGreaterOrEqual(operatorGreaterOrEqual_ast **yynode);
      bool parse_operatorGreaterThan(operatorGreaterThan_ast **yynode);
      bool parse_operatorInclusiveRange(operatorInclusiveRange_ast **yynode);
      bool parse_operatorLeftShift(operatorLeftShift_ast **yynode);
      bool parse_operatorLeftShiftAssign(operatorLeftShiftAssign_ast **yynode);
      bool parse_operatorLessOrEqual(operatorLessOrEqual_ast **yynode);
      bool parse_operatorLessThan(operatorLessThan_ast **yynode);
      bool parse_operatorLogicalAnd(operatorLogicalAnd_ast **yynode);
      bool parse_operatorLogicalAndAssign(operatorLogicalAndAssign_ast **yynode);
      bool parse_operatorLogicalOr(operatorLogicalOr_ast **yynode);
      bool parse_operatorLogicalOrAssign(operatorLogicalOrAssign_ast **yynode);
      bool parse_operatorMatch(operatorMatch_ast **yynode);
      bool parse_operatorMinus(operatorMinus_ast **yynode);
      bool parse_operatorMinusAssign(operatorMinusAssign_ast **yynode);
      bool parse_operatorMod(operatorMod_ast **yynode);
      bool parse_operatorModAssign(operatorModAssign_ast **yynode);
      bool parse_operatorNot(operatorNot_ast **yynode);
      bool parse_operatorNotEqual(operatorNotEqual_ast **yynode);
      bool parse_operatorNotMatch(operatorNotMatch_ast **yynode);
      bool parse_operatorPlus(operatorPlus_ast **yynode);
      bool parse_operatorPlusAssign(operatorPlusAssign_ast **yynode);
      bool parse_operatorPower(operatorPower_ast **yynode);
      bool parse_operatorPowerAssign(operatorPowerAssign_ast **yynode);
      bool parse_operatorQuestion(operatorQuestion_ast **yynode);
      bool parse_operatorRightShift(operatorRightShift_ast **yynode);
      bool parse_operatorRightShiftAssign(operatorRightShiftAssign_ast **yynode);
      bool parse_operatorStar(operatorStar_ast **yynode);
      bool parse_operatorStarAssign(operatorStarAssign_ast **yynode);
      bool parse_operatorUnaryMinus(operatorUnaryMinus_ast **yynode);
      bool parse_operatorUnaryPlus(operatorUnaryPlus_ast **yynode);
      bool parse_orExpression(orExpression_ast **yynode);
      bool parse_parallelAssignmentLeftOver(parallelAssignmentLeftOver_ast **yynode);
      bool parse_powerExpression(powerExpression_ast **yynode);
      bool parse_predefinedValue(predefinedValue_ast **yynode);
      bool parse_primaryExpression(primaryExpression_ast **yynode);
      bool parse_program(program_ast **yynode);
      bool parse_rangeExpression(rangeExpression_ast **yynode);
      bool parse_regex(regex_ast **yynode);
      bool parse_relationalExpression(relationalExpression_ast **yynode);
      bool parse_restMethodDefinitionArgument(restMethodDefinitionArgument_ast **yynode);
      bool parse_restMethodInvocationArgument(restMethodInvocationArgument_ast **yynode);
      bool parse_shiftExpression(shiftExpression_ast **yynode);
      bool parse_statement(statement_ast **yynode);
      bool parse_statementWithoutModifier(statementWithoutModifier_ast **yynode);
      bool parse_statements(statements_ast **yynode);
      bool parse_string(string_ast **yynode);
      bool parse_symbol(symbol_ast **yynode);
      bool parse_terminal(terminal_ast **yynode);
      bool parse_ternaryIfThenElseExpression(ternaryIfThenElseExpression_ast **yynode);
      bool parse_thenOrTerminalOrColon(thenOrTerminalOrColon_ast **yynode);
      bool parse_unaryExpression(unaryExpression_ast **yynode);
      bool parse_undefParameter(undefParameter_ast **yynode);
      bool parse_unlessExpression(unlessExpression_ast **yynode);
      bool parse_untilExpression(untilExpression_ast **yynode);
      bool parse_variable(variable_ast **yynode);
      bool parse_whileExpression(whileExpression_ast **yynode);
    };

} // end of namespace ruby

#endif


