/*
    SPDX-FileCopyrightText: 2013 Sven Brauch <svenbrauch@googlemail.com>
    SPDX-FileCopyrightText: 2014 Denis Steckelmacher <steckdenis@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef __QMLJSDUCONTEXT_H__
#define __QMLJSDUCONTEXT_H__

#include <language/duchain/duchainregister.h>
#include <language/duchain/ducontext.h>

namespace KDevelop
{
    class Declaration;
    class TopDUContext;
}

namespace QmlJS
{

template<class BaseContext, int IdentityT>
class QmlJSDUContext : public BaseContext
{
public:
    template<class Data>
    explicit QmlJSDUContext(Data& data) : BaseContext(data) {
    }

    ///Parameters will be reached to the base-class
    template<class Param1, class Param2>
    QmlJSDUContext(const Param1& p1, const Param2& p2, bool isInstantiationContext) : BaseContext(p1, p2, isInstantiationContext) {
        static_cast<KDevelop::DUChainBase*>(this)->d_func_dynamic()->setClassId(this);
    }

    ///Both parameters will be reached to the base-class. This fits TopDUContext.
    template<class Param1, class Param2, class Param3>
    QmlJSDUContext(const Param1& p1, const Param2& p2, const Param3& p3) : BaseContext(p1, p2, p3) {
        static_cast<KDevelop::DUChainBase*>(this)->d_func_dynamic()->setClassId(this);
    }
    template<class Param1, class Param2>
    QmlJSDUContext(const Param1& p1, const Param2& p2) : BaseContext(p1, p2) {
        static_cast<KDevelop::DUChainBase*>(this)->d_func_dynamic()->setClassId(this);
    }

    KDevelop::AbstractNavigationWidget*
    createNavigationWidget(KDevelop::Declaration* decl, KDevelop::TopDUContext* topContext,
                           KDevelop::AbstractNavigationWidget::DisplayHints hints) const override;

    enum {
        Identity = IdentityT
    };
};

using QmlJSTopDUContext = QmlJSDUContext<KDevelop::TopDUContext, 110>;
using QmlJSNormalDUContext = QmlJSDUContext<KDevelop::DUContext, 111>;

}

DUCHAIN_DECLARE_TYPE(QmlJS::QmlJSTopDUContext)
DUCHAIN_DECLARE_TYPE(QmlJS::QmlJSNormalDUContext)

#endif
