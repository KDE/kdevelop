/***************************************************************************
 *   Copyright (C) 2008 by Peter Penz <peter.penz@gmx.at>                  *
 *             modified by Manuel Breugelmans <mbr.nxi@gmail.com>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#include "verbosemanager.h"

#include "veritas/test.h"
#include "veritas/utils.h"

#include "runnermodel.h"
#include "verbosetoggle.h"

#include <KIconEffect>
#include <KDebug>

#include <QAbstractButton>
#include <QAbstractItemView>
#include <QAbstractProxyModel>
#include <QModelIndex>
#include <QPainter>
#include <QPaintEvent>
#include <QRect>
#include <QTimeLine>

using Veritas::VerboseManager;
using Veritas::VerboseToggle;
using Veritas::Test;
using Veritas::RunnerModel;
using Veritas::OverlayButton;

VerboseManager::VerboseManager(QAbstractItemView* parent) :
    OverlayManager(parent)
{}

VerboseManager::~VerboseManager()
{}

void VerboseManager::setButton(OverlayButton* toggle)
{
    connect(toggle,
            SIGNAL(clicked(bool)),
            SLOT(emitOpenVerbose()));
    OverlayManager::setButton(toggle);
}

void VerboseManager::emitOpenVerbose()
{
    const QModelIndex index = button()->index();
    if (index.isValid()) {
        Test* t = index2Test(index);
        if (t) {
            emit openVerbose(t);
        }
    }
}

#include "verbosemanager.moc"
