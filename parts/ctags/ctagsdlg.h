/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _CTAGSDLG_H_
#define _CTAGSDLG_H_

#include <qdialog.h>
#include <qdict.h>

class QCheckBox;
class QLineEdit;
class QListBox;
class QListBoxItem;
class QListView;
class CTagsTagInfo;
class KDevPlugin;
typedef QValueList<CTagsTagInfo> CTagsTagInfoList;
typedef QValueList<CTagsTagInfo>::Iterator CTagsTagInfoListIterator;


class CTagsDialog : public QDialog
{
    Q_OBJECT
    
public:
    CTagsDialog( KDevPlugin *part );
    ~CTagsDialog();

private slots:
    void projectChanged();
    void slotSearch();
    void slotExecuted(QListBoxItem *item);
    
private:
    void insertResult(CTagsTagInfoList *result, const QStringList &kindStringList);
    void ensureTagsLoaded();
    bool createTagsFile();
    void loadTagsFile(const QString &fileName);
    
    QLineEdit *tag_edit;
    QCheckBox *regexp_box;
    QListView *kinds_listview;
    QListBox *results_listbox;
    
    KDevPlugin *m_part;
    QDict<CTagsTagInfoList> *m_tags;
};

#endif
