/***************************************************************************
 *   Copyright (C) 2004 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "cppimplementationwidget.h"

#include <qfileinfo.h>
#include <qtextstream.h>
#include <qfile.h>
#include <qdir.h>
#include <qregexp.h>
#include <qdom.h>
#include <qlineedit.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <klistview.h>

#include <kdevlanguagesupport.h>
#include <kdevproject.h>
#include <domutil.h>
#include <filetemplate.h>

CppImplementationWidget::CppImplementationWidget(KDevLanguageSupport *part,
    QWidget *parent, const char *name, bool modal)
    :ImplementationWidget(part, parent, name, modal)
{
}

QStringList CppImplementationWidget::createClassFiles()
{
    QString template_h = "#ifndef $DEFTEXT$_H\n#define $DEFTEXT$_H\n\n#include \"$BASEINCLUDE$\"\n\nclass $CLASSNAME$: public $BASECLASSNAME$ {\nQ_OBJECT\npublic:\n    $CLASSNAME$(QWidget *parent = 0, const char *name = 0);\n};\n\n#endif\n";
    QString template_cpp = "#include \"$CLASSINCLUDE$\"\n\n$CLASSNAME$::$CLASSNAME$(QWidget *parent, const char *name)\n    :$BASECLASSNAME$(parent, name)\n{\n}\n";
    if (m_part->project()->options() == KDevProject::UsesAutotoolsBuildSystem)
        template_cpp += "\n#include \"$MOCINCLUDE$\"\n";

    QFileInfo formInfo(m_formName);
    template_h.replace(QRegExp("\\$BASEINCLUDE\\$"), formInfo.baseName()+".h");
    template_h.replace(QRegExp("\\$CLASSNAME\\$"), classNameEdit->text());
    template_h.replace(QRegExp("\\$BASECLASSNAME\\$"), m_baseClassName);
    template_h.replace(QRegExp("\\$DEFTEXT\\$"), fileNameEdit->text().upper());

    template_cpp.replace(QRegExp("\\$CLASSINCLUDE\\$"), fileNameEdit->text() + ".h");
    template_cpp.replace(QRegExp("\\$CLASSNAME\\$"), classNameEdit->text());
    template_cpp.replace(QRegExp("\\$BASECLASSNAME\\$"), m_baseClassName);
    template_cpp.replace(QRegExp("\\$MOCINCLUDE\\$"), fileNameEdit->text() + ".moc");

    template_h = FileTemplate::read(m_part, "h") + template_h;
    template_cpp = FileTemplate::read(m_part, "cpp") + template_cpp;

    QString file_h = fileNameEdit->text() + ".h";
    QString file_cpp = fileNameEdit->text() + ".cpp";
    if (!m_part->project()->activeDirectory().isEmpty())
    {
        file_h = m_part->project()->activeDirectory() + "/" + file_h;
        file_cpp = m_part->project()->activeDirectory() + "/" + file_cpp;
    }

    QFile ifile(QDir::cleanDirPath(m_part->project()->projectDirectory() + "/" + file_cpp));
    if (!ifile.open(IO_WriteOnly)) {
        KMessageBox::error(this, i18n("Cannot write to implementation file"));
        return QStringList();
    }
    QTextStream istream(&ifile);
    istream << template_cpp;
    ifile.close();

    QFile hfile(QDir::cleanDirPath(m_part->project()->projectDirectory() + "/" + file_h));
    if (!hfile.open(IO_WriteOnly)) {
        KMessageBox::error(this, i18n("Cannot write to header file"));
        return QStringList();
    }
    QTextStream hstream(&hfile);
    hstream << template_h;
    hfile.close();

    QStringList fileList;
    fileList.append(file_h);
    fileList.append(file_cpp);

    return fileList;
}

#include "cppimplementationwidget.moc"
