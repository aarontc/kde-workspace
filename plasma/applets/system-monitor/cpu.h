/*
 *   Copyright (C) 2008 Petri Damsten <damu@iki.fi>
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

#ifndef CPU_HEADER
#define CPU_HEADER

#include <ui_cpu-config.h>
#include "applet.h"
#include <Plasma/DataEngine>
#include <QStandardItemModel>
#include <QTimer>
#include <QRegExp>

class QStandardItemModel;

namespace SM {

class Cpu : public Applet
{
    Q_OBJECT
    public:
        Cpu(QObject *parent, const QVariantList &args);
        ~Cpu();

        virtual void init();
        virtual bool addMeter(const QString&);
        virtual void createConfigurationInterface(KConfigDialog *parent);
        virtual void setDetail(Detail detail);

    public slots:
        void dataUpdated(const QString &name,
                         const Plasma::DataEngine::Data &data);
        void sourceAdded(const QString &name);
        void sourcesAdded();
        void configAccepted();

    protected:
        QString cpuTitle(const QString &name);

    private:
        Ui::config ui;
        QStandardItemModel m_model;
        QStringList m_cpus;
        QHash<QString, QString> m_html;
        QTimer m_sourceTimer;
        QRegExp m_rx;

    private slots:
        void themeChanged();
};
}

K_EXPORT_PLASMA_APPLET(sm_cpu, SM::Cpu)

#endif
