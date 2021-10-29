/*
    SPDX-FileCopyrightText: 2011 Sergey Vidyuk <sir.vestnik@gmail.com>
    SPDX-FileCopyrightText: 2012 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KDEVPLATFORM_PLUGIN_GITMESSAGEHIGHLIGHTER_H
#define KDEVPLATFORM_PLUGIN_GITMESSAGEHIGHLIGHTER_H

#include <sonnet/highlighter.h>

class GitMessageHighlighter : public Sonnet::Highlighter
{
    Q_OBJECT
public:
    explicit GitMessageHighlighter(QTextEdit* parent);
    ~GitMessageHighlighter() override;

protected:
    void highlightBlock(const QString& text) override;

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
