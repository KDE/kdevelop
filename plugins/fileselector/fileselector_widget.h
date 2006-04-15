/* This file is part of the KDE project
   Copyright (C) 2003 Roberto Raggi <roberto@kdevelop.org>
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2001 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2001 Anders Lund <anders.lund@lund.tdcadsl.dk>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef FILESELECTOR_WIDGET_H
#define FILESELECTOR_WIDGET_H

#include <qwidget.h>
//Added by qt3to4:
#include <QShowEvent>
#include <QFocusEvent>
#include <QResizeEvent>
#include <QEvent>
#include <kfile.h>
#include <kurl.h>
#include <ktoolbar.h>
#include <q3frame.h>
#include <kdiroperator.h>

class KDevMainWindow;
class KDevDocumentController;
class FileSelectorPart;
class KActionCollection;
class KActionSelector;
class KDirOperator;
class KUrlComboBox;
class KHistoryCombo;

namespace KParts
{
    class Part;
}

namespace KTextEditor
{
    class Document;
}

/*
    The KDev file selector presents a directory view, in which the default action is
    to open the activated file.
    Additinally, a toolbar for managing the kdiroperator widget + sync that to
    the directory of the current file is available, as well as a filter widget
    allowing to filter the displayed files using a name filter.
*/

class KDevDirOperator: public KDirOperator
{
    Q_OBJECT
public:
    KDevDirOperator(FileSelectorPart *part, const KUrl &urlName=KUrl(), QWidget *parent=0)
        :KDirOperator(urlName, parent), m_part(part)
    {
    }

protected slots:
    virtual void activatedMenu (const KFileItem *fi, const QPoint &pos);

private:
    FileSelectorPart *m_part;
};

class KDevFileSelector : public QWidget
{
    Q_OBJECT

    friend class KFSConfigPage;

public:
    /* When to sync to current document directory */
    enum AutoSyncEvent { DocumentChanged=1, DocumentOpened=2, GotVisible=4 };

    KDevFileSelector( FileSelectorPart *part, KDevMainWindow *mainWindow=0, KDevDocumentController *partController=0,
                      QWidget * parent = 0 );
    ~KDevFileSelector();

    void readConfig( KConfig *, const QString & );
    void writeConfig( KConfig *, const QString & );
    void setupToolbar( KConfig * );
    void setView( KFile::FileView );
    KDevDirOperator *dirOperator(){ return dir; }
    KActionCollection *actionCollection() { return mActionCollection; };

public slots:
    void slotFilterChange(const QString&);
    void setDir(KUrl);
    void setDir( const QString& url ) { setDir( KUrl::fromPathOrURL( url ) ); };

private slots:
    void cmbPathActivated( const KUrl& u );
    void cmbPathReturnPressed( const QString& u );
    void dirUrlEntered( const KUrl& u );
    void dirFinishedLoading();
    void setActiveDocumentDir();
    void viewChanged();
    void btnFilterClick();
    void autoSync();
    void autoSync( KParts::Part * );
	void initialDirChangeHack();
protected:
    void focusInEvent( QFocusEvent * );
    void showEvent( QShowEvent * );
    KUrl activeDocumentUrl();

private:
    class KToolBar *toolbar;
    KActionCollection *mActionCollection;
    class KBookmarkHandler *bookmarkHandler;
    KUrlComboBox *cmbPath;
    KDevDirOperator * dir;
    class KAction *acSyncDir;
    KHistoryCombo * filter;
    class QToolButton *btnFilter;

    FileSelectorPart *m_part;
    KDevMainWindow *mainwin;
    KDevDocumentController *partController;

    QString lastFilter;
    int autoSyncEvents; // enabled autosync events
    QString waitingUrl; // maybe display when we gets visible
    QString waitingDir;
};

/*  @todo anders
    KFSFilterHelper
    A popup widget presenting a listbox with checkable items
    representing the mime types available in the current directory, and
    providing a name filter based on those.
*/

/*
    Config page for file selector.
    Allows for configuring the toolbar, the history length
    of the path and file filter combos, and how to handle
    user closed session.
*/
class KFSConfigPage : public QWidget
{
    Q_OBJECT
public:
    KFSConfigPage( QWidget* parent=0, const char *name=0, KDevFileSelector *kfs=0);
    virtual ~KFSConfigPage() {};

public slots:
    virtual void apply();
    virtual void reload();
    virtual void slotChanged();

private:
    void init();

    KDevFileSelector *fileSelector;
    bool bDirty;
    //class QListBox *lbAvailableActions, *lbUsedActions;
    KActionSelector *acSel;
    class QSpinBox *sbPathHistLength, *sbFilterHistLength;
    class QCheckBox *cbSyncOpen, *cbSyncActive, *cbSyncShow;
    class QCheckBox *cbSesLocation, *cbSesFilter;
};


#endif

