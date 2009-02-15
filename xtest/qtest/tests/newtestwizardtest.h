/*
 * This file is part of KDevelop
 * Copyright 2009 Manuel Breugelmans <mbr.nxi@gmail.com>
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

#ifndef QTEST_NEWTESTWIZARDTEST_H_INCLUDED
#define QTEST_NEWTESTWIZARDTEST_H_INCLUDED

#include <QtCore/QObject>

namespace QTest
{
class NewTestWizardFake;

/*! @unitundertest QTest::NewTestWizard */
class NewTestWizardTest : public QObject
{
Q_OBJECT
private slots:
    void init();
    void cleanup();

    void sunnySetProject();
    void defaultSelectedProjectShouldBeQSTRING();
    void setBaseDirectoryShouldChangeHeaderSource();
    void legalClassIdentifiers();
    void validateOnlyNonExistingFiles();
    void validateLegalClassPage();
    void doNotValidateEmptyClassField();
    void sourceAndHeaderFilesShouldNotEqual();

    void showWidget();

private:
    NewTestWizardFake* m_wizard; // object under test
};

}

#endif // QTEST_NEWTESTWIZARDTEST_H_INCLUDED
