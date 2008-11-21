/***************************************************************************
 *   Copyright 2008 Aleix Pol <aleixpol@gmail.com>                         *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "projectitemlineedit.h"

#include <QApplication>
#include <KDebug>
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>

ProjectItemLineEdit::ProjectItemLineEdit(QWidget* parent)
    : KLineEdit(parent)
{
    connect(this, SIGNAL(textChanged(QString)), this, SLOT(updated(QString)));
    connect(this, SIGNAL(correctnessChanged(bool)), this, SLOT(correctnessChange(bool)));
}

QModelIndex pathToIndex(const QAbstractItemModel* model, const QStringList& tofetch)
{
    if(tofetch.isEmpty())
        return QModelIndex();
    
    QModelIndex current=model->index(0,0, QModelIndex());
    
    QModelIndex ret;
    foreach(const QString& currentName, tofetch)
    {
        QModelIndexList l = model->match(current, Qt::EditRole, currentName, 1, Qt::MatchExactly);
        if(l.count()>0) {
            ret=l.first();
            current = model->index(0,0, ret);
        } else {
            ret = QModelIndex();
            break;
        }
    }
//     Q_ASSERT(model->data(ret).toString()==tofetch.last());
    return ret;
}


void ProjectItemLineEdit::updated(const QString& newText)
{
    QStringList tofetch=completer()->splitPath(newText);
    const QAbstractItemModel* model=completer()->model();
    QModelIndex idx=pathToIndex(model, tofetch);
    emit correctnessChanged(idx.isValid());
}

void ProjectItemLineEdit::correctnessChange(bool correct)
{
    QColor textColor;
    if(correct)
        textColor=qApp->palette().color(QPalette::Active, QPalette::Text);
    else
        textColor=Qt::red;
    
    QPalette p = this->palette();
    p.setColor(QPalette::Active, QPalette::Text, textColor);
    setPalette(p);
}

ProjectItemCompleter::ProjectItemCompleter(QAbstractItemModel* model, QObject* parent)
    : QCompleter(model, parent), mModel(model), sep("/")
{}

QString ProjectItemCompleter::pathFromIndex(const QModelIndex& index) const
{
    if (!index.isValid())
        return QString();

    QModelIndex idx = index;
    QStringList list;
    do {
        QString t = mModel->data(idx, Qt::EditRole).toString();
        list.prepend(t);
        QModelIndex parent = idx.parent();
        idx = parent.sibling(parent.row(), index.column());
    } while (idx.isValid());

    return list.join(sep);
}

#include "projectitemlineedit.moc"
