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
#include <QEvent>
#include <KDebug>
#include <KUrlRequester>
#include <QtCore/QUrl>

CMakeCacheDelegate::CMakeCacheDelegate(QObject * parent)
    : QItemDelegate(parent)
{
    m_sample=new KUrlRequester();
}

CMakeCacheDelegate::~CMakeCacheDelegate()
{
    delete m_sample;
}

QWidget * CMakeCacheDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    QWidget *ret=0;
    if(index.column()==2)
    {
        QModelIndex typeIdx=index.sibling(index.row(), 1);
        QString type=typeIdx.model()->data(typeIdx, Qt::DisplayRole).toString();
        if(type=="BOOL")
        {
            QCheckBox* box=new QCheckBox(parent);
            connect(box, SIGNAL(toggled(bool)), this, SLOT(checkboxToggled()));
            ret = box;
        }
        else if(type=="PATH" || type=="FILEPATH")
        {
            KUrlRequester *r=new KUrlRequester(parent);
            if(type=="FILEPATH")
                r->setMode(KFile::File);
            else
                r->setMode(KFile::Directory | KFile::ExistingOnly);
            emit const_cast<CMakeCacheDelegate*>(this)->sizeHintChanged ( index );
            kDebug() << "EMITINT!" << index;
            ret=r;
        }
        else
        {
            ret=QItemDelegate::createEditor(parent, option, index);
        }
        
        if(!ret) kDebug(9032) << "Did not recognize type " << type;
    }
    return ret;
}

void CMakeCacheDelegate::setEditorData(QWidget * editor, const QModelIndex & index) const
{
    if(index.column()==2)
    {
        QModelIndex typeIdx=index.sibling(index.row(), 1);
        QString type=index.model()->data(typeIdx, Qt::DisplayRole).toString();
        QString value=index.model()->data(index, Qt::DisplayRole).toString();
        if(type=="BOOL")
        {
            QCheckBox *boolean=qobject_cast<QCheckBox*>(editor);
            boolean->setCheckState(value=="ON" ? Qt::Checked : Qt::Unchecked);
        }
        else if(type=="PATH" || type=="FILEPATH")
        {
            KUrlRequester *url=qobject_cast<KUrlRequester*>(editor);
            url->setUrl(QUrl(value));
        }
        else
        {
            QItemDelegate::setEditorData(editor, index);
        }
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
        if(type=="BOOL")
        {
            QCheckBox *boolean=qobject_cast<QCheckBox*>(editor);
            value = boolean->isChecked() ? "ON" : "OFF";
        }
        else if(type=="PATH" || type=="FILEPATH")
        {
            KUrlRequester *urlreq=qobject_cast<KUrlRequester*>(editor);
            value = urlreq->url().url(QUrl::StripTrailingSlash); //CMake usually don't put it
        }
        else
        {
            QItemDelegate::setModelData(editor, model, index);
            return;
        }
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

QSize CMakeCacheDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
//     kDebug(9042) << "calculant" << index << bool(option.state & QStyle::State_Editing);
    QSize ret=QItemDelegate::sizeHint(option, index);
    if(index.column()==2 && option.state & QStyle::State_Editing)
    {
        QModelIndex typeIdx=index.sibling(index.row(), 1);
        QString type=index.model()->data(typeIdx, Qt::DisplayRole).toString();
        if(type=="PATH")
        {
            ret.setHeight(m_sample->sizeHint().height());
        }
    }
    return ret;
}

void CMakeCacheDelegate::checkboxToggled()
{
    // whenever the check box gets toggled, we directly want to set the
    // model data which is done by closing the editor which in turn
    // calls setModelData. otherwise, the behavior is quite confusing, see e.g.
    // https://bugs.kde.org/show_bug.cgi?id=304352
    QCheckBox* editor = qobject_cast<QCheckBox*>(sender());
    Q_ASSERT(editor);
    closeEditor(editor);
}

void CMakeCacheDelegate::closingEditor(QWidget * editor, QAbstractItemDelegate::EndEditHint hint)
{
    Q_UNUSED(editor);
    Q_UNUSED(hint);
    kDebug() << "closing...";
}

// void CMakeCacheDelegate::updateEditorGeometry ( QWidget * editor, const QStyleOptionViewItem & option,
//                     const QModelIndex & index ) const
// {
//     if(index.column()==2)
//     {
//         QModelIndex typeIdx=index.sibling(index.row(), 1);
//         QString type=index.model()->data(typeIdx, Qt::DisplayRole).toString();
//         if(type=="PATH")
//         {
//             KUrlRequester* urlreq=qobject_cast<KUrlRequester*>(editor);
//             urlreq->setGeometry(QRect(option.rect.topLeft(), urlreq->sizeHint()));
//             return;
//         }
//     }
//     QItemDelegate::updateEditorGeometry( editor, option, index );
// }

