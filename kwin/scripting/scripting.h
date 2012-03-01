/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2010 Rohan Prabhu <rohan@rohanprabhu.com>
Copyright (C) 2011 Martin Gräßlin <mgraesslin@kde.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/

#ifndef KWIN_SCRIPTING_H
#define KWIN_SCRIPTING_H

#include <QtCore/QFile>
#include <QtCore/QStringList>

class QDeclarativeView;
class QScriptEngine;
class QScriptValue;
class KConfigGroup;

namespace KWin
{
class WorkspaceWrapper;

class AbstractScript : public QObject
{
    Q_OBJECT
public:
    AbstractScript(int id, QString scriptName, QString pluginName, QObject *parent = NULL);
    ~AbstractScript();
    QString fileName() const {
        return m_scriptFile.fileName();
    }

    void printMessage(const QString &message);

    KConfigGroup config() const;

public Q_SLOTS:
    Q_SCRIPTABLE void stop();
    Q_SCRIPTABLE virtual void run() = 0;

Q_SIGNALS:
    Q_SCRIPTABLE void print(const QString &text);

protected:
    QFile &scriptFile() {
        return m_scriptFile;
    }
    const QString &pluginName() {
        return m_pluginName;
    }
    bool running() const {
        return m_running;
    }
    void setRunning(bool running) {
        m_running = running;
    }
    int scriptId() const {
        return m_scriptId;
    }

    WorkspaceWrapper *workspace() {
        return m_workspace;
    }

    void installScriptFunctions(QScriptEngine *engine);

private:
    int m_scriptId;
    QFile m_scriptFile;
    QString m_pluginName;
    bool m_running;
    WorkspaceWrapper *m_workspace;
};

class Script : public AbstractScript
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.kwin.Scripting")
public:

    Script(int id, QString scriptName, QString pluginName, QObject *parent = NULL);
    virtual ~Script();

public Q_SLOTS:
    Q_SCRIPTABLE void run();

Q_SIGNALS:
    Q_SCRIPTABLE void printError(const QString &text);

private slots:
    /**
      * A nice clean way to handle exceptions in scripting.
      * TODO: Log to file, show from notifier..
      */
    void sigException(const QScriptValue &exception);

private:
    QScriptEngine *m_engine;
};

class DeclarativeScript : public AbstractScript
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.kwin.Scripting")
public:
    explicit DeclarativeScript(int id, QString scriptName, QString pluginName, QObject *parent = 0);
    virtual ~DeclarativeScript();

public Q_SLOTS:
    Q_SCRIPTABLE void run();

private:
    QDeclarativeView *m_view;
};

/**
  * The heart of KWin::Scripting. Infinite power lies beyond
  */
class Scripting : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.kwin.Scripting")
private:
    QStringList scriptList;
    QList<KWin::AbstractScript*> scripts;

    // Preferably call ONLY at load time
    void runScripts();

public:
    Scripting(QObject *parent = NULL);
    /**
      * Start running scripts. This was essential to have KWin::Scripting
      * be initialized on stack and also have the option to disable scripting.
      */
    void start();
    ~Scripting();
    Q_SCRIPTABLE Q_INVOKABLE int loadScript(const QString &filePath, const QString &pluginName = QString());
    Q_SCRIPTABLE Q_INVOKABLE int loadDeclarativeScript(const QString &filePath, const QString &pluginName = QString());

public Q_SLOTS:
    void scriptDestroyed(QObject *object);
};

}
#endif
