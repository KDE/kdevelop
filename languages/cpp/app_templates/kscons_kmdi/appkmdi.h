%{H_TEMPLATE}

#ifndef _%{APPNAMEUC}KMDI_H
#define _%{APPNAMEUC}KMDI_H

#include <qvaluelist.h>

#include <kapplication.h>
#include <k3mdimainfrm.h>
#include <kurl.h>

class KToggleAction;
class KRecentFilesAction;

class kmdikonsole;
class KMdiChildView;
class QWidgetStack;
class %{APPNAMELC}kmdiView;
class TextProperties;
class MiscProperties;

/**
 * This class serves as the main window for %{APPNAMELC}kmdi.  It handles the
 * menus, toolbars, and status bars.
 *
 * @short Main window class
 */
class %{APPNAMELC}kmdi : public KMdiMainFrm
{
	Q_OBJECT
	public:
		%{APPNAMELC}kmdi( KMdi::MdiMode mode );
		virtual ~%{APPNAMELC}kmdi();
		void showTipOnStart();

	public slots:
		void openURL( const KURL& );

	protected:
		virtual void dragEnterEvent(QDragEnterEvent *event);
		virtual void dropEvent(QDropEvent *event);
		virtual bool queryClose();

	private slots:
		void slotFileNew();
		void slotFileClose();
		void slotFileQuit();

		/// this is called when a new file is saved
		void showTip();

		void optionsShowToolbar();
		void optionsShowStatusbar();
		void optionsConfigureKeys();
		void optionsConfigureToolbars();
		void optionsPreferences();
		void newToolbarConfig();


		void changeStatusbar(const QString& text);

		void currentChanged( KMdiChildView *current );
		bool requestClose(KMdiChildView* view);

		void settingsChanged();

	private:
		void setupAccel();
		void setupActions();

		QValueList<%{APPNAMELC}kmdiView*> m_views;

		KToggleAction *m_toolbarAction;
		KToggleAction *m_statusbarAction;

		KParts::PartManager *m_manager;

		kmdikonsole *m_console;
};

#endif // _%{APPNAMEUC}KMDI_H

