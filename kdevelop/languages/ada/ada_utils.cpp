/*
 */
#include <stdlib.h>
#include <unistd.h>
#include <iostream>

#include <qstring.h>
#include <qstringlist.h>
#include <qfile.h>
#include <qregexp.h>

#include <kdebug.h>

#include "ada_utils.hpp"
#include "adasupport.hpp"

QString qtext (const RefAdaAST& n)
{
  return QString::fromLatin1 (text (n).c_str ());
}

QStringList qnamelist (const RefAdaAST& n)
{
  QString txt = qtext (n);
  // kdDebug() << "qnamelist: input is " << txt << endl;
  return QStringList::split ('.', txt);
}

QString ada_spec_filename (const QString& comp_unit_name)
{
   QString fn (comp_unit_name.lower ());

   fn.replace (QRegExp("."), "-");
   fn += ".ads";
   return fn;
}

QString fq_specfilename (const QString& comp_unit_name)
{
    QString fname = ada_spec_filename (comp_unit_name);

    if (QFile::exists (fname))
        return fname;

    QString adaincpath = getenv ("ADA_INCLUDE_PATH");
    if (adaincpath.isNull ())
        return QString::null;

    QStringList dirs = QStringList::split (':', adaincpath);
    QString fq_filename;
    for (QStringList::Iterator it = dirs.begin (); it != dirs.end (); it++) {
        fq_filename = *it;
        if (! fq_filename.endsWith ("/"))
            fq_filename += "/";
        fq_filename += fname;
        if (QFile::exists (fq_filename))
            return fq_filename;
    }
    if (fname.startsWith ("ada-") ||
        fname.startsWith ("text_io") ||
        fname.startsWith ("system") ||
        fname.startsWith ("unchecked_")) {
        kdDebug () << "resolution of Ada predefined library is TBD" << endl;
    } else {
        kdDebug () << "Cannot find file " << fname << endl;
    }
    return QString::null;
}

