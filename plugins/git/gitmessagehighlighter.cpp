/***************************************************************************
 *   Copyright 2011 Sergey Vidyuk <sir.vestnik@gmail.com>                  *
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
#include <QtGui/QTextCharFormat>
#include <QtGui/QTextEdit>
#include <KDE/KLocalizedString>

/// Recomended summary limit from http://tbaggery.com/2008/04/19/a-note-about-git-commit-messages.html
static const int summarySoftLimit = 50;
/// Summary length limit causing warning messages from 'git push'
static const int summaryHardLimit = 65;
/// Recommended line kength from http://tbaggery.com/2008/04/19/a-note-about-git-commit-messages.html
static const int lineLenLimit = 72;

static inline
void applyErrorFormat(QTextCharFormat* format, bool warning, const QString& tooltip)
{
    format->setFontUnderline(true);
    format->setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
    format->setUnderlineColor(warning ? Qt::yellow : Qt::red);
    format->setToolTip(tooltip);
}

GitMessageHighlighter::GitMessageHighlighter(QTextEdit* parent): QSyntaxHighlighter(parent)
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
        QTextCharFormat format;
        const int lineLength = endPos - startPos;
        switch (blockState)
        {
            case Summary:
                format.setFontWeight(QFont::Bold);
                if (lineLength > summarySoftLimit)
                {
                    applyErrorFormat(
                        &format,
                        lineLength <= summaryHardLimit,
                        i18n("Try to keep summary length below %1 characters.").arg(summarySoftLimit)
                    );
                }
                break;
            case SummarySeparator:
                if (lineLength != 0)
                {
                    applyErrorFormat(
                        &format,
                        false,
                        i18n("Separate summary from details with one empty line.")
                    );
                }
                break;
            default:
                if (lineLength > lineLenLimit)
                {
                    applyErrorFormat(
                        &format,
                        false,
                        i18n("Try to keep line length below %1 characters.").arg(lineLenLimit)
                    );
                }
                break;
        }
        setFormat(startPos, endPos, format);
        startPos = endPos;
    }
    setCurrentBlockState(blockState);
}

#include "gitmessagehighlighter.moc"
