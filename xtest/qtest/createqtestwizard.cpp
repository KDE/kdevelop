/*
 * This file is part of KDevelop
 * Copyright 2009 Manuel Breugelmans <mbr.nxi@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "createqtestwizard.h"
#include "ui_classdetailspage.h"
#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <QCoreApplication>
#include <QRegExp>
#include <QFile>

using QTest::NewTestWizard;
using QTest::ClassDetailsPage;
using KDevelop::IProject;
using KDevelop::IProjectController;
using KDevelop::ICore;

ClassDetailsPage::ClassDetailsPage(QWidget* parent) : QWizardPage(parent)
{
    m_ui = new Ui::ClassDetailsPage;
    m_ui->setupUi(this);
    
    m_validClassPattern = QRegExp("[a-zA-Z_][a-zA-Z_1-9]*");    
    
    setTitle("Create QTest");
    setSubTitle("Generate QTest class skeleton implementation");

    QValidator* classValidator = new QRegExpValidator(m_validClassPattern, this);
    m_ui->identifierInput->setValidator(classValidator);
    m_ui->identifierInput->setFocus();
    registerField("classIdentifier*", ui()->identifierInput);
    
    // these connects are necessary to update enabled/disable state of the next button
    connect(m_ui->identifierInput, SIGNAL(editingFinished()),
            this, SIGNAL(completeChanged()));
    connect(m_ui->headerInput, SIGNAL(textChanged(QString)),
            this, SIGNAL(completeChanged()));
    connect(m_ui->sourceInput, SIGNAL(textChanged(QString)),
            this, SIGNAL(completeChanged()));
}

bool ClassDetailsPage::fto_isLegalClassIdentifier(const QString& identifier)
{
    return m_validClassPattern.exactMatch(identifier);
}

Ui::ClassDetailsPage* ClassDetailsPage::ui()
{
    return m_ui;
}

bool ClassDetailsPage::validatePage()
{
    if (!isComplete()) return false;
    
    // Only validate if both the header & source file candidates
    // do not exist yet
    if (QFile::exists(ui()->headerInput->url().pathOrUrl()))
        return false;
    if (QFile::exists(ui()->sourceInput->url().pathOrUrl()))
        return false;

    // not validate if source & header are equal
    if (ui()->headerInput->url() == ui()->sourceInput->url())
        return false;
    
    return QWizardPage::validatePage();
}

bool ClassDetailsPage::isComplete() const
{
    if (m_ui->identifierInput->text().isEmpty()) {
        return false;
    }
    if (!m_ui->identifierInput->hasAcceptableInput()) {
        return false;
    }
    if (m_ui->headerInput->url().isEmpty()) {
        return false;
    }
    if (m_ui->sourceInput->url().isEmpty()) {
        return false;
    }
    return QWizardPage::isComplete();
}

ClassDetailsPage::~ClassDetailsPage()
{}


NewTestWizard::NewTestWizard(QWidget* parent)
   : QWizard(parent)
{
    m_classPage = new ClassDetailsPage(this);
    connect(m_classPage->ui()->projectSelection, SIGNAL(activated(QString)),
            this, SLOT(setSelectedProject(QString)));
    connect(m_classPage->ui()->identifierInput, SIGNAL(editingFinished()),
            this, SLOT(guessFilenameFromIdentifier()));

    addPage(m_classPage);
}

void NewTestWizard::guessFilenameFromIdentifier()
{
    QString filename = m_classPage->ui()->identifierInput->text().toLower();
    if (filename.isEmpty()) return;
    KUrl srcUrl(m_classPage->ui()->sourceInput->url());
    srcUrl.setFileName(filename+".cpp");
    m_classPage->ui()->sourceInput->setUrl(srcUrl);
    KUrl headerUrl(m_classPage->ui()->headerInput->url());
    headerUrl.setFileName(filename+".h");
    m_classPage->ui()->headerInput->setUrl(headerUrl);
}

void NewTestWizard::setProjects(const QStringList& projectNames)
{
    foreach(const QString& projName, projectNames) {
        m_classPage->ui()->projectSelection->addItem(projName);
    }
    if (!projectNames.isEmpty()) {
        setSelectedProject(projectNames[0]);
    } else {
        m_classPage->ui()->projectSelection->setEnabled(false);
    }
}

void NewTestWizard::setSelectedProject(const QString& projectName)
{
    m_classPage->ui()->projectSelection->setCurrentItem(projectName);
    setBaseDirectory(rootFolderForProject(projectName));
}

NewTestWizard::~NewTestWizard()
{
}

QString NewTestWizard::testClassIdentifier() const
{
    return m_classPage->ui()->identifierInput->text();
}
    
QString NewTestWizard::selectedProject() const
{
    return m_classPage->ui()->projectSelection->currentText();
}
    
KUrl NewTestWizard::targetSourceFile() const
{
    return m_classPage->ui()->sourceInput->url();
}
    
KUrl NewTestWizard::targetHeaderFile() const
{
    return m_classPage->ui()->headerInput->url();
}

void NewTestWizard::setBaseDirectory(const KUrl& path)
{
    m_classPage->ui()->sourceInput->setUrl(path);
    m_classPage->ui()->headerInput->setUrl(path);
}

KUrl NewTestWizard::rootFolderForProject(const QString& projectName) const
{
    IProjectController* projCtrl = ICore::self()->projectController();
    foreach(IProject* proj, projCtrl->projects()) {
        if (proj->name() == projectName) {
            return proj->folder();
        }
    }
    return KUrl();
}

#include "createqtestwizard.moc"
