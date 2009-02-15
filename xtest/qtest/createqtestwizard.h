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

#ifndef QTEST_CREATEQTESTWIZARD_H
#define QTEST_CREATEQTESTWIZARD_H

#include <QWizard>
#include "qxqtestexport.h"

namespace Ui { class ClassDetailsPage; }
namespace KDevelop { class IProject; }
class KUrl;

namespace QTest
{

/*! @unittest QTest::NewTestWizardTest */
class QXQTEST_EXPORT NewTestWizard : public QWizard
{
Q_OBJECT
public:
    NewTestWizard(QWidget* parent=0);
    virtual ~NewTestWizard();

    /*! Initialize the projects that can be selected in the wizard. */
    void setProjects(const QStringList& projectNames);

    /*! Initialize default directory for the header & source input fields */
    void setBaseDirectory(const KUrl& path);
    
    /*! Name of the test class.
     *  @note namespaces currently not supported */
    QString testClassIdentifier() const;
    
    /*! Name of the project the test class should belong to.
     *  Returns QString() if nothing was selected. */
    QString selectedProject() const;
    
    /*! Full path to the sourcefile the test class should be written
      * to */
    KUrl targetSourceFile() const;
    
    /*! Full path to the headerfile the test class should be written
     *  to */
    KUrl targetHeaderFile() const;
    
public slots:
    void setSelectedProject(const QString& projectName);

private slots:
    void guessFilenameFromIdentifier();
        
protected:
    virtual KUrl rootFolderForProject(const QString& projectName) const;
    
private:
    class ClassDetailsPage* m_classPage;
};

/*! page in the NewTestWizard that asks for the classidentifier, filename etc */
class QXQTEST_EXPORT ClassDetailsPage : public QWizardPage
{
Q_OBJECT
    public:
        ClassDetailsPage(QWidget* parent);
        virtual ~ClassDetailsPage();

       /*! Returns true if @p identifier is a valid C++ class name, false
        *  if invalid */
        bool fto_isLegalClassIdentifier(const QString& identifier);
        
        /*! re-implemented from QWizardPage */
        virtual bool validatePage();
        
        /*! re-implemented from QWizardPage */
        virtual bool isComplete() const;
        
        Ui::ClassDetailsPage* ui();

    private:
        QRegExp m_validClassPattern;
        Ui::ClassDetailsPage* m_ui;
};

} // namespace QTest

#endif // CREATEQTESTWIZARD_H
