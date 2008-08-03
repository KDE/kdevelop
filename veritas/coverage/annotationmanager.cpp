
#include "annotationmanager.h"
#include "annotationmodel.h"
#include "coveredfile.h"

#include <interfaces/icore.h>
#include <interfaces/idocument.h>
#include <interfaces/idocumentcontroller.h>
#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
#include <ktexteditor/annotationinterface.h>
#include <KDebug>

using KDevelop::ICore;
using KDevelop::IDocument;
using KDevelop::IDocumentController;
using KTextEditor::AnnotationViewInterface;

using Veritas::AnnotationManager;
using Veritas::AnnotationModel;

AnnotationManager::AnnotationManager(QObject* parent)
    : QObject(parent)
{
    IDocumentController* dc = ICore::self()->documentController();
    connect(dc, SIGNAL(textDocumentCreated(KDevelop::IDocument*)),
            this, SLOT(connectKDocWithTextDoc(KDevelop::IDocument*)));
}

AnnotationManager::~AnnotationManager()
{
    foreach(CoveredFile* f, m_files) delete f;
}

void AnnotationManager::connectKDocWithTextDoc(KDevelop::IDocument* doc)
{
    if (not m_docs.contains(doc)) return;
    Q_ASSERT(doc->isTextDocument());
    KTextEditor::Document* tdoc = doc->textDocument();
    Q_ASSERT_X(tdoc, "AnnotationManager::watch()", "doc->textDoc() null.");
    m_textDocUrls[tdoc] = doc->url();
    tdoc->disconnect(this);
    connect(tdoc, SIGNAL(viewCreated(KTextEditor::Document*, KTextEditor::View*)),
            this, SLOT(fixAnnotation(KTextEditor::Document*, KTextEditor::View*)));
}

void AnnotationManager::setCoveredFiles(const QMap<KUrl, CoveredFile*>& files)
{
    m_files = files;
}

void AnnotationManager::watch(IDocument* doc)
{
    Q_ASSERT_X(doc, "AnotationManager::watch()", "got null document.");
    m_docs << doc;
    Q_ASSERT(doc->isTextDocument());
    kDebug() << doc;
}

void AnnotationManager::fixAnnotation(KTextEditor::Document* doc, KTextEditor::View* view)
{
    if (not m_textDocUrls.contains(doc)) {
        kDebug() << "Shouldn't happen, got a doc for which no url was registered.";
        return;
    }
    KUrl url = m_textDocUrls[doc];
    if (not m_files.contains(url)) {
        kDebug() << "No CoveredFile data available for " << url;
        return;
    }
    CoveredFile* f = m_files[url];
    Q_ASSERT(f);
    AnnotationModel* model = new AnnotationModel(f);
    AnnotationViewInterface *anno = qobject_cast<AnnotationViewInterface*>(view);
    Q_ASSERT(anno);
    anno->setAnnotationModel(model);
    anno->setAnnotationBorderVisible(true);
}

void AnnotationManager::stopWatching(IDocument* doc)
{
    if (not m_docs.contains(doc)) return;
    m_docs.removeOne(doc);
    KTextEditor::Document* tdoc = doc->textDocument();
    Q_ASSERT(tdoc);
    tdoc->disconnect(this);
    foreach(KTextEditor::View* view, tdoc->views()) {
        AnnotationViewInterface* anno = qobject_cast<AnnotationViewInterface*>(view);
        if (not anno) continue;
        anno->setAnnotationBorderVisible(0);
        anno->setAnnotationModel(0);
    }
}

void AnnotationManager::addCoverageData(CoveredFile* f)
{
    QMap<int, int> cc = f->callCountMap();
    QMapIterator<int, int> it(cc);
    if (!m_files.contains(f->url())) {
        kDebug() << "not contains";
        CoveredFile* cf = new CoveredFile;
        cf->setUrl(f->url());
        while(it.hasNext()) {
            it.next();
            kDebug() << it.key() << "->" << it.value();
            cf->setCallCount(it.key(), it.value());
        }
        m_files[f->url()] = cf;        
    } else {
        kDebug() << "contains";
        CoveredFile* cf = m_files[f->url()];
        QSet<int> newLines = f->reachableLines() - cf->reachableLines();
        foreach(int line, newLines) {
            cf->setCallCount(line, 0);
        }
        QMap<int, int> curcc = cf->callCountMap();
        while(it.hasNext()) {
            it.next();
            const int& line = it.key();
            const int& count = it.value();
            cf->setCallCount(line, count + curcc[line]);
        }
    }
}

#include "annotationmanager.moc"
