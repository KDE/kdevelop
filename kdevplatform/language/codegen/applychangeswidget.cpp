/*
    SPDX-FileCopyrightText: 2008 Aleix Pol <aleixpol@gmail.com>
    SPDX-FileCopyrightText: 2009 Ramón Zarazúa <killerfox512+kde@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "applychangeswidget.h"

#include <KTextEditor/Document>

#include <KParts/PartLoader>
#include <KParts/ReadWritePart>

#include <QAction>
#include <QDialogButtonBox>
#include <QDir>
#include <QLabel>
#include <QMimeType>
#include <QMimeDatabase>
#include <QPushButton>
#include <QSplitter>
#include <QTemporaryFile>
#include <QTabWidget>
#include <QVBoxLayout>

#include "coderepresentation.h"
#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>

namespace KDevelop {
class ApplyChangesWidgetPrivate
{
public:

    explicit ApplyChangesWidgetPrivate(ApplyChangesWidget* p)
        : parent(p)
        , m_index(0) {}
    ~ApplyChangesWidgetPrivate()
    {
        qDeleteAll(m_temps);
    }

    void createEditPart(const KDevelop::IndexedString& url);

    ApplyChangesWidget* const parent;
    int m_index;
    QList<KParts::ReadWritePart*> m_editParts;
    QList<QTemporaryFile*> m_temps;
    QList<IndexedString> m_files;
    QTabWidget* m_documentTabs;
    QLabel* m_info;
};

ApplyChangesWidget::ApplyChangesWidget(QWidget* parent)
    : QDialog(parent)
    , d_ptr(new ApplyChangesWidgetPrivate(this))
{
    Q_D(ApplyChangesWidget);

    setSizeGripEnabled(true);

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    auto mainLayout = new QVBoxLayout(this);
    auto okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Return));
    connect(buttonBox, &QDialogButtonBox::accepted, this, &ApplyChangesWidget::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ApplyChangesWidget::reject);

    auto* w = new QWidget(this);
    d->m_info = new QLabel(w);
    d->m_documentTabs = new QTabWidget(w);
    connect(d->m_documentTabs, &QTabWidget::currentChanged,
            this, &ApplyChangesWidget::indexChanged);

    auto* l = new QVBoxLayout(w);
    l->addWidget(d->m_info);
    l->addWidget(d->m_documentTabs);

    mainLayout->addWidget(w);
    mainLayout->addWidget(buttonBox);

    resize(QSize(800, 400));
}

ApplyChangesWidget::~ApplyChangesWidget() = default;

bool ApplyChangesWidget::hasDocuments() const
{
    Q_D(const ApplyChangesWidget);

    return d->m_editParts.size() > 0;
}

KTextEditor::Document* ApplyChangesWidget::document() const
{
    Q_D(const ApplyChangesWidget);

    return qobject_cast<KTextEditor::Document*>(d->m_editParts.value(d->m_index));
}

void ApplyChangesWidget::setInformation(const QString& info)
{
    Q_D(ApplyChangesWidget);

    d->m_info->setText(info);
}

void ApplyChangesWidget::addDocuments(const IndexedString& original)
{
    Q_D(ApplyChangesWidget);

    int idx = d->m_files.indexOf(original);
    if (idx < 0) {
        auto* w = new QWidget;
        d->m_documentTabs->addTab(w, original.str());
        d->m_documentTabs->setCurrentWidget(w);

        d->m_files.insert(d->m_index, original);
        d->createEditPart(original);
    } else {
        d->m_index = idx;
    }
}

bool ApplyChangesWidget::applyAllChanges()
{
    Q_D(ApplyChangesWidget);

    /// @todo implement safeguard in case a file saving fails

    bool ret = true;
    for (int i = 0; i < d->m_files.size(); ++i)
        if (d->m_editParts[i]->saveAs(d->m_files[i].toUrl())) {
            IDocument* doc = ICore::self()->documentController()->documentForUrl(d->m_files[i].toUrl());
            if (doc && doc->state() == IDocument::Dirty)
                doc->reload();
        } else
            ret = false;

    return ret;
}
}

namespace KDevelop {
void ApplyChangesWidgetPrivate::createEditPart(const IndexedString& file)
{
    QWidget* widget = m_documentTabs->currentWidget();
    Q_ASSERT(widget);

    auto* m = new QVBoxLayout(widget);
    auto* v = new QSplitter(widget);
    m->addWidget(v);

    QUrl url = file.toUrl();

    QMimeType mimetype = QMimeDatabase().mimeTypeForUrl(url);

    auto result = KParts::PartLoader::instantiatePartForMimeType<KTextEditor::Document>(mimetype.name(), widget, widget);
    if (!result) {
        qWarning() << "Failed to load part for mimetype" << mimetype;
    }
    auto* document = result.plugin;
    Q_ASSERT(document);

    Q_ASSERT(document->action("file_save"));
    document->action("file_save")->setEnabled(false);

    m_editParts.insert(m_index, document);

    //Open the best code representation, even if it is artificial
    CodeRepresentation::Ptr repr = createCodeRepresentation(file);
    if (!repr->fileExists()) {
        const QString templateName = QDir::tempPath() + QLatin1Char('/') +
                                     url.fileName().split(QLatin1Char('.')).last();
        auto* temp(new QTemporaryFile(templateName));
        temp->open();
        temp->write(repr->text().toUtf8());
        temp->close();

        url = QUrl::fromLocalFile(temp->fileName());

        m_temps << temp;
    }
    m_editParts[m_index]->openUrl(url);

    v->addWidget(m_editParts[m_index]->widget());
    v->setSizes(QList<int> {400, 100});
}

void ApplyChangesWidget::indexChanged(int newIndex)
{
    Q_D(ApplyChangesWidget);

    Q_ASSERT(newIndex != -1);
    d->m_index = newIndex;
}

void ApplyChangesWidget::updateDiffView(int index)
{
    Q_D(ApplyChangesWidget);

    d->m_index = index == -1 ? d->m_index : index;
}
}

#include "moc_applychangeswidget.cpp"
