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

#ifndef SHELLCHECK_FIXITACTION_H
#define SHELLCHECK_FIXITACTION_H

#include "fixit.h"

#include <iassistant.h>

namespace shellcheck {

/**
 * A single Fixit action
 */
class FixitAction :  public KDevelop::IAssistantAction
{
    Q_OBJECT
public:
    FixitAction(const Fixit& fixit);

    /**
     * @todo write docs
     *
     * @return Description of the action
     */
    QString description() const override;


public Q_SLOTS:
    void execute() override;
private:
    Fixit m_fixit;
};

}

#endif // SHELLCHECK_FIXITACTION_H
