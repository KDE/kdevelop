/***************************************************************************
 *   Copyright (C) 2003 by Harald Fernengel                                *
 *   harry@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _SQLACTION_H_
#define _SQLACTION_H_

#include <kdeversion.h>
#include <kxmlguiclient.h>
#include <kaction.h>

class SQLSupportPart;
class KComboBox;

class SqlListAction : public KWidgetAction
{
    Q_OBJECT
public:
    SqlListAction( SQLSupportPart *part, const QString &text, const KShortcut& cut,
                     const QObject *receiver, const char *slot,
                     KActionCollection *parent, const char *name );
    void setCurrentConnectionName(const QString &name);
    QString currentConnectionName() const;
    void refresh();

private slots:
    void activated(int);

private:
    SQLSupportPart *m_part;
    KComboBox* m_combo;
};

#endif
