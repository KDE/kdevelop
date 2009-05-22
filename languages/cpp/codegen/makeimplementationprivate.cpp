/*
   Copyright 2009 Ramón Zarazúa <killerfox512+kde@gmail.com>

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

#include "makeimplementationprivate.h"

#include <language/duchain/ducontext.h>
#include <language/codegen/utilities.h>
#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>

#include <kinputdialog.h>
#include <kparts/mainwindow.h>

namespace KDevelop
{

bool MakeImplementationPrivate::process()
{
    return true;
}

bool MakeImplementationPrivate::gatherInformation()
{
    bool accepted;
    CodeGenUtils::IdentifierValidator validator(m_classContext);
    
    m_privatePointerName = KInputDialog::getText( QString("Private Pointer Name"),
                                                  QString("Specify the name of the private pointer"),
                                                  QString(), &accepted, ICore::self()->uiController()->activeMainWindow()->centralWidget(),                          
                                                  &validator);
    
    return accepted;
}

bool MakeImplementationPrivate::checkPreconditions(KDevelop::DUContext * context, const KDevelop::DocumentRange &)
{
    //TODO check that it doesn't already have a private implementation
    return context->type() == DUContext::Class;
}

}
