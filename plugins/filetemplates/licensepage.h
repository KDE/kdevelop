/* This file is part of KDevelop
    Copyright 2008 Hamish Rodda <rodda@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KDEVPLATFORM_PLUGIN_LICENSEPAGE_H
#define KDEVPLATFORM_PLUGIN_LICENSEPAGE_H

#include <QWidget>



namespace KDevelop {

/**
 * Assistant dialog page for choosing the license of new source files
 *
 * @todo Add the name of the Author at the top of the license
 */
class LicensePage : public QWidget
{
    Q_OBJECT

public:
    LicensePage(QWidget* parent);
    virtual ~LicensePage();

    /**
     * @return the full license header, which might either be a pre-defined
     *         or user-supplied one.
     */
    QString license() const;

private:
    // data
    class LicensePagePrivate* const d;
    Q_PRIVATE_SLOT(d, void licenseComboChanged(int license))
};

}

#endif // KDEVPLATFORM_PLUGIN_LICENSEPAGE_H
