/*
 * KDevelop xUnit testing support
 *
 * Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
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
#include "includewriter.h"
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
#include <interfaces/contextmenuextension.h>
#include <language/duchain/declaration.h>
#include <language/duchain/duchainutils.h>
#include <language/editor/simplecursor.h>
#include <language/interfaces/editorcontext.h>
#include <language/duchain/duchainlock.h>

// kdevelop
#include <language/duchain/classdeclaration.h>

using Veritas::ClassSkeleton;
using Veritas::ClassSerializer;
using Veritas::StubConstructor;
using Veritas::IncludeSerializer;
using Veritas::IncludeGuardSerializer;
using Veritas::StubContextAction;
using namespace KDevelop;

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
        return doc->url().toLocalFile();
    }
}

QString stubUrl()
{
    ICore* core = ICore::self();
    IDocumentController* dc = core->documentController(); 
    IDocument* doc = dc->activeDocument();
    if (!doc || !doc->textDocument() || !doc->textDocument()->activeView()) {
        return "";
    } else {
        QFileInfo fi(doc->url().toLocalFile()); 
        return fi.absolutePath() + QDir::separator() + fi.baseName() + "stub." + fi.completeSuffix();
    }
}

} // end anonymous namespace

StubContextAction::StubContextAction(QObject* parent)
  : QObject(parent), m_clazz(0), m_constructStub(0)
{
}

/*! initialization */
void StubContextAction::setup()
{
    Q_ASSERT(m_clazz == 0); Q_ASSERT(m_constructStub == 0);
    m_constructStub = new KAction(this);
    m_constructStub->setText(i18n("Generate Stub Class"));
    connect(m_constructStub, SIGNAL(triggered()), this, SLOT(constructStub()));
    Q_ASSERT(m_constructStub);
}

void StubContextAction::constructStub()
{
    Q_ASSERT(m_constructStub);
    if (!m_clazz) return;

    bool owk;
    QString url;
    QString current = activeUrl();
    url = QInputDialog::getText(
              0, i18n("Generate Stub"),
              i18n("Save to "), QLineEdit::Normal,
              stubUrl(), &owk);
    if (!owk || url.isEmpty() || QFile::exists(url)) return;

    QFile target(url);
    IncludeGuardSerializer().writeOpen(url, &target);
    IncludeSerializer().write(current, url, &target);
    ClassSkeleton cs = StubConstructor().morph(m_clazz);
    ClassSerializer().write(cs, &target);
    IncludeGuardSerializer().writeClose(url, &target);

    ICore::self()->documentController()->openDocument(KUrl(url));
}

StubContextAction::~StubContextAction()
{}

#define STOP_IF(X) \
  if (X) { m_clazz = 0; return; }\
  else (void)0

void StubContextAction::appendTo(ContextMenuExtension& menu, Context* context)
{
    KDevelop::ContextMenuExtension cm;
    STOP_IF(context->type() != Context::EditorContext);

    EditorContext* ec = dynamic_cast<EditorContext*>(context);
    STOP_IF(!ec);

    DUChainWriteLocker lock(DUChain::lock());
    ///@todo this isn't neede any more, the context contains the information
    SimpleCursor sc(ec->position());
    Declaration* dcl = DUChainUtils::itemUnderCursor(ec->url(), sc);
    STOP_IF(!dcl);
    STOP_IF(dcl->kind() != Declaration::Type);

    ClassDeclaration* clazz = dynamic_cast<ClassDeclaration*>(dcl);
    STOP_IF(!clazz);

    m_clazz = clazz;
    menu.addAction(ContextMenuExtension::ExtensionGroup, m_constructStub);
    Q_ASSERT(m_clazz);
}

#include "stubcontextaction.moc"
