/*
  * This file is part of KDevelop
 *
 * Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>
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

#include "browsemanager.h"
#include <QMouseEvent>
#include <QToolButton>
#include <kdebug.h>
#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include "contextbrowserview.h"
#include <ktexteditor/smartinterface.h>
#include <interfaces/ilanguage.h>
#include <interfaces/ilanguagecontroller.h>
#include <language/interfaces/ilanguagesupport.h>
#include <language/editor/simplecursor.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <language/duchain/declaration.h>

using namespace KDevelop;
using namespace KTextEditor;

static QWidget* masterWidget(QWidget* w) {
  while(w && w->parentWidget())
    w = w->parentWidget();
  return w;
}

EditorViewWatcher::EditorViewWatcher(QWidget* sameWindow) : m_childrenOf(masterWidget(sameWindow)) {
    
    connect(ICore::self()->documentController(), SIGNAL(textDocumentCreated(KDevelop::IDocument*)), this, SLOT(documentCreated(KDevelop::IDocument*)));
    foreach(KDevelop::IDocument* document, ICore::self()->documentController()->openDocuments())
        documentCreated(document);
}

void EditorViewWatcher::documentCreated( KDevelop::IDocument* document ) {
    KTextEditor::Document* textDocument = document->textDocument();
    if(textDocument) {
        connect(textDocument, SIGNAL(viewCreated(KTextEditor::Document*, KTextEditor::View*)), this, SLOT(viewCreated(KTextEditor::Document*, KTextEditor::View*)));
        foreach(KTextEditor::View* view, textDocument->views()) {
            Q_ASSERT(view->parentWidget());
//             if(!m_childrenOf || masterWidget(view) == m_childrenOf)
                addViewInternal(view);
        }
    }
}

void EditorViewWatcher::addViewInternal(KTextEditor::View* view) {
    m_views << view;
    viewAdded(view);
    connect(view, SIGNAL(destroyed(QObject*)), this, SLOT(viewDestroyed(QObject*)));
}

void EditorViewWatcher::viewAdded(KTextEditor::View*) {
}

void EditorViewWatcher::viewDestroyed(QObject* view) {
    m_views.removeAll(static_cast<KTextEditor::View*>(view));
}

void EditorViewWatcher::viewCreated(KTextEditor::Document* /*doc*/, KTextEditor::View* view) {
    Q_ASSERT(view->parentWidget());
    //The test doesn't work porperly at this point
//     if(!m_childrenOf || masterWidget(view) == m_childrenOf)
        addViewInternal(view);
}

QList<KTextEditor::View*> EditorViewWatcher::allViews() {
    return m_views;
}

BrowseManager::BrowseManager(ContextController* controller) : QObject(controller), m_controller(controller), m_watcher(this), m_browsing(false), m_browsingByKey(false) {
}

KTextEditor::View* viewFromWidget(QWidget* widget) {
    if(!widget)
        return 0;
    KTextEditor::View* view = qobject_cast<KTextEditor::View*>(widget);
    if(view)
        return view;
    else
        return viewFromWidget(widget->parentWidget());
}

bool BrowseManager::eventFilter(QObject * watched, QEvent * event) {
    QWidget* widget = qobject_cast<QWidget*>(watched);
    Q_ASSERT(widget);
    KTextEditor::View* view = viewFromWidget(widget);
    if(!view)
        return false;
    
    const int browseKey = Qt::Key_Control;
    
    QKeyEvent* keyEvent = dynamic_cast<QKeyEvent*>(event);
    if(keyEvent && keyEvent->key() == browseKey) {
        if(!m_browsingByKey && keyEvent->type() == QEvent::KeyPress) {
            m_browsingByKey = true;
            if(!m_browsing)
                m_controller->browseButton()->setChecked(true);
            
        } else if(m_browsingByKey && keyEvent->type() == QEvent::KeyRelease) {
            m_browsingByKey = false;
            if(!m_browsing)
                m_controller->browseButton()->setChecked(false);
        }
    }
    
    if(!m_browsing && !m_browsingByKey) {
        resetChangedCursor();
        return false;
    }
    
    QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);
    if(mouseEvent) {
        KTextEditor::CoordinatesToCursorInterface* iface = dynamic_cast<KTextEditor::CoordinatesToCursorInterface*>(view);
        if(!iface) {
            kDebug() << "Update kdelibs for the browsing-mode to work";
            return false;
        }
        
        QPoint coordinatesInView = widget->mapTo(view, mouseEvent->pos());
        
        KTextEditor::Cursor textCursor = iface->coordinatesToCursor(coordinatesInView);
        if(textCursor.isValid()) {
            SmartInterface* iface = dynamic_cast<SmartInterface*>(view->document());
            if (!iface) return false;
            
            ///@todo find out why this is needed, fix the code in kate
            if(textCursor.column() > 0)
                textCursor.setColumn(textCursor.column()-1);

            KUrl viewUrl = view->document()->url();
            QList<ILanguage*> languages = ICore::self()->languageController()->languagesForUrl(viewUrl);

            QPair<KUrl, KDevelop::SimpleCursor> jumpTo;

            //Step 1: Look for a special language object(Macro, included header, etc.)
            foreach( ILanguage* language, languages) {
                jumpTo = language->languageSupport()->specialLanguageObjectJumpCursor(viewUrl, SimpleCursor(textCursor));
                if(jumpTo.first.isValid() && jumpTo.second.isValid())
                    break; //Found a special object to jump to
            }

            //Step 2: Look for a declaration/use
            if(!jumpTo.first.isValid() || !jumpTo.second.isValid()) {
                Declaration* foundDeclaration = 0;
                KDevelop::DUChainReadLocker lock( DUChain::lock() );
                foundDeclaration = DUChainUtils::declarationForDefinition( DUChainUtils::itemUnderCursor(view->document()->url(), SimpleCursor(textCursor)) );
                
                if( foundDeclaration ) {
                    jumpTo.first = foundDeclaration->url().toUrl();
                    jumpTo.second = foundDeclaration->range().start;
                }
            }
            if(jumpTo.first.isValid() && jumpTo.second.isValid()) {
                if(mouseEvent->button() == Qt::LeftButton && mouseEvent->type() == QEvent::MouseButtonPress) {
                    ICore::self()->documentController()->openDocument(jumpTo.first, jumpTo.second.textCursor());
                    event->accept();
                    return true;
                }else if(mouseEvent->type() == QEvent::MouseMove) {
                    //Make the cursor a "hand"
                    setHandCursor(widget);
                    return false;
                }
            }
        }
        resetChangedCursor();
    }
    return false;
}

void BrowseManager::resetChangedCursor() {
    QMap<QPointer<QWidget>, QCursor> cursors = m_oldCursors;
    m_oldCursors.clear();
    
    for(QMap<QPointer<QWidget>, QCursor>::iterator it = cursors.begin(); it != cursors.end(); ++it)
        if(it.key())
            it.key()->setCursor(QCursor(Qt::IBeamCursor));
}

void BrowseManager::setHandCursor(QWidget* widget) {
    if(m_oldCursors.contains(widget))
        return; //Nothing to do
    m_oldCursors[widget] = widget->cursor();
    widget->setCursor(QCursor(Qt::PointingHandCursor));
}

void BrowseManager::applyEventFilter(QWidget* object, bool install) {
    if(install)
        object->installEventFilter(this);
    else
        object->removeEventFilter(this);
    
    foreach(QObject* child, object->children())
        if(qobject_cast<QWidget*>(child))
            applyEventFilter(qobject_cast<QWidget*>(child), install);
}

void BrowseManager::Watcher::viewAdded(KTextEditor::View* view) {
    m_manager->applyEventFilter(view, true);
}

void BrowseManager::setBrowsing(bool enabled) {
    if(m_browsingByKey)
        return;
    if(enabled == m_browsing)
        return;
    m_browsing = enabled;
    
    //This collects all the views
    if(enabled) {
        kDebug() << "Enabled browsing-mode";
        ///Enable browsing, install an event-filter on all events
        foreach(KTextEditor::View* view, m_watcher.allViews())
            applyEventFilter(view, true);
    }else{
        kDebug() << "Disabled browsing-mode";
        resetChangedCursor();
    }
}

BrowseManager::Watcher::Watcher(BrowseManager* manager) : EditorViewWatcher(masterWidget(manager->m_controller->view())), m_manager(manager) {
    foreach(KTextEditor::View* view, allViews())
        m_manager->applyEventFilter(view, true);
}


