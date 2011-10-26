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

///Matches the given prefix to the given text, ignoring all whitespace
///Returns -1 if mismatched, else the position in @p text where the @p prefix match ends
int matchPrefixIgnoringWhitespace(QString text, QString prefix)
{
    int prefixPos = 0;
    int textPos = 0;
    
    while (prefixPos < prefix.length() && textPos < text.length()) {
        while (prefixPos < prefix.length() && prefix[prefixPos].isSpace())
            ++prefixPos;
        while (textPos < text.length() && text[textPos].isSpace())
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

static QString reverse( const QString& str ) {
  QString ret;
  for(int a = str.length()-1; a >= 0; --a)
      ret.append(str[a]);
  
  return ret;
}

// Returns the new text position, with all whitespace that is redundant in the given context skipped
int skipRedundantWhiteSpace( QString context, QString text, int textPosition, int textEnd )
{
    if( context.isEmpty() || !context[context.size()-1].isSpace() )
        return textPosition;
    
    // Extract trailing whitespace in the context
    int contextPosition = context.size()-1;
    while( contextPosition > 0 && context[contextPosition-1].isSpace() )
        --contextPosition;
    
    QString contextWhiteSpace = context.mid(contextPosition);
    QString textWhiteSpace = text.mid(textPosition, textEnd-textPosition);
    
    // Step 1: Remove redundant newlines
    while(contextWhiteSpace.contains('\n') && textWhiteSpace.contains('\n'))
    {
        int contextOffset = contextWhiteSpace.indexOf('\n')+1;
        int textOffset = textWhiteSpace.indexOf('\n')+1;

        contextPosition += contextOffset;
        contextWhiteSpace.remove(0, contextOffset);

        textPosition += textOffset;
        textWhiteSpace.remove(0, textOffset);
    }
    
    if(textWhiteSpace.contains('\n'))
    {
        // There are remaining newlines which are not redundant, no more matching required.
        return textPosition;
    }
    
    if(contextWhiteSpace.contains('\n'))
    {
        // There are too many newlines in the context. To make everything correct, we would have
        // to remove those newlines, however we're not editing that area, thus there is nothing we can do.
        return textPosition;
    }
    
    // Step 2: Remove redundant whitespace
    
    if( textWhiteSpace.size() > contextWhiteSpace.size() )
        return textPosition + contextWhiteSpace.size(); // Skip the context white space
    else
        return textEnd;
}

QString extractFormattedTextFromContext( const QString& _formattedMergedText, const QString& /*originalMergedText*/, const QString& text, const QString& leftContext, const QString& rightContext)
{
    ///@todo Create unit-tests for this
    QString formattedMergedText = _formattedMergedText;
    //Now remove "leftContext" and "rightContext" from the sides
    if(!leftContext.isEmpty()) {
        int endOfLeftContext = matchPrefixIgnoringWhitespace( formattedMergedText, leftContext);
        if(endOfLeftContext == -1) {
            kWarning() << "problem matching the left context";
            return text;
        }
        
        int startOfWhiteSpace = endOfLeftContext;
        // Include all leading whitespace
        while(startOfWhiteSpace > 0 && formattedMergedText[startOfWhiteSpace-1].isSpace())
            --startOfWhiteSpace;
        
        endOfLeftContext = skipRedundantWhiteSpace( leftContext, formattedMergedText, startOfWhiteSpace, endOfLeftContext );
        
        formattedMergedText = formattedMergedText.mid(endOfLeftContext);
    }

    {
        //Add a whitespace behind the text for matching, so that we definitely capture all trailing whitespace
        int endOfText = matchPrefixIgnoringWhitespace( formattedMergedText, text+" ");
        if(endOfText == -1) {
            kWarning() << "problem matching the text while formatting";
            return text;
        }

        formattedMergedText = formattedMergedText.left(endOfText);

        // Simply reverse the string, and perform the same matching logic as above to remove redundant training newlines and/or whitespace
        QString rev = reverse(formattedMergedText);

        int endOfRightContext = 0;
        while(endOfRightContext < rev.size() && rev[endOfRightContext].isSpace())
            ++endOfRightContext;
        
        endOfRightContext = skipRedundantWhiteSpace( reverse(rightContext), rev, 0, endOfRightContext );
        
        rev = rev.mid( endOfRightContext );
        
        formattedMergedText = reverse(rev);
    }

    return formattedMergedText;
}

}
