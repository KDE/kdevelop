/***************************************************************************
 *   Copyright (C) 2005 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
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
#include "pythonimplementationwidget.h"

#include <qdir.h>
#include <qfile.h>
#include <qregexp.h>
#include <qfileinfo.h>
#include <qtextstream.h>
#include <qlineedit.h>

#include <klocale.h>
#include <kmessagebox.h>

#include <filetemplate.h>
#include <kdevlanguagesupport.h>
#include <kdevproject.h>

PythonImplementationWidget::PythonImplementationWidget(KDevLanguageSupport* part, 
    QWidget* parent, const char* name, bool modal)
    : ImplementationWidget(part, parent, name, modal)
{
}


PythonImplementationWidget::~PythonImplementationWidget()
{
}


QStringList PythonImplementationWidget::createClassFiles()
{
//     QString template_py = "require '$BASEFILENAME$'\n\nclass $CLASSNAME$ < $BASECLASSNAME$\n\n    def initialize(*k)\n        super(*k)\n    end\n\nend\n";
    QString template_py = "from qt import *\nfrom $BASEFILENAME$ import *\nclass $CLASSNAME$($BASECLASSNAME$):\n\n    def __init__(self,parent,name):\n        $BASECLASSNAME$.__init__(self,parent,name)\n    \n\n\n";

    QFileInfo formInfo(m_formName);
    template_py.replace(QRegExp("\\$BASEFILENAME\\$"), formInfo.baseName()+".py");
    template_py.replace(QRegExp("\\$CLASSNAME\\$"), classNameEdit->text());
    template_py.replace(QRegExp("\\$BASECLASSNAME\\$"), m_baseClassName);
    
    template_py = FileTemplate::read(m_part, "py") + template_py;

    QString file_py = fileNameEdit->text() + ".py";
    if (!m_part->project()->activeDirectory().isEmpty())
        file_py = m_part->project()->activeDirectory() + "/" + file_py;
    QFile file(QDir::cleanDirPath(m_part->project()->projectDirectory() + "/" + file_py));
    if (!file.open(IO_WriteOnly)) {
        KMessageBox::error(0, i18n("Cannot write to file"));
        return QStringList();
    }
    QTextStream stream(&file);
    stream << template_py;
    file.close();
        
    QStringList files;
    files.append(file_py);
    return files;
}

#include "pythonimplementationwidget.moc"
