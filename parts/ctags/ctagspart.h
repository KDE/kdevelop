/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _CTAGSPART_H_
#define _CTAGSPART_H_

#include <qguardedptr.h>
#include <kdialogbase.h>
#include "kdevplugin.h"

struct CTagsTagInfo
{
    QString fileName;
    QString pattern;
    int lineNum;
    char kind;
};

typedef QValueList<CTagsTagInfo> CTagsTagInfoList;
typedef QValueList<CTagsTagInfo>::Iterator CTagsTagInfoListIterator;
typedef QValueList<CTagsTagInfo>::ConstIterator CTagsTagInfoListConstIterator;
typedef QMap<QString,CTagsTagInfoList> CTagsMap;
typedef QMap<QString,CTagsTagInfoList>::Iterator CTagsMapIterator;
typedef QMap<QString,CTagsTagInfoList>::ConstIterator CTagsMapConstIterator;

class QPopupMenu;
class Context;
class CTagsDialog;


class CTagsPart : public KDevPlugin
{
    Q_OBJECT

public:
    CTagsPart( QObject *parent, const char *name, const QStringList & );
    ~CTagsPart();

    bool ensureTagsLoaded();
    bool loadTagsFile();
    bool createTagsFile();

    CTagsMap tags()
    { return *m_tags; }
    QStringList kindStrings()
    { return m_kindStrings; }

private slots:
    void projectClosed();
    void contextMenu(QPopupMenu *popup, const Context *context);
    void slotSearchTags();
    void slotGotoDeclaration();
    void slotGotoDefinition();
    
private:
    void gotoTag(const QString &tag, const QString &kindChars);
    
    CTagsDialog *m_dialog;
    CTagsMap *m_tags;
    QStringList m_kindStrings;
    QString m_contextString;
};

#endif
