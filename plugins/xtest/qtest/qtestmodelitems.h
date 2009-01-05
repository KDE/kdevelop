/* KDevelop xUnit plugin
 *
 * Copyright 2008-2009 Manuel Breugelmans <mbr.nxi@gmail.com>
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

#ifndef KDEVELOP_XTEST_QTESTMODELITEMS_H
#define KDEVELOP_XTEST_QTESTMODELITEMS_H

#include <QString>
#include <QFileInfo>
#include <KUrl>

#include <veritas/test.h>
#include "qxqtestexport.h"

/*! This file contains the test-runner model items. Each of these corresponds
 *  to an entry in the runnerview tree. */

class KJob;

namespace QTest
{
class Executable;
class Case;
class Command;

/*! Item in the test runner tree. Aggregates QTest::Cases
 *  Since QTestLib lacks the notion of suites these are deduced
 *  from the filesystem, that is each separate directory is a suite.
 *  @unittest QTest::ut::SuiteTest */
class QXQTEST_EXPORT Suite : public Veritas::Test
{
Q_OBJECT
public:
    Suite(const QString&, const QFileInfo&, Veritas::Test* parent);
    virtual ~Suite();
    Case* child(int i) const;

    QFileInfo path();
    void setPath(const QFileInfo&);

private:
    QFileInfo m_path;
};


/*! QTestLib test class item in the test tree. Each Case corresponds
 *  to an executable.
 *  @unittest QTest::CaseTest */
class QXQTEST_EXPORT Case : public Veritas::Test
{
Q_OBJECT

public slots:
   virtual int run();
   
public:
    Case(const QString&, const QFileInfo&, Suite* parent=0);
    Case(const QString&, const KUrl&, Suite* parent=0);
    virtual ~Case();

    void emitExecutionFinished();
    
    /*! Copy this test and all it's children. */
    Case* clone() const;
    
    Command* child(int i) const;
    Executable* executable() const;
    
    virtual void kill();
    virtual bool shouldRun() const;
    virtual void toSource() const;
    virtual KUrl source() const;

    void setSource(const KUrl& source);
    void setExecutable(Executable*);
    virtual KJob* createVerboseOutputJob();
    
private:
    KUrl m_source;
    Executable* m_executable;
};

/*! Leaf item in the test tree. It represents a single test function, a
 *  private slot in the Case. 
 *  @unittest QTest::CommandTest */
class QXQTEST_EXPORT Command : public Veritas::Test
{
Q_OBJECT
public:
    Command(const QString&, Case*);
    virtual ~Command();
    QString command(); // dead code for now, but might want it again to allow
                       // for execution of individual commands (vs full case)
    virtual void toSource() const;
    Command* clone() const;
    
protected:
    /*! Open document in editor. Protected so tests can override this */
    virtual void openDocument(const KUrl&) const;
};


} // namespace QTest

#endif // KDEVELOP_XTEST_QTESTMODELITEMS_H
