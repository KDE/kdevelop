
/*
 * This file is part of KDevelop
 *
 * Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>
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

#ifndef CONTEXTBROWSERPLUGIN_H
#define CONTEXTBROWSERPLUGIN_H

#include <iplugin.h>
#include <QtCore/QVariant>
#include <QSet>
#include <QMap>
#include <QList>
#include <duchain/duchainpointer.h>
#include <ktexteditor/smartrange.h>
#include <ktexteditor/rangefeedback.h>
#include <simplecursor.h>
#include <irunprovider.h>

namespace KDevelop {
  class IDocument;
  class ParseJob;
  class DUContext;
  class DUChainBase;
}

namespace KTextEditor {
  class View;
}

using namespace KDevelop;

class ContextBrowserViewFactory;
class ContextBrowserView;

class ContextBrowserPlugin : public KDevelop::IPlugin, public KTextEditor::SmartRangeWatcher
{
    Q_OBJECT
  public:
    ContextBrowserPlugin(QObject *parent, const QVariantList & = QVariantList() );
    virtual ~ContextBrowserPlugin();

    virtual void unload();

    void registerToolView(ContextBrowserView* view);
    void unRegisterToolView(ContextBrowserView* view);

  Q_SIGNALS:
    void previousContextShortcut();
    void nextContextShortcut();
    
  private slots:
    void parseJobFinished(KDevelop::ParseJob* job);
    void documentLoaded( KDevelop::IDocument* document );
    void documentClosed( KDevelop::IDocument* document );
    void documentDestroyed( QObject* obj );
    void viewDestroyed( QObject* obj );
    void cursorPositionChanged( KTextEditor::View* view, const KTextEditor::Cursor& newPosition );
    void viewCreated( KTextEditor::Document* , KTextEditor::View* );
    void updateViews();
    
    void textHintRequested(const KTextEditor::Cursor&, QString&);
    
  private:
    void clearMouseHover();
    virtual void rangeDeleted (KTextEditor::SmartRange *range);
    virtual void mouseEnteredRange(KTextEditor::SmartRange* range, KTextEditor::View* view);
    virtual void mouseExitedRange(KTextEditor::SmartRange* range, KTextEditor::View* view);
  
    void changeHighlight( KTextEditor::SmartRange* range, bool highlight, bool declaration, bool mouseHighlight );
    void changeHighlight( KTextEditor::View* view, KDevelop::Declaration* decl, bool highlight, bool mouseHighlight );

    void registerAsRangeWatcher(KDevelop::DUChainBase* base);
    void registerAsRangeWatcher(KDevelop::DUContext* ctx);
  
    QTimer* m_updateTimer;
    QMap<KTextEditor::SmartRange*, KTextEditor::Attribute::Ptr> m_backups;
    QSet<KTextEditor::View*> m_updateViews;
    QMap<KTextEditor::View*, DeclarationPointer> m_highlightedDeclarations;
    QMap<KTextEditor::View*, KTextEditor::SmartRange*> m_highlightedRange; //Special language-object range

    //Holds a list of all active context browser tool views
    QList<ContextBrowserView*> m_views;

    KUrl m_mouseHoverDocument;
    SimpleCursor m_mouseHoverCursor;
    ContextBrowserViewFactory* m_viewFactory;
};

#endif // CONTEXTBROWSERPLUGIN_H

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
