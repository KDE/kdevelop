#ifndef KCOMBOVIEW_H
#define KCOMBOVIEW_H

#include <kcompletion.h>

#include "qcomboview.h"

class KComboView: public QComboView
{
    Q_OBJECT
public:
    KComboView( bool rw, QWidget* parent=0, const char* name=0 );

    virtual void addItem(QListViewItem *it);
    virtual void removeItem(QListViewItem *it);
    virtual void renameItem(QListViewItem *it, const QString &newName);

    virtual void clear();
private:
    KCompletion m_comp;
};

#endif
