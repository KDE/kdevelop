/***************************************************************************
 *   Copyright (C) 2003 by Jens Dagerbo                                    *
 *   jens.dagerbo@swipnet.se                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

//BEGIN Includes

#include <kparts/part.h>
#include <klibloader.h>
#include <kurl.h>
#include <kdebug.h>
#include <ktexteditor/editinterface.h>
#include <ktexteditor/editor.h>
#include <kdevcore.h>
#include "kdevmainwindow.h"
#include "kdevproject.h"
#include "kdevpartcontroller.h"

#include <qlayout.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qstringlist.h>
#include <qptrlist.h>

#include "replace_part.h"
#include "replace_widget.h"
#include "replacedlg.h"
#include "replaceitem.h"
#include "replaceview.h"

//END Includes


ReplaceWidget::ReplaceWidget(ReplacePart *part)
        : QWidget(0, "replace widget"), m_part( part ), m_dialog( new ReplaceDlg( this, "replace widget" ) )
{
    QVBoxLayout * layout = new QVBoxLayout( this );
    QHBoxLayout * buttonlayout = new QHBoxLayout( layout );

    _cancel = new QPushButton( "Cancel", this );
    _replace = new QPushButton( "Replace", this );

    _cancel->setEnabled( false );
    _replace->setEnabled( false );

    buttonlayout->addWidget( _cancel );
    buttonlayout->addWidget( _replace );

    _listview = new ReplaceView( this );
    layout->addWidget( _listview );

    connect( m_dialog->find_button, SIGNAL( clicked() ), SLOT( find() ) );
    connect( _replace, SIGNAL( clicked() ), SLOT( replace() ) );
    connect( _cancel, SIGNAL( clicked() ), SLOT( clear() ) );
    connect( _listview, SIGNAL( executed(QListViewItem*) ), SLOT( clicked(QListViewItem*) ) );
}

//BEGIN Slots

void ReplaceWidget::showDialog()
{
    kdDebug(0) << " ******* ReplaceWidget::showDialog()" << endl;

    m_dialog->all_radio->setEnabled( m_part->project() );
    m_dialog->all_radio->isEnabled() ? m_dialog->all_radio->setChecked( true ) : m_dialog->open_radio->setChecked( true );

    m_dialog->show();
}

void ReplaceWidget::find()
{
    kdDebug(0) << " ******* ReplaceWidget::find()" << endl;

    if ( m_dialog->modified_files_check->isChecked())
    {
        kdDebug(0) << " *** saving all files *** " << endl;
        m_part->partController()->saveAllFiles();
    }

    kdDebug(0) << "find string: " << m_dialog->find_line->text() << endl;
    kdDebug(0) << "replacement string: " << m_dialog->replacement_line->text() << endl;

    _listview->clear();
    m_part->mainWindow()->raiseView(this);

    _listview->showReplacements( workFiles(), m_dialog->find_line->text(), m_dialog->replacement_line->text() );

    _cancel->setEnabled( true );
    _replace->setEnabled( true );
}

void ReplaceWidget::replace()
{
    kdDebug(0) << " ******* ReplaceWidget::replace()" << endl;

    _listview->makeReplacements( m_dialog->find_line->text(), m_dialog->replacement_line->text() );

    clear();
    reloadOpenFiles();
}

void ReplaceWidget::clear()
{
    kdDebug(0) << " ******* ReplaceWidget::clear()" << endl;

    _listview->clear();

    _cancel->setEnabled( false );
    _replace->setEnabled( false );
}

void ReplaceWidget::clicked( QListViewItem * item )
{
    kdDebug(0) << " ******* ReplaceWidget::clicked()" << endl;

    if ( ReplaceItem * rii = dynamic_cast<ReplaceItem*>( item ) )
    {
        kdDebug(0) << "File: " << rii->file() << " Line: " << rii->line() << endl;
        if ( !rii->blockClick() )
        {
            m_part->partController()->editDocument( rii->file(), rii->line() );
        }
    }
}

//END Slots


//BEGIN Helpers

void ReplaceWidget::reloadOpenFiles()
{
    QPtrList<KParts::Part> * partlist = m_part->partController()->parts();
    KParts::Part * part = partlist->first();
    while ( part != 0)
    {
        if ( KTextEditor::Editor * ed = dynamic_cast<KTextEditor::Editor *>( part ) )
        {
            if ( !ed->isModified() )
            {
                ed->openURL( ed->url().path() );
            }
        }
        part = partlist->next();
    }
}

QStringList const & ReplaceWidget::workFiles()
{
    if ( m_dialog->all_radio->isChecked() )
    {
        _list = m_part->project()->allFiles();

        QStringList::iterator it = _list.begin();
        while ( it != _list.end() )
        {
            *it = fullProjectPath( *it );
            ++it;
        }
        return _list;
    }
    // else assume m_dialog->all_radio->isChecked()
    return openEditorPaths();
}

QString ReplaceWidget::relativeProjectPath( QString path )
{
    QString project = m_part->project()->projectDirectory() + "/";
    if ( path.left( project.length() ) == project )
    {
        path = path.mid( project.length() );
    }
    return path;
}

QString ReplaceWidget::fullProjectPath( QString path )
{
    QString project = m_part->project()->projectDirectory() + "/";
    if ( path.left( project.length() ) != project )
    {
        path = project + path;
    }
    return path;
}

QStringList const & ReplaceWidget::openEditorPaths()
{
    return getEditorPaths( false );
}

QStringList const & ReplaceWidget::modifiedEditorPaths()
{
    return getEditorPaths( true );
}

QStringList const & ReplaceWidget::getEditorPaths( bool is_modified )
{
    QStringList & paths = _list;
    paths.clear();

    QPtrList<KParts::Part> * partlist = m_part->partController()->parts();
    KParts::Part * part = partlist->first();
    while ( part != 0)
    {
        if ( KTextEditor::Editor * ed = dynamic_cast<KTextEditor::Editor *>( part ) )
        {
            if ( ed->isModified() == is_modified )
            {
                kdDebug(0) << " is_modified = " << is_modified << " - " << ed->url().path() << endl;
                paths.append( ed->url().path() );
            }
        }
        part = partlist->next();
    }
    return paths;
}

//END Helpers

#include "replace_widget.moc"
