/*
  Copyright 2008 Google Inc.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#include <QtGui/QMessageBox>

#include <ggadget/gadget_consts.h>
#include <ggadget/gadget.h>
#include <ggadget/messages.h>
#include <ggadget/menu_interface.h>
#include <ggadget/decorated_view_host.h>
#include <ggadget/qt/qt_view_widget.h>

#include <plasma/applet.h>
#include "panel_decorator.h"

namespace ggadget {

PanelDecorator::PanelDecorator(ViewHostInterface *host, GadgetInfo *info)
    : DockedMainViewDecorator(host), info_(info) {
  SetButtonVisible(MainViewDecoratorBase::POP_IN_OUT_BUTTON, false);
  SetButtonVisible(MainViewDecoratorBase::MENU_BUTTON, false);
  SetButtonVisible(MainViewDecoratorBase::CLOSE_BUTTON, false);
}

PanelDecorator::~PanelDecorator() {}

void PanelDecorator::OnAddDecoratorMenuItems(MenuInterface *menu) {
  int priority = MenuInterface::MENU_ITEM_PRI_DECORATOR;
  AddCollapseExpandMenuItem(menu);
  menu->AddItem(
      "Debug", 0, 0,
      NewSlot(this, &PanelDecorator::ShowDebugInfo), priority);
}

void PanelDecorator::ShowDebugInfo(const char*) {
  QString msg = "Applet size:(%1, %2)\nWidget size:(%3, %4)\nView size:(%5, %6)\n";
  qt::QtViewWidget *widget = static_cast<qt::QtViewWidget*>(info_->main_view_host->GetNativeWidget());
  ViewInterface *view = info_->main_view_host->GetViewDecorator();
  QMessageBox::information(NULL,
    "Debug",
    msg.arg(info_->applet->size().width())
       .arg(info_->applet->size().height())
       .arg(widget->size().width())
       .arg(widget->size().height())
       .arg(view->GetWidth())
       .arg(view->GetHeight()));
}
} // namespace ggadget
