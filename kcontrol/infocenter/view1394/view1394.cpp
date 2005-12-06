/*
 * view1394.cpp
 *
 *  Copyright (C) 2003 Alexander Neundorf <neundorf@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Cambridge, MA 02110-1301, USA.
 */

#include <qlayout.h>
#include <qpushbutton.h>
#include <q3listview.h>
#include <qfile.h>
#include <qtextstream.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include <kinstance.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include "view1394.h"

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>

#include <libraw1394/csr.h>
#define CONFIGROM_BASE 0x00
#define CONFIGROM_CAP  0x08
#define CONFIGROM_GUID_HI 0x0c
#define CONFIGROM_GUID_LO 0x10
#define MAX_1394_PORTS 16

int my_reset_handler(raw1394handle_t handle, unsigned int )
{
   View1394* view1394=(View1394*)raw1394_get_userdata(handle);
   if (view1394!=0)
      view1394->rescanBus();
   return 0;
}


View1394::View1394(KInstance *inst,QWidget *parent)
:KCModule(inst, parent)
,m_insideRescanBus(false)
{
   setQuickHelp( i18n("On the right hand side you can see some information about "
               "your IEEE 1394 configuration.<br>"
               "The meaning of the columns:<br>"
               "<b>Name</b>: port or node name, the number can change with each bus reset<br>"
               "<b>GUID</b>: the 64 bit GUID of the node<br>"
               "<b>Local</b>: checked if the node is an IEEE 1394 port of your computer<br>"
               "<b>IRM</b>: checked if the node is isochronous resource manager capable<br>"
               "<b>CRM</b>: checked if the node is cycle master capable<br>"
               "<b>ISO</b>: checked if the node supports isochronous transfers<br>"
               "<b>BM</b>: checked if the node is bus manager capable<br>"
               "<b>PM</b>: checked if the node is power management capable<br>"
               "<b>Acc</b>: the cycle clock accuracy of the node, valid from 0 to 100<br>"
               "<b>Speed</b>: the speed of the node<br>"));

   setButtons( KCModule::Help );

   m_ouiDb=new OuiDb();
   QVBoxLayout *box=new QVBoxLayout(this, 0, KDialog::spacingHint());
   m_view=new View1394Widget(this);
   for (int i=2; i<8; i++)
      m_view->m_listview->setColumnAlignment(i, Qt::AlignHCenter);
   m_view->m_listview->setColumnAlignment(8, Qt::AlignRight);
   m_view->m_listview->setColumnAlignment(9, Qt::AlignRight);
   box->addWidget(m_view);
   connect(m_view->m_busResetPb, SIGNAL(clicked()), this, SLOT(generateBusReset()));
   connect(&m_rescanTimer, SIGNAL(timeout()), this, SLOT(rescanBus()));
   m_notifiers.setAutoDelete(true);
   rescanBus();
}

View1394::~View1394()
{
}

bool View1394::readConfigRom(raw1394handle_t handle, nodeid_t nodeid, quadlet_t& firstQuad, quadlet_t& cap, octlet_t& guid)
{
   quadlet_t q=0;
   firstQuad=0;
   cap=0;
   guid=0;
   nodeaddr_t addr=CSR_REGISTER_BASE + CSR_CONFIG_ROM + CONFIGROM_BASE;
   for (int count=0; count<5; count++)
   {
      struct timeval tv;
      q=0;
      int res=raw1394_read(handle, nodeid|0xffc0, addr, sizeof(q), &q);
      if (res==0)
      {
         firstQuad=ntohl(q);
         break;
      }
      tv.tv_sec=0;
      tv.tv_usec=10*1000;
      select(0,0,0,0,&tv);
   }
   if (firstQuad==0)
      return false;

   addr=CSR_REGISTER_BASE + CSR_CONFIG_ROM + CONFIGROM_CAP;
   if (raw1394_read(handle, nodeid|0xffc0, addr, sizeof(q), &q)!=0)
      return false;

   cap=ntohl(q);

   addr=CSR_REGISTER_BASE + CSR_CONFIG_ROM + CONFIGROM_GUID_HI;
   if (raw1394_read(handle, nodeid|0xffc0, addr, sizeof(q), &q)!=0)
      return false;

   guid=octlet_t(ntohl(q))<<32;

   addr=CSR_REGISTER_BASE + CSR_CONFIG_ROM + CONFIGROM_GUID_LO;
   if (raw1394_read(handle, nodeid|0xffc0, addr, sizeof(q), &q)!=0)
      return false;

   guid=guid|ntohl(q);

   return true;
}

void View1394::callRaw1394EventLoop(int fd)
{
   for (QList<raw1394handle_t>::iterator it= m_handles.begin(); it!=m_handles.end(); ++it)
      if (raw1394_get_fd(*it)==fd)
      {
         raw1394_loop_iterate(*it);
         break;
      }
}

void View1394::rescanBus()
{
   if (m_insideRescanBus)
   {
      m_rescanTimer.start(100, true);
      return;
   }
   m_insideRescanBus=true;
//   static int depth=0;
//   depth++;
   m_notifiers.clear();
   for (QList<raw1394handle_t>::iterator it=m_handles.begin(); it!=m_handles.end(); ++it)
      raw1394_destroy_handle(*it);
   m_handles.clear();
   m_view->m_listview->clear();
   
   raw1394handle_t handle=raw1394_new_handle();
   if (handle==NULL)
   {
      m_insideRescanBus=false;
      return;
   }
   //now search for new stuff
   struct raw1394_portinfo p_info[MAX_1394_PORTS];
   int num_of_cards=raw1394_get_port_info(handle, p_info, MAX_1394_PORTS);
   raw1394_destroy_handle(handle);
   //iterate over all cards
   for (int i=0; i<num_of_cards; i++)
   {
      handle=raw1394_new_handle();
      if (raw1394_set_port(handle, i)!=0)
      {
         raw1394_destroy_handle(handle);
         continue;
      }
      raw1394_set_userdata(handle, this);
      raw1394_set_bus_reset_handler(handle, my_reset_handler);
      QSocketNotifier *notif=new QSocketNotifier(raw1394_get_fd(handle),QSocketNotifier::Read);
      connect(notif, SIGNAL(activated(int)), this, SLOT(callRaw1394EventLoop(int)));
      m_notifiers.append(notif);
      m_handles.append(handle);
      
      Q3ListViewItem *card=new Q3ListViewItem(m_view->m_listview,i18n("Port %1:\"%2\"").arg(i).arg(p_info[i].name));
      int num_of_nodes=raw1394_get_nodecount(handle);

      int localNodeId=raw1394_get_local_id(handle);
      //iterate over all nodes connected to this card
      for (int j=0; j<num_of_nodes; j++)
      {
         //get the guid of the node
         octlet_t guid=0;
         quadlet_t firstQuad=0;
         quadlet_t cap=0;
         bool success=readConfigRom(handle, j, firstQuad, cap, guid);

         QString nodeStr=i18n("Node %1").arg(j);
         if (!success)
         {
            new Q3ListViewItem(card,nodeStr, i18n("Not ready"));
            continue;
         }
         if (((firstQuad>>24) & 0xff)==1) //minimal config rom
         {
            QString guidStr=QString::number(firstQuad,16);
            guidStr="0x"+guidStr.rightJustified(6,'0');
            new Q3ListViewItem(card,nodeStr, guidStr);
         }
         else  //general config rom
         {
            QString guidStr;
            char buf[32];
            snprintf(buf, 32, "%llX", guid);
            guidStr=buf;
            guidStr="0x"+guidStr.rightJustified(16,'0');
            QString local=((j | 0xffc0) == localNodeId)?"X":"";
            QString irmStr=(cap & 0x80000000) ? "X":"";
            QString cmStr=(cap & 0x40000000) ? "X":"";
            QString isStr=(cap & 0x20000000) ? "X":"";
            QString bmStr=(cap & 0x10000000) ? "X":"";
            QString pmStr=(cap & 0x08000000) ? "X":"";
            QString accStr=QString::number((cap &0x00ff0000)>>16);
            int speed=( cap & 0x00000007);
            QString speedStr;
            switch(speed)
            {
            case(3):
               speedStr="800";
               break;
            case(2):
               speedStr="400";
               break;
            case(1):
               speedStr="200";
               break;
            case(0):
            default:
               speedStr="100";
               break;
            }
            Q3ListViewItem* node=new Q3ListViewItem(card);
            node->setText(0,nodeStr);
            node->setText(1, guidStr);
            node->setText(2,local);
            node->setText(3, irmStr);
            node->setText(4, cmStr);
            node->setText(5,isStr);
            node->setText(6,bmStr);
            node->setText(7,pmStr);
            node->setText(8, accStr);
            node->setText(9, speedStr);
            node->setText(10, m_ouiDb->vendor(guid));
         }
      }
      card->setOpen(true);
   }
//   depth--;
   m_insideRescanBus=false;
}

void View1394::generateBusReset()
{
   for (QList<raw1394handle_t>::iterator it=m_handles.begin(); it!=m_handles.end(); ++it)
      raw1394_reset_bus(*it);
}


OuiDb::OuiDb()
{
   QString filename=locate("data","kcmview1394/oui.db");
   if (filename.isEmpty())
      return;
   QFile f(filename);
   if (!f.open(QIODevice::ReadOnly))
      return;

   QByteArray ba=f.readAll();
   int bytesLeft=ba.size();
   char* data=ba.data();
   while(bytesLeft>8)
   {
      char *eol=(char*)memchr((const void*)data, '\n',bytesLeft);
      if (eol==0)
         break;
      if ((eol-data)<8)
         break;
      data[6]='\0';
      *eol='\0';
      m_vendorIds.insert(QLatin1String(data), QString::fromUtf8(data+7));
      bytesLeft-=(eol+1-data);
      data=eol+1;
   }

   f.close();
}

QString OuiDb::vendor(octlet_t guid)
{
   guid=(guid & 0xffffff0000000000LL)>>40;
   QString key=QString::number((unsigned int)(guid),16);
   key=key.rightJustified(6, '0').toUpper();
   QString v=m_vendorIds[key];
   if (v.isEmpty())
      v=i18n("Unknown");
   return v;
}



// ------------------------------------------------------------------------

extern "C"
{

   KDE_EXPORT KCModule *create_view1394(QWidget *parent, const char */*name*/)
   {
      KGlobal::locale()->insertCatalog("kcmview1394");
	  KInstance *inst = new KInstance("kcmview1394");
      return new View1394(inst,parent);
   }
}

#include "view1394.moc"
