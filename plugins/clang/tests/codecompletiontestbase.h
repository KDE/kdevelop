/*
    SPDX-FileCopyrightText: 2014 David Stevens <dgedstevens@gmail.com>
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>
    SPDX-FileCopyrightText: 2016 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef CODECOMPLETIONTESTBASE_H
#define CODECOMPLETIONTESTBASE_H

#include <QObject>

#include <memory>

namespace KTextEditor {
class View;
}

namespace KDevelop
{
class TestProjectController;
}

struct DeleteDocument
{
    void operator()(KTextEditor::View* view) const;
};

class CodeCompletionTestBase : public QObject
{
    Q_OBJECT

public:
    std::unique_ptr<KTextEditor::View, DeleteDocument> createView(const QUrl& url) const;

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();

protected:
    KDevelop::TestProjectController* m_projectController;
};

#endif // CODECOMPLETIONTESTBASE_H
