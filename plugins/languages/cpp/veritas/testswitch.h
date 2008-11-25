/*
 * KDevelop xUnit testing support
 *
 * Copyright 2008 Manuel Breugelmans  <mbr.nxi@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef VERITAS_TESTSWITCH_H
#define VERITAS_TESTSWITCH_H

#include "veritascppexport.h"
#include "../cppduchain/environmentmanager.h"
#include <QtCore/QObject>

class KActionCollection;
class KUrl;
namespace KDevelop { class TopDUContext; }

namespace Veritas
{

/*! Adds an action which allows for swapping between a test and its unit 
    under test. */
class VERITASCPP_EXPORT TestSwitch : public QObject
{
Q_OBJECT
public:
    TestSwitch(QObject* parent);
    virtual ~TestSwitch();

    /*! Creates and connects the QAction. Invoke this once. */
    void connectAction(KActionCollection*);
    void setStandardMacros(Cpp::LazyMacroSet*); 
    
private slots:
    /*! Connected to the action's triggered() signal */
    void swapTest_UnitUnderTest();

private:
    KDevelop::TopDUContext* documentContextFor(const KUrl& url);
    bool m_actionConnected;
    Cpp::LazyMacroSet* m_standardMacros;
    bool m_buzzySwitching;
};

}

#endif // VERITAS_TESTSWITCH_H
