/*
 * Copyright (c) 1998,1999 Tal Davidson. All rights reserved.
 *
 * astyle.h     (10 September 1999)
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
#ifndef ASTYLE_H
#define ASTYLE_H

#include "compiler_defines.h"

#include <string>
#include <vector>


/* The enums below ave been moved up from inside the namespace astyle, since they
   for some strange reason are not recognized by 'vectors' in Microsoft Visual C++ 5
   when they are part of a namespace!!! There was no such problem with GNU's g++ compiler.
*/
enum BracketMode   { NONE_MODE, ATTACH_MODE, BREAK_MODE, BDAC_MODE };
enum BracketType   { DEFINITION_TYPE = 1,
                     COMMAND_TYPE = 2,
                     ARRAY_TYPE  = 4,
                     SINGLE_LINE_TYPE = 8};


#ifdef USES_NAMESPACE
using namespace std;

namespace astyle
{
#endif


class ASSourceIterator
{
    public:
        virtual bool hasMoreLines() const = 0;
        virtual string nextLine() = 0;
};



class ASResource
{
    public:
        static const string IF, ELSE;
        static const string DO, WHILE;
        static const string FOR;
        static const string SWITCH, CASE, DEFAULT;
        static const string TRY, CATCH, THROWS, FINALLY;
        static const string PUBLIC, PROTECTED, PRIVATE;
        static const string CLASS, STRUCT, UNION, INTERFACE, NAMESPACE, EXTERN;
        static const string STATIC;
        static const string CONST;
        static const string SYNCHRONIZED;
        static const string OPERATOR, TEMPLATE;
        static const string OPEN_BRACKET, CLOSE_BRACKET;
        static const string OPEN_LINE_COMMENT, OPEN_COMMENT, CLOSE_COMMENT;
        static const string BAR_DEFINE, BAR_INCLUDE, BAR_IF, BAR_EL, BAR_ENDIF;

        static const string RETURN;
        static const string ASSIGN, PLUS_ASSIGN, MINUS_ASSIGN, MULT_ASSIGN;
        static const string DIV_ASSIGN, MOD_ASSIGN, XOR_ASSIGN, OR_ASSIGN, AND_ASSIGN;
        static const string GR_GR_ASSIGN, LS_LS_ASSIGN, GR_GR_GR_ASSIGN, LS_LS_LS_ASSIGN;
        static const string EQUAL, PLUS_PLUS, MINUS_MINUS, NOT_EQUAL, GR_EQUAL, GR_GR_GR, GR_GR;
        static const string LS_EQUAL, LS_LS_LS, LS_LS, ARROW, AND, OR;
        static const string COLON_COLON, PAREN_PAREN, BLPAREN_BLPAREN;
        static const string PLUS, MINUS, MULT, DIV, MOD, GR, LS;
        static const string NOT, BIT_XOR, BIT_OR, BIT_AND, BIT_NOT;
        static const string QUESTION, COLON, SEMICOLON, COMMA;
};

class ASBeautifier : protected ASResource
{
    public:
        ASBeautifier();
        virtual ~ASBeautifier();
        virtual void init(ASSourceIterator* iter); // pointer to dynamically created iterator.
        virtual void init();
        virtual bool hasMoreLines() const;
        virtual string nextLine();
        virtual string beautify(const string &line);
        void setTabIndentation(int length=4);
        void setSpaceIndentation(int length=4);
        void setMaxInStatementIndentLength(int max);
        void setMinConditionalIndentLength(int min);
        void setClassIndent(bool state);
        void setSwitchIndent(bool state);
        void setCaseIndent(bool state);
        void setBracketIndent(bool state);
        void setBlockIndent(bool state);
        void setNamespaceIndent(bool state);
        void setLabelIndent(bool state);
        void setCStyle();
        void setJavaStyle();
        void setEmptyLineFill(bool state);


    protected:
        int getNextProgramCharDistance(const string &line, int i);
        bool isLegalNameChar(char ch) const;
        bool isWhiteSpace(char ch) const;
        const string *findHeader(const string &line, int i,
                                 const vector<const string*> &possibleHeaders,
                                 bool checkBoundry = true);
        string trim(const string &str);
        int indexOf(vector<const string*> &container, const string *element);

    private:
        ASBeautifier(const ASBeautifier &copy);
        void operator=(ASBeautifier&); // not to be implemented

        void initStatic();
        void registerInStatementIndent(const string &line, int i, int spaceTabCount,
                                       int minIndent, bool updateParenStack);
        string preLineWS(int spaceTabCount, int tabCount);

        static vector<const string*> headers;
        static vector<const string*> nonParenHeaders;
        static vector<const string*> preprocessorHeaders;
        static vector<const string*> preBlockStatements;
        static vector<const string*> assignmentOperators;
        static vector<const string*> nonAssignmentOperators;

        static bool calledInitStatic;

        ASSourceIterator *sourceIterator;
        vector<ASBeautifier*> *waitingBeautifierStack;
        vector<ASBeautifier*> *activeBeautifierStack;
        vector<int> *waitingBeautifierStackLengthStack;
        vector<int> *activeBeautifierStackLengthStack;
        vector<const string*> *headerStack;
        vector< vector<const string*>* > *tempStacks;
        vector<int> *blockParenDepthStack;
        vector<bool> *blockStatementStack;
        vector<bool> *parenStatementStack;
        vector<int> *inStatementIndentStack;
        vector<int> *inStatementIndentStackSizeStack;
        vector<int> *parenIndentStack;
        vector<bool> *bracketBlockStateStack;
        string indentString;
        const string *currentHeader;
        const string *previousLastLineHeader;
        const string *immediatelyPreviousAssignmentOp;
        const string *probationHeader;
        bool isInQuote;
        bool isInComment;
        bool isInCase;
        bool isInQuestion;
        bool isInStatement;
        bool isInHeader;
        bool isCStyle;
        bool isInOperator;
        bool isInTemplate;
        bool isInConst;
        bool classIndent;
        bool isInClassHeader;
        bool isInClassHeaderTab;
        bool switchIndent;
        bool caseIndent;
        bool namespaceIndent;
        bool bracketIndent;
        bool blockIndent;
        bool labelIndent;
        bool isInConditional;
        bool isMinimalConditinalIndentSet;
        int minConditionalIndent;
        int parenDepth;
        int indentLength;
        int blockTabCount;
        int leadingWhiteSpaces;
        int maxInStatementIndent;
        int templateDepth;
        char quoteChar;
        char prevNonSpaceCh;
        char currentNonSpaceCh;
        char currentNonLegalCh;
        char prevNonLegalCh;
        int prevFinalLineSpaceTabCount;
        int prevFinalLineTabCount;
        bool emptyLineFill;
        bool backslashEndsPrevLine;
};


class ASFormatter : public ASBeautifier
{
    public:
        ASFormatter();
        virtual ~ASFormatter();
        virtual void init(ASSourceIterator* iter);
        virtual bool hasMoreLines() const;
        virtual string nextLine();
        void setBracketFormatMode(BracketMode mode);
        void setOperatorPaddingMode(bool mode);
        void setParenthesisPaddingMode(bool mode);
        void setBreakOneLineBlocksMode(bool state);
        void setSingleStatementsMode(bool state);


    private:
        void ASformatter(ASFormatter &copy); // not to be imlpemented
        void operator=(ASFormatter&); // not to be implemented
        void staticInit();
        bool isFormattingEnabled() const;
        void goForward(int i);
        bool getNextChar();
        char peekNextChar() const;
        bool isBeforeComment() const;
        void trimNewLine();
        BracketType getBracketType() const;
        bool isPointerOrReference() const;
        bool isUrinaryMinus() const;
        bool isInExponent() const;
        bool isOneLineBlockReached() const;
        void appendChar(char ch, bool canBreakLine = true);
        void appendCurrentChar(bool canBreakLine = true);
        void appendSequence(const string &sequence, bool canBreakLine = true);
        void appendSpacePad();
        void breakLine();
        inline bool isSequenceReached(const string &sequence) const;
        const string *findHeader(const vector<const string*> &headers, bool checkBoundry = true);

        static vector<const string*> headers;
        static vector<const string*> nonParenHeaders;
        static vector<const string*> preprocessorHeaders;
        static vector<const string*> preDefinitionHeaders;
        static vector<const string*> preCommandHeaders;
        static vector<const string*> operators;
        static vector<const string*> assignmentOperators;
        static bool calledInitStatic;

        ASSourceIterator *sourceIterator;
        vector<const string*> *preBracketHeaderStack;
        vector<BracketType> *bracketTypeStack;
        vector<int> *parenStack;
        string readyFormattedLine;
        string currentLine;
        string formattedLine;
        const string *currentHeader;
        const string *previousOperator;
        char currentChar;
        char previousChar;
        char previousNonWSChar;
        char previousCommandChar;
        char quoteChar;
        int charNum;
        BracketMode bracketFormatMode;
        bool isVirgin;
        bool shouldPadOperators;
        bool shouldPadParenthesies;
        bool isInLineComment;
        bool isInComment;
        bool isInPreprocessor;
        bool isInTemplate;
        bool doesLineStartComment;
        bool isInQuote;
        bool isSpecialChar;
        bool isNonParenHeader;
        bool foundQuestionMark;
        bool foundPreDefinitionHeader;
        bool foundPreCommandHeader;
        bool isInLineBreak;
        bool isInClosingBracketLineBreak;
        bool endOfCodeReached;
        bool isLineReady;
        bool isPreviousBracketBlockRelated;
        bool isInPotentialCalculation;
        //bool foundOneLineBlock;
        bool shouldBreakOneLineBlocks;
        bool shouldReparseCurrentChar;
        bool shouldBreakSemicolons;
        bool shouldBreakLineAfterComments;
        bool passedSemicolon;
        bool passedColon;
        bool isImmediatelyPostComment;
};


#ifdef USES_NAMESPACE
}
#endif

#endif // closes ASTYLE_H

