/***************************************************************************
 *   Copyright (C) 2004 by Alexander Dymo                                  *
 *   adymo@mksat.net                                                       *
 *   Portions Copyright (C) 2003 Roberto Raggi (roberto@kdevelop.org)      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "qtdesignerrubyintegration.h"

#include <qpair.h>
#include <qregexp.h>

#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kurl.h>
#include <ktexteditor/editinterface.h>
#include <ktexteditor/view.h>
#include <ktexteditor/viewcursorinterface.h>

#include <domutil.h>
#include <kdevpartcontroller.h>
#include <kdevcreatefile.h>
#include <kdevlanguagesupport.h>

#include "codemodel_utils.h"
#include "implementationwidget.h"

QtDesignerRubyIntegration::QtDesignerRubyIntegration(KDevLanguageSupport *part,
    ImplementationWidget *impl)
    :QtDesignerIntegration(part, impl, false, 0)
{
}

void QtDesignerRubyIntegration::addFunctionToClass(KInterfaceDesigner::Function function, ClassDom klass)
{
    m_part->partController()->editDocument( KURL( klass->fileName() ) );
    KTextEditor::EditInterface* editIface = dynamic_cast<KTextEditor::EditInterface*>( m_part->partController()->activePart() );
    if( !editIface ){
        /// @todo show messagebox
//        QDialog::accept();
        return;
    }
    
    kdDebug() << "===============" << endl;

    int line, column;
    klass->getStartPosition( &line, &column );

    // compute the insertion point
    QPair<int,int> point;
    point.first = line + 1;
    point.second = column;

    const FunctionList functionList = klass->functionList();
    if (functionList.count() > 0)
    {
        int funEndLine, funEndColumn;
        functionList.first()->getEndPosition(&funEndLine, &funEndColumn);
        point.second = funEndColumn;
    } 

    QString str = function.function;
    str += "\n    \n    end\n\n";
    str = "    def " + str;

    kdDebug() << "insert " << str << " into " << point.first << endl;
    
    editIface->insertText(point.first, 0 /*pt.second*/, str );

    KTextEditor::View *activeView = dynamic_cast<KTextEditor::View*>( m_part->partController()->activePart()->widget() );
    if (activeView)
    {
        KTextEditor::ViewCursorInterface* cursor = dynamic_cast<KTextEditor::ViewCursorInterface*>(activeView );
        if (cursor)
            cursor->setCursorPositionReal(point.first, 4);
    }
}

#include "qtdesignerrubyintegration.moc"
