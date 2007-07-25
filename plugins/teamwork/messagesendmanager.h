/***************************************************************************
Copyright 2006 David Nolden <david.nolden.kdevelop@art-master.de>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MESSAGESENDMANAGER_H
#define MESSAGESENDMANAGER_H

#include <QObject>
#include <QPointer>
#include "ui_kdevteamwork_interface.h"
#include <ktexteditor/range.h>

namespace KDevelop {
class IDocument;
}

namespace  KTextEditor {
  class View;
  class Document;
}

/**This cares about the "send message"-tab in the kdevteamwork user-interface by updating it, according to the current selection in the current document.
 * */
class MessageSendManager : public QObject {
  Q_OBJECT
  public:
    MessageSendManager( Ui::Teamwork& widgets );

  private slots:
    void documentActivated( KDevelop::IDocument* document );
    void documentClosed( KDevelop::IDocument* document );
    void documentDestroyed( QObject* obj );
    void selectionChanged( KTextEditor::View* );
    void viewCreated( KTextEditor::Document* , KTextEditor::View* );
  private:
    void  connectView( KTextEditor::View* view );
    void connectDocument( KDevelop::IDocument* );
    void disconnectDocument();
    Ui::Teamwork& m_widgets;
    KDevelop::IDocument* m_lastDocument;

    KTextEditor::View* view();
    
    KUrl m_currentFile; ///Currently edited file
    KTextEditor::Range m_currentSelection; ///Current selection within that file
};

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on

