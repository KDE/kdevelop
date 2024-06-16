/*
    SPDX-FileCopyrightText: 2006 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef QMAKEDRIVER_H
#define QMAKEDRIVER_H

#include <QString>

#include "parser_export.h"

namespace QMake
{
class ProjectAST;
/**
 * Class to parse a QMake project file or a string containing a QMake project structure
 */
class KDEVQMAKEPARSER_EXPORT Driver
{
    public:
        Driver();
        bool readFile(const QString& filename);
        void setContent( const QString& );
        void setDebug( bool );
        bool parse( ProjectAST** ast );
    private:
        QString m_content;
        bool m_debug = false;
};
}

#endif

