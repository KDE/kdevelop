/*
    SPDX-FileCopyrightText: 2014 David Stevens <dgedstevens@gmail.com>
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>
    SPDX-FileCopyrightText: 2015 Milian Wolff <mail@milianw.de>
    SPDX-FileCopyrightText: 2015 Sergey Kalinichev <kalinichev.so.0@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
    QStandardPaths::setTestModeEnabled(true);

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

#include "moc_codecompletiontestbase.cpp"
