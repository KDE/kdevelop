/*
 * KDevelop xUnit testing support
 *
 * Copyright 2008 Manuel Breugelmans
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


#ifndef VERITAS_CPP_STUBCONTEXTACTION_INCLUDED
#define VERITAS_CPP_STUBCONTEXTACTION_INCLUDED

#include "veritascppexport.h"
#include <QObject>
#include <interfaces/contextmenuextension.h>

class KAction;
namespace KDevelop { class Context; }
namespace Cpp { class ClassDeclaration; }

namespace Veritas
{

/*! Provides a context menu action on class declarations */
class VERITASCPP_EXPORT StubContextAction : public QObject
{
Q_OBJECT
public:
    StubContextAction(QObject* parent=0);
    ~StubContextAction();
    KDevelop::ContextMenuExtension extension(KDevelop::Context* context);

private slots:
    void morph2stub();
    
private:
    Cpp::ClassDeclaration* m_clazz;
    KAction* m_stub;
};

}

#endif // VERITAS_CPP_STUBCONTEXTACTION_INCLUDED
