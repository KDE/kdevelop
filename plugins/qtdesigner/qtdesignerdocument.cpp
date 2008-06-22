/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
/****************************************************************************
**
** Copyright 1992-2006 Trolltech AS. All rights reserved.
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qtdesignerdocument.h"

#include <QtDesigner/QDesignerFormWindowManagerInterface>
#include <QtDesigner/QDesignerFormWindowInterface>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtGui/QMdiArea>
#include <QtGui/QMdiSubWindow>
#include <QtCore/QFile>
#include <QApplication>
#include <KMessageBox>
#include <KLocale>

#include <icore.h>
#include <iuicontroller.h>
#include <idocumentcontroller.h>
#include <sublime/view.h>
#include <sublime/area.h>
#include <sublime/controller.h>

#include "qtdesignerview.h"
#include "qtdesignerplugin.h"

QtDesignerDocument::QtDesignerDocument( const KUrl& url , KDevelop::ICore* core )
    : Sublime::UrlDocument(core->uiController()->controller(), url),
      KDevelop::IDocument(core), m_state(KDevelop::IDocument::Clean)
{

}

KSharedPtr<KMimeType> QtDesignerDocument::mimeType() const
{
    return KMimeType::mimeType("application/x-designer");
}

KParts::Part* QtDesignerDocument::partForView(QWidget*) const
{
    return 0;
}

KTextEditor::Document* QtDesignerDocument::textDocument() const
{
    return 0;
}

bool QtDesignerDocument::save(KDevelop::IDocument::DocumentSaveMode mode)
{
    if (mode & Discard)
        return true;

    kDebug(9038) << "Going to Save";
    if( m_state == KDevelop::IDocument::Clean )
        return false;
    if( !m_form )
        return false;
    QFile f(url().toLocalFile());
    if( !f.open( QIODevice::WriteOnly ) )
    {
        kDebug(9038) << "Couldn't open file:" << f.error();
        return false;
    }
    QTextStream s(&f);
    s << m_form->contents();
    s.flush();
    f.close();
    m_state = KDevelop::IDocument::Clean;
    notifySaved();
    return true;
}

void QtDesignerDocument::reload()
{
    QFile uiFile(url().toLocalFile());
    m_form->setContents(&uiFile);
    m_state = KDevelop::IDocument::Clean;
    notifyStateChanged();
}

bool QtDesignerDocument::close(KDevelop::IDocument::DocumentSaveMode mode)
{
    kDebug() << "form:" << m_form;
    if (!(mode & Discard)) {
        if (mode & Silent) {
            if (!save(mode))
                return false;

        } else {
            if (state() == IDocument::Modified) {
                int code = KMessageBox::warningYesNoCancel(
                    qApp->activeWindow(),
                    i18n("The document \"%1\" has unsaved changes. Would you like to save them?", url().toLocalFile()),
                    i18n("Close Document"));

                if (code == KMessageBox::Yes) {
                    if (!save(mode))
                        return false;

                } else if (code == KMessageBox::Cancel) {
                    return false;
                }

            } else if (state() == IDocument::DirtyAndModified) {
                if (!save(mode))
                    return false;
            }
        }
    }

    //close all views and then delete ourself
    ///@todo test this
    foreach (Sublime::Area *area,
        KDevelop::ICore::self()->uiController()->controller()->allAreas())
    {
        QList<Sublime::View*> areaViews = area->views();
        foreach (Sublime::View *view, areaViews) {
            if (views().contains(view)) {
                kDebug() << "form before:" << m_form;
                kDebug() << "closing view" << view;
                kDebug() << "form after:" << m_form;
                area->removeView(view);
                delete view;
            }
        }
    }

//    kDebug() << "removing" << m_form << "from window manager";
 //   m_designerPlugin->designer()->formWindowManager()->removeFormWindow(m_form);

    KDevelop::ICore::self()->documentController()->notifyDocumentClosed(this);

    // Here we go...
    deleteLater();

    return true;
}

bool QtDesignerDocument::isActive() const
{
    QDesignerFormWindowInterface* activeWin =
            m_designerPlugin->designer()->formWindowManager()->activeFormWindow();
    if( activeWin == m_form )
        return true;
    return false;
}

KDevelop::IDocument::DocumentState QtDesignerDocument::state() const
{
    return m_state;
}

void QtDesignerDocument::setCursorPosition(const KTextEditor::Cursor&)
{
    return;
}

void QtDesignerDocument::activate(Sublime::View* view, KParts::MainWindow*)
{
    m_designerPlugin->designer()->formWindowManager()->setActiveFormWindow( m_form );
    notifyActivated();
}

void QtDesignerDocument::setDesignerPlugin(QtDesignerPlugin* plugin)
{
    m_designerPlugin = plugin;
}

Sublime::View *QtDesignerDocument::newView(Sublime::Document* doc)
{
    if( qobject_cast<QtDesignerDocument*>( doc ) ) {
        QFile uiFile(url().toLocalFile());

        m_form = designerPlugin()->designer()->formWindowManager()->createFormWindow();
        kDebug(9038) << "now we have" << m_form->core()->formWindowManager()->formWindowCount() << "formwindows";
        m_form->setFileName(url().toLocalFile());
        m_form->setContents(&uiFile);
        connect( m_form, SIGNAL(changed()), this, SLOT(formChanged()));
        designerPlugin()->designer()->formWindowManager()->setActiveFormWindow(m_form);
        return new QtDesignerView( this );
    }
    return 0;
}

QDesignerFormWindowInterface* QtDesignerDocument::form()
{
    return m_form;
}

void QtDesignerDocument::formChanged()
{
    m_state = KDevelop::IDocument::Modified;
    notifyStateChanged();
}

KTextEditor::Cursor QtDesignerDocument::cursorPosition( ) const
{
    return KTextEditor::Cursor();
}

QtDesignerPlugin* QtDesignerDocument::designerPlugin()
{
    return m_designerPlugin;
}

#include "qtdesignerdocument.moc"

