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

#include "compilerswidget.h"

#include <KAction>
#include <QMenu>
#include <QSignalMapper>

#include "ui_compilerswidget.h"
#include "compilersmodel.h"
#include "debugarea.h"

#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>

#include "../compilerprovider/icompilerprovider.h"

namespace
{
ICompilerProvider* compilerProvider()
{
    auto compilerProvider = KDevelop::ICore::self()->pluginController()->pluginForExtension("org.kdevelop.ICompilerProvider");
    if (!compilerProvider || !compilerProvider->extension<ICompilerProvider>()) {
        return {};
    }

    return compilerProvider->extension<ICompilerProvider>();
}
}

CompilersWidget::CompilersWidget(QWidget* parent)
    : QDialog(parent), m_ui(new Ui::CompilersWidget)
    , m_compilersModel(new CompilersModel(this))
{
    m_ui->setupUi(this);
    m_ui->compilers->setModel(m_compilersModel);
    m_ui->compilers->horizontalHeader()->setResizeMode(QHeaderView::Stretch);

    m_addMenu = new QMenu(m_ui->addButton);
    m_mapper = new QSignalMapper(m_addMenu);
    connect(m_mapper, SIGNAL(mapped(QString)), this, SLOT(addCompiler(QString)));

    m_addMenu->clear();

    if (auto cp = compilerProvider()) {
        foreach (const auto& factory, cp->compilerFactories()) {
            QAction* action = new QAction(m_addMenu);
            action->setText(factory->name());
            connect(action, SIGNAL(triggered()), m_mapper, SLOT(map()));
            m_mapper->setMapping(action, factory->name());
            m_addMenu->addAction(action);
        }
        m_ui->addButton->setMenu(m_addMenu);
    }

    connect(m_ui->removeButton, SIGNAL(clicked()), SLOT(deleteCompiler()));

    auto delAction = new KAction( i18n("Delete compiler"), this );
    delAction->setShortcut( KShortcut( "Del" ) );
    delAction->setShortcutContext( Qt::WidgetWithChildrenShortcut );
    m_ui->compilers->addAction( delAction );
    connect( delAction, SIGNAL(triggered()), SLOT(deleteCompiler()) );
}

void CompilersWidget::setCompilers(const QVector< CompilerPointer >& compilers)
{
    m_compilersModel->setCompilers(compilers);
}

void CompilersWidget::clear()
{
    m_compilersModel->setCompilers({});
}

void CompilersWidget::deleteCompiler()
{
    definesAndIncludesDebug() << "Deleting compiler";
    QModelIndexList selection = m_ui->compilers->selectionModel()->selectedIndexes();
    foreach (const QModelIndex& row, selection) {
        m_compilersModel->removeRows(row.row(), 1);
    }
}

void CompilersWidget::addCompiler(const QString& factoryName)
{
   foreach (const auto& factory, compilerProvider()->compilerFactories()) {
        if (factoryName == factory->name()) {
            //add compiler without any information, the user will fill the data in later
            m_compilersModel->addCompiler(factory->createCompiler(QString(), QString()));
            break;
        }
    }
}

QVector< CompilerPointer > CompilersWidget::compilers() const
{
    return m_compilersModel->compilers();
}
