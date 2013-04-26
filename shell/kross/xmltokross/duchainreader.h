/***************************************************************************
 *   Copyright 2008 Aleix Pol <aleixpol@gmail.com>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#ifndef KDEVPLATFORM_DUCHAINREADER_H
#define KDEVPLATFORM_DUCHAINREADER_H

#include "interfacecreator.h"
#include <QXmlStreamReader>
#include <QMap>
#include <language/duchain/types/abstracttype.h>

namespace KDevelop {
    class TopDUContext;
    class Declaration;
}

class DUChainReader : public InterfaceCreator
{
    public:
        DUChainReader(KDevelop::TopDUContext* top) : m_top(top) {}
        virtual ~DUChainReader() {}
        const KDevelop::TopDUContext* m_top;
        QStringList definedClasses;
        QString inNamespace;
        method currentMethod;
        QMap <QString, QString> classNamespace;
        QMap <QString, QStringList> sonsPerClass;
        QStringList flags;
        
        virtual int start();
        void foundClass(const KDevelop::Declaration* decl);
        QString printType(const TypePtr<KDevelop::AbstractType>& type);
};

#endif
