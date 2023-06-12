/*
    SPDX-FileCopyrightText: 2011 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "formattinghelpers.h"

#include "debug.h"

#include <QString>
#include <QStringView>

namespace KDevelop {

///Matches the given prefix to the given text, ignoring all whitespace
///Returns -1 if mismatched, else the position in @p text where the @p prefix match ends
int matchPrefixIgnoringWhitespace(QStringView text, QStringView prefix, QStringView fuzzyCharacters)
{
    int prefixPos = 0;
    int textPos = 0;

    while (prefixPos < prefix.length() && textPos < text.length()) {
skipWhiteSpace:
        while (prefixPos < prefix.length() && prefix[prefixPos].isSpace())
            ++prefixPos;
        while (textPos < text.length() && text[textPos].isSpace())
            ++textPos;

        if (prefixPos == prefix.length() || textPos == text.length())
            break;

        if (prefix[prefixPos] != text[textPos]) {
            bool skippedFuzzy = false;
            while (prefixPos < prefix.length() && fuzzyCharacters.indexOf(prefix[prefixPos]) != -1) {
                ++prefixPos;
                skippedFuzzy = true;
            }
            while (textPos < text.length() && fuzzyCharacters.indexOf(text[textPos]) != -1) {
                ++textPos;
                skippedFuzzy = true;
            }

            if (skippedFuzzy)
                goto skipWhiteSpace;

            return -1;
        }
        ++prefixPos;
        ++textPos;
    }
    return textPos;
}

static QString reverse(QStringView str)
{
    QString ret;
    ret.reserve(str.length());
    for (int a = str.length() - 1; a >= 0; --a)
        ret.append(str[a]);

    return ret;
}

// Returns the text start position with all whitespace that is redundant in the given context skipped
int skipRedundantWhiteSpace(QStringView context, QStringView text, int tabWidth)
{
    if (context.isEmpty() || !context[context.size() - 1].isSpace() || text.isEmpty() || !text[0].isSpace())
        return 0;

    int textPosition = 0;

    // Extract trailing whitespace in the context
    int contextPosition = context.size() - 1;
    while (contextPosition > 0 && context[contextPosition - 1].isSpace())
        --contextPosition;

    int textWhitespaceEnd = 0;
    while (textWhitespaceEnd < text.size() && text[textWhitespaceEnd].isSpace())
        ++textWhitespaceEnd;

    auto contextWhiteSpace = context.mid(contextPosition);
    auto textWhiteSpace = text.left(textWhitespaceEnd);

    // Step 1: Remove redundant newlines
    while (contextWhiteSpace.contains(QLatin1Char('\n')) && textWhiteSpace.contains(QLatin1Char('\n'))) {
        int contextOffset = contextWhiteSpace.indexOf(QLatin1Char('\n')) + 1;
        int textOffset = textWhiteSpace.indexOf(QLatin1Char('\n')) + 1;

        contextWhiteSpace = contextWhiteSpace.mid(contextOffset);

        textPosition += textOffset;
        textWhiteSpace = textWhiteSpace.mid(textOffset);
    }

    int contextOffset = 0;
    int textOffset = 0;
    // Skip redundant ordinary whitespace
    while (contextOffset < contextWhiteSpace.size() && textOffset < textWhiteSpace.size() &&
           contextWhiteSpace[contextOffset].isSpace() && contextWhiteSpace[contextOffset] != QLatin1Char('\n') &&
           textWhiteSpace[textOffset].isSpace() && textWhiteSpace[textOffset] != QLatin1Char('\n')) {
        bool contextWasTab = contextWhiteSpace[contextOffset] == QLatin1Char('\t');
        bool textWasTab = textWhiteSpace[contextOffset] == QLatin1Char('\t');
        ++contextOffset;
        ++textOffset;
        if (contextWasTab != textWasTab) {
            // Problem: We have a mismatch of tabs and/or ordinary whitespaces
            if (contextWasTab) {
                for (int s = 1; s < tabWidth; ++s)
                    if (textOffset < textWhiteSpace.size() && textWhiteSpace[textOffset] == QLatin1Char(' '))
                        ++textOffset;
            } else if (textWasTab) {
                for (int s = 1; s < tabWidth; ++s)
                    if (contextOffset < contextWhiteSpace.size() &&
                        contextWhiteSpace[contextOffset] == QLatin1Char(' '))
                        ++contextOffset;
            }
        }
    }

    return textPosition + textOffset;
}

QString extractFormattedTextFromContext(const QString& _formattedMergedText, const QString& text,
                                        QStringView leftContext, QStringView rightContext, int tabWidth,
                                        QStringView fuzzyCharacters)
{
    if (leftContext.isEmpty() && rightContext.isEmpty()) {
        return _formattedMergedText; // fast path, nothing to do
    }

    QStringView formattedMergedText = _formattedMergedText;
    //Now remove "leftContext" and "rightContext" from the sides

    if (!leftContext.isEmpty()) {
        int endOfLeftContext = matchPrefixIgnoringWhitespace(formattedMergedText, leftContext, QStringView{});
        if (endOfLeftContext == -1) {
            // Try 2: Ignore the fuzzy characters while matching
            endOfLeftContext = matchPrefixIgnoringWhitespace(formattedMergedText, leftContext, fuzzyCharacters);
            if (endOfLeftContext == -1) {
                qCWarning(UTIL) << "problem matching the left context";
                return text;
            }
        }

        int startOfWhiteSpace = endOfLeftContext;
        // Include all leading whitespace
        while (startOfWhiteSpace > 0 && formattedMergedText[startOfWhiteSpace - 1].isSpace())
            --startOfWhiteSpace;

        formattedMergedText = formattedMergedText.mid(startOfWhiteSpace);

        int skip = skipRedundantWhiteSpace(leftContext, formattedMergedText, tabWidth);

        formattedMergedText = formattedMergedText.mid(skip);
    }

    if (!rightContext.isEmpty()) {
        int endOfText = matchPrefixIgnoringWhitespace(formattedMergedText, text, QStringView{});
        if (endOfText == -1) {
            // Try 2: Ignore the fuzzy characters while matching
            endOfText = matchPrefixIgnoringWhitespace(formattedMergedText, text, fuzzyCharacters);
            if (endOfText == -1) {
                qCWarning(UTIL) << "problem matching the text while formatting";
                return text;
            }
        }

        // Include all trailing whitespace
        while (endOfText < formattedMergedText.size() && formattedMergedText.at(endOfText).isSpace()) {
            ++endOfText;
        }

        formattedMergedText.truncate(endOfText);

        int skip = skipRedundantWhiteSpace(reverse(rightContext), reverse(formattedMergedText), tabWidth);

        formattedMergedText.chop(skip);
    }

    return formattedMergedText.toString();
}

}
