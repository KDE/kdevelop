/*
    SPDX-FileCopyrightText: 2008 Aleix Pol <aleixpol@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
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
        explicit CMakeCacheDelegate(QObject* parent);
        ~CMakeCacheDelegate() override;
        QWidget * createEditor(QWidget * parent, const QStyleOptionViewItem & option,
                                         const QModelIndex & index ) const override;
        
        void setEditorData ( QWidget * editor, const QModelIndex & index ) const override;
        void setModelData ( QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const override;
        void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const override;
        QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index ) const override;

    private Q_SLOTS:
        void closingEditor(QWidget * editor, QAbstractItemDelegate::EndEditHint hint = NoHint);
        void checkboxToggled();

    private:
        KUrlRequester *m_sample;
};

#endif
