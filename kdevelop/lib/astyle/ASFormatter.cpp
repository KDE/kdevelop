/*
 * Copyright (c) 1998,1999,2000,2001,2002 Tal Davidson. All rights reserved.
 *
 * ASFormatter.cpp
 * by Tal Davidson (davidsont@bigfoot.com)
 * This file is a part of "Artistic Style" - an indentater and reformatter
 * of C, C++, C# and Java source files.
 *
 * The "Artistic Style" project, including all files needed to compile it,
 * is free software; you can redistribute it and/or use it and/or modify it
 * under the terms of the GNU General Public License as published 
 * by the Free Software Foundation; either version 2 of the License, 
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.
 *
 *
 * Patches:
 * 26 November 1998 - Richard Bullington -
 *        A correction of line-breaking in headers following '}',
 *        was created using a variation of a  patch by Richard Bullington.
 */

#include "compiler_defines.h"
#include "astyle.h"

#include <string>
#include <cctype>
#include <vector>
#include <algorithm>
#include <iostream>


#define INIT_CONTAINER(container, value)     {if ( (container) != NULL ) delete (container); (container) = (value); }
#define DELETE_CONTAINER(container)          {if ( (container) != NULL ) delete (container) ; }
#define IS_A(a,b)                            ( ((a) & (b)) == (b))
#ifdef USES_NAMESPACE
using namespace std;

namespace astyle {
#endif


bool ASFormatter::calledInitStatic = false;
vector<const string*> ASFormatter::headers;
vector<const string*> ASFormatter::nonParenHeaders;
vector<const string*> ASFormatter::preprocessorHeaders;
vector<const string*> ASFormatter::preDefinitionHeaders;
vector<const string*> ASFormatter::preCommandHeaders;
vector<const string*> ASFormatter::operators;
vector<const string*> ASFormatter::assignmentOperators;

/**
 * Constructor of ASFormatter
 */
ASFormatter::ASFormatter() {
    staticInit();

    preBracketHeaderStack = NULL;
    bracketTypeStack = NULL;
    parenStack = NULL;

    sourceIterator = NULL;
    bracketFormatMode = NONE_MODE;
    shouldPadOperators = false;
    shouldPadParenthesies = false;
    shouldBreakOneLineBlocks = true;
    shouldBreakOneLineStatements = true;
    shouldConvertTabs = false;
    shouldBreakBlocks = false;
    shouldBreakClosingHeaderBlocks = false;
    shouldBreakClosingHeaderBrackets = false;
    shouldBreakElseIfs = false;
}

/**
 * Destructor of ASFormatter
 */
ASFormatter::~ASFormatter() {
    DELETE_CONTAINER( preBracketHeaderStack );
}

/**
 * initialization of static data of ASFormatter.
 */
void ASFormatter::staticInit() {
    if (calledInitStatic)
        return;

    calledInitStatic = true;

    headers.push_back(&AS_IF);
    headers.push_back(&AS_ELSE);
    headers.push_back(&AS_DO);
    headers.push_back(&AS_WHILE);
    headers.push_back(&AS_FOR);
    headers.push_back(&AS_SYNCHRONIZED);
    headers.push_back(&AS_TRY);
    headers.push_back(&AS_CATCH);
    headers.push_back(&AS_FINALLY);
    headers.push_back(&AS_SWITCH);
    headers.push_back(&AS_TEMPLATE);
    headers.push_back(&AS_FOREACH);
    headers.push_back(&AS_LOCK);
    headers.push_back(&AS_UNSAFE);
    headers.push_back(&AS_FIXED);
    headers.push_back(&AS_GET);
    headers.push_back(&AS_SET);
    headers.push_back(&AS_ADD);
    headers.push_back(&AS_REMOVE);

    nonParenHeaders.push_back(&AS_ELSE);
    nonParenHeaders.push_back(&AS_DO);
    nonParenHeaders.push_back(&AS_TRY);
    nonParenHeaders.push_back(&AS_FINALLY);
    nonParenHeaders.push_back(&AS_UNSAFE);
    nonParenHeaders.push_back(&AS_GET);
    nonParenHeaders.push_back(&AS_SET);
    nonParenHeaders.push_back(&AS_ADD);
    nonParenHeaders.push_back(&AS_REMOVE);

    //    nonParenHeaders.push_back(&AS_TEMPLATE);

    preDefinitionHeaders.push_back(&AS_CLASS);
    preDefinitionHeaders.push_back(&AS_INTERFACE);
    preDefinitionHeaders.push_back(&AS_NAMESPACE);
    preDefinitionHeaders.push_back(&AS_STRUCT);

    preCommandHeaders.push_back(&AS_EXTERN);
    preCommandHeaders.push_back(&AS_THROWS);
    preCommandHeaders.push_back(&AS_CONST);

    preprocessorHeaders.push_back(&AS_BAR_DEFINE);
    //// DEVEL: removed the folowing lines
    ////preprocessorHeaders.push_back(&AS_BAR_INCLUDE);
    ////preprocessorHeaders.push_back(&AS_BAR_IF); // #if or #ifdef
    ////preprocessorHeaders.push_back(&AS_BAR_EL); // #else or #elif
    ////preprocessorHeaders.push_back(&AS_BAR_ENDIF);

    operators.push_back(&AS_PLUS_ASSIGN);
    operators.push_back(&AS_MINUS_ASSIGN);
    operators.push_back(&AS_MULT_ASSIGN);
    operators.push_back(&AS_DIV_ASSIGN);
    operators.push_back(&AS_MOD_ASSIGN);
    operators.push_back(&AS_OR_ASSIGN);
    operators.push_back(&AS_AND_ASSIGN);
    operators.push_back(&AS_XOR_ASSIGN);
    operators.push_back(&AS_EQUAL);
    operators.push_back(&AS_PLUS_PLUS);
    operators.push_back(&AS_MINUS_MINUS);
    operators.push_back(&AS_NOT_EQUAL);
    operators.push_back(&AS_GR_EQUAL);
    operators.push_back(&AS_GR_GR_GR_ASSIGN);
    operators.push_back(&AS_GR_GR_ASSIGN);
    operators.push_back(&AS_GR_GR_GR);
    operators.push_back(&AS_GR_GR);
    operators.push_back(&AS_LS_EQUAL);
    operators.push_back(&AS_LS_LS_LS_ASSIGN);
    operators.push_back(&AS_LS_LS_ASSIGN);
    operators.push_back(&AS_LS_LS_LS);
    operators.push_back(&AS_LS_LS);
    operators.push_back(&AS_ARROW);
    operators.push_back(&AS_AND);
    operators.push_back(&AS_OR);
    operators.push_back(&AS_COLON_COLON);

    //// BUGFIX: removed the folowing lines
    ////    operators.push_back(&AS_PAREN_PAREN);
    ////    operators.push_back(&AS_BLPAREN_BLPAREN);

    operators.push_back(&AS_PLUS);
    operators.push_back(&AS_MINUS);
    operators.push_back(&AS_MULT);
    operators.push_back(&AS_DIV);
    operators.push_back(&AS_MOD);
    operators.push_back(&AS_QUESTION);
    operators.push_back(&AS_COLON);
    operators.push_back(&AS_ASSIGN);
    operators.push_back(&AS_LS);
    operators.push_back(&AS_GR);
    operators.push_back(&AS_NOT);
    operators.push_back(&AS_BIT_OR);
    operators.push_back(&AS_BIT_AND);
    operators.push_back(&AS_BIT_NOT);
    operators.push_back(&AS_BIT_XOR);
    operators.push_back(&AS_OPERATOR);
    operators.push_back(&AS_COMMA);
    //    operators.push_back(&AS_SEMICOLON);
    operators.push_back(&AS_RETURN);

    assignmentOperators.push_back(&AS_PLUS_ASSIGN);
    assignmentOperators.push_back(&AS_MINUS_ASSIGN);
    assignmentOperators.push_back(&AS_MULT_ASSIGN);
    assignmentOperators.push_back(&AS_DIV_ASSIGN);
    assignmentOperators.push_back(&AS_MOD_ASSIGN);
    assignmentOperators.push_back(&AS_XOR_ASSIGN);
    assignmentOperators.push_back(&AS_OR_ASSIGN);
    assignmentOperators.push_back(&AS_AND_ASSIGN);
    assignmentOperators.push_back(&AS_GR_GR_GR_ASSIGN);
    assignmentOperators.push_back(&AS_LS_LS_LS_ASSIGN);
    assignmentOperators.push_back(&AS_ASSIGN);
}

/**
 * initialize the ASFormatter.
 *
 * init() should be called every time a ASFormatter object is to start
 * formatting a NEW source file.
 * init() recieves a pointer to a DYNAMICALLY CREATED ASSourceIterator object
 * that will be used to iterate through the source code. This object will be
 * deleted during the ASFormatter's destruction, and thus should not be
 * deleted elsewhere.
 *
 * @param si     a pointer to the DYNAMICALLY CREATED ASSourceIterator object.
 */
void ASFormatter::init(ASSourceIterator *si) {
    ASBeautifier::init(si);
    sourceIterator = si;

    INIT_CONTAINER( preBracketHeaderStack, new vector<const string*> );
    INIT_CONTAINER( bracketTypeStack, new vector<BracketType> );
    bracketTypeStack->push_back(DEFINITION_TYPE);
    INIT_CONTAINER( parenStack, new vector<int> );
    parenStack->push_back(0);

    currentHeader = NULL;
    currentLine = string("");
    formattedLine = "";
    currentChar = ' ';
    previousCommandChar = ' ';
    previousNonWSChar = ' ';
    quoteChar = '"';
    charNum = 0;
    previousOperator = NULL;

    isVirgin = true;
    isInLineComment = false;
    isInComment = false;
    isInPreprocessor = false;
    doesLineStartComment = false;
    isInQuote = false;
    isSpecialChar = false;
    isNonParenHeader = true;
    foundPreDefinitionHeader = false;
    foundPreCommandHeader = false;
    foundQuestionMark = false;
    isInLineBreak = false;
    endOfCodeReached = false;
    isLineReady = false;
    isPreviousBracketBlockRelated = true;
    isInPotentialCalculation = false;
    //foundOneLineBlock = false;
    shouldReparseCurrentChar = false;
    passedSemicolon = false;
    passedColon = false;
    isInTemplate = false;
    shouldBreakLineAfterComments = false;
    isImmediatelyPostComment = false;
    isImmediatelyPostLineComment = false;
    isImmediatelyPostEmptyBlock = false;

    isPrependPostBlockEmptyLineRequested = false;
    isAppendPostBlockEmptyLineRequested = false;
    prependEmptyLine = false;

    foundClosingHeader = false;
    previousReadyFormattedLineLength = 0;

    isImmediatelyPostHeader = false;
    isInHeader = false;
}

/**
 * get the next formatted line.
 *
 * @return    formatted line.
 */

string ASFormatter::nextLine() {
    const string *newHeader;
    bool isCharImmediatelyPostComment = false;
    bool isPreviousCharPostComment = false;
    bool isCharImmediatelyPostLineComment = false;
    bool isInVirginLine = isVirgin;
    bool isCharImmediatelyPostOpenBlock = false;
    bool isCharImmediatelyPostCloseBlock = false;
    bool isCharImmediatelyPostTemplate = false;
    bool isCharImmediatelyPostHeader = false;

    if (!isFormattingEnabled())
        return ASBeautifier::nextLine();

    while (!isLineReady) {
        if (shouldReparseCurrentChar)
            shouldReparseCurrentChar = false;
        else if (!getNextChar()) {
            breakLine();
            return beautify(readyFormattedLine);
        } else // stuff to do when reading a new character...
        {
            // make sure that a virgin '{' at the begining ofthe file will be treated as a block...
            if (isInVirginLine && currentChar == '{')
                previousCommandChar = '{';
            isPreviousCharPostComment = isCharImmediatelyPostComment;
            isCharImmediatelyPostComment = false;
            isCharImmediatelyPostTemplate = false;
            isCharImmediatelyPostHeader = false;
        }

        if (isInLineComment) {
            appendCurrentChar();

            // explicitely break a line when a line comment's end is found.
            if (/*bracketFormatMode == ATTACH_MODE &&*/ charNum+1 == (int)currentLine.length()) {
                isInLineBreak = true;
                isInLineComment = false;
                isImmediatelyPostLineComment = true;
                currentChar = 0;  //make sure it is a neutral char.
            }



            continue;
        } else if (isInComment) {
            if (isSequenceReached(AS_CLOSE_COMMENT)) {
                isInComment = false;
                isImmediatelyPostComment = true;
                appendSequence(AS_CLOSE_COMMENT);
                goForward(1);
            } else
                appendCurrentChar();

            continue;
        }

        // not in line comment or comment

        else if (isInQuote) {
            if (isSpecialChar) {
                isSpecialChar = false;
                appendCurrentChar();
            } else if (currentChar == '\\') {
                isSpecialChar = true;
                appendCurrentChar();
            } else if (quoteChar == currentChar) {
                isInQuote = false;
                appendCurrentChar();
            } else {
                appendCurrentChar();
            }

            continue;
        }



        // handle white space - needed to simplify the rest.
        if (isWhiteSpace(currentChar) || isInPreprocessor) {
            ////// DEVEL: if (isLegalNameChar(previousChar) && isLegalNameChar(peekNextChar()))
            appendCurrentChar();
            continue;
        }

        /* not in MIDDLE of quote or comment or white-space of any type ... */

        if (isSequenceReached(AS_OPEN_LINE_COMMENT)) {
            isInLineComment = true;
            if (shouldPadOperators)
                appendSpacePad();
            appendSequence(AS_OPEN_LINE_COMMENT);
            goForward(1);
            continue;
        } else if (isSequenceReached(AS_OPEN_COMMENT)) {
            isInComment = true;
            if (shouldPadOperators)
                appendSpacePad();
            appendSequence(AS_OPEN_COMMENT);
            goForward(1);
            continue;
        } else if (currentChar == '"' || currentChar == '\'') {
            isInQuote = true;
            quoteChar = currentChar;
            ////            if (shouldPadOperators)  // BUGFIX: these two lines removed. seem to be unneeded, and interfere with L"
            ////                appendSpacePad();    // BUFFIX:	@todo make sure the removal of these lines doesn't reopen old bugs...
            appendCurrentChar();
            continue;
        }

        /* not in quote or comment or white-space of any type ... */

        // check if in preprocessor
        // ** isInPreprocessor will be automatically reset at the begining
        //    of a new line in getnextChar()
        if (currentChar == '#')
            isInPreprocessor = true;

        if (isInPreprocessor) {
            appendCurrentChar();
            continue;
        }

        /* not in preprocessor ... */

        if (isImmediatelyPostComment) {
            isImmediatelyPostComment = false;
            isCharImmediatelyPostComment = true;
        }

        if (isImmediatelyPostLineComment) {
            isImmediatelyPostLineComment = false;
            isCharImmediatelyPostLineComment = true;
        }

        if (shouldBreakLineAfterComments) {
            shouldBreakLineAfterComments = false;
            shouldReparseCurrentChar = true;
            breakLine();
            continue;
        }

        // reset isImmediatelyPostHeader information
        if (isImmediatelyPostHeader) {
            isImmediatelyPostHeader = false;
            isCharImmediatelyPostHeader = true;

            // Make sure headers are broken from their succeeding blocks
            // (e.g.
            //     if (isFoo) DoBar();
            //  should become
            //     if (isFoo)
            //         DoBar;
            // )
            // But treat else if() as a special case which should not be broken!
            if (shouldBreakOneLineStatements) {
                // if may break 'else if()'s, ythen simply break the line

                if (shouldBreakElseIfs)
                    isInLineBreak = true;

                else {
                    // make sure 'else if()'s are not broken.

                    bool isInElseIf = false;
                    const string *upcomingHeader;

                    upcomingHeader = findHeader(headers);
                    if (currentHeader == &AS_ELSE && upcomingHeader == &AS_IF)
                        isInElseIf = true;

                    if (!isInElseIf)
                        isInLineBreak = true;  ////BUGFIX: SHOULD NOT BE breakLine() !!!
                }



            }
        }

        if (passedSemicolon) {
            passedSemicolon = false;
            if (parenStack->back() == 0) {
                shouldReparseCurrentChar = true;
                isInLineBreak = true;
                continue;
            }
        }

        if (passedColon) {
            passedColon = false;
            if (parenStack->back() == 0) {
                shouldReparseCurrentChar = true;
                isInLineBreak = true;
                continue;
            }
        }

        // Check if in template declaration, e.g. foo<bar> or foo<bar,fig>
        // If so, set isInTemplate to true
        //
        if (!isInTemplate && currentChar == '<') {
            int templateDepth = 0;
            const string *oper;
            for ( int i=charNum;
                    i< (int)currentLine.length();
                    i += (oper ? oper->length() : 1) ) {
                oper = ASBeautifier::findHeader(currentLine, i, operators);

                if (oper == &AS_LS) {
                    templateDepth++;
                } else if (oper == &AS_GR) {
                    templateDepth--;
                    if (templateDepth == 0) {
                        // this is a template!
                        //
                        isInTemplate = true;
                        break;
                    }
                } else if (oper == &AS_COMMA               // comma,     e.g. A<int, char>
                           || oper == &AS_BIT_AND       // reference, e.g. A<int&>
                           || oper == &AS_MULT          // pointer,   e.g. A<int*>
                           || oper == &AS_COLON_COLON)  // ::,        e.g. std::string
                {
                    continue;
                } else if (!isLegalNameChar(currentLine[i]) && !isWhiteSpace(currentLine[i])) {
                    // this is not a template -> leave...
                    //
                    isInTemplate = false;
                    break;
                }
            }
        }

        // handle parenthesies
        //
        if (currentChar == '(' || currentChar == '[' || (isInTemplate && currentChar == '<')) {
            parenStack->back()++;
        } else if (currentChar == ')' || currentChar == ']' || (isInTemplate && currentChar == '>')) {
            parenStack->back()--;
            if (isInTemplate && parenStack->back() == 0) {
                isInTemplate = false;
                isCharImmediatelyPostTemplate = true;
            }

            // check if this parenthesis closes a header, e.g. if (...), while (...)
            //
            if (isInHeader && parenStack->back() == 0) {
                isInHeader = false;
                isImmediatelyPostHeader = true;
            }

        }

        // handle brackets
        //
        BracketType bracketType = NULL_TYPE;

        if (currentChar == '{') {
            bracketType = getBracketType();
            foundPreDefinitionHeader = false;
            foundPreCommandHeader = false;

            bracketTypeStack->push_back(bracketType);
            preBracketHeaderStack->push_back(currentHeader);
            currentHeader = NULL;

            isPreviousBracketBlockRelated = !IS_A(bracketType, ARRAY_TYPE);
        } else if (currentChar == '}') {
            // if a request has been made to append a post block empty line,
            // but the block exists immediately before a closing bracket,
            // then there is not need for the post block empty line.
            //
            isAppendPostBlockEmptyLineRequested = false;

            if (!bracketTypeStack->empty()) {
                bracketType = bracketTypeStack->back();
                bracketTypeStack->pop_back();

                isPreviousBracketBlockRelated = !IS_A(bracketType, ARRAY_TYPE);
            }

            if (!preBracketHeaderStack->empty()) {
                currentHeader = preBracketHeaderStack->back();
                preBracketHeaderStack->pop_back();
            } else
                currentHeader = NULL;
        }

        if (!IS_A(bracketType, ARRAY_TYPE)) {

            if (currentChar == '{') {
                parenStack->push_back(0);
            } else if (currentChar == '}') {
                if (!parenStack->empty()) {
                    parenStack->pop_back();
                }
            }

            if (bracketFormatMode != NONE_MODE) {
                if (currentChar == '{') {
                    if ( ( bracketFormatMode == ATTACH_MODE
                            || bracketFormatMode == BDAC_MODE && bracketTypeStack->size()>=2
                            && IS_A((*bracketTypeStack)[bracketTypeStack->size()-2], COMMAND_TYPE) /*&& isInLineBreak*/)
                            && !isCharImmediatelyPostLineComment ) {
                        appendSpacePad();
                        if (!isCharImmediatelyPostComment // do not attach '{' to lines that end with /**/ comments.
                                && previousCommandChar != '{'
                                && previousCommandChar != '}'
                                && previousCommandChar != ';') // '}' , ';' chars added for proper handling of '{' immediately after a '}' or ';'
                            appendCurrentChar(false);
                        else
                            appendCurrentChar(true);
                        continue;
                    } else if (bracketFormatMode == BREAK_MODE
                               || bracketFormatMode == BDAC_MODE && bracketTypeStack->size()>=2
                               && IS_A((*bracketTypeStack)[bracketTypeStack->size()-2], DEFINITION_TYPE)) {
                        if ( shouldBreakOneLineBlocks || !IS_A(bracketType,  SINGLE_LINE_TYPE) )
                            breakLine();
                        appendCurrentChar();
                        continue;
                    }
                } else if (currentChar == '}') {
                    // bool origLineBreak = isInLineBreak;

                    // mark state of immediately after empty block
                    // this state will be used for locating brackets that appear immedately AFTER an empty block (e.g. '{} \n}').
                    if (previousCommandChar == '{')
                        isImmediatelyPostEmptyBlock = true;

                    if ( (!(previousCommandChar == '{' && isPreviousBracketBlockRelated) )          // this '{' does not close an empty block
                            && (shouldBreakOneLineBlocks || !IS_A(bracketType,  SINGLE_LINE_TYPE))  // astyle is allowed to break on line blocks
                            && !isImmediatelyPostEmptyBlock)                                        // this '}' does not immediately follow an empty block
                    {
                        breakLine();
                        appendCurrentChar();
                    } else {
                        if (!isCharImmediatelyPostComment)
                            isInLineBreak = false;
                        appendCurrentChar();
                        if (shouldBreakOneLineBlocks || !IS_A(bracketType,  SINGLE_LINE_TYPE))
                            shouldBreakLineAfterComments = true;
                    }

                    if (shouldBreakBlocks) {
                        isAppendPostBlockEmptyLineRequested =true;
                    }

                    continue;
                }
            }
        }

        if ( ( (previousCommandChar == '{'
                && isPreviousBracketBlockRelated)

                || (previousCommandChar == '}'
                    && !isImmediatelyPostEmptyBlock   // <--
                    && isPreviousBracketBlockRelated
                    && !isPreviousCharPostComment    // <-- Fixes wrongly appended newlines after '}' immediately after comments... 10/9/1999
                    && peekNextChar() != ' '))

                &&  (shouldBreakOneLineBlocks || !IS_A(bracketTypeStack->back(),  SINGLE_LINE_TYPE)) ) {
            isCharImmediatelyPostOpenBlock = (previousCommandChar == '{');
            isCharImmediatelyPostCloseBlock = (previousCommandChar == '}');

            previousCommandChar = ' ';
            isInLineBreak = true;  //<----
        }

        // reset block handling flags



        isImmediatelyPostEmptyBlock = false;

        // look for headers
        if (!isInTemplate) {
            if ( (newHeader = findHeader(headers)) != NULL) {
                foundClosingHeader = false;
                const string *previousHeader;

                // recognize closing headers of do..while, if..else, try..catch..finally
                if ( (newHeader == &AS_ELSE && currentHeader == &AS_IF)
                        || (newHeader == &AS_WHILE && currentHeader == &AS_DO)
                        || (newHeader == &AS_CATCH && currentHeader == &AS_TRY)
                        || (newHeader == &AS_CATCH && currentHeader == &AS_CATCH)
                        || (newHeader == &AS_FINALLY && currentHeader == &AS_TRY)
                        || (newHeader == &AS_FINALLY && currentHeader == &AS_CATCH) )
                    foundClosingHeader = true;

                previousHeader = currentHeader;
                currentHeader = newHeader;

                // If in ATTACH or LINUX bracket modes, attach closing headers (e.g. 'else', 'catch')
                // to their preceding bracket,
                // But do not perform the attachment if the shouldBreakClosingHeaderBrackets is set!
                if (!shouldBreakClosingHeaderBrackets && foundClosingHeader && (bracketFormatMode == ATTACH_MODE || bracketFormatMode == BDAC_MODE) && previousNonWSChar == '}') {
                    isInLineBreak = false;
                    appendSpacePad();

                    if (shouldBreakBlocks)
                        isAppendPostBlockEmptyLineRequested = false;
                }

                //Check if a template definition as been reached, e.g. template<class A>
                if (newHeader == &AS_TEMPLATE) {
                    isInTemplate = true;
                }

                // check if the found header is non-paren header
                isNonParenHeader = ( find(nonParenHeaders.begin(), nonParenHeaders.end(),
                                          newHeader) != nonParenHeaders.end() );
                appendSequence(*currentHeader);
                goForward(currentHeader->length() - 1);
                // if padding is on, and a paren-header is found
                // then add a space pad after it.
                if (shouldPadOperators && !isNonParenHeader)
                    appendSpacePad();


                // Signal that a header has been reached
                // *** But treat a closing while() (as in do...while)
                //     as if it where NOT a header since a closing while()
                //     should never have a block after it!
                if (!(foundClosingHeader && currentHeader == &AS_WHILE)) {
                    isInHeader = true;
                    if (isNonParenHeader) {
                        isImmediatelyPostHeader = true;
                        isInHeader = false;
                    }
                }

                if (currentHeader == &AS_IF && previousHeader == &AS_ELSE)
                    isInLineBreak = false;

                if (shouldBreakBlocks) {
                    if (previousHeader == NULL
                            && !foundClosingHeader
                            && !isCharImmediatelyPostOpenBlock) {
                        isPrependPostBlockEmptyLineRequested = true;
                    }

                    if (currentHeader == &AS_ELSE
                            || currentHeader == &AS_CATCH
                            || currentHeader == &AS_FINALLY
                            || foundClosingHeader) {
                        isPrependPostBlockEmptyLineRequested = false;
                    }

                    if (shouldBreakClosingHeaderBlocks
                            &&  isCharImmediatelyPostCloseBlock) {
                        isPrependPostBlockEmptyLineRequested = true;
                    }

                }

                continue;
            } else if ( (newHeader = findHeader(preDefinitionHeaders)) != NULL) {
                foundPreDefinitionHeader = true;
                appendSequence(*newHeader);
                goForward(newHeader->length() - 1);

                if (shouldBreakBlocks)
                    isPrependPostBlockEmptyLineRequested = true;

                continue;
            } else if ( (newHeader = findHeader(preCommandHeaders)) != NULL) {
                foundPreCommandHeader = true;
                appendSequence(*newHeader);
                goForward(newHeader->length() - 1);

                continue;
            }
        }

        if (previousNonWSChar == '}' || currentChar == ';') {
            if (shouldBreakOneLineStatements && currentChar == ';'
                    && (shouldBreakOneLineBlocks || !IS_A(bracketTypeStack->back(),  SINGLE_LINE_TYPE))) {
                passedSemicolon = true;
            }

            if (shouldBreakBlocks && currentHeader != NULL && parenStack->back() == 0) {
                isAppendPostBlockEmptyLineRequested = true;
            }

            if (currentChar != ';')
                currentHeader = NULL; //DEVEL: is this ok?

            foundQuestionMark = false;
            foundPreDefinitionHeader = false;
            foundPreCommandHeader = false;
            isInPotentialCalculation = false;

        }

        if (currentChar == ':'
                && shouldBreakOneLineStatements
                && !foundQuestionMark // not in a ... ? ... : ... sequence
                && !foundPreDefinitionHeader // not in a definition block (e.g. class foo : public bar
                && previousCommandChar != ')' // not immediately after closing paren of a method header, e.g. ASFormatter::ASFormatter(...) : ASBeautifier(...)
                && previousChar != ':' // not part of '::'
                && peekNextChar() != ':') // not part of '::'
        {
            passedColon = true;
            if (shouldBreakBlocks)
                isPrependPostBlockEmptyLineRequested = true;
        }

        if (currentChar == '?')
            foundQuestionMark = true;

        if (shouldPadOperators) {
            if ((newHeader = findHeader(operators)) != NULL) {
                bool shouldPad = (newHeader != &AS_COLON_COLON
                                  && newHeader != &AS_PAREN_PAREN
                                  && newHeader != &AS_BLPAREN_BLPAREN
                                  && newHeader != &AS_PLUS_PLUS
                                  && newHeader != &AS_MINUS_MINUS
                                  && newHeader != &AS_NOT
                                  && newHeader != &AS_BIT_NOT
                                  && newHeader != &AS_ARROW
                                  && newHeader != &AS_OPERATOR
                                  && !(newHeader == &AS_MINUS && isInExponent())
                                  && !(newHeader == &AS_PLUS && isInExponent())
                                  && previousOperator != &AS_OPERATOR
                                  && !((newHeader == &AS_MULT || newHeader == &AS_BIT_AND)
                                       && isPointerOrReference())
                                  && !( (isInTemplate || isCharImmediatelyPostTemplate)
                                        && (newHeader == &AS_LS || newHeader == &AS_GR))
                                 );

                if (!isInPotentialCalculation)
                    if (find(assignmentOperators.begin(), assignmentOperators.end(), newHeader)
                            != assignmentOperators.end())
                        isInPotentialCalculation = true;

                // pad before operator
                if (shouldPad
                        && !(newHeader == &AS_COLON && !foundQuestionMark)
                        && newHeader != &AS_SEMICOLON
                        && newHeader != &AS_COMMA)
                    appendSpacePad();
                appendSequence(*newHeader);
                goForward(newHeader->length() - 1);

                // since this block handles '()' and '[]',
                // the parenStack must be updated here accordingly!
                if (newHeader == &AS_PAREN_PAREN
                        || newHeader == &AS_BLPAREN_BLPAREN)
                    parenStack->back()--;

                currentChar = (*newHeader)[newHeader->length() - 1];
                // pad after operator
                // but do not pad after a '-' that is a unary-minus.
                if ( shouldPad && !(newHeader == &AS_MINUS && isUnaryMinus()) )
                    appendSpacePad();

                previousOperator = newHeader;
                continue;
            }
        }
        if (shouldPadParenthesies) {
            if (currentChar == '(' || currentChar == '[' ) {
                char peekedChar = peekNextChar();

                isInPotentialCalculation = true;
                appendCurrentChar();
                if (!(currentChar == '(' && peekedChar == ')')
                        && !(currentChar == '[' && peekedChar == ']'))
                    appendSpacePad();
                continue;
            } else if (currentChar == ')' || currentChar == ']') {
                char peekedChar = peekNextChar();

                if (!(previousChar == '(' && currentChar == ')')
                        && !(previousChar == '[' && currentChar == ']'))
                    appendSpacePad();

                appendCurrentChar();

                if (peekedChar != ';' && peekedChar != ',' && peekedChar != '.'
                        && !(currentChar == ']' && peekedChar == '['))
                    appendSpacePad();
                continue;
            }
        }

        appendCurrentChar();
    }

    // return a beautified (i.e. correctly indented) line.

    string beautifiedLine;
    int readyFormattedLineLength = trim(readyFormattedLine).length();

    if (prependEmptyLine
            && readyFormattedLineLength > 0
            && previousReadyFormattedLineLength > 0) {
        isLineReady = true; // signal that a readyFormattedLine is still waiting
        beautifiedLine = beautify("");
    } else {
        isLineReady = false;
        beautifiedLine = beautify(readyFormattedLine);
    }

    prependEmptyLine = false;
    previousReadyFormattedLineLength = readyFormattedLineLength;

    return beautifiedLine;

}


/**
* check if there are any indented lines ready to be read by nextLine()
*
* @return    are there any indented lines ready?
*/
bool ASFormatter::hasMoreLines() const {
    if (!isFormattingEnabled())
        return ASBeautifier::hasMoreLines();
    else
        return !endOfCodeReached;
}

/**
 * check if formatting options are enabled, in addition to indentation.
 *
 * @return     are formatting options enabled?
 */
bool ASFormatter::isFormattingEnabled() const {
    return (bracketFormatMode != NONE_MODE
            || shouldPadOperators
            || shouldConvertTabs);
}

/**
 * set the bracket formatting mode.
 * options:
 *    astyle::NONE_MODE     no formatting of brackets.
 *    astyle::ATTACH_MODE   Java, K&R style bracket placement.
 *    astyle::BREAK_MODE    ANSI C/C++ style bracket placement.
 *
 * @param mode         the bracket formatting mode.
 */
void ASFormatter::setBracketFormatMode(BracketMode mode) {
    bracketFormatMode = mode;
}

/**
 * set closing header bracket breaking mode
 * options:
 *    true     brackets just before closing headers (e.g. 'else', 'catch')
 *             will be broken, even if standard brackets are attached.
 *    false    closing header brackets will be treated as standard brackets.
 *
 * @param state         the closing header bracket breaking mode.
 */
void ASFormatter::setBreakClosingHeaderBracketsMode(bool state) {
    shouldBreakClosingHeaderBrackets = state;
}

/**
 * set 'else if()' breaking mode
 * options:
 *    true     'else' headers will be broken from their succeeding 'if' headers.
 *    false    'else' headers will be attached to their succeeding 'if' headers.
 *
 * @param state         the 'else if()' breaking mode.
 */
void ASFormatter::setBreakElseIfsMode(bool state) {
    shouldBreakElseIfs = state;
}

/**
 * set operator padding mode.
 * options:
 *    true     statement operators will be padded with spaces around them.
 *    false    statement operators will not be padded.
 *
 * @param state         the padding mode.
 */
void ASFormatter::setOperatorPaddingMode(bool state) {
    shouldPadOperators = state;
}

/**
* set parentheies padding mode.
* options:
*    true     statement parenthesies will be padded with spaces around them.
*    false    statement parenthesies will not be padded.
*
* @param state         the padding mode.
*/
void ASFormatter::setParenthesisPaddingMode(bool state) {
    shouldPadParenthesies = state;
}

/**
 * set option to break/not break one-line blocks
 *
 * @param state        true = break, false = don't break.
 */
void ASFormatter::setBreakOneLineBlocksMode(bool state) {
    shouldBreakOneLineBlocks = state;
}

/**
 * set option to break/not break lines consisting of multiple statements.
 *
 * @param state        true = break, false = don't break.
 */
void ASFormatter::setSingleStatementsMode(bool state) {
    shouldBreakOneLineStatements = state;
}

/**
 * set option to convert tabs to spaces.
 *
 * @param state        true = convert, false = don't convert.
 */
void ASFormatter::setTabSpaceConversionMode(bool state) {
    shouldConvertTabs = state;
}


/**
 * set option to break unrelated blocks of code with empty lines.
 *
 * @param state        true = convert, false = don't convert.
 */
void ASFormatter::setBreakBlocksMode(bool state) {
    shouldBreakBlocks = state;
}

/**
 * set option to break closing header blocks of code (such as 'else', 'catch', ...) with empty lines.
 *
 * @param state        true = convert, false = don't convert.
 */
void ASFormatter::setBreakClosingHeaderBlocksMode(bool state) {
    shouldBreakClosingHeaderBlocks = state;
}

/**
 * check if a specific sequence exists in the current placement of the current line
 *
 * @return             whether sequence has been reached.
 * @param sequence     the sequence to be checked
 */
bool ASFormatter::isSequenceReached(const string &sequence) const {
    return currentLine.COMPARE(charNum, sequence.length(), sequence) == 0;

}

/**
 * jump over several characters.
 *
 * @param i       the number of characters to jump over.
 */
void ASFormatter::goForward(int i) {
    while (--i >= 0)
        getNextChar();
}

/**
* peek at the next unread character.
*
* @return     the next unread character.
*/
char ASFormatter::peekNextChar() const {
    int peekNum = charNum + 1;
    int len = currentLine.length();
    char ch = ' ';

    while (peekNum < len) {
        ch = currentLine[peekNum++];
        if (!isWhiteSpace(ch))
            return ch;
    }

    if (shouldConvertTabs && ch == '\t')
        ch = ' ';

    return ch;
}

/**
* check if current placement is before a comment or line-comment
*
* @return     is before a comment or line-comment.
*/
bool ASFormatter::isBeforeComment() const {
    int peekNum = charNum + 1;
    int len = currentLine.length();
    // char ch = ' ';
    bool foundComment = false;

    for (peekNum = charNum + 1;
            peekNum < len && isWhiteSpace(currentLine[peekNum]);
            ++peekNum)
        ;

    if (peekNum < len)
        foundComment = ( currentLine.COMPARE(peekNum, 2, AS_OPEN_COMMENT) == 0
                         || currentLine.COMPARE(peekNum, 2, AS_OPEN_LINE_COMMENT) == 0 );

    return foundComment;
}

/**
* get the next character, increasing the current placement in the process.
* the new character is inserted into the variable currentChar.
*
* @return   whether succeded to recieve the new character.
*/
bool ASFormatter::getNextChar() {
    isInLineBreak = false;
    bool isAfterFormattedWhiteSpace = false;

    if (shouldPadOperators && !isInComment && !isInLineComment
            && !isInQuote && !doesLineStartComment && !isInPreprocessor
            && !isBeforeComment()) {
        int len = formattedLine.length();
        if (len > 0 && isWhiteSpace(formattedLine[len-1]))
            isAfterFormattedWhiteSpace = true;
    }

    previousChar = currentChar;
    if (!isWhiteSpace(currentChar)) {
        previousNonWSChar = currentChar;
        if (!isInComment && !isInLineComment && !isInQuote
                && !isSequenceReached(AS_OPEN_COMMENT)
                && !isSequenceReached(AS_OPEN_LINE_COMMENT) )
            previousCommandChar = previousNonWSChar;
    }

    int currentLineLength = currentLine.length();

    if (charNum+1 < currentLineLength
            && (!isWhiteSpace(peekNextChar()) || isInComment || isInLineComment)) {
        currentChar = currentLine[++charNum];
        if (isAfterFormattedWhiteSpace)
            while (isWhiteSpace(currentChar) && charNum+1 < currentLineLength)
                currentChar = currentLine[++charNum];

        if (shouldConvertTabs && currentChar == '\t')
            currentChar = ' ';

        return true;
    } else {
        if (sourceIterator->hasMoreLines()) {
            currentLine = sourceIterator->nextLine();
            if (currentLine.length() == 0) {
                /*think*/ currentLine = string(" ");
            }

            // unless reading in the first line of the file,
            // break a new line.
            if (!isVirgin)
                isInLineBreak = true;
            else
                isVirgin = false;

            if (isInLineComment)
                isImmediatelyPostLineComment = true;
            isInLineComment = false;

            trimNewLine();
            currentChar = currentLine[charNum];

            // check if is in preprocessor right after the line break and line trimming
            if (previousNonWSChar != '\\')
                isInPreprocessor = false;

            if (shouldConvertTabs && currentChar == '\t')
                currentChar = ' ';

            return true;
        } else {
            endOfCodeReached = true;
            return false;
        }
    }
}

/**
* jump over the leading white space in the current line,
* IF the line does not begin a comment or is in a preprocessor definition.
*/
void ASFormatter::trimNewLine() {
    int len = currentLine.length();
    charNum = 0;

    if (isInComment || isInPreprocessor)
        return;

    while (isWhiteSpace(currentLine[charNum]) && charNum+1 < len)
        ++charNum;

    doesLineStartComment = false;
    if (isSequenceReached(string("/*"))) {
        charNum = 0;
        doesLineStartComment = true;
    }
}

/**
 * append a character to the current formatted line.
 * Unless disabled (via canBreakLine == false), first check if a 
 * line-break has been registered, and if so break the 
 * formatted line, and only then append the character into
 * the next formatted line.
 *
 * @param ch               the character to append.
 * @param canBreakLine     if true, a registered line-break
 */
void ASFormatter::appendChar(char ch, bool canBreakLine) {
    if (canBreakLine && isInLineBreak)
        breakLine();
    formattedLine.append(1, ch);
}

/**
 * append the CURRENT character (curentChar)to the current
 * formatted line. Unless disabled (via canBreakLine == false),
 * first check if a line-break has been registered, and if so
 * break the formatted line, and only then append the character
 * into the next formatted line.
 *
 * @param canBreakLine     if true, a registered line-break
 */
void ASFormatter::appendCurrentChar(bool canBreakLine) {
    appendChar(currentChar, canBreakLine);
}

/**
 * append a string sequence to the current formatted line.
 * Unless disabled (via canBreakLine == false), first check if a 
 * line-break has been registered, and if so break the 
 * formatted line, and only then append the sequence into
 * the next formatted line.
 *
 * @param sequence         the sequence to append.
 * @param canBreakLine     if true, a registered line-break
 */
void ASFormatter::appendSequence(const string &sequence, bool canBreakLine) {
    if (canBreakLine && isInLineBreak)
        breakLine();
    formattedLine.append(sequence);
}

/**
 * append a space to the current formattedline, UNLESS the 
 * last character is already a white-space character.
 */
void ASFormatter::appendSpacePad() {
    int len = formattedLine.length();
    if (len == 0 || !isWhiteSpace(formattedLine[len-1]))
        formattedLine.append(1, ' ');
}

/**
 * register a line break for the formatted line.
 */
void ASFormatter::breakLine() {
    isLineReady = true;
    isInLineBreak = false;

    // queue an empty line prepend request if one exists
    prependEmptyLine = isPrependPostBlockEmptyLineRequested;

    readyFormattedLine =  formattedLine;
    if (isAppendPostBlockEmptyLineRequested) {
        isAppendPostBlockEmptyLineRequested = false;
        isPrependPostBlockEmptyLineRequested = true;
    } else {
        isPrependPostBlockEmptyLineRequested = false;
    }

    formattedLine = "";
}

/**
 * check if the currently reached open-bracket (i.e. '{')
 * opens a:
 * - a definition type block (such as a class or namespace),
 * - a command block (such as a method block)
 * - a static array
 * this method takes for granted that the current character
 * is an opening bracket.
 *
 * @return    the type of the opened block.
 */
BracketType ASFormatter::getBracketType() const {
    BracketType returnVal;

    if (foundPreDefinitionHeader)
        returnVal = DEFINITION_TYPE;
    else {
        bool isCommandType;
        isCommandType = ( foundPreCommandHeader
                          || ( currentHeader != NULL && isNonParenHeader )
                          || ( previousCommandChar == ')' )
                          || ( previousCommandChar == ':' && !foundQuestionMark )
                          || ( previousCommandChar == ';' )
                          || ( ( previousCommandChar == '{' ||  previousCommandChar == '}')
                               && isPreviousBracketBlockRelated ) );

        returnVal = (isCommandType ? COMMAND_TYPE : ARRAY_TYPE);
    }

    if (isOneLineBlockReached())
        returnVal = (BracketType) (returnVal | SINGLE_LINE_TYPE);

    return returnVal;
}

/**
 * check if the currently reached  '*' or '&' character is
 * a pointer-or-reference symbol, or another operator.
 * this method takes for granted that the current character
 * is either a '*' or '&'.
 *
 * @return        whether current character is a reference-or-pointer 
 */
bool ASFormatter::isPointerOrReference() const {
    bool isPR;
    isPR = ( !isInPotentialCalculation
             || IS_A(bracketTypeStack->back(), DEFINITION_TYPE)
             || (!isLegalNameChar(previousNonWSChar)
                 && previousNonWSChar != ')'
                 && previousNonWSChar != ']')
           );

    if (!isPR) {
        char nextChar = peekNextChar();
        isPR |= (!isWhiteSpace(nextChar)
                 && nextChar != '-'
                 && nextChar != '('
                 && nextChar != '['
                 && !isLegalNameChar(nextChar));
    }

    return isPR;
}


/**
 * check if the currently reached '-' character is
 * a unary minus
 * this method takes for granted that the current character
 * is a '-'.
 *
 * @return        whether the current '-' is a unary minus.
 */
bool ASFormatter::isUnaryMinus() const {
    return ( (previousOperator == &AS_RETURN || !isalnum(previousCommandChar))
             && previousCommandChar != '.'
             && previousCommandChar != ')'
             && previousCommandChar != ']' );
}


/**
 * check if the currently reached '-' or '+' character is
 * part of an exponent, i.e. 0.2E-5.
 * this method takes for granted that the current character
 * is a '-' or '+'.
 *
 * @return        whether the current '-' is in an exponent.
 */
bool ASFormatter::isInExponent() const {
    int formattedLineLength = formattedLine.length();
    if (formattedLineLength >= 2) {
        char prevPrevFormattedChar = formattedLine[formattedLineLength - 2];
        char prevFormattedChar = formattedLine[formattedLineLength - 1];

        return ( (prevFormattedChar == 'e' || prevFormattedChar == 'E')
                 && (prevPrevFormattedChar == '.' || isdigit(prevPrevFormattedChar)) );
    } else
        return false;
}

/**
 * check if a one-line bracket has been reached,
 * i.e. if the currently reached '{' character is closed
 * with a complimentry '}' elsewhere on the current line.
 *
 * @return        has a one-line bracket been reached?
 */
bool ASFormatter::isOneLineBlockReached() const {
    bool isInComment = false;
    bool isInQuote = false;
    int bracketCount = 1;
    int currentLineLength = currentLine.length();
    int i = 0;
    char ch = ' ';
    char quoteChar = ' ';

    for (i = charNum + 1; i < currentLineLength; ++i) {
        ch = currentLine[i];

        if (isInComment) {
            if (currentLine.COMPARE(i, 2, "*/") == 0) {
                isInComment = false;
                ++i;
            }
            continue;
        }

        if (ch == '\\') {
            ++i;
            continue;
        }

        if (isInQuote) {
            if (ch == quoteChar)
                isInQuote = false;
            continue;
        }

        if (ch == '"' || ch == '\'') {
            isInQuote = true;
            quoteChar = ch;
            continue;
        }

        if (currentLine.COMPARE(i, 2, "//") == 0)
            break;

        if (currentLine.COMPARE(i, 2, "/*") == 0) {
            isInComment = true;
            ++i;
            continue;
        }

        if (ch == '{')
            ++bracketCount;
        else if (ch == '}')
            --bracketCount;

        if(bracketCount == 0)
            return true;
    }

    return false;
}


/**
 * check if one of a set of headers has been reached in the
 * current position of the current line.
 *
 * @return             a pointer to the found header. Or a NULL if no header has been reached.
 * @param headers      a vector of headers
 * @param checkBoundry 
 */
const string *ASFormatter::findHeader(const vector<const string*> &headers, bool checkBoundry) {
    return ASBeautifier::findHeader(currentLine, charNum, headers, checkBoundry);
}



#ifdef USES_NAMESPACE
}
#endif


