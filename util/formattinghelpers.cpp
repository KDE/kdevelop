/* This file is part of KDevelop
*  Copyright 2011 David Nolden <david.nolden.kdevelop@art-master.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */

#include "formattinghelpers.h"
#include <QString>
#include <kdebug.h>

namespace KDevelop
{

///Matches the given prefix to the given text, ignoring all whitespace, but not ignoring newlines
///Returns -1 if mismatched, else the position in @p text where the @p prefix match ends
int matchPrefixIgnoringWhitespace(QString text, QString prefix)
{
    int prefixPos = 0;
    int textPos = 0;
    while (prefixPos < prefix.length() && textPos < text.length()) {
        while (prefixPos < prefix.length() && prefix[prefixPos].isSpace() && prefix[prefixPos] != '\n')
            ++prefixPos;
        while (textPos < text.length() && text[textPos].isSpace() && prefix[prefixPos] != '\n')
            ++textPos;

        if(prefixPos == prefix.length() || textPos == text.length())
            break;

        if(prefix[prefixPos] != text[textPos])
            return -1;
        ++prefixPos;
        ++textPos;
    }
    return textPos;
}

//Returns the closest newline position before the actual text, or -1
int leadingNewLine(QString str) {
    int ret = -1;
    for(int a = 0; a < str.length(); ++a) {
        if(!str[a].isSpace())
            return ret;
        if(str[a] == '\n')
            ret = a;
    }
    return ret;
}

int firstNonWhiteSpace(QString str) {
    for(int a = 0; a < str.length(); ++a)
        if(!str[a].isSpace())
            return a;
    return -1;
}

static QString reverse( const QString& str ) {
  QString ret;
  for(int a = str.length()-1; a >= 0; --a)
      ret.append(str[a]);
  
  return ret;
}

///Removes parts of the white-space at the start that are in @p output but not in @p text
QString equalizeWhiteSpaceAtStart(QString original, QString output, bool removeIndent = false) {
    int outputNewline = leadingNewLine(output);
    if(outputNewline != -1) {
        if(leadingNewLine(original) != -1)
            return output.mid(outputNewline); //Exactly include the leading newline as in the original text
        else
            output = output.mid(outputNewline+1); //Skip the leading newline, the orginal had none as well
    }

    if(removeIndent && output[0].isSpace() && !original[0].isSpace()) {
        //The original text has no leading white space, remove all leading white space
        int nonWhite = firstNonWhiteSpace(output);
        if(nonWhite != -1)
            output = output.mid(nonWhite);
        else
            output.clear();
    }
    return output;
}

QString extractFormattedTextFromContext( const QString& _formattedMergedText, const QString& /*originalMergedText*/, const QString& text, const QString& leftContext, const QString& rightContext)
{
    QString formattedMergedText = _formattedMergedText;
    //Now remove "leftContext" and "rightContext" from the sides

    if(!leftContext.isEmpty()) {
        int endOfLeftContext = matchPrefixIgnoringWhitespace( formattedMergedText, leftContext);
        if(endOfLeftContext == -1) {
            kWarning() << "problem matching the left context";
            return text;
        }
        formattedMergedText = formattedMergedText.mid(endOfLeftContext);
        formattedMergedText = equalizeWhiteSpaceAtStart(text, formattedMergedText);
    }

    if(!rightContext.isEmpty()) {
        //Add a whitespace behind the text for matching, so that we definitely capture all trailing whitespace
        int endOfText = matchPrefixIgnoringWhitespace( formattedMergedText, text+" ");
        if(endOfText == -1) {
            kWarning() << "problem matching the text while formatting";
            return text;
        }
        formattedMergedText = formattedMergedText.left(endOfText);
        formattedMergedText = reverse(equalizeWhiteSpaceAtStart(reverse(text), reverse( formattedMergedText), true));
    }

    return formattedMergedText;
}

}
