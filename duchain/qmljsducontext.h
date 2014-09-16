/*
 * This file is part of qmljs, the QML/JS language support plugin for KDevelop
 * Copyright (c) 2013 Sven Brauch <svenbrauch@googlemail.com>
 * Copyright (c) 2014 Denis Steckelmacher <steckdenis@yahoo.fr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef __QMLJSDUCONTEXT_H__
#define __QMLJSDUCONTEXT_H__

#include <language/duchain/ducontext.h>

class QWidget;

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
    QmlJSDUContext(Data& data) : BaseContext(data) {
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

    virtual QWidget* createNavigationWidget(KDevelop::Declaration* decl,
                                            KDevelop::TopDUContext* topContext,
                                            const QString& htmlPrefix,
                                            const QString& htmlSuffix) const;

    enum {
        Identity = IdentityT
    };
};

typedef QmlJSDUContext<KDevelop::TopDUContext, 110> QmlJSTopDUContext;
typedef QmlJSDUContext<KDevelop::DUContext, 111> QmlJSNormalDUContext;

}


#endif
