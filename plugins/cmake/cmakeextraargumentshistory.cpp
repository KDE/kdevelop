/* KDevelop CMake Support
 *
 * Copyright 2016 René J.V. Bertin <rjvbertin@gmail.com>
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

#include <KSharedConfig>
#include <KConfigGroup>

#include "cmakeextraargumentshistory.h"

namespace {

const int maxExtraArgumentsInHistory = 15;

}

CMakeExtraArgumentsHistory::CMakeExtraArgumentsHistory(KComboBox* widget)
    : m_arguments(widget)
{
    if (m_arguments) {
        KConfigGroup config = KSharedConfig::openConfig()->group("CMakeBuildDirChooser");
        QStringList lastExtraArguments = config.readEntry("LastExtraArguments", QStringList());
        m_arguments->addItem(QString());
        m_arguments->addItems(lastExtraArguments);
        m_arguments->setInsertPolicy(QComboBox::InsertAtTop);
        KCompletion *comp = m_arguments->completionObject();
        KComboBox::connect(m_arguments, static_cast<void(KComboBox::*)(const QString&)>(&KComboBox::returnPressed),
                comp, static_cast<void(KCompletion::*)(const QString&)>(&KCompletion::addItem));
        comp->insertItems(lastExtraArguments);
    } else {
        qFatal("CMakeExtraArgumentsHistory initialised with invalid widget");
    }
}

CMakeExtraArgumentsHistory::~CMakeExtraArgumentsHistory()
{
    KConfigGroup config = KSharedConfig::openConfig()->group("CMakeBuildDirChooser");
    config.writeEntry("LastExtraArguments", list());
    config.sync();
}

QStringList CMakeExtraArgumentsHistory::list() const
{
    QStringList list;
    if (!m_arguments->currentText().isEmpty()) {
        list << m_arguments->currentText();
    }
    for (int i = 0; i < qMin(maxExtraArgumentsInHistory, m_arguments->count()); ++i) {
        if (!m_arguments->itemText(i).isEmpty() &&
            (m_arguments->currentText() != m_arguments->itemText(i))) {
            list << m_arguments->itemText(i);
        }
    }
    return list;
}
