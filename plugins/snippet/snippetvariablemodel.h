/***************************************************************************
 *   Copyright 2007 Rober Gruber <rgruber@users.sourceforge.net>                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __SNIPPETVARIABLEMODEL_H__
#define __SNIPPETVARIABLEMODEL_H__

#include <QAbstractItemModel>

/**
 * An object of this type is one variable.
 * It contains the variable's name and the replacement value.
 * Each object is one row in the SnippetVariableModel.
 * @author Robert Gruber <rgruber@users.sourceforge.net>
 */
class SnippetVariableItem
{
public:
    SnippetVariableItem(QString name) { name_ = name; }

    void setValue(QString value) { value_ = value; }
    QString value() { return value_; }

    void setName(QString name) { name_ = name; }
    QString name() { return name_; }

    QString data(int column);

private:
    QString name_;
    QString value_;
};

/**
 * Each row of this model hold one variable. The first column holds the
 * variable's name. The second column is editable and stores the value the
 * user can insert as replacement for the variable.
 * After the user entered the values, calling getVariableList() will return
 * all variables and their values.
 * @author Robert Gruber <rgruber@users.sourceforge.net>
 */
class SnippetVariableModel : public QAbstractItemModel
{
public:
    SnippetVariableModel(QObject * parent = 0);
    ~SnippetVariableModel();

    void appendVariable(const QString& name);
    QList<SnippetVariableItem*>& getVariableList() { return variables_; }

    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &index) const;

    virtual int rowCount(const QModelIndex &parent) const;
    virtual int columnCount(const QModelIndex &parent) const;

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role);
    virtual Qt::ItemFlags flags (const QModelIndex & index) const;

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;

private:
    QList<SnippetVariableItem*> variables_;
};

#endif

