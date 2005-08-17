/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "digraphview.h"

#include <math.h>
#include <stdlib.h>
#include <qapplication.h>
#include <qpainter.h>
#include <q3paintdevicemetrics.h>
#include <qtextstream.h>
//Added by qt3to4:
#include <Q3PointArray>
#include <Q3MemArray>
#include <QMouseEvent>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <kstandarddirs.h>
#include <kglobalsettings.h>
#include <ktempfile.h>
#include <kdeversion.h>

struct DigraphNode
{
    int x;
    int y;
    int w;
    int h;
    QString name;
};


struct DigraphEdge
{
    Q3PointArray points;
};


DigraphView::DigraphView(QWidget *parent, const char *name)
    : Q3ScrollView(parent, name, Qt::WNoAutoErase|Qt::WStaticContents|Qt::WResizeNoErase)
{
    QPalette p;
    viewport()->setBackgroundRole(QPalette::Base);

    Q3PaintDeviceMetrics m(this);
    xscale = m.logicalDpiX();
    yscale = m.logicalDpiY();

    width = -1;
    height = -1;

    nodes.setAutoDelete(true);
    edges.setAutoDelete(true);
    selNode = 0;
}


DigraphView::~DigraphView()
{
}


int DigraphView::toXPixel(double x)
{
    return (int) (x*xscale);
}


int DigraphView::toYPixel(double y)
{
    return height - (int) (y*yscale);
}


void DigraphView::setRenderedExtent(double w, double h)
{
    width = (int) (w*xscale);
    height = (int) (h*yscale);
    resizeContents(width+1, height+1);
}


void DigraphView::addRenderedNode(const QString &name,
                                  double x, double y, double w, double h)
{
    DigraphNode *node = new DigraphNode;
    node->x = toXPixel(x);
    node->y = toYPixel(y);
    node->w = (int) (w*xscale);
    node->h = (int) (h*yscale);
    node->name = name;
    nodes.append(node);
}


void DigraphView::addRenderedEdge(const QString &/*name1*/, const QString &/*name2*/,
                                  Q3MemArray<double> coords)
{
    if (coords.count() < 4)
        return;

    DigraphEdge *edge = new DigraphEdge;
    edge->points.resize(coords.count()/2);

    for (int i = 0; i < edge->points.count(); ++i)
        edge->points[i] = QPoint(toXPixel(coords[2*i]), toYPixel(coords[2*i+1]));

    edges.append(edge);
}


void DigraphView::addEdge(const QString &name1, const QString &name2)
{
    QString line;
    line = "\"";
    line += name1;
    line += "\" -> \"";
    line += name2;
    line += "\";";
    inputs.append(line);
}


void DigraphView::clear()
{
    nodes.clear();
    edges.clear();
    selNode = 0;
    viewport()->update();
}


void DigraphView::setSelected(const QString &name)
{
    Q3PtrListIterator<DigraphNode> it(nodes);
    for (; it.current(); ++it) {
        if (it.current()->name == name) {
            updateContents(selNode->x-selNode->w/2, selNode->y-selNode->h/2,
                           selNode->w, selNode->h);
            selNode = it.current();
            updateContents(selNode->x-selNode->w/2, selNode->y-selNode->h/2,
                           selNode->w, selNode->h);
            return;
        }
    }
}


void DigraphView::ensureVisible(const QString &name)
{
    Q3PtrListIterator<DigraphNode> it(nodes);
    for (; it.current(); ++it) {
        if (it.current()->name == name) {
            Q3ScrollView::ensureVisible((*it)->x, (*it)->y, (*it)->w, (*it)->h);
            return;
        }
    }
}


QStringList DigraphView::splitLine(QString str)
{
    QStringList result;

    while (!str.isEmpty()) {
        if (str[0] == '"') {
            int pos = str.find('"', 1);
            if (pos == -1)
                pos = str.length();
            result << str.mid(1, pos-1);
            str.remove(0, pos+1);
        } else {
            int pos = str.find(' ');
            if (pos == -1)
                pos = str.length();
            result << str.left(pos);
            str.remove(0, pos+1);
        }
        int i = 0; while (i<str.length() && str[i] == QLatin1Char(' ')) ++i;
        str.remove(0, i);
    }

    return result;
}


void DigraphView::parseDotResults(const QStringList &list)
{
    QStringList::ConstIterator it;
    for (it = list.begin(); it != list.end(); ++it) {
        QStringList tokens = splitLine(*it);
        if (tokens.count() == 0)
            continue;
        if (tokens[0] == "graph") {
            if (tokens.count() < 4)
                continue;
            setRenderedExtent(tokens[2].toDouble(), tokens[3].toDouble());
        } else if (tokens[0] == "node") {
            if (tokens.count() < 6)
                continue;
            addRenderedNode(tokens[1], tokens[2].toDouble(), tokens[3].toDouble(),
                            tokens[4].toDouble(), tokens[5].toDouble());
        } else if (tokens[0] == "edge") {
            if (tokens.count() < 8)
                continue;
            Q3MemArray<double> coords(tokens.count()-6);
            for (int i=0; i != tokens.count()-6; ++i)
                coords[i] = tokens[i+4].toDouble();
            addRenderedEdge(tokens[1], tokens[2], coords);
        }
    }
}


void DigraphView::process()
{
    QString cmd = KGlobal::dirs()->findExe("dot");
    if (cmd.isEmpty()) {
        KMessageBox::sorry(0, i18n("You do not have 'dot' installed.\nIt can be downloaded from www.graphviz.org."));
        return;
    }

    QStringList results;

    KTempFile ifile, ofile;
    QTextStream &is = *ifile.textStream();
    is << "digraph G {" << endl;
    is << "rankdir=LR;" << endl;
    is << "node [shape=box,fontname=Helvetica,fontsize=12];" << endl;
    QStringList::Iterator it;
    for (it = inputs.begin(); it != inputs.end(); ++it)
        is << (*it) << endl;
    is << "}" << endl;
    ifile.close();

    KProcess proc;
    proc << cmd << "-Tplain" << ifile.name() << "-o" << ofile.name();
    proc.start(KProcess::Block);

    QTextStream &os = *ofile.textStream();
    while (!os.atEnd())
        results << os.readLine();
    ofile.close();

    parseDotResults(results);
    inputs.clear();

    if (nodes.first())
        selNode = nodes.first();
    viewport()->update();
}


void DigraphView::drawContents(QPainter* p, int clipx, int clipy, int clipw, int cliph)
{
    QRect clipRect(clipx, clipy, clipw, cliph);
    p->eraseRect(clipRect);

    p->setFont(KGlobalSettings::generalFont());
    Q3PtrListIterator<DigraphNode> it1(nodes);
    for (; it1.current(); ++it1) {
        QRect r((*it1)->x-(*it1)->w/2, (*it1)->y-(*it1)->h/2, (*it1)->w, (*it1)->h);
        if (r.intersects(clipRect)) {
            if (it1.current() == selNode)
                p->fillRect(r, QBrush(Qt::lightGray, Qt::SolidPattern));
            else
                p->drawRect(r);
            p->drawText(r, Qt::AlignCenter, (*it1)->name);
        }
    }
    p->setBrush(QBrush(Qt::black, Qt::SolidPattern));
    Q3PtrListIterator<DigraphEdge> it2(edges);
    for (; it2.current(); ++it2) {
        int n = (*it2)->points.count();
        for (int i=0; i+3 < n; i+=3)
            {
                Q3PointArray a(4);
                Q3PointArray &b = (*it2)->points;
                for (int j=0; j<4; ++j)
                    a.setPoint(j, b.point(i+j));
                if (a.boundingRect().intersects(clipRect))
                    p->drawCubicBezier((*it2)->points, i);
            }
        QPoint p1 = (*it2)->points[n-2];
        QPoint p2 = (*it2)->points[n-1];
        QPoint d = p1-p2;
        double l = sqrt(d.x()*d.x()+d.y()*d.y());
        double d11 = (10.0)/l*d.x();
        double d12 = (10.0)/l*d.y();
        double d21 = -(3.0/l)*d.y();
        double d22 = (3.0/l)*d.x();
        Q3PointArray triangle(3);
        triangle[0] = p2 + QPoint((int)(d11+d21),(int)(d12+d22));
        triangle[1] = p2 + QPoint((int)(d11-d21),(int)(d12-d22));
        triangle[2] = p2;
        p->drawPolygon(triangle, true);
    }
}


void DigraphView::contentsMousePressEvent(QMouseEvent *e)
{
    Q3PtrListIterator<DigraphNode> it1(nodes);
    for (; it1.current(); ++it1) {
        QRect r((*it1)->x-(*it1)->w/2, (*it1)->y-(*it1)->h/2, (*it1)->w, (*it1)->h);
        if (r.contains(e->pos())) {
            if (selNode) {
                QRect oldr(selNode->x-selNode->w/2, selNode->y-selNode->h/2,
                           selNode->w, selNode->h);
                updateContents(oldr);
            }
            selNode = it1.current();
            emit selected(selNode->name);
            updateContents(r);
        }

    }
}


QSize DigraphView::sizeHint() const
{
    if (width == -1)
        return QSize(100, 100); // arbitrary

#if defined(KDE_IS_VERSION)
#if (KDE_IS_VERSION(3,1,90))
    QSize dsize = KGlobalSettings::desktopGeometry(viewport()).size();
#else
    QSize dsize = QApplication::desktop()->size();
#endif
#else
    QSize dsize = QApplication::desktop()->size();
#endif
    return QSize(width, height).boundedTo(QSize(dsize.width()*2/3, dsize.height()*2/3));
}


#if 0
int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    DigraphView *dw = new DigraphView(0, "dot widget");
dw->addEdge(	"5th Edition", "6th Edition");
dw->addEdge(	"5th Edition", "PWB 1.0");
dw->addEdge(	"6th Edition", "LSX");
dw->addEdge(	"6th Edition", "1 BSD");
dw->addEdge(	"6th Edition", "Mini Unix");
dw->addEdge(	"6th Edition", "Wollongong");
dw->addEdge(	"6th Edition", "Interdata");
dw->addEdge(	"Interdata", "Unix/TS 3.0");
dw->addEdge(	"Interdata", "PWB 2.0");
dw->addEdge(	"Interdata", "7th Edition");
dw->addEdge(	"7th Edition", "8th Edition");
dw->addEdge(	"7th Edition", "32V");
dw->addEdge(	"7th Edition", "V7M");
dw->addEdge(	"7th Edition", "Ultrix-11");
dw->addEdge(	"7th Edition", "Xenix");
dw->addEdge(	"7th Edition", "UniPlus+");
dw->addEdge(	"V7M", "Ultrix-11");
dw->addEdge(	"8th Edition", "9th Edition");
dw->addEdge(	"1 BSD", "2 BSD");
dw->addEdge(	"2 BSD", "2.8 BSD");
dw->addEdge(	"2.8 BSD", "Ultrix-11");
dw->addEdge(	"2.8 BSD", "2.9 BSD");
dw->addEdge(	"32V", "3 BSD");
dw->addEdge(	"3 BSD", "4 BSD");
dw->addEdge(	"4 BSD", "4.1 BSD");
dw->addEdge(	"4.1 BSD", "4.2 BSD");
dw->addEdge(	"4.1 BSD", "2.8 BSD");
dw->addEdge(	"4.1 BSD", "8th Edition");
dw->addEdge(	"4.2 BSD", "4.3 BSD");
dw->addEdge(	"4.2 BSD", "Ultrix-32");
dw->addEdge(	"PWB 1.0", "PWB 1.2");
dw->addEdge(	"PWB 1.0", "USG 1.0");
dw->addEdge(	"PWB 1.2", "PWB 2.0");
dw->addEdge(	"USG 1.0", "CB Unix 1");
dw->addEdge(	"USG 1.0", "USG 2.0");
dw->addEdge(	"CB Unix 1", "CB Unix 2");
dw->addEdge(	"CB Unix 2", "CB Unix 3");
dw->addEdge(	"CB Unix 3", "Unix/TS++");
dw->addEdge(	"CB Unix 3", "PDP-11 Sys V");
dw->addEdge(	"USG 2.0", "USG 3.0");
dw->addEdge(	"USG 3.0", "Unix/TS 3.0");
dw->addEdge(	"PWB 2.0", "Unix/TS 3.0");
dw->addEdge(	"Unix/TS 1.0", "Unix/TS 3.0");
dw->addEdge(	"Unix/TS 3.0", "TS 4.0");
dw->addEdge(	"Unix/TS++", "TS 4.0");
dw->addEdge(	"CB Unix 3", "TS 4.0");
dw->addEdge(	"TS 4.0", "System V.0");
dw->addEdge(	"System V.0", "System V.2");
dw->addEdge(	"System V.2", "System V.3");
    dw->process();
    dw->show();

    return app.exec();
}
#endif

#include "digraphview.moc"
