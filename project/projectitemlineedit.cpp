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

#include <QAction>
#include <QCompleter>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QTreeView>
#include <QValidator>
#include <QVBoxLayout>

#include <KLocalizedString>

#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <project/projectmodel.h>
#include <util/kdevstringhandler.h>
#include <interfaces/iproject.h>
#include "projectproxymodel.h"

static const QChar sep = '/';
static const QChar escape = '\\';


class ProjectItemCompleter : public QCompleter
{
    Q_OBJECT
public:
    explicit ProjectItemCompleter(QObject* parent=nullptr);

    QString separator() const { return sep; }
    QStringList splitPath(const QString &path) const override;
    QString pathFromIndex(const QModelIndex& index) const override;

    void setBaseItem( KDevelop::ProjectBaseItem* item ) { mBase = item; }

private:
    KDevelop::ProjectModel* mModel;
    KDevelop::ProjectBaseItem* mBase;
};

class ProjectItemValidator : public QValidator
{
    Q_OBJECT
public:
    explicit ProjectItemValidator(QObject* parent = nullptr );
    QValidator::State validate( QString& input, int& pos ) const override;

    void setBaseItem( KDevelop::ProjectBaseItem* item ) { mBase = item; }

private:
    KDevelop::ProjectBaseItem* mBase;
};

ProjectItemCompleter::ProjectItemCompleter(QObject* parent)
    : QCompleter(parent)
    , mModel(KDevelop::ICore::self()->projectController()->projectModel())
    , mBase( nullptr )
{
    setModel(mModel);
    setCaseSensitivity( Qt::CaseInsensitive );
}


QStringList ProjectItemCompleter::splitPath(const QString& path) const
{
    return joinProjectBasePath( KDevelop::splitWithEscaping( path, sep, escape ), mBase );
}

QString ProjectItemCompleter::pathFromIndex(const QModelIndex& index) const
{
    QString postfix;
    if(mModel->itemFromIndex(index)->folder())
        postfix=sep;
    return KDevelop::joinWithEscaping(removeProjectBasePath( mModel->pathFromIndex(index), mBase ), sep, escape)+postfix;
}


ProjectItemValidator::ProjectItemValidator(QObject* parent): QValidator(parent), mBase(nullptr)
{
}


QValidator::State ProjectItemValidator::validate(QString& input, int& pos) const
{
    Q_UNUSED( pos );
    KDevelop::ProjectModel* model = KDevelop::ICore::self()->projectController()->projectModel();
    QStringList path = joinProjectBasePath( KDevelop::splitWithEscaping( input, sep, escape ), mBase );
    QModelIndex idx = model->pathToIndex( path );
    QValidator::State state = input.isEmpty() ? QValidator::Intermediate : QValidator::Invalid;
    if( idx.isValid() )
    {
        state = QValidator::Acceptable;
    } else if( path.count() > 1 )
    {
        // Check beginning of path and if that is ok, then try to find a child
        QString end = path.takeLast();
        idx = model->pathToIndex( path );
        if( idx.isValid() )
        {
            for( int i = 0; i < model->rowCount( idx ); i++ )
            {
                if( model->data( model->index( i, 0, idx ) ).toString().startsWith( end, Qt::CaseInsensitive ) )
                {
                    state = QValidator::Intermediate;
                    break;
                }
            }
        }
    } else if( path.count() == 1 )
    {
        // Check for a project whose name beings with the input
        QString first = path.first();
        foreach( KDevelop::IProject* project, KDevelop::ICore::self()->projectController()->projects() )
        {
            if( project->name().startsWith( first, Qt::CaseInsensitive ) )
            {
                state = QValidator::Intermediate;
                break;
            }
        }
    }
    return state;
}

ProjectItemLineEdit::ProjectItemLineEdit(QWidget* parent)
    : QLineEdit(parent),
      m_base(nullptr),
      m_completer( new ProjectItemCompleter( this ) ),
      m_validator( new ProjectItemValidator( this ) ),
      m_suggestion( nullptr )
{
    setCompleter( m_completer );
    setValidator( m_validator );
    setPlaceholderText( i18n("Enter the path to an item from the projects tree" ) );

    QAction* selectItemAction = new QAction(QIcon::fromTheme(QStringLiteral("folder-document")), i18n("Select..."), this);
    connect(selectItemAction, &QAction::triggered, this, &ProjectItemLineEdit::selectItemDialog);
    addAction(selectItemAction);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &ProjectItemLineEdit::customContextMenuRequested, this, &ProjectItemLineEdit::showCtxMenu);
}

void ProjectItemLineEdit::showCtxMenu(const QPoint& p)
{
    QScopedPointer<QMenu> menu(createStandardContextMenu());
    menu->addActions(actions());
    menu->exec(mapToGlobal(p));
}

bool ProjectItemLineEdit::selectItemDialog()
{
    KDevelop::ProjectModel* model=KDevelop::ICore::self()->projectController()->projectModel();

    QDialog dialog;
    dialog.setWindowTitle(i18n("Select an item..."));

    auto mainLayout = new QVBoxLayout(&dialog);

    QTreeView* view = new QTreeView(&dialog);
    ProjectProxyModel* proxymodel = new ProjectProxyModel(view);
    proxymodel->setSourceModel(model);
    view->header()->hide();
    view->setModel(proxymodel);
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    mainLayout->addWidget(new QLabel(i18n("Select the item you want to get the path from.")));
    mainLayout->addWidget(view);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    mainLayout->addWidget(buttonBox);

    if (m_suggestion) {
        const QModelIndex idx = proxymodel->proxyIndexFromItem(m_suggestion->projectItem());
        view->selectionModel()->select(idx, QItemSelectionModel::ClearAndSelect);
    }

    int res = dialog.exec();

    if(res==QDialog::Accepted && view->selectionModel()->hasSelection()) {
        QModelIndex idx=proxymodel->mapToSource(view->selectionModel()->selectedIndexes().first());

        setText(KDevelop::joinWithEscaping(model->pathFromIndex(idx), sep, escape));
        selectAll();
        return true;
    }
    return false;
}

void ProjectItemLineEdit::setItemPath(const QStringList& list)
{
    setText( KDevelop::joinWithEscaping( removeProjectBasePath( list, m_base ), sep, escape ) );
}

QStringList ProjectItemLineEdit::itemPath() const
{
    return joinProjectBasePath( KDevelop::splitWithEscaping( text(), sep, escape ), m_base );
}

void ProjectItemLineEdit::setBaseItem(KDevelop::ProjectBaseItem* item)
{
    m_base = item;
    m_validator->setBaseItem( m_base );
    m_completer->setBaseItem( m_base );
}

KDevelop::ProjectBaseItem* ProjectItemLineEdit::baseItem() const
{
    return m_base;
}

KDevelop::ProjectBaseItem* ProjectItemLineEdit::currentItem() const
{
    KDevelop::ProjectModel* model = KDevelop::ICore::self()->projectController()->projectModel();
    return model->itemFromIndex(model->pathToIndex(KDevelop::splitWithEscaping(text(),'/', '\\')));
}

void ProjectItemLineEdit::setSuggestion(KDevelop::IProject* project)
{
    m_suggestion = project;
}

#include "projectitemlineedit.moc"
#include "moc_projectitemlineedit.cpp"
