/*
 * This file was generated by dbusxml2cpp version 0.6
 * Command line was: dbusxml2cpp -c KWinAdaptor -m -p kwinadaptor -- org.kde.KWin.xml
 *
 * dbusxml2cpp is Copyright (C) 2006 Trolltech AS. All rights reserved.
 *
 * This is an auto-generated file.
 * This file may have been hand-edited. Look for HAND-EDIT comments
 * before re-generating it.
 */

#include "kwinadaptor.h"

/*
 * Implementation of interface class KWinAdaptor
 */

KWinAdaptor::KWinAdaptor(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
}

KWinAdaptor::~KWinAdaptor()
{
}


#include "kwinadaptor.moc"
