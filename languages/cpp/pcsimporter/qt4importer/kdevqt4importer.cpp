/***************************************************************************
 *   Copyright (C) 2003 by Roberto Raggi                                   *
 *   roberto@kdevelop.org                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kdevqt4importer.h"
#include "kdevqt4importer.moc"
#include "settingsdialog.h"

#include <kgenericfactory.h>
#include <ktempfile.h>
#include <kprocess.h>
#include <kdebug.h>

#include <qtextstream.h>
#include <qlabel.h>
#include <qdir.h>

K_EXPORT_COMPONENT_FACTORY( libkdevqt4importer, KGenericFactory<KDevQt4Importer>( "kdevqt4importer" ) )

KDevQt4Importer::KDevQt4Importer( QObject * parent, const char * name, const QStringList& )
    : KDevPCSImporter( parent, name )
{
    m_qtfile = 0;
}

KDevQt4Importer::~KDevQt4Importer()
{
    if (m_qtfile)
        delete m_qtfile;

    m_qtfile = 0;
}

QStringList KDevQt4Importer::fileList()
{
    if( !m_settings )
        return QStringList();

    if (m_qtfile)
        delete m_qtfile;

    KTempFile ifile;
    QTextStream &is = *ifile.textStream();

    is << "#include <QtCore/QtCore>\n"
       << "#include <QtGui/QtGui>\n"
       << "#include <QtNetwork/QtNetwork>\n"
       << "#include <QtXml/QtXml>\n"
       << "#include <Qt3Compat/Qt3Compat>\n";

    KProcess proc;
    proc << "cpp" << "-nostdinc" << "-xc++";

    m_qtfile = new KTempFile();

    // include paths
    QStringList paths = includePaths();
    for (QStringList::Iterator it = paths.begin(); it != paths.end(); ++it)
        proc << "-I" << *it;

    ifile.close();

    QString o;
    o += "-o";
    o += m_qtfile->name();

    proc << ifile.name() << o;
    proc.start(KProcess::Block);

    return m_qtfile->name();
}

QStringList KDevQt4Importer::includePaths()
{
    if( !m_settings || !m_qtfile)
        return QStringList();

    QStringList includePaths;
    includePaths.push_back( m_settings->qtDir() );
    includePaths.push_back( m_settings->qtDir() + "/Qt" );

    /// @todo add mkspec
    return includePaths;
}

QWidget * KDevQt4Importer::createSettingsPage( QWidget * parent, const char * name )
{
    m_settings = new SettingsDialog( parent, name );
    return m_settings;
}

