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


class QString;

/**
This class represents one CodeSnippet-Item in the listview.
It also holds the needed data for one snippet.
@author Robert Gruber
*/
class SnippetItem : public QListViewItem {
public:
    SnippetItem(QListView * parent, QString name, QString text);

    ~SnippetItem();
    QString getName();
    QString getText();
    void setText(QString text);
    void setName(QString name);

private:
  QString strName;
  QString strText;
};

#endif
