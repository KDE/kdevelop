%{H_TEMPLATE}

#ifndef __KDEVPART_%{APPNAMEUC}_H__
#define __KDEVPART_%{APPNAMEUC}_H__


#include <kdevelop/kdevplugin.h>
#include <kdevelop/codemodel.h>
#include <kdevelop/kdevlanguagesupport.h>
#include <qstringlist.h>
#include <kdialogbase.h>

class KAction;

/*
 Please read the README.dox file for more info about this part
 */
class %{APPNAME}Part : public KDevLanguageSupport
{
	Q_OBJECT
	public:
		%{APPNAME}Part(QObject *parent, const char *name, const QStringList &);
		~%{APPNAME}Part();
	protected:
		virtual Features features();
		virtual KMimeType::List mimeTypes();

	private slots:
		void slotRun();
		void projectConfigWidget(KDialogBase *dlg);
		void projectOpened();
		void projectClosed();
		void savedFile(const KURL &fileName);
		void addedFilesToProject(const QStringList &fileList);
		void removedFilesFromProject(const QStringList &fileList);
		void parse();
		void slotActivePartChanged(KParts::Part *part);
	private:
		KAction *m_build;
};


#endif
