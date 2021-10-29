/*
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "cmakenavigationwidget.h"
#include <language/duchain/navigation/abstractnavigationcontext.h>
#include <language/duchain/navigation/abstractdeclarationnavigationcontext.h>
#include <interfaces/idocumentation.h>

using namespace KDevelop;

class CMakeNavigationContext: public AbstractNavigationContext
{
        Q_OBJECT
    public:
        CMakeNavigationContext(const TopDUContextPointer& top, const QString& name, const QString& html)
            : AbstractNavigationContext(top, nullptr), mName(name), mDescription(html) {}
        QString name() const override { return mName; }
        QString html(bool shorten = false) override
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
        Q_OBJECT
    public:
        CMakeDeclarationNavigationContext(const DeclarationPointer& decl, const TopDUContextPointer& top)
            : AbstractDeclarationNavigationContext(decl, top) {}
};

CMakeNavigationWidget::CMakeNavigationWidget(const TopDUContextPointer& top, const IDocumentation::Ptr& doc)
{
    setContext(NavigationContextPointer(new CMakeNavigationContext(top, doc->name(), doc->description())));
}

CMakeNavigationWidget::CMakeNavigationWidget(const KDevelop::DUChainPointer< KDevelop::TopDUContext >& top, KDevelop::Declaration* decl)
{
    setContext(NavigationContextPointer(new CMakeDeclarationNavigationContext(DeclarationPointer(decl), top)));
}

#include "cmakenavigationwidget.moc"
