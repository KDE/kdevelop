/***************************************************************************
 *   Copyright (C) 2004 by Alexander Dymo                                  *
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
#include "rubyimplementationwidget.h"

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

RubyImplementationWidget::RubyImplementationWidget(KDevLanguageSupport* part, 
    QWidget* parent, const char* name, bool modal)
    :ImplementationWidget(part, parent, name, modal)
{
}

QStringList RubyImplementationWidget::createClassFiles()
{
    QString template_rb = "require '$BASEFILENAME$'\n\nclass $CLASSNAME$ < $BASECLASSNAME$\n\n    def initialize(*k)\n        super(*k)\n    end\n\nend\n";

    QFileInfo formInfo(m_formName);
    template_rb.replace(QRegExp("\\$BASEFILENAME\\$"), formInfo.baseName()+".rb");
    template_rb.replace(QRegExp("\\$CLASSNAME\\$"), classNameEdit->text());
    template_rb.replace(QRegExp("\\$BASECLASSNAME\\$"), m_baseClassName);
    
    template_rb = FileTemplate::read(m_part, "rb") + template_rb;

    QString file_rb = fileNameEdit->text() + ".rb";
    if (!m_part->project()->activeDirectory().isEmpty())
        file_rb = m_part->project()->activeDirectory() + "/" + file_rb;
    QFile file(QDir::cleanDirPath(m_part->project()->projectDirectory() + "/" + file_rb));
    if (!file.open(IO_WriteOnly)) {
        KMessageBox::error(0, i18n("Cannot write to file"));
        return QStringList();
    }
    QTextStream stream(&file);
    stream << template_rb;
    file.close();
        
    QStringList files;
    files.append(file_rb);
    return files;
}

#include "rubyimplementationwidget.moc"
