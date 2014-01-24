/*
   Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>
   
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

#ifndef KDEVPLATFORM_PROBLEMNAVIGATIONCONTEXT_H
#define KDEVPLATFORM_PROBLEMNAVIGATIONCONTEXT_H

#include <language/duchain/navigation/abstractnavigationcontext.h>
#include <language/duchain/problem.h>
#include "../../languageexport.h"
#include <qpointer.h>

namespace KDevelop {

class KDEVPLATFORMLANGUAGE_EXPORT ProblemNavigationContext : public AbstractNavigationContext
{
  public:
    ProblemNavigationContext(KDevelop::ProblemPointer problem);
    ~ProblemNavigationContext();
    virtual QString name() const;  
    virtual QString html(bool shorten = false);    
    virtual QWidget* widget() const;
    virtual bool isWidgetMaximized() const;
  private:
    QPointer<QWidget> m_widget;
    ProblemPointer m_problem;
};

}

#endif // KDEVPLATFORM_PROBLEMNAVIGATIONCONTEXT_H
