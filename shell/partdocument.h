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
    PartDocument(const QUrl &url, ICore* core, const QString& preferredPart = QString() );
    virtual ~PartDocument();

    virtual QUrl url() const override;
    void setUrl(const QUrl& newUrl);

    virtual QWidget *createViewWidget(QWidget *parent = 0) override;
    virtual KParts::Part *partForView(QWidget *view) const override;

    virtual QMimeType mimeType() const override;
    virtual KTextEditor::Document* textDocument() const override;
    virtual bool save(DocumentSaveMode mode = Default) override;
    virtual void reload() override;
    ///Closes and deletes the document. Asks the user before if needed.
    virtual bool close(DocumentSaveMode mode = Default) override;
    virtual bool isActive() const override;
    virtual DocumentState state() const override;

    virtual void setPrettyName(QString name) override;
    
    virtual void activate(Sublime::View *activeView, KParts::MainWindow *mainWindow) override;

    virtual KTextEditor::Cursor cursorPosition() const override;
    virtual void setCursorPosition(const KTextEditor::Cursor &cursor) override;
    virtual void setTextSelection(const KTextEditor::Range &range) override;

    //Overridden from Sublime::Document
    virtual bool closeDocument(bool silent) override;
    virtual bool askForCloseFeedback() override;
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

