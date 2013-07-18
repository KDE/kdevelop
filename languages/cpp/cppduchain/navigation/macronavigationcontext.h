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

#ifndef MACRONAVIGATIONCONTEXT_H
#define MACRONAVIGATIONCONTEXT_H

#include <language/duchain/navigation/abstractnavigationcontext.h>
#include "../../parser/rpp/pp-macro.h"

#include "../cppduchainexport.h"

namespace KTextEditor {
class View;
}

namespace Cpp {

class KDEVCPPDUCHAIN_EXPORT MacroNavigationContext : public KDevelop::AbstractNavigationContext
{
public:
  MacroNavigationContext(const rpp::pp_macro& macro, QString preprocessedBody);
  ~MacroNavigationContext();

  QString body() const;
  virtual QWidget* widget() const;
  virtual QString html(bool shorten);
  virtual QString name() const;

private:
  rpp::pp_macro* m_macro;
  QString m_body;
  KTextEditor::Document* m_preprocessed;
  KTextEditor::Document* m_definition;
  QWidget* m_widget;
};

}

#endif
