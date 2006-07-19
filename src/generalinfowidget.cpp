/***************************************************************************
 *   Copyright (C) 2002 by Yann Hodique                                    *
 *   Yann.Hodique@lifl.fr                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <QLineEdit>
#include <q3textedit.h>
#include <QComboBox>
#include <QLabel>
#include <qfileinfo.h>
#include <QDir>

#include <kurl.h>
#include <klocale.h>
#include <kiconloader.h>
#include "ui_generalinfowidgetbase.h"
#include "generalinfowidget.h"
#include "generalinfowidget.moc"
#include "domutil.h"

QString makeRelativePath(const QString& fromPath, const QString& toPath);

GeneralInfoWidget::GeneralInfoWidget(QDomDocument &projectDom, QWidget *parent)
        : QWidget(parent), m_projectDom(projectDom)
{
    m_baseUi = new Ui::GeneralInfoWidgetBase();
    m_baseUi->setupUi(this);
    connect(m_baseUi->project_directory_edit, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotProjectDirectoryChanged(const QString&)));
    connect(m_baseUi->project_directory_combo, SIGNAL(activated(int)),
            this, SLOT(slotProjectDirectoryComboChanged()));
    readConfig();
}



GeneralInfoWidget::~GeneralInfoWidget()
{
    delete m_baseUi;
}

void GeneralInfoWidget::readConfig() {
    if(DomUtil::readBoolEntry(m_projectDom,"/general/absoluteprojectpath",false))
        m_baseUi->project_directory_combo->setCurrentIndex(0);
    else
        m_baseUi->project_directory_combo->setCurrentIndex(1);
    m_baseUi->project_directory_edit->setText(DomUtil::readEntry(m_projectDom,"/general/projectdirectory","."));
    m_baseUi->author_edit->setText(DomUtil::readEntry(m_projectDom,"/general/author"));
    m_baseUi->email_edit->setText(DomUtil::readEntry(m_projectDom,"/general/email"));
    m_baseUi->version_edit->setText(DomUtil::readEntry(m_projectDom,"/general/version"));
    m_baseUi->description_edit->setText(DomUtil::readEntry(m_projectDom,"/general/description"));
}

void GeneralInfoWidget::writeConfig() {
    DomUtil::writeEntry(m_projectDom,"/general/projectdirectory", m_baseUi->project_directory_edit->text());
    DomUtil::writeBoolEntry(m_projectDom,"/general/absoluteprojectpath",isProjectDirectoryAbsolute());
    DomUtil::writeEntry(m_projectDom,"/general/email", m_baseUi->email_edit->text());
    DomUtil::writeEntry(m_projectDom,"/general/author", m_baseUi->author_edit->text());
    DomUtil::writeEntry(m_projectDom,"/general/email", m_baseUi->email_edit->text());
    DomUtil::writeEntry(m_projectDom,"/general/version", m_baseUi->version_edit->text());
    DomUtil::writeEntry(m_projectDom,"/general/description", m_baseUi->description_edit->text());
}

void GeneralInfoWidget::accept() {
    writeConfig();
}

bool GeneralInfoWidget::isProjectDirectoryAbsolute() {
    return m_baseUi->project_directory_combo->currentIndex() == 0;
}

QString GeneralInfoWidget::projectDirectory() {
    //FIXME
    return QString::null;
}

void GeneralInfoWidget::slotProjectDirectoryChanged( const QString& text ) {
    if(text.isEmpty())
    {
       setProjectDirectoryError(i18n("Please enter a path."));
    }
    else if(isProjectDirectoryAbsolute() && text[0] != '/')
    {
       setProjectDirectoryError(
           i18n("'%1' is not an absolute path.",
               m_baseUi->project_directory_edit->text()));
    }
    else if(!isProjectDirectoryAbsolute() && text[0] == '/')
    {
       setProjectDirectoryError(
           i18n("'%1' is not a relative path.",
               m_baseUi->project_directory_edit->text()));
    }
    else
    {
        QFileInfo info(projectDirectory());
        if(!info.exists())
           setProjectDirectoryError(
               i18n("'%1' does not exist.",
                   m_baseUi->project_directory_edit->text()));
        else if(!info.isDir())
           setProjectDirectoryError(
               i18n("'%1' is not a directory.",
                   m_baseUi->project_directory_edit->text()));
        else
           setProjectDirectorySuccess();
    }
}

void GeneralInfoWidget::slotProjectDirectoryComboChanged() {
    //FIXME
}

void GeneralInfoWidget::setProjectDirectoryError( const QString& error ) {
    m_baseUi->project_directory_diagnostic_icon->setPixmap(SmallIcon("no"));
    m_baseUi->project_directory_diagnostic_label->setText( error );
}

void GeneralInfoWidget::setProjectDirectorySuccess() {
    m_baseUi->project_directory_diagnostic_icon->setPixmap(SmallIcon("ok"));
    if(isProjectDirectoryAbsolute())
        m_baseUi->project_directory_diagnostic_label->setText(
            i18n("'%1' is a valid project directory.", projectDirectory()));
    else
        m_baseUi->project_directory_diagnostic_label->setText(
            i18n("'%1' is a valid project directory.", projectDirectory()));
}

QString makeRelativePath(const QString& fromPath, const QString& toPath)
{
    if ( fromPath == toPath )
        return ".";

    QStringList fromDirs = fromPath.split( '/' );
    QStringList toDirs   = toPath.split( '/' );
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

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
