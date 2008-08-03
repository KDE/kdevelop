/* KDevelop xUnit plugin
 *
 * Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
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

#ifndef COVERAGE_ANNOTATIONMANAGER_TEST_H
#define COVERAGE_ANNOTATIONMANAGER_TEST_H

#include <QtCore/QObject>
#include <KUrl>

namespace KDevelop { class ICore; class IDocument; }
namespace KTextEditor { class View; }
namespace TestStubs { class Core; class KDevDocument; class DocumentController; }
namespace Veritas { class CoveredFile; }

namespace Veritas
{

class AnnotationManager;
class AnnotationManagerTest : public QObject
{
Q_OBJECT
private slots:
    void initTestCase();
    void init();
    void cleanup();

    void instantiate();
    void singleLine();
    void multipleLines();
    void multipleViews();
    void multipleFiles();
    void notCovered();
    void stopWatching();
    void multiCoverageOneFile();
    
private: // helpers
    void initManager(Veritas::CoveredFile* f);
    KTextEditor::View* triggerAnnotationsOnView(KDevelop::IDocument*);
    TestStubs::KDevDocument* createKDevDocument(const KUrl& u);
    Veritas::CoveredFile* createCoveredFile(int line, int callCount, KUrl);
    void assertAnnoCallCountEquals(int line, int count, KTextEditor::View*);
    void assertNoAnnotationsOnLine(int line, KTextEditor::View* v);
    void assertAnnotationsGone(KTextEditor::View* v);

private: // fixture
    AnnotationManager* m_manager;
    TestStubs::Core* m_core;
    TestStubs::DocumentController* m_docCtrl;
    KUrl m_someUrl;
    QWidget* m_widget;
    int m_lineNr;       // source line number
    int m_nrofCalls;    // number of calls on this line
};

}

#endif // COVERAGE_ANNOTATIONMANAGER_TEST_H
