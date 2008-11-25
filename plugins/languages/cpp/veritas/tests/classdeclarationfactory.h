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


#ifndef VERITAS_CPP_CLASSDECLARATIONFACTORY
#define VERITAS_CPP_CLASSDECLARATIONFACTORY

#include <QList>
#include "../veritascppexport.h"
#include "../parser/control.h"

namespace Cpp { class ClassDeclaration; }
namespace KDevelop { class TopDUContext; class DUChainWriteLocker; class Declaration;}

namespace Veritas { namespace Test {

class VERITASCPP_EXPORT DeclarationFactory
{
public:
    DeclarationFactory();
    virtual ~DeclarationFactory();

    Cpp::ClassDeclaration* classFromText(const QByteArray& text);
    QList<KDevelop::Declaration*> variablesFromText(const QByteArray& text);
    QList<KDevelop::Declaration*> unresolvedVariablesFromText(const QByteArray& text);

    /*! Call this after you are done with a declaration. 
        Cleanup of created topducontexts. */
    void release();

    KDevelop::DUChainWriteLocker* m_lock;

private:
    KDevelop::TopDUContext* parseText(const QByteArray& text);
    static int m_count;
    Control m_control;
    QList<KDevelop::TopDUContext*> m_tops;
};

}}

#endif // VERITAS_CPP_CLASSDECLARATIONFACTORY
