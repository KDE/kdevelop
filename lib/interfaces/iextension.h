/****************************************************************************
**
** Copyright (C) 1992-2006 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#ifndef IEXTENSION_H
#define IEXTENSION_H

#include <QtDesigner/QAbstractExtensionFactory>

/**
 * These two macros can be used to declare an extension interface for the KDevPlatform
 * the _NS version is to be used when the interface is inside a namespace, the other
 * one should be used when the namespace is in the global namespace
 * When using either of the two you also need to use Q_DECLARE_INTERFACE()
 * macro from Qt else moc will complain.
 * The base code is from Qt's designer code
 *@TODO: Try to get in contact with moc-developers to check why
 *       Q_DECLARE_EXTENSION_INTERFACE does work when used the same way as
 *       KDEV_DECLARE_EXTENSION_INTERFACE
 */
#define KDEV_DECLARE_EXTENSION_INTERFACE_NS(Namespace, IFace, IId) \
namespace Namespace { \
const char * const IFace##_iid = IId; \
} \
template <> inline Namespace::IFace *qt_extension<Namespace::IFace *>( \
QAbstractExtensionManager *manager, QObject *object) \
{ QObject *extension = manager->extension(object, Q_TYPEID(Namespace::IFace)); \
return extension ? static_cast<Namespace::IFace *>(extension->qt_metacast( \
Namespace::IFace##_iid)) : static_cast<Namespace::IFace *>(0); }

#define KDEV_DECLARE_EXTENSION_INTERFACE(IFace, IId) \
const char * const IFace##_iid = IId; \
template <> inline IFace *qt_extension<IFace *>( \
QAbstractExtensionManager *manager, QObject *object) \
{ QObject *extension = manager->extension(object, Q_TYPEID(IFace)); \
return extension ? static_cast<IFace *>(extension->qt_metacast( \
IFace##_iid)) : static_cast<IFace *>(0); }

#endif

// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on

