/***************************************************************************
 *   Copyright 2013 Aleix Pol Gonzalez <aleixpol@kde.org>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "areadisplay.h"
#include "mainwindow.h"
#include "workingsetcontroller.h"
#include <sublime/area.h>
#include <interfaces/iuicontroller.h>
#include <KLocalizedString>

#include <QMenu>
#include <QPushButton>
#include <QHBoxLayout>
#include <qlayout.h>

using namespace KDevelop;

AreaDisplay::AreaDisplay(KDevelop::MainWindow* parent)
    : QWidget(parent)
    , m_mainWindow(parent)
{
    connect(parent, SIGNAL(areaChanged(Sublime::Area*)), SLOT(newArea(Sublime::Area*)));
    setLayout(new QHBoxLayout);

    m_button = new QPushButton(this);
    m_button->setMaximumHeight(16);
    m_button->setFlat(true);
    layout()->addWidget(m_button);
}

void AreaDisplay::newArea(Sublime::Area* area)
{
    if(m_button->menu())
        m_button->menu()->deleteLater();

    Sublime::Area* currentArea = m_mainWindow->area();
    
    m_button->setText(currentArea->title());
    m_button->setIcon(KIcon(currentArea->iconName()));

    QMenu* m = new QMenu(m_button);
//     m->addActions(area->actions());
    if(currentArea->objectName() != "code") {
        m->addSeparator();
        m->addAction(KIcon("go-back"), i18n("Back to code"), this, SLOT(backToCode()));
    }
    m_button->setMenu(m);

    QBoxLayout* l = qobject_cast<QBoxLayout*>(layout());
    if(l->count()>=2) {
        QLayoutItem* item = l->takeAt(0);
        delete item->widget();
        delete item;
    }
    l->insertWidget(0, m_mainWindow->customButtonForAreaSwitcher(area));
}

void AreaDisplay::backToCode()
{
    ICore::self()->uiController()->switchToArea("code", IUiController::ThisWindow);
}
