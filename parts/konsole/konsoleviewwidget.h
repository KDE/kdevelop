#ifndef __KONSOLEVIEWWIDGET_H__
#define __KONSOLEVIEWWIDGET_H__


#include <qwidget.h>
#include <qstring.h>


class KDevProject;
class KonsoleWidgetPrivate;
class KonsoleViewPart;
namespace KParts {
  class ReadOnlyPart;
  class Part;
}
class QVBoxLayout;


class KonsoleViewWidget : public QWidget
{
    Q_OBJECT
public:
    KonsoleViewWidget(KonsoleViewPart *part);
    virtual ~KonsoleViewWidget();

public slots:
    void setDirectory(const KURL &dirUrl);

private slots:
    void activePartChanged(KParts::Part *activatedPart);
    void partDestroyed();

private:
    void activate();

    KParts::ReadOnlyPart *part;
    QVBoxLayout *vbox;
    KonsoleViewPart *owner;
};


#endif
