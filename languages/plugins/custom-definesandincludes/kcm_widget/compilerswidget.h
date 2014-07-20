/*
 * This file is part of KDevelop
 *
 * Copyright 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef COMPILERWIDGET_H
#define COMPILERWIDGET_H

#include <QAbstractItemModel>
#include <QVector>
#include <QWidget>

#include "../compilerprovider/icompiler.h"

namespace Ui
{
class CompilersWidget;
}

class CompilersModel;
class QMenu;
class QSignalMapper;

class CompilersWidget : public QWidget
{
    Q_OBJECT

public:
    CompilersWidget( QWidget* parent = 0 );
    void setCompilers( const QVector<CompilerPointer>& compilers );
    QVector<CompilerPointer> compilers() const;
    void clear();

private slots:
    void deleteCompiler();
    void addCompiler(const QString& factoryName);

private:
    Ui::CompilersWidget* m_ui;
    CompilersModel* m_compilersModel;
    QMenu *m_addMenu;
    QSignalMapper *m_mapper;
};

#endif
