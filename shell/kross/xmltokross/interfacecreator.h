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

#ifndef KDEVPLATFORM_INTERFACECREATOR_H
#define KDEVPLATFORM_INTERFACECREATOR_H

#include <QStringList>
#include <QString>
#include <QList>

struct method
{
    struct argument { QString name, type, def; };
    QString funcname;
    QString returnType;
    QString access;
    bool isConst;
    bool isVirtual;
    bool isAbstract;
    bool isConstructor;
    QList<argument> args;
};

class InterfaceCreator
{
    public:
        QStringList includes;
        QString filename;
        int inclass;
        
        void setIncludes(const QStringList& _includes) { includes=_includes; }
        void setFileName(const QString& fn) { filename=fn; }
        
        virtual ~InterfaceCreator() {}
        virtual void writeDocument()=0;
        virtual void writeClass(const QString& classname, const QString& baseclass, const QList<QStringList>& enums)=0;
        virtual void writeNamespace(const QString& name)=0;
        virtual void writeVariable(const QString& name, const QString& type, bool isConst)=0;
        virtual void writeEndClass()=0;
        virtual void writeEndDocument()=0;
        virtual void writeEndFunction(const method& m)=0;
        virtual void writeEndEnum(const QStringList& flags)=0;
        virtual int start()=0;
};

#endif
