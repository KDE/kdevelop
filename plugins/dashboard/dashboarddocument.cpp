#include "dashboarddocument.h"
#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <interfaces/iuicontroller.h>
#include <QLabel>
#include <sublime/view.h>

using namespace KDevelop;

DashboardDocument::DashboardDocument(KDevelop::IProject* project)
    : IDocument(ICore::self()), Sublime::UrlDocument(ICore::self()->uiController()->controller(), project->projectFileUrl()), m_project(project)
{}

void DashboardDocument::activate(Sublime::View* activeView, KParts::MainWindow* mainWindow)
{}

void DashboardDocument::setTextSelection(const KTextEditor::Range& range) {}
void DashboardDocument::setCursorPosition(const KTextEditor::Cursor& cursor) {}

KTextEditor::Cursor DashboardDocument::cursorPosition() const
{
    return KTextEditor::Cursor();
}

IDocument::DocumentState DashboardDocument::state() const
{
    return IDocument::DocumentState::Clean;
}

bool DashboardDocument::isActive() const
{
    return true;
}

bool DashboardDocument::close(KDevelop::IDocument::DocumentSaveMode mode)
{
    return true;
}

void DashboardDocument::reload()
{}

bool DashboardDocument::save(KDevelop::IDocument::DocumentSaveMode mode)
{
    return true;
}

KTextEditor::Document* DashboardDocument::textDocument() const
{
    return 0;
}

KParts::Part* DashboardDocument::partForView(QWidget* view) const
{
    return 0;
}

KMimeType::Ptr DashboardDocument::mimeType() const
{
    return KMimeType::mimeType("text/x-kdevelop");
}

class DashboardView : public Sublime::View
{
    public:
        DashboardView(Sublime::Document* doc, WidgetOwnership ws = DoNotTakeOwnerShip)
            : View(doc, ws) {}
            
        virtual QWidget* createWidget(QWidget* parent)
        {
            QLabel* label=new QLabel("hello world", parent);
            label->setPixmap(QPixmap("/home/kde-devel/imatges/kamoso_09462009_174659.png"));
            return label;
        }
};

Sublime::View* DashboardDocument::newView(Sublime::Document* doc)
{
    if( dynamic_cast<DashboardDocument*>( doc ) ) {
        return new DashboardView(doc);
    }
    return 0;
}

KUrl DashboardDocument::url() const
{
    return m_project->projectFileUrl();
}
