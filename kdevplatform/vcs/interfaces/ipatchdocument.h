/*
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_IPATCHDOCUMENT_H
#define KDEVPLATFORM_IPATCHDOCUMENT_H

#include <vcs/vcsexport.h>

class QUrl;
class QString;

namespace KDevelop
{

class KDEVPLATFORMVCS_EXPORT IPatchDocument
{
    public:
        virtual ~IPatchDocument();
        virtual void setDiff(const QString& tocompare, const QUrl& file) = 0;
};

}
#endif

