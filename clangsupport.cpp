/*
    This file is part of KDevelop

    Copyright 2013 Olivier de Gaalon <olivier.jg@gmail.com>
    Copyright 2013 Milian Wolff <mail@milianw.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "clangsupport.h"

#include "clangparsejob.h"
#include "version.h"

#include "duchain/clangtypes.h"

#include "codecompletion/model.h"

#include "documentfinderhelpers.h"

#include <interfaces/icore.h>
#include <interfaces/ilanguage.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/contextmenuextension.h>

#include "codegen/simplerefactoring.h"
#include "codegen/codeassistant.h"

#include <language/codecompletion/codecompletion.h>
#include <language/highlighting/codehighlighting.h>
#include <language/interfaces/editorcontext.h>

#include <KAction>
#include <KActionCollection>

K_PLUGIN_FACTORY(KDevClangSupportFactory, registerPlugin<ClangSupport>(); )
K_EXPORT_PLUGIN(KDevClangSupportFactory(
    KAboutData("kdevclangsupport", 0, ki18n("Clang Plugin"), "0.1",
    ki18n("Support for C, C++ and Objective-C languages."), KAboutData::License_GPL)))

using namespace KDevelop;

ClangSupport::ClangSupport(QObject* parent, const QVariantList& )
: IPlugin( KDevClangSupportFactory::componentData(), parent )
, ILanguageSupport()
, m_highlighting(new KDevelop::CodeHighlighting(this))
, m_index(new ClangIndex)
, m_assistant(new CodeAssistant)
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::ILanguageSupport )
    setXMLFile( "kdevclangsupport.rc" );

    m_refactoring = new SimpleRefactoring(this);
    new KDevelop::CodeCompletion( this, new ClangCodeCompletionModel(this), name() );
    for(const auto& type : DocumentFinderHelpers::mimeTypesList()){
        KDevelop::IBuddyDocumentFinder::addFinder(type, this);
    }
}

ClangSupport::~ClangSupport()
{
    for(const auto& type : DocumentFinderHelpers::mimeTypesList()) {
        KDevelop::IBuddyDocumentFinder::removeFinder(type);
    }
}

ParseJob* ClangSupport::createParseJob(const IndexedString& url)
{
    return new ClangParseJob(url, this);
}

QString ClangSupport::name() const
{
    return "clang";
}

ICodeHighlighting* ClangSupport::codeHighlighting() const
{
    return m_highlighting;
}

ClangIndex* ClangSupport::index()
{
    return m_index.data();
}

bool ClangSupport::areBuddies(const KUrl& url1, const KUrl& url2)
{
    return DocumentFinderHelpers::areBuddies(url1, url2);
}

bool ClangSupport::buddyOrder(const KUrl& url1, const KUrl& url2)
{
    return DocumentFinderHelpers::buddyOrder(url1, url2);
}

QVector< KUrl > ClangSupport::getPotentialBuddies(const KUrl& url) const
{
    return DocumentFinderHelpers::getPotentialBuddies(url);
}

void ClangSupport::createActionsForMainWindow (Sublime::MainWindow* /*window*/, QString& _xmlFile, KActionCollection& actions)
{
    _xmlFile = xmlFile();

    KAction* renameDeclarationAction = actions.addAction("code_rename_declaration");
    renameDeclarationAction->setText( i18n("Rename Declaration") );
    renameDeclarationAction->setIcon(KIcon("edit-rename"));
    renameDeclarationAction->setShortcut( Qt::CTRL | Qt::ALT | Qt::Key_R);
    connect(renameDeclarationAction, SIGNAL(triggered(bool)), m_refactoring, SLOT(executeRenameAction()));
}

KDevelop::ContextMenuExtension ClangSupport::contextMenuExtension(KDevelop::Context* context)
{
  ContextMenuExtension cm;
  EditorContext *ec = dynamic_cast<KDevelop::EditorContext *>(context);

  if (ec && ICore::self()->languageController()->languagesForUrl(ec->url()).contains(language())) {
    // It's a C++ file, let's add our context menu.
    m_refactoring->fillContextMenu(cm, context);
  }
  return cm;
}

#include "clangsupport.moc"
