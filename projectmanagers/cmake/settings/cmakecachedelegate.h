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

#ifndef CMAKECACHEDELEGATE_H
#define CMAKECACHEDELEGATE_H

#include <QItemDelegate>
#include <QWidget>
class KUrlRequester;

class CMakeCacheDelegate : public QItemDelegate
{
    Q_OBJECT
    public:
        CMakeCacheDelegate(QObject* parent);
        virtual ~CMakeCacheDelegate();
        virtual QWidget * createEditor(QWidget * parent, const QStyleOptionViewItem & option,
                                         const QModelIndex & index ) const override;
        
        virtual void setEditorData ( QWidget * editor, const QModelIndex & index ) const override;
        virtual void setModelData ( QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const override;
        virtual void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const override;
        virtual QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index ) const override;

    private slots:
        void closingEditor(QWidget * editor, QAbstractItemDelegate::EndEditHint hint = NoHint);
        void checkboxToggled();

    private:
        KUrlRequester *m_sample;
};

#endif
