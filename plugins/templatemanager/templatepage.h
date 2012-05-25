#ifndef TEMPLATEPAGE_H
#define TEMPLATEPAGE_H

#include <QWidget>

namespace KDevelop
{
class ITemplateProvider;
}

namespace Ui
{
    class TemplatePage;
}

class TemplatePage : public QWidget
{
    Q_OBJECT
public:
    explicit TemplatePage (KDevelop::ITemplateProvider* provider, QWidget* parent = 0);
    virtual ~TemplatePage();
    
private slots:
    void loadFromFile();
    void getMoreTemplates();
    void shareTemplates();
    
private:
    KDevelop::ITemplateProvider* m_provider;
    Ui::TemplatePage* ui;
};

#endif // TEMPLATEPAGE_H
