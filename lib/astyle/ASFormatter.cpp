/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *   ASFormatter.cpp
 *
 *   This file is a part of "Artistic Style" - an indentation and
 *   reformatting tool for C, C++, C# and Java source files.
 *   http://astyle.sourceforge.net
 *
 *   The "Artistic Style" project, including all files needed to 
 *   compile it, is free software; you can redistribute it and/or 
 *   modify it under the terms of the GNU Lesser General Public 
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later 
 *   version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this project; if not, write to the 
 *   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *   Boston, MA  02110-1301, USA.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

#include "astyle.h"

#include <cctype>
#include <algorithm>
#include <fstream>
#include <iostream>

// can trace only if _DEBUG is defined (Visual C)
#ifdef _DEBUG
// #define TRACEunpad
// #define TRACEcomment
// #define TRACEheader
#if defined(TRACEunpad) || defined(TRACEcomment) || defined(TRACEheader)
ofstream *traceOutF;
#define TRACEF
#endif
#endif

#ifdef TRACEunpad
#define TRunpad(a,b,c)  if(b > 0 || c > 0) *traceOutF << inLineNumber << " " << b << a << c << endl;
#else
#define TRunpad(a,b,c)  ((void)0)
#endif

#ifdef TRACEcomment
#define TRcomment(a)    *traceOutF << inLineNumber << " " << a << endl;
#else
#define TRcomment(a)    ((void)0)
#endif

#ifdef TRACEheader
#define TRxtra(a)       *traceOutF << inLineNumber << " " << a << endl;
#else
#define TRxtra(a)    ((void)0)
#endif

#define INIT_CONTAINER(container, value)     {if ( (container) != NULL ) delete (container); (container) = (value); }
#define DELETE_CONTAINER(container)          {if ( (container) != NULL ) delete (container); }
#define IS_A(a,b)                            ( ((a) & (b)) == (b))

using namespace std;

namespace astyle
{
bool ASFormatter::calledInitStatic = false;
vector<const string*> ASFormatter::headers;
vector<const string*> ASFormatter::nonParenHeaders;
vector<const string*> ASFormatter::preDefinitionHeaders;
vector<const string*> ASFormatter::preCommandHeaders;
vector<const string*> ASFormatter::operators;
vector<const string*> ASFormatter::assignmentOperators;
vector<const string*> ASFormatter::castOperators;

/**
 * Constructor of ASFormatter
 */
ASFormatter::ASFormatter()
{
	staticInit();

	preBracketHeaderStack = NULL;
	bracketTypeStack = NULL;
	parenStack = NULL;
	lineCommentNoIndent = false;
	lineCommentNoBeautify = false;
	sourceIterator = NULL;
	bracketFormatMode = NONE_MODE;
	shouldPadOperators = false;
	shouldPadParensOutside = false;
	shouldPadParensInside = false;
	shouldUnPadParens = false;
	shouldBreakOneLineBlocks = true;
	shouldBreakOneLineStatements = true;
	shouldConvertTabs = false;
	shouldBreakBlocks = false;
	shouldBreakClosingHeaderBlocks = false;
	shouldBreakClosingHeaderBrackets = false;
	shouldBreakElseIfs = false;
#ifdef TRACEF
	// create a trace text file
	string filename = "trace.txt";
	char* env = getenv("HOME");
	if (env != NULL)
		filename = string(env) + string("/tracef.txt");
	else
	{
		env = getenv("USERPROFILE");
		if (env != NULL)
			filename = string(env) + string("\\My Documents\\tracef.txt");
		else
		{
			cout << "\nCould not open trace.txt\n" << endl;
			exit(1);
		}
	}
	traceOutF = new ofstream(filename.c_str());
#endif
}

/**
 * Destructor of ASFormatter
 */
ASFormatter::~ASFormatter()
{
	DELETE_CONTAINER(preBracketHeaderStack);
#ifdef TRACEF
	delete traceOutF;
#endif
}

/**
 * initialization of static data of ASFormatter.
 */
void ASFormatter::staticInit()
{
	if (calledInitStatic)
		return;

	calledInitStatic = true;

	ASResource::buildHeaders(headers);
	ASResource::buildNonParenHeaders(nonParenHeaders);
	ASResource::buildAssignmentOperators(assignmentOperators);
	ASResource::buildOperators(operators);
	ASResource::buildPreDefinitionHeaders(preDefinitionHeaders);
	ASResource::buildPreCommandHeaders(preCommandHeaders);
	ASResource::buildCastOperators(castOperators);
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
	ASEnhancer::init(ASBeautifier::getIndentLength(),
	                 ASBeautifier::getIndentString(),
	                 ASBeautifier::getCStyle(),
	                 ASBeautifier::getCaseIndent(),
					 ASBeautifier::getEmptyLineFill());
	sourceIterator = si;

	INIT_CONTAINER(preBracketHeaderStack, new vector<const string*>);
	INIT_CONTAINER(bracketTypeStack, new vector<BracketType>);
	bracketTypeStack->push_back(DEFINITION_TYPE);
	INIT_CONTAINER(parenStack, new vector<int>);
	parenStack->push_back(0);

	currentHeader = NULL;
	currentLine = string("");
	formattedLine = "";
	currentChar = ' ';
	previousCommandChar = ' ';
	previousNonWSChar = ' ';
	quoteChar = '"';
	charNum = 0;
	spacePadNum = 0;
	previousOperator = NULL;
	inLineNumber = 0;           // for debugging

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
	foundCastOperator = false;
	foundQuestionMark = false;
	isInLineBreak = false;
	endOfCodeReached = false;
	isLineReady = false;
	isPreviousBracketBlockRelated = true;
	isInPotentialCalculation = false;
	shouldReparseCurrentChar = false;
	passedSemicolon = false;
	passedColon = false;
	isInTemplate = false;
	isInBlParen = false;
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
#ifdef TRACEF
	*traceOutF << "New file -------------" << endl;
#endif
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
	bool isCharImmediatelyPostLineComment = false;
	bool isInVirginLine = isVirgin;
	bool isCharImmediatelyPostOpenBlock = false;
	bool isCharImmediatelyPostCloseBlock = false;
	bool isCharImmediatelyPostTemplate = false;
	bool prevIsParenHeader = false;

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
			isCharImmediatelyPostTemplate = false;
		}
//		if (inLineNumber >= 53)
//			int x = 1;

		if (isInLineComment)
		{
			appendCurrentChar();

			// explicitely break a line when a line comment's end is found.
			if (charNum + 1 == (int) currentLine.length())
			{
				isInLineBreak = true;
				isInLineComment = false;
				isImmediatelyPostLineComment = true;
				currentChar = 0;  //make sure it is a neutral char.
			}
			continue;
		}
		else if (isInComment)
		{
			if (isSequenceReached("*/"))
			{
				isInComment = false;
				isImmediatelyPostComment = true;
				appendSequence(AS_CLOSE_COMMENT);
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

		if (isSequenceReached("//"))
		{
			if (currentLine[charNum+2] == '\xf2')		// check for windows line marker
				isAppendPostBlockEmptyLineRequested = false;
			isInLineComment = true;
			// do not indent if in column 1 or 2
			if (lineCommentNoIndent == false)
			{
				if (charNum == 0)
					lineCommentNoIndent = true;
				else if (charNum == 1 && currentLine[0] == ' ')
					lineCommentNoIndent = true;
			}
			// move comment if spaces were added or deleted
			if (lineCommentNoIndent == false && spacePadNum != 0)
			{
				size_t len = formattedLine.length();
				// if spaces were removed, need to add spaces before the comment
				if (spacePadNum < 0)
				{
					int adjust = -spacePadNum;          // make the number positive
					if (formattedLine[len-1] != '\t')   // don't adjust if a tab
						formattedLine.append(adjust, ' ');
					else
						adjust = 0;
					TRcomment(adjust);                  // trace macro
				}
				// if spaces were added, need to delete spaces before the comment, if possible
				else
				{
					int adjust = spacePadNum;
					if (formattedLine.find_last_not_of(' ') < len - adjust - 1
					        && formattedLine[len-1] != '\t')    // don't adjust a tab
						formattedLine.resize(len - adjust);
					// the following are commented out to avoid a Borland compiler warning
//                    else
//                        adjust = 0;
					TRcomment(-adjust);                 // trace macro
				}
			}
			appendSequence(AS_OPEN_LINE_COMMENT);
			goForward(1);
			// explicitely break a line when a line comment's end is found.
			if (charNum + 1 == (int) currentLine.length())
			{
				isInLineBreak = true;
				isInLineComment = false;
				isImmediatelyPostLineComment = true;
				currentChar = 0;  //make sure it is a neutral char.
			}
			continue;
		}
		else if (isSequenceReached("/*"))
		{
			isInComment = true;
			appendSequence(AS_OPEN_COMMENT);
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

		// check if in preprocessor
		// ** isInPreprocessor will be automatically reset at the begining
		//    of a new line in getnextChar()
		if (currentChar == '#')
		{
			isInPreprocessor = true;
			appendCurrentChar();
			continue;
		}

		/* not in preprocessor ... */

		if (isImmediatelyPostComment)
		{
			isImmediatelyPostComment = false;
			isCharImmediatelyPostComment = true;
		}

		if (isImmediatelyPostLineComment)
		{
			isImmediatelyPostLineComment = false;
			isCharImmediatelyPostLineComment = true;
		}

		if (shouldBreakLineAfterComments)
		{
			shouldBreakLineAfterComments = false;
			shouldReparseCurrentChar = true;
			breakLine();
			continue;
		}

		// reset isImmediatelyPostHeader information
		if (isImmediatelyPostHeader)
		{
			isImmediatelyPostHeader = false;

			// Make sure headers are broken from their succeeding blocks
			// (e.g.
			//     if (isFoo) DoBar();
			//  should become
			//     if (isFoo)
			//         DoBar;
			// )
			// But treat else if() as a special case which should not be broken!
			if (shouldBreakOneLineStatements)
			{
				// if may break 'else if()'s, then simply break the line

				if (shouldBreakElseIfs)
					isInLineBreak = true;
			}
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
				isInLineBreak = true;
				continue;
			}
		}

		// Check if in template declaration, e.g. foo<bar> or foo<bar,fig>
		// If so, set isInTemplate to true
		if (!isInTemplate && currentChar == '<')
		{
			int templateDepth = 0;
			const string *oper;
			for (size_t i = charNum;
			        i < currentLine.length();
			        i += (oper ? oper->length() : 1))
			{
				oper = ASBeautifier::findHeader(currentLine, i, operators);

				if (oper == &AS_LS)
				{
					templateDepth++;
				}
				else if (oper == &AS_GR)
				{
					templateDepth--;
					if (templateDepth == 0)
					{
						// this is a template!
						isInTemplate = true;
						break;
					}
				}
				else if (oper == &AS_COMMA            // comma,     e.g. A<int, char>
				         || oper == &AS_BIT_AND       // reference, e.g. A<int&>
				         || oper == &AS_MULT          // pointer,   e.g. A<int*>
				         || oper == &AS_COLON_COLON)  // ::,        e.g. std::string
				{
					continue;
				}
				else if (!isLegalNameChar(currentLine[i]) && !isWhiteSpace(currentLine[i]))
				{
					// this is not a template -> leave...
					isInTemplate = false;
					break;
				}
			}
		}

		// handle parenthesies
		if (currentChar == '(' || currentChar == '[' || (isInTemplate && currentChar == '<'))
		{
			parenStack->back()++;
			if (currentChar == '[')
				isInBlParen = true;
		}
		else if (currentChar == ')' || currentChar == ']' || (isInTemplate && currentChar == '>'))
		{
			parenStack->back()--;
			if (isInTemplate && currentChar == '>')
			{
				isInTemplate = false;
				isCharImmediatelyPostTemplate = true;
			}

			// check if this parenthesis closes a header, e.g. if (...), while (...)
			if (isInHeader && parenStack->back() == 0)
			{
				isInHeader = false;
				isImmediatelyPostHeader = true;
			}
			if (currentChar == ']')
				isInBlParen = false;
			if (currentChar == ')')
				foundCastOperator = false;
		}

		// handle brackets
		BracketType bracketType = NULL_TYPE;

		if (currentChar == '{')
		{
			bracketType = getBracketType();
			foundPreDefinitionHeader = false;
			foundPreCommandHeader = false;
			isInPotentialCalculation = false;

			bracketTypeStack->push_back(bracketType);
			preBracketHeaderStack->push_back(currentHeader);
			currentHeader = NULL;

			isPreviousBracketBlockRelated = !IS_A(bracketType, ARRAY_TYPE);
		}
		else if (currentChar == '}')
		{
			// if a request has been made to append a post block empty line,
			// but the block exists immediately before a closing bracket,
			// then there is not need for the post block empty line.
			//
			isAppendPostBlockEmptyLineRequested = false;

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

			if (currentChar == '{')
			{
				if ((bracketFormatMode == ATTACH_MODE
				        || bracketFormatMode == BDAC_MODE && bracketTypeStack->size() >= 2
				        && IS_A((*bracketTypeStack)[bracketTypeStack->size()-2], COMMAND_TYPE) /*&& isInLineBreak*/)
				        && !isCharImmediatelyPostLineComment)
				{
					appendSpacePad();
					if (!isCharImmediatelyPostComment       // do not attach '{' to lines that end with /**/ comments.
					        && previousCommandChar != '{'
					        && previousCommandChar != '}'
					        && previousCommandChar != ';')  // '}' , ';' chars added for proper handling of '{' immediately after a '}' or ';'
						appendCurrentChar(false);
					else
						appendCurrentChar(true);
					continue;
				}
				else if (bracketFormatMode == BREAK_MODE
				         || bracketFormatMode == BDAC_MODE && bracketTypeStack->size() >= 2
				         && IS_A((*bracketTypeStack)[bracketTypeStack->size()-2], DEFINITION_TYPE))
				{
					if (shouldBreakOneLineBlocks || !IS_A(bracketType,  SINGLE_LINE_TYPE))
						breakLine();
					appendCurrentChar();
					continue;
				}
				else if (bracketFormatMode == NONE_MODE)
				{
					if (lineBeginsWith('{'))                    // is opening bracket broken?
						appendCurrentChar(true);
					else
						appendCurrentChar(false);
					continue;
				}
			}
			else if (currentChar == '}')
			{
				// mark state of immediately after empty block
				// this state will be used for locating brackets that appear immedately AFTER an empty block (e.g. '{} \n}').
				if (previousCommandChar == '{')
					isImmediatelyPostEmptyBlock = true;

				if ((!(previousCommandChar == '{' && isPreviousBracketBlockRelated))            // this '{' does not close an empty block
				        && (shouldBreakOneLineBlocks || !IS_A(bracketType,  SINGLE_LINE_TYPE))  // astyle is allowed to break on line blocks
				        && (!(bracketFormatMode == NONE_MODE && IS_A(bracketType,  SINGLE_LINE_TYPE)))
				        && !isImmediatelyPostEmptyBlock)                                        // this '}' does not immediately follow an empty block
				{
					breakLine();
					appendCurrentChar();
				}
				else
				{
					if (!isCharImmediatelyPostComment && !bracketFormatMode == NONE_MODE)
						isInLineBreak = false;
					appendCurrentChar();

					if (!bracketFormatMode == NONE_MODE)
						if ((shouldBreakOneLineBlocks || !IS_A(bracketType,  SINGLE_LINE_TYPE))
						        && !(currentChar == '}' && peekNextChar() == ';'))      // fixes }; placed on separate lines
							shouldBreakLineAfterComments = true;
				}

				if (shouldBreakBlocks)
				{
					isAppendPostBlockEmptyLineRequested = true;
				}

				continue;
			}
		}

		if (((previousCommandChar == '{'
		        && isPreviousBracketBlockRelated)
		        || (previousCommandChar == '}'
		            && !isImmediatelyPostEmptyBlock
		            && isPreviousBracketBlockRelated
		            && !isPreviousCharPostComment       // Fixes wrongly appended newlines after '}' immediately after comments
		            && peekNextChar() != ' '))
		        && (shouldBreakOneLineBlocks
		            || !IS_A(bracketTypeStack->back(),  SINGLE_LINE_TYPE)))
		{
			isCharImmediatelyPostOpenBlock = (previousCommandChar == '{');
			isCharImmediatelyPostCloseBlock = (previousCommandChar == '}');

			if (bracketFormatMode != NONE_MODE)
			{
				previousCommandChar = ' ';
				isInLineBreak = true;
			}
		}

		// reset block handling flags
		isImmediatelyPostEmptyBlock = false;

		// look for headers
		if (!isInTemplate)
		{
			if ((newHeader = findHeader(headers)) != NULL)
			{
				foundClosingHeader = false;
				const string *previousHeader;

				// recognize closing headers of do..while, if..else, try..catch..finally
				if ((newHeader == &AS_ELSE && currentHeader == &AS_IF)
				        || (newHeader == &AS_WHILE && currentHeader == &AS_DO)
				        || (newHeader == &AS_CATCH && currentHeader == &AS_TRY)
				        || (newHeader == &AS_CATCH && currentHeader == &AS_CATCH)
				        || (newHeader == &AS_FINALLY && currentHeader == &AS_TRY)
				        || (newHeader == &AS_FINALLY && currentHeader == &AS_CATCH))
					foundClosingHeader = true;

				previousHeader = currentHeader;
				currentHeader = newHeader;

				// If in ATTACH or LINUX bracket modes, attach closing headers (e.g. 'else', 'catch')
				// to their preceding bracket,
				// But do not perform the attachment if the shouldBreakClosingHeaderBrackets is set!
				if (!shouldBreakClosingHeaderBrackets
				        && foundClosingHeader
				        && (bracketFormatMode == ATTACH_MODE || bracketFormatMode == BDAC_MODE)
				        && previousNonWSChar == '}')
				{
					isInLineBreak = false;
					appendSpacePad();

					if (shouldBreakBlocks)
						isAppendPostBlockEmptyLineRequested = false;
				}

				// If NONE bracket mode, leave closing headers as they are (e.g. 'else', 'catch')
				if (foundClosingHeader && bracketFormatMode == NONE_MODE && previousNonWSChar == '}')
				{
					if (lineBeginsWith('}'))                    // is closing bracket broken?
					{
						isInLineBreak = false;
						appendSpacePad();
					}

					if (shouldBreakBlocks)
						isAppendPostBlockEmptyLineRequested = false;
				}

				//Check if a template definition as been reached, e.g. template<class A>
				if (newHeader == &AS_TEMPLATE)
				{
					isInTemplate = true;
				}

				// check if the found header is non-paren header
				isNonParenHeader = (find(nonParenHeaders.begin(), nonParenHeaders.end(),
				                         newHeader) != nonParenHeaders.end());

				appendSequence(*currentHeader);
				goForward(currentHeader->length() - 1);
				// if a paren-header is found add a space after it, if needed
				// this checks currentLine, appendSpacePad() checks formattedLine
				if (!isNonParenHeader && charNum < (int) currentLine.length() && !isWhiteSpace(currentLine[charNum+1]))
					appendSpacePad();

				// Signal that a header has been reached
				// *** But treat a closing while() (as in do...while)
				//     as if it where NOT a header since a closing while()
				//     should never have a block after it!
				if (!(foundClosingHeader && currentHeader == &AS_WHILE))
				{
					isInHeader = true;
					if (isNonParenHeader)
					{
						isImmediatelyPostHeader = true;
						isInHeader = false;
					}
				}

				if (currentHeader == &AS_IF && previousHeader == &AS_ELSE)
					isInLineBreak = false;

				if (shouldBreakBlocks)
				{
					if (previousHeader == NULL
					        && !foundClosingHeader
					        && !isCharImmediatelyPostOpenBlock)
					{
						isPrependPostBlockEmptyLineRequested = true;
					}

					if (currentHeader == &AS_ELSE
					        || currentHeader == &AS_CATCH
					        || currentHeader == &AS_FINALLY
					        || foundClosingHeader)
					{
						isPrependPostBlockEmptyLineRequested = false;
					}

					if (shouldBreakClosingHeaderBlocks
					        &&  isCharImmediatelyPostCloseBlock)
					{
						isPrependPostBlockEmptyLineRequested = true;
					}

				}

				continue;
			}
			else if ((newHeader = findHeader(preDefinitionHeaders)) != NULL)
			{
				foundPreDefinitionHeader = true;
				appendSequence(*newHeader);
				goForward(newHeader->length() - 1);

				if (shouldBreakBlocks)
					isPrependPostBlockEmptyLineRequested = true;

				continue;
			}
			else if ((newHeader = findHeader(preCommandHeaders)) != NULL)
			{
				foundPreCommandHeader = true;
				appendSequence(*newHeader);
				goForward(newHeader->length() - 1);

				continue;
			}
			else if ((newHeader = findHeader(castOperators)) != NULL)
			{
				foundCastOperator = true;
				appendSequence(*newHeader);
				goForward(newHeader->length() - 1);

				continue;
			}

		}

		if (isInLineBreak)          // OK to break line here **************************************
			breakLine();

		if (previousNonWSChar == '}' || currentChar == ';')
		{
			if (shouldBreakOneLineStatements && currentChar == ';'
			        && (shouldBreakOneLineBlocks || !IS_A(bracketTypeStack->back(),  SINGLE_LINE_TYPE))
			        && (! bracketFormatMode == NONE_MODE))
			{
				passedSemicolon = true;
			}

			if (shouldBreakBlocks && currentHeader != NULL && parenStack->back() == 0)
			{
				isAppendPostBlockEmptyLineRequested = true;
			}

			if (currentChar != ';')
				currentHeader = NULL;

			foundQuestionMark = false;
			foundPreDefinitionHeader = false;
			foundPreCommandHeader = false;
			foundCastOperator = false;
			isInPotentialCalculation = false;
		}

		if (currentChar == ':'
		        && shouldBreakOneLineStatements
		        && !foundQuestionMark           // not in a ... ? ... : ... sequence
		        && !foundPreDefinitionHeader    // not in a definition block (e.g. class foo : public bar
		        && previousCommandChar != ')'   // not immediately after closing paren of a method header, e.g. ASFormatter::ASFormatter(...) : ASBeautifier(...)
		        && previousChar != ':'          // not part of '::'
		        && peekNextChar() != ':')       // not part of '::'
		{
			passedColon = true;
			if (shouldBreakBlocks)
				isPrependPostBlockEmptyLineRequested = true;
		}

		if (currentChar == '?')
			foundQuestionMark = true;

		// determine if this is a potential calculation
		newHeader = findHeader(operators);

		if (newHeader != NULL)
		{
			if (!isInPotentialCalculation)
			{
				if (find(assignmentOperators.begin(), assignmentOperators.end(), newHeader)
				        != assignmentOperators.end())
				{
					char peekedChar = peekNextChar();
					isInPotentialCalculation = (newHeader != &AS_RETURN                          
					                            && !(newHeader == &AS_EQUAL && peekedChar == '*') 
					                            && !(newHeader == &AS_EQUAL && peekedChar == '&')); 
				}
			}
		}
		else
		{
			// the following are not calculations           %%%%%%%%%%%%
			if (currentLine.compare(charNum, 3, "new") == 0 && !isLegalNameChar(currentLine[charNum+3]))
				isInPotentialCalculation = false;
		}

		if (shouldPadOperators)
		{
			if (newHeader != NULL)
			{
				bool shouldPad = (newHeader != &AS_COLON_COLON
				                  && newHeader != &AS_PAREN_PAREN
				                  && newHeader != &AS_BLPAREN_BLPAREN
				                  && newHeader != &AS_PLUS_PLUS
				                  && newHeader != &AS_MINUS_MINUS
				                  && newHeader != &AS_NOT
				                  && newHeader != &AS_BIT_NOT
				                  && newHeader != &AS_ARROW
				                  && newHeader != &AS_OPERATOR
				                  && newHeader != &AS_RETURN
				                  && !(newHeader == &AS_MINUS && isInExponent())
				                  && !(newHeader == &AS_MINUS                   // check for negative number $$$$$$$$$$$
				                       && (previousNonWSChar == '('            // $$$$$$$$$$$
				                           || previousNonWSChar == '='         // $$$$$$$$$$$
				                           || previousNonWSChar == ','))       // $$$$$$$$$$$
				                  && !(newHeader == &AS_PLUS && isInExponent())
				                  && previousOperator != &AS_OPERATOR
				                  && !((newHeader == &AS_MULT || newHeader == &AS_BIT_AND)
				                       && isPointerOrReference())
				                  && !((isInTemplate || isCharImmediatelyPostTemplate)
				                       && (newHeader == &AS_LS || newHeader == &AS_GR))
				                 );
				// pad before operator
				if (shouldPad
				        && !isInBlParen
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
				if (shouldPad
				        && !isInBlParen
				        && !isBeforeComment()
				        && !(newHeader == &AS_MINUS && isUnaryMinus())
				        && !(currentLine.compare(charNum + 1, 1,  ";") == 0)
				        && !(currentLine.compare(charNum + 1, 2, "::") == 0)
				   )
					appendSpaceAfter();

				previousOperator = newHeader;
				continue;
			}
		}

		if (shouldPadParensOutside || shouldPadParensInside || shouldUnPadParens)
		{
			if (currentChar == '(')
			{
				int spacesOutsideToDelete = formattedLine.length() - 1;
				int spacesInsideToDelete = 0;

				// compute spaces outside the opening paren to delete
				if (shouldUnPadParens)
				{
					char lastChar = ' ';
					prevIsParenHeader = false;
					size_t i = formattedLine.find_last_not_of(" \t");
					if (i != string::npos)
					{
						spacesOutsideToDelete -= i;
						lastChar = formattedLine[i];
						// was last word a paren header?
						int start;          // start of the previous word
						for (start = i; start > 0; start--)
						{
							if (isLegalNameChar(formattedLine[start]))
								continue;
							start++;
							break;
						}
						// if previous word is a header, it will be a paren header
						const string *prevWord = ASBeautifier::findHeader(formattedLine, start, headers);
						if (prevWord != NULL
						        || (formattedLine.compare(start, 6, "return") == 0
						            && !isLegalNameChar(formattedLine[6])))
						{
							prevIsParenHeader = true;
							TRxtra(*prevWord);      // trace macro
						}
					}
					// do not unpad operators, but leave them if already padded
					if (shouldPadParensOutside || prevIsParenHeader)
						spacesOutsideToDelete--;
					else if (lastChar == '|'          // check for ||
					         || lastChar == '&'      // check for &&
					         || lastChar == ','
					         || (lastChar == '>' && !foundCastOperator)
					         || lastChar == '<'
					         || lastChar == '?'
					         || lastChar == ':'
					         || lastChar == ';'
					         || lastChar == '='
					         || lastChar == '+'
					         || lastChar == '-'
					         || (lastChar == '*' && isInPotentialCalculation)
					         || lastChar == '/'
					         || lastChar == '%')
						spacesOutsideToDelete--;

					if (spacesOutsideToDelete > 0)
					{
						formattedLine.erase(i + 1, spacesOutsideToDelete);
						spacePadNum -= spacesOutsideToDelete;
					}
				}

				// pad open paren outside
				char peekedCharOutside = peekNextChar();
				if (shouldPadParensOutside)
					if (!(currentChar == '(' && peekedCharOutside == ')'))
						appendSpacePad();

				appendCurrentChar();

				// unpad open paren inside
				if (shouldUnPadParens)
				{
					size_t j = currentLine.find_first_not_of(" \t", charNum + 1);
					if (j != string::npos)
						spacesInsideToDelete = j - charNum - 1;
					if (shouldPadParensInside)
						spacesInsideToDelete--;
					if (spacesInsideToDelete > 0)
					{
						currentLine.erase(charNum + 1, spacesInsideToDelete);
						spacePadNum -= spacesInsideToDelete;
					}
				}

				// pad open paren inside
				char peekedCharInside = peekNextChar();
				if (shouldPadParensInside)
					if (!(currentChar == '(' && peekedCharInside == ')'))
						appendSpaceAfter();

				TRunpad('(', spacesOutsideToDelete, spacesInsideToDelete);       // trace macro

				continue;
			}
			else if (currentChar == ')' /*|| currentChar == ']'*/)
			{
				int spacesOutsideToDelete = 0;
				int spacesInsideToDelete = formattedLine.length();

				// unpad close paren inside
				if (shouldUnPadParens)
				{
					size_t i = formattedLine.find_last_not_of(" \t");
					if (i != string::npos)
						spacesInsideToDelete = formattedLine.length() - 1 - i;
					if (shouldPadParensInside)
						spacesInsideToDelete--;
					if (spacesInsideToDelete > 0)
					{
						formattedLine.erase(i + 1, spacesInsideToDelete);
						spacePadNum -= spacesInsideToDelete;
					}
				}

				// pad close paren inside
				if (shouldPadParensInside)
					if (!(previousChar == '(' && currentChar == ')'))
						appendSpacePad();

				appendCurrentChar();

				// unpad close paren outside
				if (shouldUnPadParens)
				{
					// may have end of line comments
					size_t j = currentLine.find_first_not_of(" \t", charNum + 1);
					if (j != string::npos)
						if (currentLine[j] == '[' || currentLine[j] == ']')
							spacesOutsideToDelete = j - charNum - 1;
					if (shouldPadParensOutside)
						spacesOutsideToDelete--;
//                    spacesOutsideToDelete--;            // always leave 1 space

					if (spacesOutsideToDelete > 0)
					{
						currentLine.erase(charNum + 1, spacesOutsideToDelete);
						spacePadNum -= spacesOutsideToDelete;
					}
				}

				// pad close paren outside
				char peekedCharOutside = peekNextChar();
				if (shouldPadParensOutside)
					if (peekedCharOutside != ';'
					        && peekedCharOutside != ','
					        && peekedCharOutside != '.'
					        && peekedCharOutside != '-')    // check for ->
//                            && !(currentChar == ']' && peekedCharOutside == '['))
						appendSpaceAfter();

				TRunpad(')', spacesInsideToDelete, 0 /*spacesOutsideToDelete*/);       // trace macro

				continue;
			}
		}

		appendCurrentChar();
	}   // end of while loop  *  end of while loop  *  end of while loop  *  end of while loop

	// return a beautified (i.e. correctly indented) line.

	string beautifiedLine;
	int readyFormattedLineLength = trim(readyFormattedLine).length();

	if (prependEmptyLine
	        && readyFormattedLineLength > 0
	        && previousReadyFormattedLineLength > 0)
	{
		isLineReady = true;             // signal that a readyFormattedLine is still waiting
		beautifiedLine = beautify("");
	}
	else
	{
		isLineReady = false;
		beautifiedLine = beautify(readyFormattedLine);
		lineCommentNoBeautify = lineCommentNoIndent;
		lineCommentNoIndent = false;
	}

	prependEmptyLine = false;
	previousReadyFormattedLineLength = readyFormattedLineLength;

	enhance(beautifiedLine);                // call the enhancer function
	return beautifiedLine;

}


/**
* check if there are any indented lines ready to be read by nextLine()
*
* @return    are there any indented lines ready?
*/
bool ASFormatter::hasMoreLines() const
{
	return !endOfCodeReached;
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
 * set closing header bracket breaking mode
 * options:
 *    true     brackets just before closing headers (e.g. 'else', 'catch')
 *             will be broken, even if standard brackets are attached.
 *    false    closing header brackets will be treated as standard brackets.
 *
 * @param state         the closing header bracket breaking mode.
 */
void ASFormatter::setBreakClosingHeaderBracketsMode(bool state)
{
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
void ASFormatter::setBreakElseIfsMode(bool state)
{
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
void ASFormatter::setOperatorPaddingMode(bool state)
{
	shouldPadOperators = state;
}

/**
* set parenthesis outside padding mode.
* options:
*    true     statement parenthesiss will be padded with spaces around them.
*    false    statement parenthesiss will not be padded.
*
* @param state         the padding mode.
*/
void ASFormatter::setParensOutsidePaddingMode(bool state)
{
	shouldPadParensOutside = state;
}

/**
* set parenthesis inside padding mode.
* options:
*    true     statement parenthesis will be padded with spaces around them.
*    false    statement parenthesis will not be padded.
*
* @param state         the padding mode.
*/
void ASFormatter::setParensInsidePaddingMode(bool state)
{
	shouldPadParensInside = state;
}

/**
* set parenthesis unpadding mode.
* options:
*    true     statement parenthesis will be unpadded with spaces removed around them.
*    false    statement parenthesis will not be unpadded.
*
* @param state         the padding mode.
*/
void ASFormatter::setParensUnPaddingMode(bool state)
{
	shouldUnPadParens = state;
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
	shouldBreakOneLineStatements = state;
}

/**
 * set option to convert tabs to spaces.
 *
 * @param state        true = convert, false = don't convert.
 */
void ASFormatter::setTabSpaceConversionMode(bool state)
{
	shouldConvertTabs = state;
}


/**
 * set option to break unrelated blocks of code with empty lines.
 *
 * @param state        true = convert, false = don't convert.
 */
void ASFormatter::setBreakBlocksMode(bool state)
{
	shouldBreakBlocks = state;
}

/**
 * set option to break closing header blocks of code (such as 'else', 'catch', ...) with empty lines.
 *
 * @param state        true = convert, false = don't convert.
 */
void ASFormatter::setBreakClosingHeaderBlocksMode(bool state)
{
	shouldBreakClosingHeaderBlocks = state;
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
	char ch = ' ';
	size_t peekNum = currentLine.find_first_not_of(" \t", charNum + 1);

	if (peekNum == string::npos)
		return ch;

	ch = currentLine[peekNum];

	if (shouldConvertTabs && ch == '\t')
		ch = ' ';

	return ch;
}

/**
* check if current placement is before a comment or line-comment
*
* @return     is before a comment or line-comment.
*/
bool ASFormatter::isBeforeComment() const
{
	bool foundComment = false;
	size_t peekNum = currentLine.find_first_not_of(" \t", charNum + 1);

	if (peekNum == string::npos)
		return foundComment;

	foundComment = (currentLine.compare(peekNum, 2, "/*") == 0
	                || currentLine.compare(peekNum, 2, "//") == 0);

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
	previousChar = currentChar;

	if (!isWhiteSpace(currentChar))
	{
		previousNonWSChar = currentChar;
		if (!isInComment && !isInLineComment && !isInQuote
		        && !isSequenceReached("/*")
		        && !isSequenceReached("//"))
			previousCommandChar = previousNonWSChar;
	}

	int currentLineLength = currentLine.length();

	if (charNum + 1 < currentLineLength
	        && (!isWhiteSpace(peekNextChar()) || isInComment || isInLineComment))
	{
		currentChar = currentLine[++charNum];

		if (shouldConvertTabs && currentChar == '\t')
			currentChar = ' ';

		return true;
	}
	else        // end of line has been reached
	{
		if (sourceIterator->hasMoreLines())
		{
			currentLine = sourceIterator->nextLine();
			spacePadNum = 0;
			inLineNumber++;

			if (currentLine.length() == 0)
			{
				currentLine = string(" ");        // a null is inserted if this is not done
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

			// check if is in preprocessor before line trimming
			if (previousNonWSChar != '\\')
				isInPreprocessor = false;

			trimNewLine();
			currentChar = currentLine[charNum];

			if (shouldConvertTabs && currentChar == '\t')
				currentChar = ' ';

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
* IF the line does not begin a comment or is in a preprocessor definition.
*/
void ASFormatter::trimNewLine()
{
	int len = currentLine.length();
	charNum = 0;

	if (isInComment || isInPreprocessor)
		return;

	while (isWhiteSpace(currentLine[charNum]) && charNum + 1 < len)
		++charNum;

	doesLineStartComment = false;
	if (isSequenceReached("/*"))
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
	if (len > 0 && !isWhiteSpace(formattedLine[len-1]))
	{
		formattedLine.append(1, ' ');
		spacePadNum++;
	}
}

/**
 * append a space to the current formattedline, UNLESS the
 * next character is already a white-space character.
 */
void ASFormatter::appendSpaceAfter()
{
	int len = currentLine.length();
	if (charNum + 1 < len && !isWhiteSpace(currentLine[charNum+1]))
	{
		formattedLine.append(1, ' ');
		spacePadNum++;
	}
}

/**
 * register a line break for the formatted line.
 */
void ASFormatter::breakLine()
{
	isLineReady = true;
	isInLineBreak = false;
	spacePadNum = 0;

	// queue an empty line prepend request if one exists
	prependEmptyLine = isPrependPostBlockEmptyLineRequested;

	readyFormattedLine =  formattedLine;
	if (isAppendPostBlockEmptyLineRequested)
	{
		isAppendPostBlockEmptyLineRequested = false;
		isPrependPostBlockEmptyLineRequested = true;
	}
	else
	{
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
BracketType ASFormatter::getBracketType() const
{
	BracketType returnVal;

	if (foundPreDefinitionHeader)
		returnVal = DEFINITION_TYPE;
	else
	{
		bool isCommandType;
		isCommandType = (foundPreCommandHeader
		                 || (currentHeader != NULL && isNonParenHeader)
		                 || (previousCommandChar == ')')
		                 || (previousCommandChar == ':' && !foundQuestionMark)
		                 || (previousCommandChar == ';')
		                 || ((previousCommandChar == '{' ||  previousCommandChar == '}')
		                     && isPreviousBracketBlockRelated));

		returnVal = (isCommandType ? COMMAND_TYPE : ARRAY_TYPE);
	}

	if (isOneLineBlockReached())
		returnVal = (BracketType)(returnVal | SINGLE_LINE_TYPE);

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
	isPR = (!isInPotentialCalculation
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
 * a unary minus
 * this method takes for granted that the current character
 * is a '-'.
 *
 * @return        whether the current '-' is a unary minus.
 */
bool ASFormatter::isUnaryMinus() const
{
	return ((previousOperator == &AS_RETURN || !isalnum(previousCommandChar))
	        && previousCommandChar != '.'
	        && previousCommandChar != ')'
	        && previousCommandChar != ']');
}


/**
 * check if the currently reached '-' or '+' character is
 * part of an exponent, i.e. 0.2E-5.
 * this method takes for granted that the current character
 * is a '-' or '+'.
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

		return ((prevFormattedChar == 'e' || prevFormattedChar == 'E')
		        && (prevPrevFormattedChar == '.' || isdigit(prevPrevFormattedChar)));
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
	char quoteChar = ' ';

	for (int i = charNum + 1; i < currentLineLength; ++i)
	{
		char ch = currentLine[i];

		if (isInComment)
		{
			if (currentLine.compare(i, 2, "*/") == 0)
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

		if (currentLine.compare(i, 2, "//") == 0)
			break;

		if (currentLine.compare(i, 2, "/*") == 0)
		{
			isInComment = true;
			++i;
			continue;
		}

		if (ch == '{')
			++bracketCount;
		else if (ch == '}')
			--bracketCount;

		if (bracketCount == 0)
			return true;
	}

	return false;
}

/**
 * check if one of a set of headers has been reached in the
 * current position of the current line.
 *
 * @return             a pointer to the found header. Or a NULL if no header has been reached.
 * @param headers      a vector of headers.
 * @param checkBoundry
 */
const string *ASFormatter::findHeader(const vector<const string*> &headers, bool checkBoundry)
{
	return ASBeautifier::findHeader(currentLine, charNum, headers, checkBoundry);
}

/**
 * check if a line begins with the specified character
 * i.e. if the current line begins with a open bracket.
 *
 * @return        true or false
 */
bool ASFormatter::lineBeginsWith(char charToCheck) const
{
	bool beginsWith = false;
	size_t i = currentLine.find_first_not_of(" \t");

	if (i != string::npos)
		if (currentLine[i] == charToCheck && (int) i == charNum)
			beginsWith = true;

	return beginsWith;
}


}   // end namespace astyle
