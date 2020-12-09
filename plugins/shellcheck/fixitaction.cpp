/*
 * This file is part of KDevelop
 * Copyright 2020  Morten Danielsen Volden <mvolden2@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "fixitaction.h"

#include <language/duchain/duchainlock.h>
#include <language/codegen/documentchangeset.h>


namespace shellcheck {

FixitAction::FixitAction(const Fixit& fixit) :
m_fixit(fixit)
{
}

void FixitAction::execute()
{
    KDevelop::DocumentChangeSet changes;
    {
        KDevelop::DUChainReadLocker lock;

        KDevelop::DocumentChange change(m_fixit.m_range.document, m_fixit.m_range,
                    m_fixit.m_currentText, m_fixit.m_replacementText);
        change.m_ignoreOldText = !m_fixit.m_currentText.isEmpty();
        changes.addChange(change);
    }

    changes.setReplacementPolicy(KDevelop::DocumentChangeSet::WarnOnFailedChange);
    changes.applyAllChanges();

    emit executed(this);
}

QString FixitAction::description() const
{
    return m_fixit.m_description;
}

}
