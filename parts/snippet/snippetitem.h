/*
 *  File : snippetitem.h
 *
 *  Author: Robert Gruber <rgruber@users.sourceforge.net>
 *
 *  Copyright: See COPYING file that comes with this distribution
 */

#ifndef SNIPPETITEM_H
#define SNIPPETITEM_H

#include <klistview.h>
#include <klocale.h>

class QString;

class SnippetGroup;

/**
This class represents one CodeSnippet-Item in the listview.
It also holds the needed data for one snippet.
@author Robert Gruber
*/
class SnippetItem : public QListViewItem {
friend class SnippetGroup;
public:
    SnippetItem(QListViewItem * parent, QString name, QString text);

    ~SnippetItem();
    QString getName();
    QString getText();
    int getParent() { return iParent; }
    void resetParent();
    void setText(QString text);
    void setName(QString name);
    static SnippetItem * findItemByName(QString name, QPtrList<SnippetItem> &list);
    static SnippetGroup * findGroupById(int id, QPtrList<SnippetItem> &list);
    
private:
  SnippetItem(QListView * parent, QString name, QString text);
  QString strName;
  QString strText;
  int iParent;
};

/**
This class represents one group in the listview.
It is derived from SnippetItem in order to allow storing 
it in the main QPtrList<SnippetItem>.
@author Robert Gruber
*/
class SnippetGroup : public SnippetItem {
public:
    SnippetGroup(QListView * parent, QString name, int id, QString lang=i18n("All"));
    ~SnippetGroup();

    int getId() { return iId; }
    static int getMaxId() { return iMaxId; }
    QString getLanguage() { return strLanguage; }
    
    void setId(int id);
    void setLanguage(QString lang) { strLanguage = lang; }

private:
    static int iMaxId;
    int iId;
    QString strLanguage;
};

#endif
