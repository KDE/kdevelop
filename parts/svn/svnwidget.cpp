
#include <qpainter.h>
#include <qregexp.h>

#include "kdevpartcontroller.h"
#include "kdevcore.h"
#include "svnwidget.h"
#include "svnwidget.moc"
#include "svn_part.h"

class SvnListBoxItem : public ProcessListBoxItem {
public:
    SvnListBoxItem(const QString &s1);
    QString fileName() {return filename;}
    bool containsFileName() {return !filename.isEmpty();}
    virtual bool isCustomItem();

private:
    virtual void paint(QPainter *p);
    QString str1;
    QString filename;
};


SvnListBoxItem::SvnListBoxItem(const QString &s1)
        : ProcessListBoxItem(s1, Normal) {
    str1 = s1;
    QRegExp re("[ACMPRU?] (.*)");
    if (re.exactMatch(s1)) filename = re.cap(1);
}


bool SvnListBoxItem::isCustomItem() {
    return true;
}


void SvnListBoxItem::paint(QPainter *p) {
    QFontMetrics fm = p->fontMetrics();
    int y = fm.ascent()+fm.leading()/2;
    int x = 3;

    p->setPen(Qt::darkGreen);
    p->drawText(x, y, str1);
}

SvnWidget::SvnWidget(SvnPart *part)
        : ProcessWidget(0, "svn widget") {
    connect( this, SIGNAL(highlighted(int)),
             this, SLOT(lineHighlighted(int)) );

    m_part = part;
}

SvnWidget::~SvnWidget() {}


void SvnWidget::lineHighlighted(int line) {
    ProcessListBoxItem *i = static_cast<ProcessListBoxItem*>(item(line));
    if (i->isCustomItem()) {
        SvnListBoxItem *ci = static_cast<SvnListBoxItem*>(i);
	if (ci->containsFileName()) {
	    m_part->partController()->editDocument(dir + "/" + ci->fileName());
	    m_part->mainWindow()->lowerView(this);
	}
    }
}

void SvnWidget::insertStdoutLine(const QString & line) {
    insertItem(new SvnListBoxItem(line));
}
