/*
 * Copyright 2014  David Stevens <dgedstevens@gmail.com>
 * Copyright 2014  Kevin Funk <kfunk@kde.org>
 * Copyright 2015 Milian Wolff <mail@milianw.de>
 * Copyright 2015 Sergey Kalinichev <kalinichev.so.0@gmail.com>
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

#include "codecompletiontestbase.h"

#include <tests/testcore.h>
#include <tests/autotestshell.h>
#include <tests/testproject.h>

#include <interfaces/idocumentcontroller.h>

#include "clangsettings/clangsettingsmanager.h"

#include <language/codecompletion/codecompletiontesthelper.h>

#include <KTextEditor/Editor>
#include <KTextEditor/Document>
#include <KTextEditor/View>

#include <QLoggingCategory>

void DeleteDocument::operator()(KTextEditor::View* view) const
{
    // explicitly close the document when all references are valid, otherwise we have problems during cleanup
    const auto url = view->document()->url();
    ICore::self()->documentController()->documentForUrl(url)->close(IDocument::Discard);
}

std::unique_ptr<KTextEditor::View, DeleteDocument> CodeCompletionTestBase::createView(const QUrl& url) const
{
    auto doc = KDevelop::ICore::self()
                ->documentController()
                ->openDocument(url, KTextEditor::Range::invalid(), KDevelop::IDocumentController::DoNotActivate)
                ->textDocument();
    Q_ASSERT(doc);

    auto view = doc->views().first();
    Q_ASSERT(view);
    return std::unique_ptr<KTextEditor::View, DeleteDocument>(view);
}

void CodeCompletionTestBase::initTestCase()
{
    QLoggingCategory::setFilterRules(QStringLiteral("*.debug=false\ndefault.debug=true\nkdevelop.plugins.clang.debug=true\n"));
    QVERIFY(qputenv("KDEV_CLANG_DISPLAY_DIAGS", "1"));
    AutoTestShell::init({QStringLiteral("kdevclangsupport")});
    auto core = TestCore::initialize();
    delete core->projectController();
    m_projectController = new TestProjectController(core);
    core->setProjectController(m_projectController);
    ICore::self()->documentController()->closeAllDocuments();

    ClangSettingsManager::self()->m_enableTesting = true;
}

void CodeCompletionTestBase::cleanupTestCase()
{
    TestCore::shutdown();
}

void CodeCompletionTestBase::init()
{
    m_projectController->closeAllProjects();
}
