/* KDevelop CMake Support
 *
 * Copyright 2008 Aleix Pol <aleixpol@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "cmakecachedelegate.h"

#include <QLineEdit>
#include <QCheckBox>
#include <KDebug>

CMakeCacheDelegate::CMakeCacheDelegate(QObject * parent)
    : QItemDelegate(parent)
{
}

QWidget * CMakeCacheDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    if(index.column()==2)
    {
        QModelIndex typeIdx=index.sibling(index.row(), 1);
        QString type=typeIdx.model()->data(typeIdx, Qt::DisplayRole).toString();
        if(type=="PATH" || type=="FILEPATH") {
            return new QLineEdit(parent);
        } else if(type=="STRING" || type=="STATIC") {
            return new QLineEdit(parent);
        } else if(type=="BOOL") {
            return new QCheckBox(parent);
        }
        
        kDebug(9032) << "Did not recognize type " << type;
    }
    return 0;
}

void CMakeCacheDelegate::setEditorData(QWidget * editor, const QModelIndex & index) const
{
    if(index.column()==2)
    {
        QModelIndex typeIdx=index.sibling(index.row(), 1);
        QString type=index.model()->data(typeIdx, Qt::DisplayRole).toString();
        QString value=index.model()->data(index, Qt::DisplayRole).toString();
        if(type=="FILEPATH" || type=="PATH" || type=="STRING" || type=="STATIC")
        {
            QLineEdit *line=qobject_cast<QLineEdit*>(editor);
            line->setText(value);
        }
        else if(type=="BOOL")
        {
            QCheckBox *boolean=qobject_cast<QCheckBox*>(editor);
            boolean->setCheckState(value=="ON" ? Qt::Checked : Qt::Unchecked);
        }
        else
            kDebug(9032) << "Did not recognize type " << type;
    }
    else
        kDebug(9032) << "Error. trying to edit a read-only field";
}

void CMakeCacheDelegate::setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const
{
    if(index.column()==2)
    {
        QModelIndex typeIdx=index.sibling(index.row(), 1);
        QString type=model->data(typeIdx, Qt::DisplayRole).toString();
        QString value;
        if(type=="FILEPATH" || type=="PATH" || type=="STRING" || type=="STATIC")
        {
            QLineEdit *line=qobject_cast<QLineEdit*>(editor);
            value=line->text();
        }
        else if(type=="BOOL")
        {
            QCheckBox *boolean=qobject_cast<QCheckBox*>(editor);
            value = boolean->isChecked() ? "ON" : "OFF";
        }
        else
            kDebug(9032) << "Did not recognize type " << type;
        model->setData(index, value, Qt::DisplayRole);
    }
    else
        kDebug(9032) << "Error. trying to edit a read-only field";
}

void CMakeCacheDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    if(index.column()==2)
    {
        QModelIndex typeIdx=index.sibling(index.row(), 1);
        QString type=index.model()->data(typeIdx, Qt::DisplayRole).toString();
        if(type=="BOOL")
            return;
    }
    QItemDelegate::paint(painter, option, index);
}

#include "cmakecachedelegate.moc"
