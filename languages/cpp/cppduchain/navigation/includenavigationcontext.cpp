/*
   Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "includenavigationcontext.h"

#include <QTextDocument>

#include <klocale.h>

#include <language/duchain/duchain.h>
#include <language/duchain/parsingenvironment.h>
#include <language/duchain/declaration.h>

#include "../environmentmanager.h"

namespace Cpp {
using namespace KDevelop;

IncludeNavigationContext::IncludeNavigationContext(const IncludeItem& item, KDevelop::TopDUContextPointer topContext)
    : AbstractIncludeNavigationContext(item, topContext, CppParsingEnvironment)
{}

bool IncludeNavigationContext::filterDeclaration(Declaration* decl)
{
    QString declId = decl->identifier().identifier().str();
      //filter out forward-declarations and macro-expansions without a range
      //And filter out declarations with reserved identifiers
      return !decl->qualifiedIdentifier().toString().isEmpty() && !decl->range().isEmpty() && !decl->isForwardDeclaration()
                && !(declId.startsWith("__") || (declId.startsWith("_") && declId.length() > 1 && declId[1].isUpper()) );
}

void IncludeNavigationContext::getFileInfo(TopDUContext* duchain)
{
    const Cpp::EnvironmentFile* f = dynamic_cast<const Cpp::EnvironmentFile*>(duchain->parsingEnvironmentFile().data());
    Q_ASSERT(f); //Should always be for c++
    modifyHtml() += QString("%1: %2 %3: %4 %5: %6")
                      .arg(labelHighlight(i18nc("Headers included into this header", "Includes"))).arg(duchain->importedParentContexts().count())
                      .arg(labelHighlight(i18nc("Count of files this header was included into", "Included by"))).arg(duchain->importers().count())
                      .arg(labelHighlight(i18nc("Count of macros defined in this header", "Defined macros"))).arg(f->definedMacros().set().count());
    modifyHtml() += "<br />";
}

}
