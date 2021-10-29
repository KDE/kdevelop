/*
    SPDX-FileCopyrightText: 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
