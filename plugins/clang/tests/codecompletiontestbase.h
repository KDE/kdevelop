/*
 * Copyright 2014  David Stevens <dgedstevens@gmail.com>
 * Copyright 2014  Kevin Funk <kfunk@kde.org>
 * Copyright 2016  Milian Wolff <mail@milianw.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
