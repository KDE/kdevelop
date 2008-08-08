/*
 * KDevelop xUnit testing support
 *
 * Copyright 2008 Manuel Breugelmans
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "stubcontextaction.h"
#include "stubconstructor.h"
#include "classwriter.h"
#include "classskeleton.h"

#include <KAction>
#include <KDebug>
#include <KLocale>
#include <QFile>
#include <QDir>
#include <QInputDialog>
#include <QFileInfo>

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>

// kdevplatform
#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/context.h>
#include <language/duchain/declaration.h>
#include <language/duchain/duchainutils.h>
#include <language/editor/simplecursor.h>
#include <language/duchain/duchainlock.h>

// kdevelop
#include "languages/cpp/cppduchain/classdeclaration.h"

using Veritas::ClassSkeleton;
using Veritas::ClassSerializer;
using Veritas::StubConstructor;
using Veritas::StubContextAction;
using namespace KDevelop;
using namespace Cpp;

namespace
{
QString activeUrl()
{
    ICore* core = ICore::self();
    IDocumentController* dc = core->documentController(); 
    IDocument* doc = dc->activeDocument();
    if (!doc || !doc->textDocument() || !doc->textDocument()->activeView()) {
        return "";
    } else {
        QFileInfo fi(doc->url().path()); 
        return fi.absolutePath() + QDir::separator() + fi.baseName() + "stub." + fi.completeSuffix();
    }
}
} // end anonymous namespace

StubContextAction::StubContextAction(QObject* parent)
  : QObject(parent), m_clazz(0)
{
    m_stub = new KAction(this);
    m_stub->setText(i18n("Generate Stub Class"));
    connect(m_stub, SIGNAL(triggered()), this, SLOT(morph2stub()));
}

void StubContextAction::morph2stub()
{
    if (!m_clazz) return;

    bool owk;
    QString url;
    url = QInputDialog::getText(
              0, i18n("Generate Stub"),
              i18n("Save to "), QLineEdit::Normal,
              activeUrl(), &owk);
    if (!owk || url.isEmpty() || QFile::exists(url)) return;

    QFile target(url);
    ClassSkeleton cs = StubConstructor().morph(m_clazz);
    ClassSerializer().write(cs, &target);

    ICore::self()->documentController()->openDocument(KUrl(url));
}

StubContextAction::~StubContextAction()
{}

#define CONTINUE_UNLESS(X) \
  if (X) { m_clazz = 0; return cm; }

ContextMenuExtension StubContextAction::extension(Context* context)
{
    KDevelop::ContextMenuExtension cm;
    CONTINUE_UNLESS(context->type() != Context::EditorContext)
 
    EditorContext* ec = dynamic_cast<EditorContext*>(context);
    CONTINUE_UNLESS(not ec)
    
    DUChainWriteLocker lock(DUChain::lock());
    SimpleCursor sc(ec->position());
    Declaration* dcl = DUChainUtils::itemUnderCursor(ec->url(), sc);
    CONTINUE_UNLESS(!dcl)
    CONTINUE_UNLESS(dcl->kind() != Declaration::Type)

    ClassDeclaration* clazz = dynamic_cast<ClassDeclaration*>(dcl);
    CONTINUE_UNLESS(not clazz)

    m_clazz = clazz;
    cm.addAction(ContextMenuExtension::ExtensionGroup, m_stub);
    return cm;
}

#include "stubcontextaction.moc"
