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

#include <QCompleter>
#include <KDebug>
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <project/projectmodel.h>
#include <util/kdevstringhandler.h>
#include <kcolorscheme.h>

class ProjectItemCompleter : public QCompleter
{
    Q_OBJECT
    public:
        ProjectItemCompleter(QObject* parent=0);
        
        QString separator() const { return sep; }
        QStringList splitPath(const QString &path) const;
        QString pathFromIndex(const QModelIndex& index) const;
        
    private:
        KDevelop::ProjectModel* mModel;
        QChar sep;
};

ProjectItemCompleter::ProjectItemCompleter(QObject* parent)
: QCompleter(parent), mModel(KDevelop::ICore::self()->projectController()->projectModel()), sep('/')
{
    setModel(mModel);
}


QStringList ProjectItemCompleter::splitPath(const QString& path) const
{
    return KDevelop::splitWithEscaping( path, sep, '\\' ); 
}

QString ProjectItemCompleter::pathFromIndex(const QModelIndex& index) const
{
    QString postfix;
    if(mModel->item(index)->folder())
        postfix=sep;
    
    return KDevelop::joinWithEscaping(mModel->pathFromIndex(index), sep, '\\')+postfix;
}

//TODO: use a proper QValidator for the validation instead of doing it manually.
ProjectItemLineEdit::ProjectItemLineEdit(QWidget* parent)
    : KLineEdit(parent)
{
    connect(this, SIGNAL(textChanged(QString)), this, SLOT(updated(QString)));
    connect(this, SIGNAL(correctnessChanged(bool)), this, SLOT(correctnessChange(bool)));
    setCompleter( new ProjectItemCompleter( this ) );
}

void ProjectItemLineEdit::updated(const QString& newText)
{
    QStringList tofetch=completer()->splitPath(newText);
    const KDevelop::ProjectModel* model=static_cast<const KDevelop::ProjectModel* >(completer()->model());
    QModelIndex idx=model->pathToIndex(tofetch);
    emit correctnessChanged(idx.isValid());
}

void ProjectItemLineEdit::correctnessChange(bool correct)
{
    KStatefulBrush brush;
    if(correct) {
        brush=KStatefulBrush( KColorScheme::View, KColorScheme::PositiveText );
    } else {
        brush=KStatefulBrush( KColorScheme::View, KColorScheme::NegativeText );
    }
    
    QPalette pal = palette();
    pal.setBrush( QPalette::Text, brush.brush( this ) );
    setPalette( pal );
}

#include "projectitemlineedit.moc"
#include "moc_projectitemlineedit.cpp"
