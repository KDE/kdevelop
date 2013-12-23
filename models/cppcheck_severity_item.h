/* This file is part of KDevelop
 * Copyright 2011 Mathieu Lornac <mathieu.lornac@gmail.com>
 * Copyright 2011 Damien Coppel <damien.coppel@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef _CPPCHECK_SEVERITYITEM_H_
#define _CPPCHECK_SEVERITYITEM_H_

#include <QString>
#include <QMap>
#include <QStringList>
#include <KUrl>

#include "imodel.h"

namespace cppcheck
{

class CppcheckSeverityItem : public ModelItem
{

public:

    CppcheckSeverityItem();
    CppcheckSeverityItem(bool);
    virtual ~CppcheckSeverityItem();

    enum Columns {
        ColumnSeverity = 0,
        ColumnMessage = 1,
        ColumnProjectPath = 2,
        ColumnMessageVerbose = 3,
        ColumnErrorFile = 4,
        ColumnErrorLine = 5,
        ColumnMax = 6
    };

    /*
     * Called when data related to the error has been parsed
     */
    void incomingData(const QString &name, const QString &value, int ErrorLine, QString ErrorFile, QString Message, QString MessageVerbose, QString ProjectPath, QString Severity);
    void incomingAlloc(const QString &value);

    // use by the model
    void appendChild(CppcheckSeverityItem *child);
    void setParent(CppcheckSeverityItem *parent);
    CppcheckSeverityItem *child(int row);
    void setIsChild(bool isChild);
    bool isChild();
    int childCount() const;
    int columnCount() const;
    QVariant data(int column, int role = Qt::DisplayRole) const;
    int row() const;
    CppcheckSeverityItem *parent() const;

    KUrl url() const;
    int getLine() const;

private:
    QMap<QString, QString> m_values;
    bool m_child;

    int m_line;
    QString m_dir, m_file;



    // use by the model
    QList<CppcheckSeverityItem*> m_childItems;
    CppcheckSeverityItem *m_parentItem;

public:
    int ErrorLine;
    QString ErrorFile;
    QString Message;
    QString MessageVerbose;
    QString ProjectPath;
    QString Severity;
};

}

#endif /* _CPPCHECK_SEVERITYITEM_H_ */
