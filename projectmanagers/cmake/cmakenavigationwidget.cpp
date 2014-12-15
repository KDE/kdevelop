/* KDevelop CMake Support
 *
 * Copyright 2009 Aleix Pol <aleixpol@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "cmakenavigationwidget.h"
#include <language/duchain/navigation/abstractnavigationcontext.h>
#include <language/duchain/navigation/abstractdeclarationnavigationcontext.h>
#include <interfaces/idocumentation.h>

using namespace KDevelop;

class CMakeNavigationContext: public AbstractNavigationContext
{
    public:
        CMakeNavigationContext(TopDUContextPointer top, const QString& name, const QString& html)
            : AbstractNavigationContext(top, 0), mName(name), mDescription(html) {}
        virtual QString name() const { return mName; }
        virtual QString html(bool shorten = false)
        {
            Q_UNUSED(shorten);
            return mDescription;
        }
        
    private:
        QString mName;
        QString mDescription;
    
};

class CMakeDeclarationNavigationContext: public AbstractDeclarationNavigationContext
{
    public:
        CMakeDeclarationNavigationContext(DeclarationPointer decl, TopDUContextPointer top)
            : AbstractDeclarationNavigationContext(decl, top) {}
};

CMakeNavigationWidget::CMakeNavigationWidget(TopDUContextPointer top, const IDocumentation::Ptr doc)
{
    setContext( NavigationContextPointer(new CMakeNavigationContext(top, doc->name(), doc->description())) );
}

CMakeNavigationWidget::CMakeNavigationWidget(KDevelop::DUChainPointer< KDevelop::TopDUContext > top, KDevelop::Declaration* decl)
{
    setContext( NavigationContextPointer(new CMakeDeclarationNavigationContext(DeclarationPointer(decl), top)) );
}
