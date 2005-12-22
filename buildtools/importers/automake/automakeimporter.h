/* KDevelop Automake Support
 *
 * Copyright (C)  2005  Matt Rogers <mattr@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef AUTOMAKEIMPORTER_H
#define AUTOMAKEIMPORTER_H

#include <QtCore/QList>
#include <QtCore/QString>
#include "kdevprojecteditor.h"

class QObject;
class KDevProject;
class KDevProjectItem;
class AutoMakeItem;


class AutoMakeImporter : public KDevProjectEditor
{
public:
	AutoMakeImporter( QObject* parent = 0, const char* name = 0,
	                  const QStringList& args = QStringList() );
	
	virtual ~AutoMakeImporter();
	
	virtual KDevProject* project() const;
	virtual KDevProjectEditor* editor() const;
	
	virtual Features features() const { return KDevProjectEditor::All; }
	virtual bool addFolder( KDevProjectFolderItem* /*folder */,
	                        KDevProject* /*parent*/ ) { return false; }
	virtual bool addFolder( KDevProjectFolderItem*,
	                        KDevProjectFolderItem* ) { return false; }
	virtual bool addTarget( KDevProjectTargetItem* /*target*/,
	                        KDevProjectFolderItem* /*parent*/ ) { return false; }
	virtual bool addFile( KDevProjectFileItem*,
	                      KDevProjectFolderItem* ) { return false; }
	virtual bool addFile( KDevProjectFileItem*,
	                      KDevProjectTargetItem* ) { return false; }
	virtual bool removeFolder( KDevProjectFolderItem*,
	                           KDevProjectFolderItem* ) { return false; }
	virtual bool removeTarget( KDevProjectTargetItem*,
	                           KDevProjectFolderItem* ) { return false; }
	virtual bool removeFile( KDevProjectFileItem*,
	                         KDevProjectFolderItem* ) { return false; }
	virtual bool removeFile( KDevProjectFileItem*,
	                         KDevProjectTargetItem* ) { return false; }
	
	virtual QList<KDevProjectFolderItem*> parse( KDevProjectFolderItem* dom );
	virtual KDevProjectItem* import( KDevProjectModel* model,
	                                 const QString& fileName );
	virtual QString findMakefile( KDevProjectFolderItem* dom ) const;
	virtual QStringList findMakefiles( KDevProjectFolderItem* dom ) const;
	
	
private:
	KDevProject* m_project;
	KDevProjectItem* m_rootItem;
};

#endif
// kate: indent-mode csands; space-indent off; tab-width 4; auto-insert-doxygen on;

