/***************************************************************************
 *   Copyright (C) 1999-2002 by Bernd Gehrmann                            *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2002 by Sebastian Kratzert                              *
 *   skratzert@gmx.de                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _DOCTREEWIDGET_H_
#define _DOCTREEWIDGET_H_

#include <qmap.h>
#include <qstringlist.h>
#include <qlayout.h>
#include <qapplication.h>
#include <qvbox.h>
#include <qlineedit.h>
#include <qcheckbox.h>

#include <klistview.h>
#include <ktabctl.h>


class DocTreeItem;
class DocTreeDoxygenFolder;
class DocTreeBookmarksFolder;
class DocTreeProjectFolder;
class DocTreeDocbaseFolder;
class DocTreeQtFolder;
class DocTreeKDELibsFolder;
class DocTreeTocFolder;
class DocTreeDevHelpFolder;
class DocTreeViewPart;
class KDevProject;
class CustomizeDialog;
class QToolButton;
class KHistoryCombo;
class ChooseDlg;
class DocLineEdit;

class DocSearchDialog;

class IndexTreeData
{
public:
    IndexTreeData(const QString &text, const QString &parent, const QString &filename);

    void setVisible(bool v)     { m_visible = v;}

    bool isVisible()            { return m_visible; }
    QString text()              { return m_text; }
    QString fileName()          { return m_filename; }
    QString parent()            { return m_parent; }

private:
    QString m_text;
    QString m_parent;
    QString m_filename;
    bool m_visible;
};

class DocTreeViewWidget : public QVBox
{
    Q_OBJECT
    
public: 
    DocTreeViewWidget(DocTreeViewPart *part);
    ~DocTreeViewWidget();

    void configurationChanged();
    void projectChanged(KDevProject *project);

    static QString locatehtml(const QString &fileName);

private slots:
    void refresh();
    void slotConfigure();
    void slotConfigureProject();
    void slotItemExecuted(QListViewItem *item);
    void slotContextMenu(KListView *, QListViewItem *item, const QPoint &p);
    void slotJumpToNextMatch();
    void slotJumpToPrevMatch();
    void slotStartSearching();
    void slotHistoryReturnPressed ( const QString& );
    void slotSelectionChanged ( QListViewItem* );
    void slotAddBookmark();
    void slotRemoveBookmark();

    void slotFilterTextChanged(const QString &);
    void slotFilterReturn();
    void slotIndexItemExecuted(QListViewItem *item);
    void slotCurrentTabChanged(int);
    void slotSubstringCheckClicked();
    void slotIndexModeCheckClicked();
    void slotIndexNextMatch();
    void slotIndexPrevMatch();

protected:
    void searchForItem ( const QString& );
    bool initKDocKDELibs();
    void filterMultiReferences();

private:
    bool kdelibskdoc;
    KListView* docView;
    QPtrList<QListViewItem> searchResultList;

    QListViewItem *contextItem;
    QPtrList<DocTreeDoxygenFolder> folder_doxygen;
//    DocTreeKDELibsFolder *folder_kdelibs;
    QPtrList<DocTreeKDELibsFolder> folder_kdoc;
    DocTreeBookmarksFolder *folder_bookmarks;
    DocTreeDocbaseFolder *folder_docbase;
    DocTreeProjectFolder *folder_project;
    QPtrList<DocTreeTocFolder> folder_toc;
    QPtrList<DocTreeDevHelpFolder> folder_devhelp;
    DocTreeViewPart *m_part;
    QPtrList<DocTreeQtFolder> folder_qt;

    QHBox* searchToolbar;

    QToolButton* prevButton;
    QToolButton* nextButton;
    QToolButton* startButton;
    KHistoryCombo* completionCombo;

    DocTreeItem* m_activeTreeItem;

    KTabCtl *modeSwitch;
    QVBox *treeWidget;
    QVBox *indexWidget;
    DocLineEdit *filterEdit;
    KListView *indexView;
    QToolButton *subSearchButton;
    QToolButton *indexModeSwitch;
    int indexMode;
    bool subStringSearch;

    enum { filteredMode, plainListMode };

    QPtrList<IndexTreeData> indexItems;
};
#endif
