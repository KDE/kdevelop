/*
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "use.h"
#include "topducontext.h"

using namespace KTextEditor;

namespace KDevelop {
KDevelop::Declaration* Use::usedDeclaration(KDevelop::TopDUContext* topContext) const
{
    return topContext->usedDeclarationForIndex(m_declarationIndex);
}
}
