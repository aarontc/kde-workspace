/*
 *   Copyright (c) 2009 Chani Armitage <chani@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#ifndef MOUSEINPUTBUTTON_H
#define MOUSEINPUTBUTTON_H

#include <QHash>
#include <QPushButton>

#include "plasmagenericshell_export.h"

class QEvent;

class PLASMAGENERICSHELL_EXPORT MouseInputButton : public QPushButton
{
    Q_OBJECT
public:
    MouseInputButton(QWidget *parent = 0);

    QString trigger();
    void setTrigger(const QString &trigger);

signals:
    void triggerChanged(const QString &oldTrigger, const QString &newTrigger);

protected:
    bool event(QEvent *event);

private slots:
    void getTrigger();

private:
    void changeTrigger(const QString& newTrigger);

    QString m_trigger;
    QHash<QString,QString> m_prettyStrings;
};

#endif
