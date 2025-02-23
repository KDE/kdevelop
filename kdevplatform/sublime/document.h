/*
    SPDX-FileCopyrightText: 2006-2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_SUBLIMEDOCUMENT_H
#define KDEVPLATFORM_SUBLIMEDOCUMENT_H

#include <QObject>
#include <QList>


#include "sublimeexport.h"

class QIcon;

class QWidget;

namespace Sublime {

class Area;
class View;
class Controller;
class DocumentPrivate;

/**
@short Abstract base class for all Sublime documents

Subclass from Document and implement createViewWidget() method
to return a new widget for a view.
*/
class KDEVPLATFORMSUBLIME_EXPORT Document: public QObject {
    Q_OBJECT
public:
    /**Creates a document and adds it to a @p controller.*/
    Document(const QString &title, Controller *controller);
    ~Document() override;

    /**@return the new view for this document.
    @note it will not create a widget, just return a view object.*/
    View *createView();
    /**@return the list of all views in all areas for this document.*/
    const QList<View*> &views() const;

    enum TitleType { Normal, Extended};
    /**@return the document title.*/
    virtual QString title(TitleType type = Normal) const;
    /**Set the document title.*/
    void setTitle(const QString& newTitle);
    void setToolTip(const QString& newToolTip);
    QString toolTip() const;
    /**@return the type of document which can be written to config.*/
    virtual QString documentType() const = 0;

    /**@return the specifics of this document which can be written to config.*/
    virtual QString documentSpecifier() const = 0;

    /**
     * If the document is in a state where data may be lost while closing,
     * asks the user whether he really wants to close the document.
     * 
     * This function may also take actions like saving the document before closing
     * if the user desires so.
     * 
     * @return true if the document is allowed to be closed, otherwise false.
     *
     * The default implementation always returns true.
     *
     * */
    virtual bool askForCloseFeedback();
    
    /**Should try closing the document, eventually asking the user for feedback.
      *
      *If closing is successful, all views should be deleted, and the document itself
      *be scheduled for deletion using deleteLater().
      *
      * @param silent If this is true, the user must not be asked.
      * 
      * Returns whether closing was successful (The user did not push 'Cancel') */
    virtual bool closeDocument(bool silent = false);

    void setStatusIcon(const QIcon& icon);

    /**
     * @return The status icon of the document.
     */
    QIcon statusIcon() const;

    /**
     * @return The status icon of the document, or, if none is present, an icon
     *         that resembles the document, i.e. based on its mime type.
     * @see defaultIcon()
     */
    QIcon icon() const;

    /**
     * Optionally override this to return a default icon when no status
     * icon is set for the document. The default returns an invalid icon.
     */
    virtual QIcon defaultIcon() const;

Q_SIGNALS:
    /**Emitted when the document is about to be deleted but is still in valid state.*/
    void aboutToDelete(Sublime::Document *doc);
    /**Emitted when the document's title is changed.*/
    void titleChanged(Sublime::Document *doc);
   /**Emitted when the document status-icon has changed */
    void statusIconChanged(Sublime::Document *doc);

protected:
    /**Creates and returns the new view. Reimplement in subclasses to instantiate
    views of derived from Sublime::View classes.*/
    virtual View *newView(Document *doc);
    /**Reimplement this to create and return the new widget to display
    this document in the view. This method is used by View class when it
    is asked for its widget.*/
    virtual QWidget *createViewWidget(QWidget *parent = nullptr) = 0;
    /** Closes all views associated to this document */
    virtual void closeViews();

private:
    const QScopedPointer<class DocumentPrivate> d_ptr;
    Q_DECLARE_PRIVATE(Document)

    friend class DocumentPrivate;
    friend class View;
};

}

#endif

