/*
 * KDevelop xUnit testing support
 * Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
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

#include "classwriter.h"
#include "classskeleton.h"
#include "constructorskeleton.h"
#include "methodskeleton.h"
#include <QIODevice>
#include <QTextStream>

using Veritas::ClassSerializer;
using Veritas::ClassSkeleton;
using Veritas::ConstructorSkeleton;
using Veritas::MethodSkeleton;

namespace
{

const QString INDENT = "    "; 
QString indent() { return INDENT; }

void intro(const ClassSkeleton& source, QTextStream& str)
{
    str << "\nclass " << source.name();
    if (!source.super().isEmpty()) {
        str << " : public " << source.super();
    }
    str << "\n" << "{\n";
}

void writeConstructor(const ConstructorSkeleton& ctr, QTextStream& str)
{
    if(ctr.isEmpty()) return;
    str << indent() << ctr.name() << "() ";
    QStringList inits = ctr.initializerList();
    if (inits.count()!=0) str << ": ";
    for(int i=0; i<inits.count(); i++) {
        str << inits[i];
        if (i!=inits.count()-1) str << " ,";
    }
    str << " {}\n";
}

void writeDestructor(const MethodSkeleton& destr, QTextStream& str)
{
    if (!destr.isEmpty()) {
      str << indent() << "virtual " << destr.name() << "() {}\n"; 
    }
}

void writeMethod(const MethodSkeleton& mtd, QTextStream& str)
{
    str << indent() << "virtual " << mtd.returnType() 
        << " " << mtd.name() << mtd.arguments() << " {\n";
    foreach(QString line, mtd.body().split("\n", QString::SkipEmptyParts)) {
        str << indent() << indent() << line << "\n";
    }
    str << indent() << "}\n";
}

} // end anonymous namespace

void ClassSerializer::write(const ClassSkeleton& source, QIODevice* target)
{
    target->open(QIODevice::Append);
    QTextStream str(target);
    intro(source, str);
    if (source.hasSomething()) {
        str << "public:\n";
        writeConstructor(source.constructor(), str);
        writeDestructor(source.destructor(), str);
        foreach(MethodSkeleton ms, source.methods()) {
            writeMethod(ms, str);
        }
        for(int i=0; i<source.memberCount(); i++) {
            str << INDENT << source.member(i) << ";\n";
        }
    }
    str << "};\n";
    target->close();
}

