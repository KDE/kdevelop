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

#include "newtestwizardtest.h"
#include "../createqtestwizard.h"
#include <qtest_kde.h>
#include "../../kasserts.h"
#include "../ui_classdetailspage.h"

using QTest::NewTestWizardTest;
using QTest::NewTestWizard;
using QTest::ClassDetailsPage;

namespace QTest
{

class NewTestWizardFake : public NewTestWizard
{
public:
    QMap<QString, KUrl> m_folderForProject; // projectname x projectrootfolder
                                            // used in the overriden rootFolderForPRoject method below
protected:
    // overriden from NewTestWizard to allow for proper unittesting
    virtual KUrl rootFolderForProject(const QString& projectName) const {
	return m_folderForProject[projectName];
    }
};

}

using QTest::NewTestWizardFake;

void NewTestWizardTest::init()
{
    m_wizard = new NewTestWizardFake;
}

void NewTestWizardTest::cleanup()
{
  delete m_wizard;
}

void NewTestWizardTest::sunnySetProject()
{
    m_wizard->setProjects(QStringList() << "FooProject" << "BarProject");
    // initialize stub
    KUrl fooRoot("/path/to/foo");
    m_wizard->m_folderForProject["FooProject"] = fooRoot;

    m_wizard->setSelectedProject("FooProject");

    KOMPARE("FooProject", m_wizard->selectedProject());
    // now the base directory should have be seen set to FooProject's dir
    KOMPARE(fooRoot, m_wizard->targetHeaderFile());
    KOMPARE(fooRoot, m_wizard->targetSourceFile());    
}

void NewTestWizardTest::defaultSelectedProjectShouldBeQSTRING()
{
    // by default the selectedproject should equal 'QString()'
    KOMPARE(QString(), m_wizard->selectedProject());
}

void NewTestWizardTest::setBaseDirectoryShouldChangeHeaderSource()
{
    // a call to setBaseDirectory() should initialize the
    // header & source input fields

    KUrl foo("/path/to/foo");

    // defaults should be empty still
    KOMPARE(KUrl(), m_wizard->targetHeaderFile());
    KOMPARE(KUrl(), m_wizard->targetSourceFile());

    m_wizard->setBaseDirectory(foo);

    KOMPARE(foo, m_wizard->targetHeaderFile());
    KOMPARE(foo, m_wizard->targetSourceFile());
}

void NewTestWizardTest::legalClassIdentifiers()
{   
    ClassDetailsPage* page = new ClassDetailsPage(0);
    KVERIFY(!page->fto_isLegalClassIdentifier(""));
    KVERIFY(!page->fto_isLegalClassIdentifier("1a"));
    KVERIFY(!page->fto_isLegalClassIdentifier("1"));

    KVERIFY(page->fto_isLegalClassIdentifier("a"));
    KVERIFY(page->fto_isLegalClassIdentifier("A"));
    KVERIFY(page->fto_isLegalClassIdentifier("ab"));
    KVERIFY(page->fto_isLegalClassIdentifier("_"));
    KVERIFY(page->fto_isLegalClassIdentifier("a_b"));
    KVERIFY(page->fto_isLegalClassIdentifier("a1b2"));

    delete page;
}

void NewTestWizardTest::validateLegalClassPage()
{
    ClassDetailsPage* page = new ClassDetailsPage(0);
    page->ui()->identifierInput->setText("SomeClass");
    page->ui()->sourceInput->lineEdit()->setText("/path/to/source.cpp");
    page->ui()->headerInput->lineEdit()->setText("/path/to/header.h");
    
    KVERIFY(page->isComplete());
    KVERIFY(page->validatePage());

    delete page;
}

void NewTestWizardTest::doNotValidateEmptyClassField()
{
    ClassDetailsPage* page = new ClassDetailsPage(0);
    page->ui()->identifierInput->setText("");
    page->ui()->sourceInput->lineEdit()->setText("/path/to/source.cpp");
    page->ui()->headerInput->lineEdit()->setText("/path/to/header.h");
    
    KVERIFY(!page->isComplete());
    KVERIFY(!page->validatePage());

    delete page;
}

void NewTestWizardTest::sourceAndHeaderFilesShouldNotEqual()
{
    ClassDetailsPage* page = new ClassDetailsPage(0);
    page->ui()->identifierInput->setText("FooClass");
    page->ui()->sourceInput->lineEdit()->setText("/path/to/file.x");
    page->ui()->headerInput->lineEdit()->setText("/path/to/file.x");
    
    KVERIFY(page->isComplete());
    KVERIFY(!page->validatePage());

    delete page;
}

void NewTestWizardTest::validateOnlyNonExistingFiles()
{
    ClassDetailsPage* page = new ClassDetailsPage(0);

    KUrl existingFile(QDir::tempPath());
    existingFile.adjustPath(KUrl::AddTrailingSlash);
    existingFile.addPath("foo_file.cxx");
    QFile f(existingFile.toLocalFile());
    f.open(QIODevice::WriteOnly);
    f.write("123");
    f.close();
    Q_ASSERT(f.exists());

    page->ui()->identifierInput->setText("SomeClass");
    page->ui()->sourceInput->setUrl(existingFile); // this file does exist, so validation should fail
    page->ui()->headerInput->setUrl(KUrl("/path/to/header.h")); // this one does not exist
    
    KVERIFY(page->isComplete());
    KVERIFY(!page->validatePage());

    // now swap header & source input fields. ie an existing file in headerInput
    page->ui()->sourceInput->setUrl(KUrl("/path/to/header.h")); // this one does not exist
    page->ui()->headerInput->setUrl(existingFile); // this file does exist, so validation should fail

    KVERIFY(page->isComplete());
    KVERIFY(!page->validatePage());

    f.remove();
    delete page;
}

void NewTestWizardTest::showWidget()
{
    m_wizard->show();
//    QTest::qWait(20000);
}


QTEST_KDEMAIN( NewTestWizardTest, GUI )
#include "newtestwizardtest.moc"
