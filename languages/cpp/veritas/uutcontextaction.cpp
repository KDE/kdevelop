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

#include "uutcontextaction.h"
#include "uutconstructor.h"
#include "classwriter.h"
#include "includewriter.h"
#include "classskeleton.h"
#include "documentaccess.h"

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
#include <language/duchain/duchainlock.h>

// kdevelop
#include "languages/cpp/cppduchain/classdeclaration.h"

using Veritas::ClassSkeleton;
using Veritas::ClassSerializer;
using Veritas::IncludeGuardSerializer;
using Veritas::UUTConstructor;
using Veritas::UUTContextAction;
using Veritas::DocumentAccess;
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
        return fi.absolutePath() + QDir::separator() + fi.baseName() + "gen." + fi.completeSuffix();
    }
}
} // end anonymous namespace

UUTContextAction::UUTContextAction(QObject* parent)
  : QObject(parent), m_clazz(0), m_createImplementation(0), m_constructor(0)
{}

void UUTContextAction::setup()
{
    Q_ASSERT(m_clazz == 0); Q_ASSERT(m_createImplementation == 0); Q_ASSERT(m_constructor == 0);
    m_createImplementation = new KAction(this);
    m_createImplementation->setText(i18n("Generate Unit Under Test"));
    connect(m_createImplementation, SIGNAL(triggered()),
            this, SLOT(createImplementation()));
    DocumentAccess* docAccess = new DocumentAccess(this);
    m_constructor = new UUTConstructor;
    m_constructor->setDocumentAccess(docAccess);
    Q_ASSERT(m_createImplementation && m_constructor);
}

void UUTContextAction::createImplementation()
{
    Q_ASSERT(m_createImplementation); Q_ASSERT(m_constructor);
    if (!m_clazz) return;
    
    bool owk;
    QString url;
    url = QInputDialog::getText(
              0, i18n("Generate Unit Under Test"),
              i18n("Save to "), QLineEdit::Normal,
              activeUrl(), &owk);
    if (!owk || url.isEmpty() || QFile::exists(url)) return;

    QFile target(url);
    IncludeGuardSerializer().writeOpen(url, &target);
    ClassSkeleton cs = m_constructor->morph(m_clazz);
    ClassSerializer().write(cs, &target);
    IncludeGuardSerializer().writeClose(url, &target);

    ICore::self()->documentController()->openDocument(KUrl(url));
}

UUTContextAction::~UUTContextAction()
{
    if (m_createImplementation) delete m_createImplementation;
    if (m_constructor) delete m_constructor;
}

#define STOP_IF(X) \
if (X) { m_clazz = 0; return; } \
else (void)(0)

#define STOP_IF_(X, MSG) \
if (X) {\
    m_clazz = 0;\
    kDebug() << "Not appending UUT action because " << MSG;\
    return;\
} else (void)(0)


void UUTContextAction::appendTo(ContextMenuExtension& menu, Context* context)
{
    Q_ASSERT(m_createImplementation); Q_ASSERT(m_constructor);
    STOP_IF(context->type() != Context::EditorContext);
    EditorContext* ec = dynamic_cast<EditorContext*>(context);
    STOP_IF(!ec);

    DUChainWriteLocker lock(DUChain::lock());
    SimpleCursor sc(ec->position());
    Declaration* decl = DUChainUtils::itemUnderCursor(ec->url(), sc);
    STOP_IF_(!decl, "no declaration under cursor.");
    STOP_IF_(decl->kind() != Declaration::Instance, "Not an instance declaration.");
    STOP_IF_(!decl->isDefinition(), "Not a definition");

    DelayedType::Ptr type = decl->type<DelayedType>();
    STOP_IF_(!type, "Not a delayed/unresolved type (null)");
    STOP_IF_(type->kind() != DelayedType::Unresolved, "Not an unresolved type [but delayed]");

    m_clazz = decl;
    menu.addAction(ContextMenuExtension::ExtensionGroup, m_createImplementation);
    Q_ASSERT(m_clazz);
}

#include "uutcontextaction.moc"

