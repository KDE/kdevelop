/***************************************************************************
 *   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
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
#include <qvaluelist.h>
#include "ctagspart.h"

class QCheckBox;
class QLineEdit;
class QListBox;
class QListBoxItem;
class QListView;


class CTagsDialog : public QDialog
{
    Q_OBJECT
    
public:
    CTagsDialog( CTagsPart *part );
    ~CTagsDialog();

private slots:
    void slotSearch();
    void slotRegenerate();
    void slotResultClicked(QListBoxItem *item);
    
private:
    void updateInfo();
    void insertResult(const CTagsTagInfoList &result, const QStringList &kindStringList);
    
    CTagsMap m_tags;
    QStringList m_kindStrings;

    QLineEdit *tagEdit;
    QCheckBox *regexpBox;
    QListView *kindsListView;
    QListBox *resultsListBox;
    
    CTagsPart *m_part;
};

#endif
