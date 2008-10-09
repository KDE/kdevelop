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

#ifndef VERITAS_COVERAGE_ANNOTATIONMANAGER_H
#define VERITAS_COVERAGE_ANNOTATIONMANAGER_H

#include "coverageexport.h"

#include <KUrl>
#include <QMap>

namespace KDevelop { class IDocument; }
namespace KTextEditor { class Document; class View; }

namespace Veritas
{

class CoveredFile;
/*!
 * Makes sure document views are annotated with coverage info
 * @unittest AnnotationManagerTest
 */
class VERITAS_COVERAGE_EXPORT AnnotationManager : public QObject
{
Q_OBJECT
public:
    AnnotationManager(QObject* parent=0);
    virtual ~AnnotationManager();

    void setCoveredFiles(const QMap<KUrl, CoveredFile*>& files);
    void watch(KDevelop::IDocument*);
    void stopWatching(KDevelop::IDocument*);

public Q_SLOTS:
    void addCoverageData(CoveredFile* f);
 
private Q_SLOTS:
    void connectKDocWithTextDoc(KDevelop::IDocument*);
    void fixAnnotation(KTextEditor::Document*, KTextEditor::View*);

private:
    QMap<KUrl, CoveredFile*> m_files;
    QList<KDevelop::IDocument*> m_docs;
    QMap<KTextEditor::Document*, KUrl> m_textDocUrls;
};

}

#endif // VERITAS_COVERAGE_ANNOTATIONMANAGER_H
