/* 
   Copyright (C) 2003 ian reinhart geiser <geiseri@kde.org> 

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   version 2, License as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/


#ifndef __KDEVPART_KJSSUPPORT_H__
#define __KDEVPART_KJSSUPPORT_H__


#include <kdevelop/kdevplugin.h>
#include <kdevelop/codemodel.h>
#include <kdevelop/kdevlanguagesupport.h>
#include <kdialogbase.h>
#include <qstringlist.h>
#include <qdict.h>

namespace KJSEmbed
{
class KJSEmbedPart;
class KJSConsoleWidget;
};

class QPopupMenu;
class KAction;
class KJSProblems;
class JSCodeCompletion;
class Context;

class typeProperty;


class kjsSupportPart : public KDevLanguageSupport
{
	Q_OBJECT
	public:
		kjsSupportPart(QObject *parent, const char *name, const QStringList &);
		~kjsSupportPart();
	protected:
		virtual Features features();
		virtual KMimeType::List mimeTypes();
		
	private slots:
		void slotRun();
		void projectConfigWidget(KDialogBase *dlg);
		void projectOpened();
		void projectClosed();
		void savedFile(const QString &fileName);
		void addedFilesToProject(const QStringList &fileList);
		void removedFilesFromProject(const QStringList &fileList);
		void parse();
		void slotActivePartChanged(KParts::Part *part);
		void contextMenu(QPopupMenu *popupMenu, const Context *context);
		void implementSlots();
	private:
		void parse(const QString &fileName);
		void addAttribute(const QString &name, ClassDom clazz, uint lineNo);
		void addMethod(const QString &name, ClassDom clazz, uint lineNo);
		void addAttribute(const QString &name, FileDom file, uint lineNo);
		void addMethod(const QString &name, FileDom file, uint lineNo);
		ClassDom addClass(const QString &name, FileDom file, uint lineNo);
		KAction *m_build;
		KJSEmbed::KJSEmbedPart *m_js;
		KJSProblems *m_problemReporter;
		QDict<typeProperty> m_typeMap;
		JSCodeCompletion *m_cc;
		
		QString m_selectedUI;
};


#endif
