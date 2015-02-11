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
#ifndef KDEVPLATFORM_PLUGIN_GITMESSAGEHIGHLIGHTER_H
#define KDEVPLATFORM_PLUGIN_GITMESSAGEHIGHLIGHTER_H

#include <sonnet/highlighter.h>

class GitMessageHighlighter : public Sonnet::Highlighter
{
    Q_OBJECT
public:
    explicit GitMessageHighlighter(QTextEdit* parent);
    virtual ~GitMessageHighlighter();

protected:
    virtual void highlightBlock(const QString& text) override;

private:
    void applyErrorFormat(GitMessageHighlighter* text, bool warning, const QString& tooltip, int startPos, int endPos);
    
    enum BlockState {
        NoState = -1,
        Summary,
        SummarySeparator,
        DetailedMessage
    };
};

#endif // KDEVPLATFORM_PLUGIN_GITMESSAGEHIGHLIGHTER_H
