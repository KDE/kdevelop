/*
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_CODECOMPLETIONHELPER_H
#define KDEVPLATFORM_CODECOMPLETIONHELPER_H

#include <language/languageexport.h>
#include "../duchain/duchainpointer.h"
#include "../duchain/declaration.h"

namespace KTextEditor {
class Document;
class Range;
class View;
}
namespace KDevelop {
class Declaration;

void KDEVPLATFORMLANGUAGE_EXPORT insertFunctionParenText(KTextEditor::View* view, const KTextEditor::Cursor& pos,
                                                         const KDevelop::DeclarationPointer& declaration,
                                                         bool jumpForbidden = false);
}
#endif
