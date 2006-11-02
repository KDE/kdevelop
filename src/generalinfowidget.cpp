/***************************************************************************
 *   Copyright (C) 2002 by Yann Hodique                                    *
 *   Yann.Hodique@lifl.fr                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <klineedit.h>
#include <qtextedit.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qfileinfo.h>
#include <qdir.h>

#include <kurl.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kcharsets.h>
#include <qregexp.h>
#include <kmessagebox.h>

#include "generalinfowidget.h"
#include "generalinfowidget.moc"
#include "domutil.h"
#include "projectmanager.h"

QString makeRelativePath(const QString& fromPath, const QString& toPath);

GeneralInfoWidget::GeneralInfoWidget(QDomDocument &projectDom, QWidget *parent, const char *name)
        : GeneralInfoWidgetBase(parent, name), m_projectDom(projectDom) {

    connect(project_directory_edit, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotProjectDirectoryChanged(const QString&)));
    connect(project_directory_combo, SIGNAL(activated(int)),
            this, SLOT(slotProjectDirectoryComboChanged()));
    readConfig();
}



GeneralInfoWidget::~GeneralInfoWidget() {}

void GeneralInfoWidget::readConfig() {
    if(DomUtil::readBoolEntry(m_projectDom,"/general/absoluteprojectpath",false))
        this->project_directory_combo->setCurrentItem(0);
    else
        this->project_directory_combo->setCurrentItem(1);
    this->project_directory_edit->setText(DomUtil::readEntry(m_projectDom,"/general/projectdirectory","."));
    this->author_edit->setText(DomUtil::readEntry(m_projectDom,"/general/author"));
    this->email_edit->setText(DomUtil::readEntry(m_projectDom,"/general/email"));
    this->version_edit->setText(DomUtil::readEntry(m_projectDom,"/general/version"));
    this->description_edit->setText(DomUtil::readEntry(m_projectDom,"/general/description"));

    QStringList encodings;
    encodings << i18n("Use global editor settings");
    encodings += KGlobal::charsets()->descriptiveEncodingNames();
    QStringList::const_iterator it = encodings.constBegin();
    while ( it != encodings.constEnd() )
    {
        encoding_combo->insertItem( *it );
        ++it;
    }
    encoding_combo->setCurrentItem( 0 );

//    const QString DefaultEncoding = KGlobal::charsets()->encodingForName( DomUtil::readEntry( m_projectDom, "/general/defaultencoding", QString::null ) );
    const QString DefaultEncoding = DomUtil::readEntry( m_projectDom, "/general/defaultencoding", QString::null );
    for ( int i = 0; i < encoding_combo->count(); i++ )
    {
        if ( KGlobal::charsets()->encodingForName( encoding_combo->text( i ) ) == DefaultEncoding )
        {
            encoding_combo->setCurrentItem( i );
            break;
        }
    }

}
/**
 * Update the configure.in, configure.in or configure.ac file with the version value from project options.
 * Very basic updating - uses regex to update the field in
 * AC_INIT, AM_INIT_AUTOMAKE, and AC_DEFINE macros.
 * On next make, the build system re-runs configure to update config.h
 * version info.
 *
 * @param configureinpath Full path to configure.in file
 * @param newVersion The new version number or string
*/
void GeneralInfoWidget::configureinUpdateVersion( QString configureinpath, QString newVersion )
{
    QFile configurein(configureinpath);

    if ( !configurein.open( IO_ReadOnly ) ){
        KMessageBox::error(this, i18n("Could not open %1 for reading.").arg(configureinpath));
        return;
    }

    QTextStream stream( &configurein);
    QStringList list;

    // Options for version:

    // we ignore old AC_INIT that had no version..
    // only match the if there is a comma and at least two args..
    // AC_INIT (package, version, [bug-report], [tarname])
    QRegExp ac_init("^AC_INIT\\s*\\(\\s*([^,]+),([^,\\)]+)(.*)");

    // AM_INIT_AUTOMAKE([OPTIONS])
    // example: AM_INIT_AUTOMAKE([gnits 1.5 no-define dist-bzip2])
    QRegExp am_autoSpace("^AM_INIT_AUTOMAKE\\s{0,}\\(\\s{0,}([\\[\\s]{0,}[^\\s]+)\\s+([^\\s\\)\\]]+)(.*)");

    // AM_INIT_AUTOMAKE(PACKAGE, VERSION, [NO-DEFINE])
    QRegExp am_autoComma("^AM_INIT_AUTOMAKE\\s*\\(\\s*([^,]+),([^,\\)]+)(.*)");

    // look for version in a define.
    // AC_DEFINE(VERSION, "5.6")
    QRegExp ac_define("^AC_DEFINE\\s*\\(");
    QRegExp version("(\\bversion\\b)");
    version.setCaseSensitive(FALSE);

    while ( !stream.eof() ) {
        QString line = stream.readLine();
        if ( ac_init.search(line) >= 0){
            line = "AC_INIT(" + ac_init.cap(1).stripWhiteSpace();
            line += ", ";
            line += newVersion;
            line += ac_init.cap(3).stripWhiteSpace();
        }
        else if ( am_autoComma.search(line) >= 0 ){
            line="AM_INIT_AUTOMAKE(";
            line += am_autoComma.cap(1).stripWhiteSpace();
            line += ", ";
            line += newVersion;
            line += am_autoComma.cap(3).stripWhiteSpace();
        }
        else if ( am_autoSpace.search(line) >= 0 ){
            line = "AM_INIT_AUTOMAKE(" + am_autoSpace.cap(1).stripWhiteSpace();
            line += " ";
            line += newVersion;
            line += " ";
            line += am_autoSpace.cap(3).stripWhiteSpace();
        }
        else if ( ac_define.search(line) >=0 && version.search(line) >=0) {
            // replace version in: AC_DEFINE(VERSION,"0.1")
            line="AC_DEFINE(" + version.cap(1).stripWhiteSpace()+", \"" + newVersion +"\")";
        }
        list.push_back(line);
    }
    configurein.close();

    // write our changes..
    QFile configureout(configureinpath);
    if ( !configureout.open( IO_WriteOnly ) ){
        KMessageBox::error(this, i18n("Could not open %1 for writing.").arg(configureinpath));
        return ;
    }
    QTextStream output( &configureout);
    for(QStringList::iterator iter = list.begin();iter!=list.end();iter++){
        output << (*iter) <<"\n";
    }
    configureout.close();
}

void GeneralInfoWidget::writeConfig() {
    DomUtil::writeEntry(m_projectDom,"/general/projectdirectory",project_directory_edit->text());
    DomUtil::writeBoolEntry(m_projectDom,"/general/absoluteprojectpath",isProjectDirectoryAbsolute());
    DomUtil::writeEntry(m_projectDom,"/general/email",email_edit->text());
    DomUtil::writeEntry(m_projectDom,"/general/author",author_edit->text());
    DomUtil::writeEntry(m_projectDom,"/general/email",email_edit->text());
    if ( DomUtil::readEntry(m_projectDom,"/general/version") != version_edit->text() ){
        // update the configure.in.in, configure.in or configure.ac file.
		QFile inInFile(projectDirectory() + "/configure.in.in");
        QFile inFile(projectDirectory() + "/configure.in");
        QFile acFile(projectDirectory() + "/configure.ac");
		if ( inInFile.exists()){
			configureinUpdateVersion( inInFile.name(), version_edit->text() );
		}
        if ( inFile.exists()){
            configureinUpdateVersion( inFile.name(), version_edit->text() );
        }
		if (acFile.exists()){
            configureinUpdateVersion( acFile.name(), version_edit->text() );
        }
		if (! inInFile.exists()&& !inFile.exists() && !acFile.exists()) {
            KMessageBox::error(this, i18n("Could find configure.in.in, configure.in or configure.ac to update the project version."));
        }
    }
    DomUtil::writeEntry(m_projectDom,"/general/version",version_edit->text());
    DomUtil::writeEntry(m_projectDom,"/general/description",description_edit->text());

    QString DefaultEncoding = QString::null;
    if ( encoding_combo->currentItem() > 0 )
    {
        DefaultEncoding = KGlobal::charsets()->encodingForName( encoding_combo->currentText() );
    }
    DomUtil::writeEntry( m_projectDom, "/general/defaultencoding", DefaultEncoding );
}

void GeneralInfoWidget::accept() {
    writeConfig();
}

bool GeneralInfoWidget::isProjectDirectoryAbsolute() {
    return project_directory_combo->currentItem() == 0;
}

QString GeneralInfoWidget::projectDirectory() {
    return ProjectManager::projectDirectory( project_directory_edit->text(), isProjectDirectoryAbsolute() );
}

void GeneralInfoWidget::slotProjectDirectoryChanged( const QString& text ) {
    if(text.isEmpty())
    {
       setProjectDirectoryError(i18n("Please enter a path."));
    }
    else if(isProjectDirectoryAbsolute() && text[0] != '/')
    {
       setProjectDirectoryError(
           i18n("'%1' is not an absolute path.").arg(
               project_directory_edit->text()));
    }
    else if(!isProjectDirectoryAbsolute() && text[0] == '/')
    {
       setProjectDirectoryError(
           i18n("'%1' is not a relative path.").arg(
               project_directory_edit->text()));
    }
    else
    {
        QFileInfo info(projectDirectory());
        if(!info.exists())
           setProjectDirectoryError(
               i18n("'%1' does not exist.").arg(
                   project_directory_edit->text()));
        else if(!info.isDir())
           setProjectDirectoryError(
               i18n("'%1' is not a directory.").arg(
                   project_directory_edit->text()));
        else
           setProjectDirectorySuccess();
    }
}

void GeneralInfoWidget::slotProjectDirectoryComboChanged() {
    QString text = project_directory_edit->text();
    if(isProjectDirectoryAbsolute() && text[0] != '/' )
        project_directory_edit->setText(ProjectManager::projectDirectory(text,false));
    else if(!isProjectDirectoryAbsolute() && text[0] == '/')
    {
        project_directory_edit->setText(KURL(ProjectManager::getInstance()->projectFile(), text).url());
    }
}

void GeneralInfoWidget::setProjectDirectoryError( const QString& error ) {
    project_directory_diagnostic_icon->setPixmap(SmallIcon("no"));
    project_directory_diagnostic_label->setText( error );
}

void GeneralInfoWidget::setProjectDirectorySuccess() {
    project_directory_diagnostic_icon->setPixmap(SmallIcon("ok"));
    if(isProjectDirectoryAbsolute())
        project_directory_diagnostic_label->setText(
            i18n("'%1' is a valid project directory.").arg(projectDirectory()));
    else
        project_directory_diagnostic_label->setText(
            i18n("'%1' is a valid project directory.").arg(projectDirectory()));
}

QString makeRelativePath(const QString& fromPath, const QString& toPath)
{
    if ( fromPath == toPath )
        return ".";

    QStringList fromDirs = QStringList::split( '/', fromPath );
    QStringList toDirs   = QStringList::split( '/', toPath );
    QStringList::iterator fromIt = fromDirs.begin();
    QStringList::iterator toIt   = toDirs.begin();

    QString relative;

    for ( ; (*fromIt) == (*toIt); ++fromIt, ++toIt )
        ;

    for ( ; fromIt != fromDirs.end(); ++fromIt )
        relative += "../";

    for ( ; toIt != toDirs.end(); ++toIt )
        relative += *toIt + "/";

    return relative;
}
