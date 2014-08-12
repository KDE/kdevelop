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

#ifndef CPP_NAVIGATIONWIDGET_H
#define CPP_NAVIGATIONWIDGET_H

#include <language/duchain/navigation/abstractnavigationwidget.h>
#include <language/util/includeitem.h>
#include "../../parser/rpp/pp-macro.h"
#include "cppduchainexport.h"

namespace Cpp {

class KDEVCPPDUCHAIN_EXPORT NavigationWidget : public KDevelop::AbstractNavigationWidget
{
  Q_OBJECT
public:

    /**
      * @param htmlPrefix and @param htmlSuffix can be used to add own text before/behind the
      * normal text in the navigation-widget.
      * The texts may contain links to files, using a simple special Syntax:
      * KDEV_FILE_LINK{File}. Every occurrence of KDEV_FILE_LINK will be replaced.
      * */
    NavigationWidget(KDevelop::DeclarationPointer declaration, KDevelop::TopDUContextPointer topContext, const QString& htmlPrefix = QString(), const QString& htmlSuffix = QString());

    /**
      * @param htmlPrefix and @param htmlSuffix can be used to add own text before/behind the
      * normal text in the navigation-widget.
      * The texts may contain links to files, see above.
      * */
    NavigationWidget(const KDevelop::IncludeItem& includeItem, KDevelop::TopDUContextPointer topContext, const QString& htmlPrefix = QString(), const QString& htmlSuffix = QString());

    /**
      * @param htmlPrefix and @param htmlSuffix can be used to add own text before/behind the
      * normal text in the navigation-widget.
      * The texts may contain links to files, see above.
      * */
    NavigationWidget(const rpp::pp_macro& macro, const QString& preprocessedBody = QString(), const QString& htmlPrefix = QString(), const QString& htmlSuffix = QString());

    /**
    * Creates a compact html description-text
    **/
    static QString shortDescription(KDevelop::Declaration* declaration);
    static QString shortDescription(const KDevelop::IncludeItem& includeItem);

protected:
    KDevelop::DeclarationPointer m_declaration;
};

}

#endif
