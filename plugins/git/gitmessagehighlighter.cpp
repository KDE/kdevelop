/*
    SPDX-FileCopyrightText: 2011 Sergey Vidyuk <sir.vestnik@gmail.com>
    SPDX-FileCopyrightText: 2012 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "gitmessagehighlighter.h"

#include <QString>
#include <QTextCharFormat>
#include <QTextEdit>
#include <KLocalizedString>

/// Recommended summary limit from http://tbaggery.com/2008/04/19/a-note-about-git-commit-messages.html
static const int summarySoftLimit = 50;
/// Summary length limit causing warning messages from 'git push'
static const int summaryHardLimit = 65;
/// Recommended line length from http://tbaggery.com/2008/04/19/a-note-about-git-commit-messages.html
static const int lineLenLimit = 72;

void GitMessageHighlighter::applyErrorFormat(GitMessageHighlighter* text, bool warning, const QString& tooltip, int startPos, int endPos)
{
    QTextCharFormat format;
    format.setFontUnderline(true);
    format.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
    format.setUnderlineColor(warning ? Qt::yellow : Qt::red);
    format.setToolTip(tooltip);
    text->setFormat(startPos, endPos, format);
}

GitMessageHighlighter::GitMessageHighlighter(QTextEdit* parent): Sonnet::Highlighter(parent)
{
}

GitMessageHighlighter::~GitMessageHighlighter()
{
}

void GitMessageHighlighter::highlightBlock(const QString& text)
{
    int blockState = previousBlockState();
    if (blockState < DetailedMessage)
        ++blockState;
    const int textLength = text.length();
    int startPos = 0;
    int endPos = 0;
    while (startPos < textLength)
    {
        // Switch block state for multiline blocks
        if (startPos != 0 && blockState < DetailedMessage)
            ++blockState;
        endPos = text.indexOf(QLatin1Char('\n'), startPos);
        if (endPos < 0)
            endPos = textLength;
        const int lineLength = endPos - startPos;
        
        Highlighter::highlightBlock( text );
        switch (blockState)
        {
            case Summary:
                if (lineLength > summarySoftLimit)
                {
                    applyErrorFormat(this,
                        lineLength <= summaryHardLimit,
                        i18n("Try to keep summary length below %1 characters.", summarySoftLimit),
                        startPos, endPos
                    );
                } else {
                    for(int i=startPos; i<endPos; i++) {
                        QTextCharFormat f = format(i);
                        f.setFontWeight(QFont::Bold);
                        setFormat(i, 1, f);
                    }
                }
                break;
            case SummarySeparator:
                if (lineLength != 0)
                {
                    applyErrorFormat(this,
                        false,
                        i18n("Separate summary from details with one empty line."),
                        startPos, endPos
                    );
                }
                break;
            default:
                if (lineLength > lineLenLimit)
                {
                    applyErrorFormat(this,
                        false,
                        i18n("Try to keep line length below %1 characters.", lineLenLimit),
                        startPos+lineLenLimit, endPos
                    );
                }
                break;
        }
        startPos = endPos;
    }
    setCurrentBlockState(blockState);
}

