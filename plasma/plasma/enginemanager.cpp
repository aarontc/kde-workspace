/*
 *   Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <KDebug>
#include <KServiceTypeTrader>
#include <KParts/ComponentFactory>

#include "enginemanager.h"

DataEngineManager::DataEngineManager()
{
}

DataEngineManager::~DataEngineManager()
{
    foreach (Plasma::DataEngine* engine, m_engines)
    {
        delete engine;
    }
    m_engines.clear();
}

Plasma::DataEngine* DataEngineManager::dataEngine(const QString& name) const
{
    Plasma::DataEngine::Dict::const_iterator it = m_engines.find(name);
    if (it != m_engines.end())
    {
        // ref and return the engine
        //Plasma::DataEngine *engine = *it;
        return *it;
    }

    return 0;
}

Plasma::DataEngine* DataEngineManager::loadDataEngine(const QString& name)
{
    Plasma::DataEngine* engine = dataEngine(name);

    if (engine) {
        engine->ref();
        return engine;
    }

    // load the engine, add it to the engines
    QString constraint = QString("[X-EngineName] == '%1'").arg(name);
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/DataEngine",
                                                              constraint);

    if (offers.isEmpty()) {
        kDebug() << "offers are empty for " << name << " with constraint " << constraint << endl;
        return 0;
    }

    int errorCode = 0;
    engine = KService::createInstance<Plasma::DataEngine>(offers.first(), 0);
    if (!engine) {
        kDebug() << errorCode << " couldn't load engine! " << name << endl;
        return 0;
    }

    engine->ref();
    m_engines[name] = engine;
    return engine;
}

void DataEngineManager::unloadDataEngine(const QString& name)
{
    Plasma::DataEngine* engine = dataEngine(name);

    if (engine) {
        engine->deref();

        if (!engine->isUsed()) {
            m_engines.remove(name);
            delete engine;
        }
    }
}

