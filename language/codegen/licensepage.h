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

#ifndef KDEV_LICENSEPAGE_H
#define KDEV_LICENSEPAGE_H

#include <QWidget>

#include "../languageexport.h"

class KTextEdit;

namespace KDevelop {

struct ClassDescription;
class OverridesPage;

//!@todo Add the name of the Author at the top of the license
class KDEVPLATFORMLANGUAGE_EXPORT LicensePage : public QWidget
{
    Q_OBJECT

public:
    LicensePage(QWidget* parent);
    virtual ~LicensePage();

    KTextEdit* licenseTextEdit();
    bool validatePage();

public Q_SLOTS:
    virtual void licenseComboChanged(int license);

private:
    // data
    class LicensePagePrivate* const d;

    ///FIXME: ugly internal api, move to *Private
    // methods
    void        initializeLicenses();
    QString&    readLicense(int licenseIndex);
    bool        saveLicense();
};

}

#endif // KDEV_LICENSEPAGE_H
