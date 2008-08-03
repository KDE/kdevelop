
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
 * @test AnnotationManagerTest
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
