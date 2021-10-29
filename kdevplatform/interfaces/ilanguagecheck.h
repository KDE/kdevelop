/*
    SPDX-FileCopyrightText: 2011 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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

