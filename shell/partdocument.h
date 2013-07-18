/***************************************************************************
 *   Copyright 2007 Alexander Dymo  <adymo@kdevelop.org>            *
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
#ifndef KDEVPLATFORM_PARTDOCUMENT_H
#define KDEVPLATFORM_PARTDOCUMENT_H

#include <interfaces/idocument.h>
#include <sublime/urldocument.h>

#include "shellexport.h"

namespace KParts {
class Part;
}

namespace KDevelop {

/**
The generic document which represents KParts.

This document is used by shell when more specific document classes
are incapable of loading the url.

This document loads one KPart (read-only or read-write) per view
and sets part widget to be a view widget.
*/
class KDEVPLATFORMSHELL_EXPORT PartDocument: public Sublime::UrlDocument, public KDevelop::IDocument {
    Q_OBJECT
public:
    PartDocument(const KUrl &url, ICore* core, const QString& preferredPart = QString() );
    virtual ~PartDocument();

    virtual KUrl url() const;
    void setUrl(const KUrl& newUrl);

    virtual QWidget *createViewWidget(QWidget *parent = 0);
    virtual KParts::Part *partForView(QWidget *view) const;

    virtual KMimeType::Ptr mimeType() const;
    virtual KTextEditor::Document* textDocument() const;
    virtual bool save(DocumentSaveMode mode = Default);
    virtual void reload();
    ///Closes and deletes the document. Asks the user before if needed.
    virtual bool close(DocumentSaveMode mode = Default);
    virtual bool isActive() const;
    virtual DocumentState state() const;

    virtual void setPrettyName(QString name);
    
    virtual void activate(Sublime::View *activeView, KParts::MainWindow *mainWindow);

    virtual KTextEditor::Cursor cursorPosition() const;
    virtual void setCursorPosition(const KTextEditor::Cursor &cursor);
    virtual void setTextSelection(const KTextEditor::Range &range);

    //Overridden from Sublime::Document
    virtual bool closeDocument(bool silent);
    virtual bool askForCloseFeedback();
protected:
    /** Gives us access to the KParts */
    QMap<QWidget*, KParts::Part*> partForView() const;
    
    /** Lets us override the createViewWidget method safely */
    void addPartForView(QWidget* widget, KParts::Part* part);
    
private:
    class PartDocumentPrivate * const d;
};

}

#endif

