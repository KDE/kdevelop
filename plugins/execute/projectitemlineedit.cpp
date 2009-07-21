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
#include <QValidator>

static const QChar sep = '/';
static const QChar escape = '\\';

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
};

class ProjectItemValidator : public QValidator
{
    Q_OBJECT
public:
    ProjectItemValidator(QObject* parent = 0 );
    QValidator::State validate( QString& input, int& pos ) const;
};

ProjectItemCompleter::ProjectItemCompleter(QObject* parent)
: QCompleter(parent), mModel(KDevelop::ICore::self()->projectController()->projectModel())
{
    setModel(mModel);
}


QStringList ProjectItemCompleter::splitPath(const QString& path) const
{
    return KDevelop::splitWithEscaping( path, sep, escape ); 
}

QString ProjectItemCompleter::pathFromIndex(const QModelIndex& index) const
{
    QString postfix;
    if(mModel->item(index)->folder())
        postfix=sep;
    
    return KDevelop::joinWithEscaping(mModel->pathFromIndex(index), sep, escape)+postfix;
}


ProjectItemValidator::ProjectItemValidator(QObject* parent): QValidator(parent)
{
}


QValidator::State ProjectItemValidator::validate(QString& input, int& pos) const
{
    QStringList path = KDevelop::splitWithEscaping( input, sep, escape );
    KDevelop::ProjectModel* model = KDevelop::ICore::self()->projectController()->projectModel();
    QModelIndex idx = model->pathToIndex( path );
    QValidator::State state = QValidator::Invalid;
    if( idx.isValid() ) {
        state = QValidator::Acceptable;
    } else {
        path.takeLast();
        idx = model->pathToIndex( path );
        if( idx.isValid() ) {
            state = QValidator::Intermediate;
        }
    }
    return state;
}

//TODO: use a proper QValidator for the validation instead of doing it manually.
ProjectItemLineEdit::ProjectItemLineEdit(QWidget* parent)
    : KLineEdit(parent)
{
    setCompleter( new ProjectItemCompleter( this ) );
    setValidator( new ProjectItemValidator( this ) );
}

#include "projectitemlineedit.moc"
#include "moc_projectitemlineedit.cpp"
