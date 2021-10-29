/*
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef ITESTNONGUIINTERFACE
#define ITESTNONGUIINTERFACE

#include <QObject>

class ITestNonGuiInterface
{
public:
    virtual ~ITestNonGuiInterface() {}
};

Q_DECLARE_INTERFACE( ITestNonGuiInterface, "org.kdevelop.ITestNonGuiInterface" )


#endif

