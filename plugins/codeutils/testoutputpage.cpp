/*
 *
 */

#include "testoutputpage.h"

#include <language/codegen/sourcefiletemplate.h>

#include <KUrl>
#include <QHash>
#include <QFormLayout>
#include <QLabel>

TestOutputPage::TestOutputPage(QWidget* parent)
 : QWidget(parent)
{
    m_layout = new QFormLayout(this);
    setLayout(m_layout);
}

TestOutputPage::~TestOutputPage()
{

}

void TestOutputPage::setFileUrls (const TestOutputPage::UrlHash& urls, const QHash<QString,QString>& labels)
{
    for (UrlHash::const_iterator it = urls.constBegin(); it != urls.constEnd(); ++it)
    {
        KUrlRequester* r = new KUrlRequester(it.value(), this);
        m_layout->addRow(new QLabel(labels[it.key()]), r);
        m_requesters.insert(it.key(), r);
    }
}

TestOutputPage::UrlHash TestOutputPage::fileUrls() const
{
    UrlHash urls;
    QHash<QString,KUrlRequester*>::const_iterator it;
    for (it = m_requesters.constBegin(); it != m_requesters.constEnd(); ++it)
    {
        urls.insert(it.key(), it.value()->url());
    }
    return urls;
}