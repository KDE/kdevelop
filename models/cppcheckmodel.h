/* This file is part of KDevelop
 * Copyright 2006-2008 Hamish Rodda <rodda@kde.org>
 * Copyright 2011 Mathieu Lornac <mathieu.lornac@gmail.com>
 * Copyright 2011 Damien Coppel <damien.coppel@gmail.com>
 * Copyright 2011 Lionel Duc <lionel.data@gmail.com>
 * Copyright 2013 Christoph Thielecke <crissi99@gmx.de>

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

#ifndef _CPPCHECKMODEL_H_
#define _CPPCHECKMODEL_H_


#include <QHash>
#include <QStack>
#include <QAbstractItemModel>

#include "imodel.h"
#include "cppcheckitems.h"

namespace cppcheck
{

class CppcheckError;


/**
 * A class that represents the item model
 * \author Hamish Rodda \<roddakde.org\>
 */
class CppcheckModel : public QAbstractItemModel,
                      public cppcheck::Model,
                      public CppcheckItem

{
    Q_OBJECT

public:

    CppcheckModel(QObject* parent = 0);
    virtual ~CppcheckModel();

    virtual QAbstractItemModel  *getQAbstractItemModel(int) {return this;}

    enum Columns {
        //Index = 0,
//         Severity,
        ErrorFile,
        ErrorLine,
        Message,
        ProjectPath,
        MessageVerbose
    };

    static const int numColumns = 5;

    // Item
    virtual CppcheckItem* parent() const {
        return 0L;
    }

    // Model
    QModelIndex indexForItem(CppcheckItem* item, int column = 0) const;
    CppcheckItem* itemForIndex(const QModelIndex& index) const;

    virtual void incomingData(QString name, QString value, int ErrorLine, QString ErrorFile, QString Message, QString MessageVerbose, QString ProjectPath, QString Severity);
    virtual int columnCount(const QModelIndex & parent = QModelIndex()) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    virtual QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex & index) const;
    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;

    void newStack();
    void newFrame();
    void newStartError();

    /////SLOTS WRAPPER////
    /**
     * Reception of a new item in the model
     */
    virtual void newElement(cppcheck::Model::eElementType);

    /**
     * Reception of data to register to the current item
     */
    virtual void newData(cppcheck::Model::eElementType, QString name, QString value, int ErrorLine, QString ErrorFile, QString Message, QString MessageVerbose, QString ProjectPath, QString Severity);

    /**
     * Resets the model content
     */
    void reset();

    /**
     * Reception of a new item
     */
    virtual void newItem(cppcheck::ModelItem *);
    ////END SLOTS WRAPER////
private:
    QList<CppcheckError *> m_errors;


};

}
#endif /* _CPPCHECKMODEL_H_ */
