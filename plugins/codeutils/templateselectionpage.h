/*
 *
 */

#ifndef TEMPLATESELECTIONPAGE_H
#define TEMPLATESELECTIONPAGE_H

#include <QWidget>
#include <KUrl>

class QModelIndex;
namespace KDevelop
{
class TemplatesModel;
}

namespace Ui
{
class TemplateSelectionPage;
}

class TemplateSelectionPage : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString selectedTemplate READ selectedTemplate)
    Q_PROPERTY(QString identifier READ identifier)
    Q_PROPERTY(KUrl baseUrl READ baseUrl WRITE setBaseUrl)

public:
    TemplateSelectionPage(QWidget* parent = 0);
    virtual ~TemplateSelectionPage();

    QString selectedTemplate() const;

    QString identifier() const;

    KUrl baseUrl() const;
    void setBaseUrl(const KUrl& url);

private:
    Ui::TemplateSelectionPage* ui;
    KDevelop::TemplatesModel* m_model;

signals:
    void templateValid(bool valid);

public slots:
    void contentsChanged();
};

#endif // TEMPLATESELECTIONPAGE_H
