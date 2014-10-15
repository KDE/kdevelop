/*
 * This file is part of KDevelop
 * Copyright 2014 Alex Richardson <arichardson.kde@gmail.com>
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
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KDEVPLATFORM_EDITORCONFIGPAGE_H
#define KDEVPLATFORM_EDITORCONFIGPAGE_H

#include <interfaces/configpage.h>

namespace KDevelop {

/**
 * This class makes all the KTextEditor config pages avaible for use
 * as a single KDevelop::ConfigPage
 */
class EditorConfigPage : public ConfigPage
{
    Q_OBJECT
public:
    EditorConfigPage(QWidget* parent);
    virtual ~EditorConfigPage();

    virtual QString name() const override;
    virtual QIcon icon() const override;
    virtual QString fullName() const override;
    virtual int childPages() const override;
    virtual ConfigPage* childPage(int number) override;
public Q_SLOTS:
    // nothing to edit on this page, only subpages have actual configuration
    virtual void apply() override {};
    virtual void reset() override {};
    virtual void defaults() override {};
};
}

#endif // KDEVPLATFORM_EDITORCONFIGPAGE_H
