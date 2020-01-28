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

#ifndef COMPILERSWIDGET_H
#define COMPILERSWIDGET_H

#include <QVector>
#include <QWidget>

#include <interfaces/configpage.h>

#include "../compilerprovider/icompiler.h"

namespace Ui
{
class CompilersWidget;
}

class CompilersModel;
class QMenu;

class CompilersWidget : public KDevelop::ConfigPage
{
    Q_OBJECT

public:
    explicit CompilersWidget(QWidget* parent = nullptr);
    ~CompilersWidget() override;

    void setCompilers(const QVector<CompilerPointer>& compilers);
    QVector<CompilerPointer> compilers() const;
    void clear();

    QString name() const override;
    QString fullName() const override;
    QIcon icon() const override;

    KDevelop::ConfigPage::ConfigPageType configPageType() const override;

    void apply() override;
    void reset() override;
    void defaults() override;

private Q_SLOTS:
    void deleteCompiler();
    void addCompiler(const QString& factoryName);
    void compilerSelected(const QModelIndex& index);
    void compilerEdited();

Q_SIGNALS:
    void compilerChanged();

private:
    void enableItems(bool enable);

    QScopedPointer<Ui::CompilersWidget> m_ui;
    CompilersModel* m_compilersModel;
    QMenu *m_addMenu;
};

#endif
