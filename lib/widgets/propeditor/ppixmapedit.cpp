/***************************************************************************
 *   Copyright (C) 2003 Cedric Pasteur                                     *
 *   <cedric.pasteur@free.fr>                                              *
 *   Copyright (C) 2004 by Alexander Dymo                                  *
 *   cloudtemple@mskat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "ppixmapedit.h"

#include <qlayout.h>
#include <qpainter.h>
#include <qlabel.h>
#include <qcursor.h>

#ifndef PURE_QT
#include <klocale.h>
#else
#include "compat_tools.h"
#endif

#ifndef PURE_QT
#include <kfiledialog.h>
#else
#include <qfiledialog.h>
#endif
#include <qpushbutton.h>

namespace PropertyLib{

PPixmapEdit::PPixmapEdit(MultiProperty* property, QWidget* parent, const char* name)
    :PropertyWidget(property, parent, name)
{
    QHBoxLayout *l = new QHBoxLayout(this, 0, 0);
    m_edit = new QLabel(this);
    m_edit->setAlignment(Qt::AlignTop);
    m_edit->resize(width(), height()-1);
    m_edit->setBackgroundMode(Qt::PaletteBase);
    m_edit->installEventFilter(this);
    
    m_button = new QPushButton(i18n("..."), this);
    m_button->resize(height(), height()-8);
    m_button->move(width() - m_button->width() -1, 0);
    m_button->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    l->addWidget(m_edit);
    l->addWidget(m_button);
    m_popup = new QLabel(0, 0, Qt::WStyle_NoBorder|Qt::WX11BypassWM|WStyle_StaysOnTop);
    m_popup->hide();
    

    connect(m_button, SIGNAL(clicked()), this, SLOT(updateProperty()));
}

QVariant PPixmapEdit::value() const
{
    return QVariant(*(m_edit->pixmap()));
}

void PPixmapEdit::drawViewer(QPainter* p, const QColorGroup& cg, const QRect& r, const QVariant& value)
{
    p->setPen(Qt::NoPen);
    p->setBrush(cg.background());
    p->drawRect(r);
    p->drawPixmap(r.topLeft().x(), r.topLeft().y(), value.toPixmap());    
}

void PPixmapEdit::setValue(const QVariant& value, bool emitChange)
{
    m_edit->setPixmap(value.toPixmap());
    if (emitChange)
        emit propertyChanged(m_property, value);
}

void PPixmapEdit::updateProperty()
{
#ifndef PURE_QT
    KURL url = KFileDialog::getImageOpenURL(QString::null, this);
    if (!url.isEmpty())
    {
        m_edit->setPixmap(QPixmap(url.path()));
        emit propertyChanged(m_property, value());
    }
#else
    QString url = QFileDialog::getOpenFileName();
    if (!url.isEmpty())
    {
        m_edit->setPixmap(QPixmap(url));
        emit propertyChanged(m_property, value());
    }
#endif
}

void PPixmapEdit::resizeEvent(QResizeEvent *ev)
{
    m_edit->resize(ev->size().width(), ev->size().height()-1);
    m_button->move(ev->size().width() - m_button->width(), 0);
    m_edit->setMaximumHeight(m_button->height());
}

bool PPixmapEdit::eventFilter(QObject *o, QEvent *ev)
{
    if(o == m_edit)
    {
        if(ev->type() == QEvent::MouseButtonPress)
        {
            if(m_edit->pixmap()->size().height() < height()-2
                    && m_edit->pixmap()->size().width() < width()-20)
                    return false;
            m_popup->setPixmap(*(m_edit->pixmap()));
            m_popup->resize(m_edit->pixmap()->size());
            m_popup->move(QCursor::pos());
            m_popup->show();
        }
        if(ev->type() == QEvent::MouseButtonRelease)
        {
            if(m_popup->isVisible())
                    m_popup->hide();
        }
        if(ev->type() == QEvent::KeyPress)
        {
            QKeyEvent* e = static_cast<QKeyEvent*>(ev);
            if((e->key() == Key_Enter) || (e->key()== Key_Space) || (e->key() == Key_Return))
            {
                    m_button->animateClick();
                    return true;
            }
        }
    }
    return PropertyWidget::eventFilter(o, ev);
}

}

#ifndef PURE_QT
#include "ppixmapedit.moc"
#endif
