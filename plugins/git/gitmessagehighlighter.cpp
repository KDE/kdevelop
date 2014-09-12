/***************************************************************************
 *   Copyright 2011 Sergey Vidyuk <sir.vestnik@gmail.com>                  *
 *   Copyright 2012 Aleix Pol Gonzalez <aleixpol@kde.org>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/
#include "gitmessagehighlighter.h"

#include <QtCore/QString>
#include <QTextCharFormat>
#include <QTextEdit>
#include <KLocalizedString>

/// Recomended summary limit from http://tbaggery.com/2008/04/19/a-note-about-git-commit-messages.html
static const int summarySoftLimit = 50;
/// Summary length limit causing warning messages from 'git push'
static const int summaryHardLimit = 65;
/// Recommended line kength from http://tbaggery.com/2008/04/19/a-note-about-git-commit-messages.html
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
        endPos = text.indexOf('\n', startPos);
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

