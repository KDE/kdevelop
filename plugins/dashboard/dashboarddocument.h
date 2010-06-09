#ifndef DASHBOARDDOCUMENT_H
#define DASHBOARDDOCUMENT_H

#include <interfaces/idocument.h>
#include <sublime/urldocument.h>

namespace KDevelop {
    class IProject;
}

class DashboardDocument : public KDevelop::IDocument, public Sublime::UrlDocument
{
    public:
        DashboardDocument(KDevelop::IProject* project);
        
        virtual void activate(Sublime::View* activeView, KParts::MainWindow* mainWindow);
        virtual void setTextSelection(const KTextEditor::Range& range);
        virtual void setCursorPosition(const KTextEditor::Cursor& cursor);
        virtual KTextEditor::Cursor cursorPosition() const;
        virtual KDevelop::IDocument::DocumentState state() const;
        virtual bool isActive() const;
        virtual bool close(KDevelop::IDocument::DocumentSaveMode mode = Default);
        virtual void reload();
        virtual bool save(KDevelop::IDocument::DocumentSaveMode mode = Default);
        virtual KTextEditor::Document* textDocument() const;
        virtual KParts::Part* partForView(QWidget* view) const;
        virtual KMimeType::Ptr mimeType() const;
        virtual KUrl url() const;
        
    protected:
        virtual Sublime::View* newView(Document* doc);
        
    private:
        KDevelop::IProject* m_project;
};

#endif // DASHBOARDDOCUMENT_H
