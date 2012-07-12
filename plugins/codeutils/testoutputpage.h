/*
 *
 */

#ifndef TESTNAMEPAGE_H
#define TESTNAMEPAGE_H

#include <QWidget>
#include <QMetaType>
#include <QHash>
#include <KUrl>
#include <KUrlRequester>

class QFormLayout;

namespace Ui
{
class TestOutputPage;
}

class TestOutputPage : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(UrlHash fileUrls READ fileUrls)

public:
    typedef QHash<QString, KUrl> UrlHash;
    
    TestOutputPage (QWidget* parent);
    virtual ~TestOutputPage();

    UrlHash fileUrls() const;
    void setFileUrls(const TestOutputPage::UrlHash& urls, const QHash< QString, QString >& labels);

private:
    QFormLayout* m_layout;
    QHash<QString,KUrlRequester*> m_requesters;
};

Q_DECLARE_METATYPE(TestOutputPage::UrlHash)

#endif // TESTNAMEPAGE_H
