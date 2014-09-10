/*  This file is part of KDevelop
    Copyright 2011 Aleix Pol Gonzalez <aleixpol@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KDEVPLATFORM_ILANGUAGECHECK_H
#define KDEVPLATFORM_ILANGUAGECHECK_H

#include "interfacesexport.h"

#include <QUrl>
#include <QObject>

namespace KDevelop
{
class DataAccessRepository;
class ControlFlowGraph;
class TopDUContext;
class ControlFlowGraph;

class CheckData
{
    public:
        QUrl url;
        TopDUContext* top;
        ControlFlowGraph* flow;
        DataAccessRepository* access;
};

class KDEVPLATFORMINTERFACES_EXPORT ILanguageCheck
{
    public:
        virtual ~ILanguageCheck();

        /** Runs the check. This will add problems to the TopDUContext if necessary */
        virtual void runCheck(const CheckData& data)=0;

        /** @returns the name of the check */
        virtual QString name() const=0;
};

}

Q_DECLARE_INTERFACE( KDevelop::ILanguageCheck, "org.kdevelop.ILanguageCheck")

#endif

