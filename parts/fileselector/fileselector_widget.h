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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef FILESELECTOR_WIDGET_H
#define FILESELECTOR_WIDGET_H

#include <qwidget.h>
#include <kfile.h>
#include <kurl.h>
#include <ktoolbar.h>
#include <qframe.h>
#include <kdiroperator.h>

class KDevMainWindow;
class KDevPartController;
class FileSelectorPart;
class KActionCollection;
class KActionSelector;
class KDirOperator;
class KURLComboBox;
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

/* I think this fix for not moving toolbars is better */
class KDevFileSelectorToolBar: public KToolBar
{
    Q_OBJECT
public:
    KDevFileSelectorToolBar(QWidget *parent);
    virtual ~KDevFileSelectorToolBar();

    virtual void setMovingEnabled( bool b );
};

class KDevFileSelectorToolBarParent: public QFrame
{
    Q_OBJECT
public:
    KDevFileSelectorToolBarParent(QWidget *parent);
    ~KDevFileSelectorToolBarParent();
    void setToolBar(KDevFileSelectorToolBar *tb);

private:
    KDevFileSelectorToolBar *m_tb;

protected:
    virtual void resizeEvent ( QResizeEvent * );
};

class KDevDirOperator: public KDirOperator
{
    Q_OBJECT
public:
    KDevDirOperator(FileSelectorPart *part, const KURL &urlName=KURL(), QWidget *parent=0, const char *name=0)
        :KDirOperator(urlName, parent, name), m_part(part)
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

    KDevFileSelector( FileSelectorPart *part, KDevMainWindow *mainWindow=0, KDevPartController *partController=0,
                      QWidget * parent = 0, const char * name = 0 );
    ~KDevFileSelector();

    void readConfig( KConfig *, const QString & );
    void writeConfig( KConfig *, const QString & );
    void setupToolbar( KConfig * );
    void setView( KFile::FileView );
    KDevDirOperator *dirOperator(){ return dir; }
    KActionCollection *actionCollection() { return mActionCollection; };

public slots:
    void slotFilterChange(const QString&);
    void setDir(KURL);
    void setDir( const QString& url ) { setDir( KURL( url ) ); };

private slots:
    void cmbPathActivated( const KURL& u );
    void cmbPathReturnPressed( const QString& u );
    void dirUrlEntered( const KURL& u );
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
    bool eventFilter( QObject *, QEvent * );
    KURL activeDocumentUrl();

private:
    class KDevFileSelectorToolBar *toolbar;
    KActionCollection *mActionCollection;
    class KBookmarkHandler *bookmarkHandler;
    KURLComboBox *cmbPath;
    KDevDirOperator * dir;
    class KAction *acSyncDir;
    KHistoryCombo * filter;
    class QToolButton *btnFilter;

    FileSelectorPart *m_part;
    KDevMainWindow *mainwin;
    KDevPartController *partController;

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

