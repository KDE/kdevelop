/*
    SPDX-FileCopyrightText: 2016 René J.V. Bertin <rjvbertin@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "cmakeextraargumentshistory.h"

#include <KSharedConfig>
#include <KConfigGroup>

namespace {

const int maxExtraArgumentsInHistory = 15;

}

CMakeExtraArgumentsHistory::CMakeExtraArgumentsHistory(KComboBox* widget)
    : m_arguments(widget)
{
    if (m_arguments) {
        KConfigGroup config = KSharedConfig::openConfig()->group(QStringLiteral("CMakeBuildDirChooser"));
        QStringList lastExtraArguments = config.readEntry("LastExtraArguments", QStringList());
        m_arguments->addItem(QString());
        m_arguments->addItems(lastExtraArguments);
        m_arguments->setInsertPolicy(QComboBox::InsertAtTop);
        KCompletion *comp = m_arguments->completionObject();
        QObject::connect(m_arguments, QOverload<const QString&>::of(&KComboBox::returnPressed),
                        comp, QOverload<const QString&>::of(&KCompletion::addItem));
        comp->insertItems(lastExtraArguments);
    } else {
        qFatal("CMakeExtraArgumentsHistory initialised with invalid widget");
    }
}

CMakeExtraArgumentsHistory::~CMakeExtraArgumentsHistory()
{
    KConfigGroup config = KSharedConfig::openConfig()->group(QStringLiteral("CMakeBuildDirChooser"));
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
