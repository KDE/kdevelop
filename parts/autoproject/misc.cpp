/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qdir.h>
#include <qwidget.h>
#include <kdebug.h>
#include <qfile.h>
#include <qtextstream.h>
#include <klibloader.h>
#include <kregexp.h>
#include <kservice.h>

#include "kdevcompileroptions.h"
#include "misc.h"


static KDevCompilerOptions *createCompilerOptions(const QString &name, QObject *parent)
{
    KService::Ptr service = KService::serviceByName(name);
    if (!service) {
        kdDebug(9020) << "Can't find service " << name;
        return 0;
    }
    
    KLibFactory *factory = KLibLoader::self()->factory(service->library());

    QStringList args;
    QVariant prop = service->property("X-KDevelop-Args");
    if (prop.isValid())
        args = QStringList::split(" ", prop.toString());
    
    QObject *obj = factory->create(parent, service->name().latin1(),
                                   "KDevCompilerOptions", args);

    if (!obj->inherits("KDevCompilerOptions")) {
        kdDebug(9000) << "Component does not inherit KDevCompilerOptions" << endl;
        return 0;
    }
    KDevCompilerOptions *dlg = (KDevCompilerOptions*) obj;
    
    return dlg;
}


QString AutoProjectTool::execFlagsDialog(const QString &compiler, const QString &flags, QWidget *parent)
{
    KDevCompilerOptions *plugin = createCompilerOptions(compiler, parent);
    
    if (plugin) {
        QString newflags = plugin->exec(parent, flags);
        delete plugin;
        return newflags;
    }
    return QString::null;
}


QCString AutoProjectTool::canonicalize(QString str)
{
    QCString res;
    for (uint i=0; i < str.length(); ++i)
        res += str[i].isLetterOrNumber()? str[i].latin1() : '_';

    return res;
}


void AutoProjectTool::parseMakefileam(const QString &filename, QMap<QCString,QCString> *variables)
{
    QFile f(filename);
    if (!f.open(IO_ReadOnly))
        return;
    QTextStream stream(&f);
    
    KRegExp re("^([A-Za-z][A-Za-z0-9_]*)[ \\t]*:?=[ \\t]*(.*)$");

    while (!stream.eof()) {
        QString line;
        QString s = stream.readLine();
        while (!s.isEmpty() && s[s.length()-1] == '\\' && !stream.eof()) {
            // Read continuation lines
            line += s.left(s.length()-1);
            s = stream.readLine();
        }
        line += s;
        
        if (re.match(line)) {
            QCString lhs = re.group(1);
            QCString rhs = re.group(2);
            variables->insert(lhs, rhs);
        }
    }
    
    f.close();
}


void AutoProjectTool::modifyMakefileam(const QString &filename, QMap<QCString,QCString> variables)
{
    QFile fin(filename);
    if (!fin.open(IO_ReadOnly))
        return;
    QTextStream ins(&fin);
    
    QFile fout(filename + "#");
    if (!fout.open(IO_WriteOnly)) {
        fin.close();
        return;
    }
    QTextStream outs(&fout);
    
    KRegExp re("^([A-Za-z][A-Za-z0-9_]*)[ \\t]*:?=[ \\t]*(.*)$");
    
    while (!ins.eof()) {
        QString line;
        QString s = ins.readLine();
        if (re.match(s)) {
            QCString lhs = re.group(1);
            QCString rhs = re.group(2);
            QMap<QCString,QCString>::Iterator it;
            for (it = variables.begin(); it != variables.end(); ++it)
                if (lhs == it.key())
                    break;
            if (it != variables.end()) {
                // Skip continuation lines
                while (!s.isEmpty() && s[s.length()-1] == '\\' && !ins.eof())
                    s = ins.readLine();
                s = it.key() + " = " + it.data();
                variables.remove(it);
            } else {
                while (!s.isEmpty() && s[s.length()-1] == '\\' && !ins.eof()) {
                    outs << s << endl;
                    s = ins.readLine();
                }
            }
        }
        
        outs << s << endl;
    }
    
    // Write new variables out
    QMap<QCString,QCString>::Iterator it2;
    for (it2 = variables.begin(); it2 != variables.end(); ++it2)
        outs << it2.key() + " = " + it2.data() << endl;
    
    fin.close();
    fout.close();

    QDir().rename(filename + "#", filename);
}
