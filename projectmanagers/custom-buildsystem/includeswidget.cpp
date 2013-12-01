/************************************************************************
 * KDevelop4 Custom Buildsystem Support                                 *
 *                                                                      *
 * Copyright 2010 Andreas Pakulat <apaku@gmx.de>                        *
 *                                                                      *
 * This program is free software; you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation; either version 2 or version 3 of the License, or    *
 * (at your option) any later version.                                  *
 *                                                                      *
 * This program is distributed in the hope that it will be useful, but  *
 * WITHOUT ANY WARRANTY; without even the implied warranty of           *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU     *
 * General Public License for more details.                             *
 *                                                                      *
 * You should have received a copy of the GNU General Public License    *
 * along with this program; if not, see <http://www.gnu.org/licenses/>. *
 ************************************************************************/

#include "includeswidget.h"

#include <QToolButton>

#include <KDebug>
#include <KLineEdit>
#include <KAction>
#include <kfiledialog.h>
#include <kmessagebox.h>

#include "ui_includeswidget.h"
#include "includesmodel.h"
#include <interfaces/iproject.h>

extern int cbsDebugArea(); // from debugarea.cpp

IncludesWidget::IncludesWidget( QWidget* parent )
    : QWidget ( parent ), ui( new Ui::IncludesWidget )
    , includesModel( new IncludesModel( this ) )
{
    ui->setupUi( this );

    // Hack to workaround broken setIcon(QIcon) overload in KPushButton, the function does not set the icon at all
    // So need to explicitly use the KIcon overload
    ui->addIncludePath->setIcon(KIcon("list-add"));
    ui->removeIncludePath->setIcon(KIcon("list-remove"));

    // hack taken from kurlrequester, make the buttons a bit less in height so they better match the url-requester
    ui->addIncludePath->setFixedHeight( ui->includePathRequester->sizeHint().height() );
    ui->removeIncludePath->setFixedHeight( ui->includePathRequester->sizeHint().height() );

    connect( ui->addIncludePath, SIGNAL(clicked(bool)), SLOT(addIncludePath()) );
    connect( ui->removeIncludePath, SIGNAL(clicked(bool)), SLOT(deleteIncludePath()) );

    ui->includePathRequester->setMode( KFile::Directory | KFile::LocalOnly | KFile::ExistingOnly );

    ui->includePaths->setModel( includesModel );
    connect( ui->includePaths->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), SLOT(includePathSelected(QModelIndex)) );
    connect( ui->includePathRequester, SIGNAL(textChanged(QString)), SLOT(includePathEdited()) );
    connect( ui->includePathRequester, SIGNAL(urlSelected(KUrl)), SLOT(includePathUrlSelected(KUrl)) );
    connect( includesModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(includesChanged()) );
    connect( includesModel, SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(includesChanged())  );
    connect( includesModel, SIGNAL(rowsRemoved(QModelIndex,int,int)), SLOT(includesChanged())  );

    KAction* delIncAction = new KAction( i18n("Delete Include Path"), this );
    delIncAction->setShortcut( KShortcut( "Del" ) );
    delIncAction->setShortcutContext( Qt::WidgetWithChildrenShortcut );
    ui->includePaths->addAction( delIncAction );
    connect( delIncAction, SIGNAL(triggered()), SLOT(deleteIncludePath()) );
}

void IncludesWidget::setIncludes( const QStringList& paths )
{
    bool b = blockSignals( true );
    clear();
    includesModel->setIncludes( paths );
    blockSignals( b );
    updateEnablements();
}
void IncludesWidget::includesChanged()
{
    kDebug(cbsDebugArea()) << "includes changed";
    emit includesChanged( includesModel->includes() );
}

void IncludesWidget::includePathSelected( const QModelIndex& selected )
{
    kDebug(cbsDebugArea()) << "include path list entry selected:" << selected;
    updateEnablements();
}

void IncludesWidget::includePathEdited()
{
    kDebug(cbsDebugArea()) << "include path edited:" << ui->includePathRequester->url();
    updateEnablements();
}

void IncludesWidget::clear()
{
    includesModel->setIncludes( QStringList() );
    updateEnablements();
}

void IncludesWidget::addIncludePath()
{
    kDebug(cbsDebugArea()) << "adding include path" << ui->includePathRequester->url();
    includesModel->addInclude( makeIncludeDirAbsolute(ui->includePathRequester->url()) );
    ui->includePathRequester->clear();
    updateEnablements();
}

void IncludesWidget::deleteIncludePath()
{
    kDebug(cbsDebugArea()) << "deleting include path" << ui->includePaths->currentIndex();
    const QModelIndex curidx = ui->includePaths->currentIndex();
    if( curidx.isValid() ) {
        if( KMessageBox::questionYesNo( this, i18n("Are you sure you want to remove the selected include path '%1'?", includesModel->data(curidx, Qt::DisplayRole).toString() ), i18n("Delete Include Path") ) == KMessageBox::Yes ) {
            includesModel->removeRows( curidx.row(), 1 );
        }
    }
    updateEnablements();
}

void IncludesWidget::includePathUrlSelected(const KUrl& url)
{
    Q_UNUSED(url);
    updateEnablements();
}

void IncludesWidget::setProject(KDevelop::IProject* w_project)
{
    ui->includePathRequester->setStartDir( w_project->folder() );
}

void IncludesWidget::updateEnablements() {
    // Disable removal of the project root entry which is always first in the list
    ui->addIncludePath->setEnabled( QFileInfo(makeIncludeDirAbsolute(ui->includePathRequester->url())).exists() && !ui->includePathRequester->text().isEmpty() );
    ui->removeIncludePath->setEnabled( ui->includePaths->currentIndex().isValid() );
}

QString IncludesWidget::makeIncludeDirAbsolute(const KUrl& url) const
{
    QString localFile = url.toLocalFile();
    if( url.isRelative() ) {
        // Relative, make absolute based on startDir of the requester
        localFile = ui->includePathRequester->startDir().toLocalFile( KUrl::AddTrailingSlash ) + url.path();
    }
    return localFile;
}

#include "includeswidget.moc"

