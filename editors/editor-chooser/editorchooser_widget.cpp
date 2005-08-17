#include <qcombobox.h>
#include <q3buttongroup.h>
#include <qradiobutton.h>

#include <kapplication.h>
#include <kdeversion.h>
#include <kservice.h>
#include <kdebug.h>
#include <kconfig.h>


#include "editorchooser_widget.h"


EditorChooserWidget::EditorChooserWidget(QWidget *parent, const char *name)
        : EditChooser(parent, name)
{
    // ask the trader which editors he has to offer
    m_offers = KTrader::self()->query("text/plain", "'KTextEditor/Document' in ServiceTypes");

	// remove the vim-part, it's known to crash
	KTrader::OfferList::Iterator it = m_offers.begin();
	while( it != m_offers.end() )
	{
		if ( (*it)->desktopEntryName() == "vimpart" )
		{
			m_offers.remove( it );
			break;
		}
		++it;
	}

	load();
	slotEditPartChanged(QString());
}


void EditorChooserWidget::load()
{
    EditorPart->clear();

    // find the editor to use
    KConfig *config = kapp->config();
    config->setGroup("Editor");
    QString editor = config->readPathEntry("EmbeddedKTextEditor");

    // add the entries to the listview
    KTrader::OfferList::Iterator it;
    int index=-1, current=0;
    for (it = m_offers.begin(); it != m_offers.end(); ++it)
    {
        EditorPart->insertItem((*it)->name());
        if ( (*it)->desktopEntryName() == editor )
            index = current;
        ++current;
    }

    if (index >=0)
        EditorPart->setCurrentItem(index);

	QString dirtyAction = config->readEntry( "DirtyAction" );
	
	if ( dirtyAction == "reload" )
	{
		reload->setChecked( true );
	}
	else if ( dirtyAction == "alert" )
	{
		alert->setChecked( true );
	}
	else
	{
		nothing->setChecked( true );
	}
}


void EditorChooserWidget::save()
{
    KConfig *config = kapp->config();
    config->setGroup("Editor");

    KTrader::OfferList::Iterator it;
    for (it = m_offers.begin(); it != m_offers.end(); ++it)
        if ( EditorPart->currentText() == (*it)->name() )
        {
            config->writePathEntry("EmbeddedKTextEditor", (*it)->desktopEntryName());
        }

	if ( reload->isChecked() )
	{
		config->writeEntry( "DirtyAction", "reload" );
	}
	else if ( alert->isChecked() )
	{
		config->writeEntry( "DirtyAction", "alert" );
	}
	else 
	{
		config->writeEntry( "DirtyAction", "nothing" );
	}

    config->sync();
}


void EditorChooserWidget::accept()
{
    save();
}

void EditorChooserWidget::slotEditPartChanged( const QString & )
{
	KTrader::OfferList::Iterator it;
	for (it = m_offers.begin(); it != m_offers.end(); ++it)
	{
		if ( EditorPart->currentText() == (*it)->name() )
		{
			external_changes_group->setEnabled( (*it)->desktopEntryName() == "katepart" );
			return;
		}
	}
	external_changes_group->setEnabled( false );
}


#include "editorchooser_widget.moc"




