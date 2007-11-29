/***************************************************************************
 *   Copyright (C) 2007 by Andreas Pakulat                                 *
 *   apaku@gmx.de                                                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SELECTNEWFILESDIALOG_H
#define SELECTNEWFILESDIALOG_H

#include "kdialogbase.h"

class QCheckListItem;
class QStringList;
class SelectNewFilesDialogBase;

class SelectNewFilesDialog : public KDialogBase
{
Q_OBJECT

public:
    SelectNewFilesDialog( QStringList paths, QWidget* parent = 0, const char* name = 0 );
    ~SelectNewFilesDialog();

    QStringList excludedPaths() const;
    QStringList includedPaths() const;

public slots:
    /*$PUBLIC_SLOTS$*/

protected:
    /*$PROTECTED_FUNCTIONS$*/

protected slots:
    /*$PROTECTED_SLOTS$*/
    virtual void slotCancel();
    virtual void slotOk();
private:
    void addPath( QCheckListItem* , const QString& );
    void checkItem( QCheckListItem* item, const QString& curpath );
    QCheckListItem* createItem( QCheckListItem*, const QString&, int );
    SelectNewFilesDialogBase* m_widget;
    QStringList excludePaths;
    QStringList includePaths;
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
