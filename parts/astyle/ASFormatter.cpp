/*
 * Copyright (c) 1998,1999 Tal Davidson. All rights reserved.
 *
 * ASFormatter.cpp   (18 September 1999)
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

namespace astyle
{
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
ASFormatter::ASFormatter()
{
    staticInit();

    preBracketHeaderStack = NULL;
    bracketTypeStack = NULL;
    parenStack = NULL;

    sourceIterator = NULL;
    bracketFormatMode = NONE_MODE;
    shouldPadOperators = false;
    shouldPadParenthesies = false;
    shouldBreakOneLineBlocks = true;
    shouldBreakSemicolons = true;
}

/**
 * Destructor of ASFormatter
 */
ASFormatter::~ASFormatter()
{
    DELETE_CONTAINER( preBracketHeaderStack );
}

/**
 * initialization of static data of ASFormatter.
 */
void ASFormatter::staticInit()
{
    if (calledInitStatic)
        return;

    calledInitStatic = true;

    headers.push_back(&IF);
    headers.push_back(&ELSE);
    headers.push_back(&DO);
    headers.push_back(&WHILE);
    headers.push_back(&FOR);
    headers.push_back(&SYNCHRONIZED);
    headers.push_back(&TRY);
    headers.push_back(&CATCH);
    headers.push_back(&FINALLY);
    headers.push_back(&SWITCH);
    headers.push_back(&TEMPLATE);

    nonParenHeaders.push_back(&ELSE);
    nonParenHeaders.push_back(&DO);
    nonParenHeaders.push_back(&TRY);
    nonParenHeaders.push_back(&FINALLY);
    nonParenHeaders.push_back(&TEMPLATE);

    preDefinitionHeaders.push_back(&CLASS);
    preDefinitionHeaders.push_back(&INTERFACE);
    preDefinitionHeaders.push_back(&NAMESPACE);
    preDefinitionHeaders.push_back(&STRUCT);

    preCommandHeaders.push_back(&EXTERN);
    preCommandHeaders.push_back(&THROWS);
    preCommandHeaders.push_back(&CONST);

    preprocessorHeaders.push_back(&BAR_DEFINE);
    preprocessorHeaders.push_back(&BAR_INCLUDE);
    preprocessorHeaders.push_back(&BAR_IF); // #if or #ifdef
    preprocessorHeaders.push_back(&BAR_EL); // #else or #elif
    preprocessorHeaders.push_back(&BAR_ENDIF);

    operators.push_back(&PLUS_ASSIGN);
    operators.push_back(&MINUS_ASSIGN);
    operators.push_back(&MULT_ASSIGN);
    operators.push_back(&DIV_ASSIGN);
    operators.push_back(&MOD_ASSIGN);
    operators.push_back(&OR_ASSIGN);
    operators.push_back(&AND_ASSIGN);
    operators.push_back(&XOR_ASSIGN);
    operators.push_back(&EQUAL);
    operators.push_back(&PLUS_PLUS);
    operators.push_back(&MINUS_MINUS);
    operators.push_back(&NOT_EQUAL);
    operators.push_back(&GR_EQUAL);
    operators.push_back(&GR_GR_GR_ASSIGN);
    operators.push_back(&GR_GR_ASSIGN);
    operators.push_back(&GR_GR_GR);
    operators.push_back(&GR_GR);
    operators.push_back(&LS_EQUAL);
    operators.push_back(&LS_LS_LS_ASSIGN);
    operators.push_back(&LS_LS_ASSIGN);
    operators.push_back(&LS_LS_LS);
    operators.push_back(&LS_LS);
    operators.push_back(&ARROW);
    operators.push_back(&AND);
    operators.push_back(&OR);
    operators.push_back(&COLON_COLON);
    operators.push_back(&PAREN_PAREN);
    operators.push_back(&BLPAREN_BLPAREN);

    operators.push_back(&PLUS);
    operators.push_back(&MINUS);
    operators.push_back(&MULT);
    operators.push_back(&DIV);
    operators.push_back(&MOD);
    operators.push_back(&QUESTION);
    operators.push_back(&COLON);
    operators.push_back(&ASSIGN);
    operators.push_back(&LS);
    operators.push_back(&GR);
    operators.push_back(&NOT);
    operators.push_back(&BIT_OR);
    operators.push_back(&BIT_AND);
    operators.push_back(&BIT_NOT);
    operators.push_back(&BIT_XOR);
    operators.push_back(&OPERATOR);
    operators.push_back(&COMMA);
    operators.push_back(&SEMICOLON);
    operators.push_back(&RETURN);


    assignmentOperators.push_back(&PLUS_ASSIGN);
    assignmentOperators.push_back(&MINUS_ASSIGN);
    assignmentOperators.push_back(&MULT_ASSIGN);
    assignmentOperators.push_back(&DIV_ASSIGN);
    assignmentOperators.push_back(&MOD_ASSIGN);
    assignmentOperators.push_back(&XOR_ASSIGN);
    assignmentOperators.push_back(&OR_ASSIGN);
    assignmentOperators.push_back(&AND_ASSIGN);
    assignmentOperators.push_back(&GR_GR_GR_ASSIGN);
    assignmentOperators.push_back(&LS_LS_LS_ASSIGN);
    assignmentOperators.push_back(&ASSIGN);
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
 * @param iter     a pointer to the DYNAMICALLY CREATED ASSourceIterator object.
 */
void ASFormatter::init(ASSourceIterator *si)
{
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
}

/**
 * get the next formatted line.
 *
 * @return    formatted line.
 */

string ASFormatter::nextLine()
{
    const string *newHeader;
    bool isCharImmediatelyPostComment = false;
    bool isPreviousCharPostComment = false;
    bool isInVirginLine = isVirgin;

    if (!isFormattingEnabled())
        return ASBeautifier::nextLine();

    while (!isLineReady)
    {
        if (shouldReparseCurrentChar)
            shouldReparseCurrentChar = false;
        else if (!getNextChar())
        {
            breakLine();
            return beautify(readyFormattedLine);
        }
        else // stuff to do when reading a new character...
        {
            // make sure that a virgin '{' at the begining ofthe file will be treated as a block...
            if (isInVirginLine && currentChar == '{')
                previousCommandChar = '{';
            isPreviousCharPostComment = isCharImmediatelyPostComment;
            isCharImmediatelyPostComment = false;
        }

        if (isInLineComment)
        {
            appendCurrentChar();

            // explicitely break a line when a line comment's end is found.
            if (/*bracketFormatMode == ATTACH_MODE &&*/ charNum+1 == currentLine.length())
            {
                isInLineBreak = true;
                isInLineComment = false;
                isImmediatelyPostComment = true;
                currentChar = '{';
            }
            continue;
        }
        else if (isInComment)
        {
            if (isSequenceReached(CLOSE_COMMENT))
            {
                isInComment = false;
                isImmediatelyPostComment = true;
                appendSequence(CLOSE_COMMENT);
                goForward(1);
            }
            else
                appendCurrentChar();

            continue;
        }

        // not in line comment or comment

        else if (isInQuote)
        {
            if (isSpecialChar)
            {
                isSpecialChar = false;
                appendCurrentChar();
            }
            else if (currentChar == '\\')
            {
                isSpecialChar = true;
                appendCurrentChar();
            }
            else if (quoteChar == currentChar)
            {
                isInQuote = false;
                appendCurrentChar();
            }
            else
            {
                appendCurrentChar();
            }

            continue;
        }

        // handle white space - needed to simplify the rest.
        if (isWhiteSpace(currentChar) || isInPreprocessor)
        {
            appendCurrentChar();
            continue;
        }

        /* not in MIDDLE of quote or comment or white-space of any type ... */

        if (isSequenceReached(OPEN_LINE_COMMENT))
        {
            isInLineComment = true;
            appendSequence(OPEN_LINE_COMMENT);
            goForward(1);
            continue;
        }
        else if (isSequenceReached(OPEN_COMMENT))
        {
            isInComment = true;
            appendSequence(OPEN_COMMENT);
            goForward(1);
            continue;
        }
        else if (currentChar == '"' || currentChar == '\'')
        {
            isInQuote = true;
            quoteChar = currentChar;
            appendCurrentChar();
            continue;
        }

        /* not in quote or comment or white-space of any type ... */

        if (isImmediatelyPostComment)
        {
            isImmediatelyPostComment = false;
            isCharImmediatelyPostComment = true;
        }

        if (shouldBreakLineAfterComments)
        {
            shouldBreakLineAfterComments = false;
            shouldReparseCurrentChar = true;
            breakLine();
            continue;
        }

        if (passedSemicolon)
        {
            passedSemicolon = false;
            if (parenStack->back() == 0)
            {
                shouldReparseCurrentChar = true;
                isInLineBreak = true;
                continue;
            }
        }

        if (passedColon)
        {
            passedColon = false;
            if (parenStack->back() == 0)
            {
                shouldReparseCurrentChar = true;
                if (bracketFormatMode == BREAK_MODE)
                    isInLineBreak = true;
                else
                    breakLine();
                continue;
            }
        }

        if (currentChar == '(' || currentChar == '[' || (isInTemplate && currentChar == '<'))
        {
            parenStack->back()++;
        }
        else if (currentChar == ')' || currentChar == ']' || (isInTemplate && currentChar == '>'))
        {
            parenStack->back()--;
            if (isInTemplate && parenStack->back() == 0)
            {
                isInTemplate = false;
            }
        }

        BracketType bracketType;

        if (currentChar == '{')
        {
            bracketType = getBracketType();
            foundPreDefinitionHeader = false;
            foundPreCommandHeader = false;

            bracketTypeStack->push_back(bracketType);
            preBracketHeaderStack->push_back(currentHeader);
            //foundOneLineBlock = isOneLineBlockReached();

            isPreviousBracketBlockRelated = !IS_A(bracketType, ARRAY_TYPE);
        }
        else if (currentChar == '}')
        {
            if (!bracketTypeStack->empty())
            {
                bracketType = bracketTypeStack->back();
                bracketTypeStack->pop_back();

                isPreviousBracketBlockRelated = !IS_A(bracketType, ARRAY_TYPE);
            }

            if (!preBracketHeaderStack->empty())
            {
                currentHeader = preBracketHeaderStack->back();
                preBracketHeaderStack->pop_back();
            }
            else
                currentHeader = NULL;
        }

        if (!IS_A(bracketType, ARRAY_TYPE))
        {

            if (currentChar == '{')
            {
                parenStack->push_back(0);
            }
            else if (currentChar == '}')
            {
                if (!parenStack->empty())
                {
                    parenStack->pop_back();
                }
            }

            if (bracketFormatMode != NONE_MODE)
            {
                if (currentChar == '{')
                {
                    if (bracketFormatMode == ATTACH_MODE
                            || bracketFormatMode == BDAC_MODE && bracketTypeStack->size()>=2
                            && IS_A((*bracketTypeStack)[bracketTypeStack->size()-2], COMMAND_TYPE) /*&& isInLineBreak*/)
                    {
                        appendSpacePad();
                        if (previousCommandChar != '{' && previousCommandChar != '}' ) // '}' charadded for proper handling of '{' immediately after a '}' 10/9/1999
                            appendCurrentChar(false);
                        else
                            appendCurrentChar(true);
                        continue;
                    }
                    else if (bracketFormatMode == BREAK_MODE
                             || bracketFormatMode == BDAC_MODE && bracketTypeStack->size()>=2
                             && IS_A((*bracketTypeStack)[bracketTypeStack->size()-2], DEFINITION_TYPE))
                    {
                        if ( shouldBreakOneLineBlocks || !IS_A(bracketType,  SINGLE_LINE_TYPE) )
                            breakLine();
                        appendCurrentChar();
                        continue;
                    }
                }
                else if (currentChar == '}')
                {
                    bool origLineBreak = isInLineBreak;

                    if ( (!(previousCommandChar == '{' && isPreviousBracketBlockRelated) )
                            && (shouldBreakOneLineBlocks || !IS_A(bracketType,  SINGLE_LINE_TYPE)) )
                    {
                        breakLine();
                        appendCurrentChar();
                    }
                    else
                    {
                        if (!isCharImmediatelyPostComment)
                            isInLineBreak = false;
                        appendCurrentChar();
                        if (shouldBreakOneLineBlocks || !IS_A(bracketType,  SINGLE_LINE_TYPE))
                            shouldBreakLineAfterComments = true;
                    }
                    continue;
                }
            }
        }

        if ( ((previousCommandChar == '{'
                && isPreviousBracketBlockRelated)
                ||(previousCommandChar == '}'
                   && isPreviousBracketBlockRelated
                   && !isPreviousCharPostComment // <-- Fixes wrongly appended newlines after '}' immediately after comments... 10/9/1999
                   && peekNextChar() != ' '))
                &&  (shouldBreakOneLineBlocks || !IS_A(bracketTypeStack->back(),  SINGLE_LINE_TYPE)) )
        {
            previousCommandChar = ' ';
            isInLineBreak = true;
        }

        // look for headers
        if (!isInTemplate)
        {
            if ( (newHeader = findHeader(headers)) != NULL)
            {
                bool foundClosingHeader = false;

                // recognize closing headers of do..while and if..else
                if ( (newHeader == &ELSE && currentHeader == &IF)
                        || (newHeader == &WHILE && currentHeader == &DO)
                        || (newHeader == &CATCH && currentHeader == &TRY)
                        || (newHeader == &FINALLY && currentHeader == &CATCH) )
                    foundClosingHeader = true;

                currentHeader = newHeader;

                if (foundClosingHeader && (bracketFormatMode == ATTACH_MODE || bracketFormatMode == BDAC_MODE) && previousNonWSChar == '}')
                {
                    isInLineBreak = false;
                    appendSpacePad();
                }

                //check if a template as been reached
                if (newHeader == &TEMPLATE)
                {
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


                continue;
            }
            else if ( (newHeader = findHeader(preDefinitionHeaders)) != NULL)
            {
                foundPreDefinitionHeader = true;
                appendSequence(*newHeader);
                goForward(newHeader->length() - 1);

                continue;
            }
            else if ( (newHeader = findHeader(preCommandHeaders)) != NULL)
            {
                foundPreCommandHeader = true;
                appendSequence(*newHeader);
                goForward(newHeader->length() - 1);

                continue;
            }
            else if ( (newHeader = findHeader(preprocessorHeaders)) != NULL)
                isInPreprocessor = true;
        }

        if (previousNonWSChar == '}' || currentChar == ';')
        {
            if (shouldBreakSemicolons && currentChar == ';'
                    && (shouldBreakOneLineBlocks || !IS_A(bracketTypeStack->back(),  SINGLE_LINE_TYPE)))
            {
                passedSemicolon = true;
            }

            currentHeader = NULL;
            foundQuestionMark = false;
            foundPreDefinitionHeader = false;
            foundPreCommandHeader = false;
            isInPotentialCalculation = false;
        }

        if (currentChar == ':'
                && shouldBreakSemicolons
                && !foundQuestionMark // not in a ... ? ... : ... sequence
                && !foundPreDefinitionHeader // not in a definition block (e.g. class foo : public bar
                && previousCommandChar != ')' // not immediately after closing paren of a method header, e.g. ASFormatter::ASFormatter(...) : ASBeautifier(...)
                && previousChar != ':' // not part of '::'
                && peekNextChar() != ':') // not part of '::'
        {
            passedColon = true;
        }

        if (currentChar == '?')
            foundQuestionMark = true;

        if (shouldPadOperators)
        {
            if ((newHeader = findHeader(operators/*, false*/)) != NULL)
            {
                bool shouldPad = (newHeader != &COLON_COLON
                                  && newHeader != &PAREN_PAREN
                                  && newHeader != &BLPAREN_BLPAREN
                                  && newHeader != &PLUS_PLUS
                                  && newHeader != &MINUS_MINUS
                                  && newHeader != &NOT
                                  && newHeader != &BIT_NOT
                                  && newHeader != &ARROW
                                  && newHeader != &OPERATOR
                                  && !(newHeader == &MINUS && isInExponent())
                                  && previousOperator != &OPERATOR
                                  && !((newHeader == &MULT || newHeader == &BIT_AND)
                                       && isPointerOrReference())
                                 );

                if (!isInPotentialCalculation)
                    if (find(assignmentOperators.begin(), assignmentOperators.end(), newHeader)
                            != assignmentOperators.end())
                        isInPotentialCalculation = true;

                // pad before operator
                if (shouldPad
                        && !(newHeader == &COLON && !foundQuestionMark)
                        && newHeader != &SEMICOLON
                        && newHeader != &COMMA)
                    appendSpacePad();
                appendSequence(*newHeader);
                goForward(newHeader->length() - 1);
                currentChar = (*newHeader)[newHeader->length() - 1];
                // pad after operator
                // but do not pad after a '-' that is a urinary-minus.
                if ( shouldPad && !(newHeader == &MINUS && isUrinaryMinus()) )
                    appendSpacePad();

                previousOperator = newHeader;
                continue;
            }
        }
        if (shouldPadParenthesies)
        {
            if (currentChar == '(' || currentChar == '[')
            {
                isInPotentialCalculation = true;
                appendCurrentChar();
                if (!isInPreprocessor)
                    appendSpacePad();
                continue;
            }
            else if (currentChar == ')' || currentChar == ']')
            {
                char peekedChar = peekNextChar();
                if (!isInPreprocessor)
                    appendSpacePad();
                appendCurrentChar();
                if (!isInPreprocessor
                        && peekedChar != ';' && peekedChar != ',' && peekedChar != '.'
                        && !(currentChar == ']' && peekedChar == '['))
                    appendSpacePad();
                continue;
            }
        }


        appendCurrentChar();
    }

    isLineReady = false;

    // return a beautified (i.e. correctly indented) line.
    return  beautify(readyFormattedLine);
}


/**
* check if there are any indented lines ready to be read by nextLine()
*
* @return    are there any indented lines ready?
*/
bool ASFormatter::hasMoreLines() const
{
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
bool ASFormatter::isFormattingEnabled() const
{
    return (bracketFormatMode != NONE_MODE || shouldPadOperators);
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
void ASFormatter::setBracketFormatMode(BracketMode mode)
{
    bracketFormatMode = mode;
}

/**
* set operator padding mode.
* options:
*    true     statement operators will be padded with spaces around them.
*    false    statement operators will not be padded.
*
* @param mode         the padding mode.
*/
void ASFormatter::setOperatorPaddingMode(bool state)
{
    shouldPadOperators = state;
}

/**
* set parentheies padding mode.
* options:
*    true     statement parenthesies will be padded with spaces around them.
*    false    statement parenthesies will not be padded.
*
* @param mode         the padding mode.
*/
void ASFormatter::setParenthesisPaddingMode(bool state)
{
    shouldPadParenthesies = state;
}

/**
 * set option to break/not break one-line blocks
 *
 * @param state        true = break, false = don't break.
 */
void ASFormatter::setBreakOneLineBlocksMode(bool state)
{
    shouldBreakOneLineBlocks = state;
}

/**
 * set option to break/not break lines consisting of multiple statements.
 *
 * @param state        true = break, false = don't break.
 */
void ASFormatter::setSingleStatementsMode(bool state)
{
    shouldBreakSemicolons = state;
}
/**
* check if a specific sequence exists in the current placement of the current line
*
* @return             whether sequence has been reached.
* @param sequence     the sequence to be checked
*/
bool ASFormatter::isSequenceReached(const string &sequence) const
{
    return currentLine.COMPARE(charNum, sequence.length(), sequence) == 0;

}

/**
* jump over several characters.
*
* @param i       the number of characters to jump over.
*/
void ASFormatter::goForward(int i)
{
    while (--i >= 0)
        getNextChar();
}

/**
* peek at the next unread character.
*
* @return     the next unread character.
*/
char ASFormatter::peekNextChar() const
{
    int peekNum = charNum + 1;
    int len = currentLine.length();
    char ch = ' ';

    while (peekNum < len)
    {
        ch = currentLine[peekNum++];
        if (!isWhiteSpace(ch))
            return ch;
    }

    return ch;
}

/**
* check if current placement is before a comment or line-comment
*
* @return     is before a comment or line-comment.
*/
bool ASFormatter::isBeforeComment() const
{
    int peekNum = charNum + 1;
    int len = currentLine.length();
    char ch = ' ';
    bool foundComment = false;
    ;

    for (peekNum = charNum + 1;
            peekNum < len && isWhiteSpace(currentLine[peekNum]);
            ++peekNum)
        ;

    if (peekNum < len)
        foundComment = ( currentLine.COMPARE(peekNum, 2, OPEN_COMMENT) == 0
                         || currentLine.COMPARE(peekNum, 2, OPEN_LINE_COMMENT) == 0 );

    return foundComment;
}

/**
* get the next character, increasing the current placement in the process.
* the new character is inserted into the variable currentChar.
*
* @return   whether succeded to recieve the new character.
*/
bool ASFormatter::getNextChar()
{
    isInLineBreak = false;
    bool isAfterFormattedWhiteSpace = false;

    if (shouldPadOperators && !isInComment && !isInLineComment
            && !isInQuote && !doesLineStartComment && !isInPreprocessor
            && !isBeforeComment())
    {
        int len = formattedLine.length();
        if (len > 0 && isWhiteSpace(formattedLine[len-1]))
            isAfterFormattedWhiteSpace = true;
    }

    previousChar = currentChar;
    if (!isWhiteSpace(currentChar))
    {
        previousNonWSChar = currentChar;
        if (!isInComment && !isInLineComment && !isInQuote
                && !isSequenceReached(OPEN_COMMENT)
                && !isSequenceReached(OPEN_LINE_COMMENT) )
            previousCommandChar = previousNonWSChar;
    }

    int currentLineLength = currentLine.length();

    if (charNum+1 < currentLineLength
            && (!isWhiteSpace(peekNextChar()) || isInComment || isInLineComment))
    {
        currentChar = currentLine[++charNum];
        if (isAfterFormattedWhiteSpace)
            while (isWhiteSpace(currentChar) && charNum+1 < currentLineLength)
                currentChar = currentLine[++charNum];

        return true;
    }
    else
    {
        if (sourceIterator->hasMoreLines())
        {
            currentLine = sourceIterator->nextLine();
            if (currentLine.length() == 0)
            {
                /*think*/ currentLine = string(" ");
            }

            // unless reading in the first line of the file,
            // break a new line.
            if (!isVirgin)
                isInLineBreak = true;
            else
                isVirgin = false;

            isInLineComment = false;

            if (previousNonWSChar != '\\')
                isInPreprocessor = false;

            trimNewLine();
            currentChar = currentLine[charNum];

            return true;
        }
        else
        {
            endOfCodeReached = true;
            return false;
        }
    }
}

/**
* jump over the leading white space in the current line,
* IF the line does not begin a comment.
*/
void ASFormatter::trimNewLine()
{
    int len = currentLine.length();
    charNum = 0;

    if (isInComment)
        return;

    while (isWhiteSpace(currentLine[charNum]) && charNum+1 < len)
        ++charNum;

    doesLineStartComment = false;
    if (isSequenceReached(string("/*")))
    {
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
void ASFormatter::appendChar(char ch, bool canBreakLine)
{
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
void ASFormatter::appendCurrentChar(bool canBreakLine)
{
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
void ASFormatter::appendSequence(const string &sequence, bool canBreakLine)
{
    if (canBreakLine && isInLineBreak)
        breakLine();
    formattedLine.append(sequence);
}

/**
 * append a space to the current formattedline, UNLESS the 
 * last character is already a white-space character.
 */
void ASFormatter::appendSpacePad()
{
    int len = formattedLine.length();
    if (len == 0 || !isWhiteSpace(formattedLine[len-1]))
        formattedLine.append(1, ' ');
}

/**
 * register a line break for the formatted line.
 */
void ASFormatter::breakLine()
{
    isLineReady = true;
    isInLineBreak = false;
    //isInLineComment = false;
    readyFormattedLine = formattedLine;
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
BracketType ASFormatter::getBracketType() const
{
    BracketType returnVal;

    if (foundPreDefinitionHeader)
        returnVal = DEFINITION_TYPE;
    else
    {
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
bool ASFormatter::isPointerOrReference() const
{
    bool isPR;
    isPR = ( !isInPotentialCalculation
             || IS_A(bracketTypeStack->back(), DEFINITION_TYPE)
             || (!isLegalNameChar(previousNonWSChar)
                 && previousNonWSChar != ')'
                 && previousNonWSChar != ']')
           );

    if (!isPR)
    {
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
 * a urinary minus
 * this method takes for granted that the current character
 * is a '-'.
 *
 * @return        whether the current '-' is a urinary minus.
 */
bool ASFormatter::isUrinaryMinus() const
{
    return ( (previousOperator == &RETURN || !isalnum(previousCommandChar))
             && previousCommandChar != '.'
             && previousCommandChar != ')'
             && previousCommandChar != ']' );
}


/**
 * check if the currently reached '-' character is
 * part of an exponent, i.e. 0.2E-5.
 * this method takes for granted that the current character
 * is a '-'.
 *
 * @return        whether the current '-' is in an exponent.
 */
bool ASFormatter::isInExponent() const
{
    int formattedLineLength = formattedLine.length();
    if (formattedLineLength >= 2)
    {
        char prevPrevFormattedChar = formattedLine[formattedLineLength - 2];
        char prevFormattedChar = formattedLine[formattedLineLength - 1];

        return ( (prevFormattedChar == 'e' || prevFormattedChar == 'E')
                 && prevPrevFormattedChar == '.' || isdigit(prevPrevFormattedChar) );
    }
    else
        return false;
}

/**
 * check if a one-line bracket has been reached,
 * i.e. if the currently reached '{' character is closed
 * with a complimentry '}' elsewhere on the current line,
 *.
 * @return        has a one-line bracket been reached?
 */
bool ASFormatter::isOneLineBlockReached() const
{
    bool isInComment = false;
    bool isInQuote = false;
    int bracketCount = 1;
    int currentLineLength = currentLine.length();
    int i = 0;
    char ch = ' ';
    char quoteChar = ' ';

    for (i = charNum + 1; i < currentLineLength; ++i)
    {
        ch = currentLine[i];

        if (isInComment)
        {
            if (currentLine.COMPARE(i, 2, "*/") == 0)
            {
                isInComment = false;
                ++i;
            }
            continue;
        }

        if (ch == '\\')
        {
            ++i;
            continue;
        }

        if (isInQuote)
        {
            if (ch == quoteChar)
                isInQuote = false;
            continue;
        }

        if (ch == '"' || ch == '\'')
        {
            isInQuote = true;
            quoteChar = ch;
            continue;
        }

        if (currentLine.COMPARE(i, 2, "//") == 0)
            break;

        if (currentLine.COMPARE(i, 2, "/*") == 0)
        {
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
const string *ASFormatter::findHeader(const vector<const string*> &headers, bool checkBoundry)
{
    return ASBeautifier::findHeader(currentLine, charNum, headers, checkBoundry);
}


#ifdef USES_NAMESPACE
}
#endif


