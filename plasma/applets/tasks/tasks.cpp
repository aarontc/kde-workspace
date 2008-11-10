/***************************************************************************
 *   Copyright (C) 2007 by Robert Knight <robertknight@gmail.com>          *
 *   Copyright (C) 2008 by Alexis Ménard <darktears31@gmail.com>           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

// Own
#include "tasks.h"
#include "windowtaskitem.h"
#include "taskgroupitem.h"
#include "ui_tasksConfig.h"

//Taskmanager
#include <taskmanager/taskgroup.h>
#include <taskmanager/taskitem.h>

// KDE
#include <KConfigDialog>
#include <KWindowSystem>

// Qt
#include <QGraphicsSceneWheelEvent>
#include <QTimeLine>
#include <QGraphicsScene>
#include <QGraphicsLinearLayout>
#include <QVariant>

// Plasma
#include <Plasma/Containment>
#include <Plasma/FrameSvg>
#include <Plasma/Theme>

Tasks::Tasks(QObject* parent, const QVariantList &arguments)
     : Plasma::Applet(parent, arguments),
       m_taskItemBackground(0),
       m_colorScheme(0),
       m_leftMargin(0),
       m_topMargin(0),
       m_rightMargin(0),
       m_bottomMargin(0),
       m_groupManager(0),
       m_groupModifierKey(Qt::AltModifier)
{
    setHasConfigurationInterface(true);
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    m_screenTimer.setSingleShot(true);
    m_screenTimer.setInterval(300);
    resize(500, 58);
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(themeRefresh()));

    setAcceptDrops(true);

}

Tasks::~Tasks()
{
    delete m_colorScheme;
    delete m_groupManager;
}

void Tasks::init()
{
    //kDebug();

    m_groupManager = new TaskManager::GroupManager(this);
    Plasma::Containment* appletContainment = containment();
    if (appletContainment) {
        m_groupManager->setScreen(appletContainment->screen());
    }

   // connect(m_groupManager, SIGNAL(reload()), this, SLOT(reload()));
    connect(this, SIGNAL(settingsChanged()), m_groupManager, SLOT(reconnect()));
    connect(m_groupManager, SIGNAL(itemRemoved(AbstractGroupableItem*)), this, SLOT(itemRemoved(AbstractGroupableItem*)));

    m_rootGroupItem = new TaskGroupItem(dynamic_cast<QGraphicsWidget*>(this), this, false);
    m_rootGroupItem->setGroup(m_groupManager->rootGroup());

    /*
    foreach (TaskManager::AbstractGroupableItem *item, m_groupManager->rootGroup()->members()) {
        kDebug() << item->name();
    }
    */

    m_groupTaskItems.insert(m_rootGroupItem->group(), m_rootGroupItem);
    connect(m_rootGroupItem, SIGNAL(sizeHintChanged(Qt::SizeHint)), this, SLOT(changeSizeHint(Qt::SizeHint)));

    setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
    //like in Qt's designer
    //TODO : Qt's bug??
    setMaximumSize(INT_MAX,INT_MAX);

    layout = new QGraphicsLinearLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
    //TODO : Qt's bug??
    layout->setMaximumSize(INT_MAX,INT_MAX);
    layout->setOrientation(Qt::Vertical);
    layout->addItem(m_rootGroupItem);


    setLayout(layout);

    KConfigGroup cg = config();

    m_groupManager->setShowOnlyCurrentDesktop( cg.readEntry("showOnlyCurrentDesktop", false));
    m_groupManager->setShowOnlyCurrentScreen( cg.readEntry("showOnlyCurrentScreen", false));
    m_groupManager->setShowOnlyMinimized( cg.readEntry("showOnlyMinimized", false));
    m_groupManager->setOnlyGroupWhenFull(cg.readEntry("groupWhenFull", true));
    m_showTooltip = cg.readEntry("showTooltip", true);

    m_groupManager->setGroupingStrategy( static_cast<TaskManager::GroupManager::TaskGroupingStrategy>(cg.readEntry("groupingStrategy", static_cast<int>(TaskManager::GroupManager::ProgramGrouping))));

    m_groupManager->setSortingStrategy( static_cast<TaskManager::GroupManager::TaskSortingStrategy>(cg.readEntry("sortingStrategy", static_cast<int>(TaskManager::GroupManager::AlphaSorting))));
    m_rootGroupItem->setMaxRows( cg.readEntry("maxRows", 2));
    m_rootGroupItem->setForceRows( cg.readEntry("forceRows", false));

    m_rootGroupItem->expand();
    emit settingsChanged();
}

void Tasks::reload()
{
    m_rootGroupItem->reload();
}

TaskManager::GroupManager &Tasks::groupManager() const
{
    return *m_groupManager;
}

Qt::KeyboardModifiers Tasks::groupModifierKey() const
{
    return m_groupModifierKey;
}

WindowTaskItem * Tasks::createStartingTask(TaskManager::TaskItem* task)
{
    WindowTaskItem* item = new WindowTaskItem(m_rootGroupItem, this, m_showTooltip);
    item->setStartupTask(task);
    m_startupTaskItems.insert(task->startup(), item);
    return item;
}



void Tasks::removeStartingTask(StartupPtr task)
{
    if (m_startupTaskItems.contains(task)) {
        WindowTaskItem *item = m_startupTaskItems.take(task);
        item->close();
    }
}

WindowTaskItem *Tasks::createWindowTask(TaskManager::TaskItem* taskItem)
{
    WindowTaskItem *item = 0;
    TaskPtr task = taskItem->task();

    if (m_windowTaskItems.contains(task)) {
        return m_windowTaskItems.value(task);
    }

    foreach (const StartupPtr &startup, m_startupTaskItems.keys()) {
        if (startup->matchesWindow(task->window())) {
            item = dynamic_cast<WindowTaskItem *>(m_startupTaskItems.take(startup));
            Q_ASSERT(item);
            item->setWindowTask(taskItem);
            break;
        }
    }

    if (!item) { //Task isnt a startup task
        item = new WindowTaskItem(m_rootGroupItem, this, m_showTooltip);
        item->setWindowTask(taskItem);
    }

    m_windowTaskItems.insert(task, item);
    return item;
    //kDebug();
}

void Tasks::itemRemoved(TaskManager::AbstractGroupableItem *item)
{
    //kDebug();
    if (!item->isGroupItem()) {
        removeItem(m_items.value(item));
    }
}


void Tasks::removeItem(AbstractTaskItem *item)
{
    //kDebug();
    if (!m_items.contains(m_items.key(item)) || !item) {
        //kDebug() << "Not in list or null pointer";
        return;
    }
    m_items.remove(m_items.key(item));
    if (item->isWindowItem()) {
        WindowTaskItem *windowItem = dynamic_cast<WindowTaskItem*>(item);
        if (m_windowTaskItems.values().contains(windowItem)) {
            m_windowTaskItems.remove(m_windowTaskItems.key(windowItem));
        } else if (m_startupTaskItems.values().contains(windowItem)) {
            m_startupTaskItems.remove(m_startupTaskItems.key(windowItem));
        }
    } else {
        m_groupTaskItems.remove(m_groupTaskItems.key(dynamic_cast<TaskGroupItem*>(item)));
    }
    item->close();
}



AbstractTaskItem *Tasks::createAbstractItem(TaskManager::AbstractItemPtr groupableItem)
{
    AbstractTaskItem *item = 0;
    if (!m_items.contains(groupableItem)) {
        if (groupableItem->isGroupItem()) {
            item = dynamic_cast<AbstractTaskItem*>(createTaskGroup(dynamic_cast<GroupPtr>(groupableItem)));
        } else {
            TaskManager::TaskItem* task = dynamic_cast<TaskManager::TaskItem*>(groupableItem);
            if (!task->task()) { //startuptask
                item = dynamic_cast<AbstractTaskItem*>(createStartingTask(task));
            } else {
                item = dynamic_cast<AbstractTaskItem*>(createWindowTask(task));
            }
        }
        m_items.insert(groupableItem,item);
    } else {
        item = m_items.value(groupableItem);
    }

    if (!item) {
        //kDebug() << "invalid Item";
        return 0;
    }

    return item;
}

TaskGroupItem *Tasks::createTaskGroup(GroupPtr group)
{
    if (!group) {
        //kDebug() << "null group";
        return 0;
    }
    Q_ASSERT(m_rootGroupItem);

    TaskGroupItem *item;
    if (!m_groupTaskItems.contains(group)) {
        item = new TaskGroupItem(m_rootGroupItem, this, m_showTooltip);
        item->setGroup(group);
        m_groupTaskItems.insert(group, item);
    } else {
        item = m_groupTaskItems.value(group);
    }
    return item;
}


void Tasks::constraintsEvent(Plasma::Constraints constraints)
{
    //kDebug();
    if (m_groupManager && constraints & Plasma::ScreenConstraint) {
        Plasma::Containment* appletContainment = containment();
        if (appletContainment) {
            m_groupManager->setScreen(appletContainment->screen());
        }
    }

    if (constraints & Plasma::SizeConstraint) {
        adjustGroupingStrategy();
    }

    emit constraintsChanged(constraints);
}

Plasma::FrameSvg* Tasks::itemBackground()
{
    if (!m_taskItemBackground) {
        QString tasksThemePath = Plasma::Theme::defaultTheme()->imagePath("widgets/tasks");

        if (!tasksThemePath.isEmpty()) {
            m_taskItemBackground = new Plasma::FrameSvg(this);
            m_taskItemBackground->setImagePath(tasksThemePath);
            m_taskItemBackground->setCacheAllRenderedFrames(true);
        }
    }

    return m_taskItemBackground;
}

void Tasks::resizeItemBackground(const QSizeF &size)
{
  //kDebug();
    if (!m_taskItemBackground) {
        itemBackground();

        if (!m_taskItemBackground) {
            //kDebug() << "Error1";
            return;
        }
    }

    if (m_taskItemBackground->frameSize() == size) {
        //kDebug() << "Error2";
        return;
    }


    //m_taskItemBackground->clearCache();
    m_taskItemBackground->resizeFrame(size);
    //get the margins now
    m_taskItemBackground->getMargins(m_leftMargin, m_topMargin, m_rightMargin, m_bottomMargin);
    //if the task height is too little shrink the top and bottom margins
    if (size.height() - m_topMargin - m_bottomMargin < KIconLoader::SizeSmall) {
        m_topMargin = m_bottomMargin = qMax(1, int((size.height() - KIconLoader::SizeSmall)/2));
    }
}

KColorScheme *Tasks::colorScheme()
{
    if (!m_colorScheme) {
        m_colorScheme = new KColorScheme(QPalette::Active, KColorScheme::View, Plasma::Theme::defaultTheme()->colorScheme());
    }

    return m_colorScheme;
}


QSizeF Tasks::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    if (which == Qt::PreferredSize) {
        return m_rootGroupItem->preferredSize();
    } else {
        return Plasma::Applet::sizeHint(which, constraint);
    }
}

void Tasks::adjustGroupingStrategy()
{
    //FIXME: should use AbstractTaskItem::basicPreferredSize() but it seems to cause crashes
    QSize itemSize = QSize(300, 30);
    m_groupManager->setFullLimit(((size().width()*size().height()) / (itemSize.width()*itemSize.height())));  
    //kDebug() << ((size().width()*size().height()) / (itemSize.width()*itemSize.height()));
}

void Tasks::changeSizeHint(Qt::SizeHint which)
{
    emit sizeHintChanged(which);
    adjustGroupingStrategy();
}

void Tasks::createConfigurationInterface(KConfigDialog *parent)
{
     QWidget *widget = new QWidget;
     m_ui.setupUi(widget);
     parent->setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Apply);
     connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
     connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
     parent->addPage(widget, parent->windowTitle(), icon());

    m_ui.showTooltip->setChecked(m_showTooltip);
    m_ui.showOnlyCurrentDesktop->setChecked(m_groupManager->showOnlyCurrentDesktop());
    m_ui.showOnlyCurrentScreen->setChecked(m_groupManager->showOnlyCurrentScreen());
    m_ui.showOnlyMinimized->setChecked(m_groupManager->showOnlyMinimized());
    m_ui.fillRows->setChecked(m_rootGroupItem->forceRows());

    m_ui.groupingStrategy->addItem(i18n("Do Not Group"),QVariant(TaskManager::GroupManager::NoGrouping));
    m_ui.groupingStrategy->addItem(i18n("Manually"),QVariant(TaskManager::GroupManager::ManualGrouping));
    m_ui.groupingStrategy->addItem(i18n("By Program Name"),QVariant(TaskManager::GroupManager::ProgramGrouping));

    connect(m_ui.groupingStrategy, SIGNAL(currentIndexChanged(int)), this, SLOT(dialogGroupingChanged(int)));

    switch (m_groupManager->groupingStrategy()) {
        case TaskManager::GroupManager::NoGrouping:
            m_ui.groupingStrategy->setCurrentIndex(0);
            break;
        case TaskManager::GroupManager::ManualGrouping:
            m_ui.groupingStrategy->setCurrentIndex(1);
            break;
        case TaskManager::GroupManager::ProgramGrouping:
            m_ui.groupingStrategy->setCurrentIndex(2);
            break;
        default:
             m_ui.groupingStrategy->setCurrentIndex(-1);
    }
    kDebug() << m_groupManager->groupingStrategy();

    m_ui.groupWhenFull->setChecked(m_groupManager->onlyGroupWhenFull());


    m_ui.sortingStrategy->addItem(i18n("Do Not Sort"),QVariant(TaskManager::GroupManager::NoSorting));
    m_ui.sortingStrategy->addItem(i18n("Manually"),QVariant(TaskManager::GroupManager::ManualSorting));
    m_ui.sortingStrategy->addItem(i18n("Alphabetically"),QVariant(TaskManager::GroupManager::AlphaSorting));
    m_ui.sortingStrategy->addItem(i18n("By Desktop"),QVariant(TaskManager::GroupManager::DesktopSorting));


    switch (m_groupManager->sortingStrategy()) {
        case TaskManager::GroupManager::NoSorting:
            m_ui.sortingStrategy->setCurrentIndex(0);
            break;
        case TaskManager::GroupManager::ManualSorting:
            m_ui.sortingStrategy->setCurrentIndex(1);
            break;
        case TaskManager::GroupManager::AlphaSorting:
            m_ui.sortingStrategy->setCurrentIndex(2);
            break;
        case TaskManager::GroupManager::DesktopSorting:
            m_ui.sortingStrategy->setCurrentIndex(3);
            break;
        default:
             m_ui.sortingStrategy->setCurrentIndex(-1);
    }
 //   kDebug() << m_groupManager->sortingStrategy();
    m_ui.maxRows->setValue(m_rootGroupItem->maxRows());
}

void Tasks::dialogGroupingChanged(int index)
{
     m_ui.groupWhenFull->setEnabled(static_cast<TaskManager::GroupManager::TaskGroupingStrategy>(m_ui.groupingStrategy->itemData(index).toInt()) == TaskManager::GroupManager::ProgramGrouping);
}

void Tasks::configAccepted()
{
    kDebug();
    bool changed = false;

    if (m_groupManager->showOnlyCurrentDesktop() != (m_ui.showOnlyCurrentDesktop->isChecked())) {
        m_groupManager->setShowOnlyCurrentDesktop(!m_groupManager->showOnlyCurrentDesktop());
        KConfigGroup cg = config();
        cg.writeEntry("showOnlyCurrentDesktop", m_groupManager->showOnlyCurrentDesktop());
        changed = true;
    }
    if (m_groupManager->showOnlyCurrentScreen() != (m_ui.showOnlyCurrentScreen->isChecked())) {
        m_groupManager->setShowOnlyCurrentScreen(!m_groupManager->showOnlyCurrentScreen());
        KConfigGroup cg = config();
        cg.writeEntry("showOnlyCurrentScreen", m_groupManager->showOnlyCurrentScreen());
        changed = true;
    }
    if (m_groupManager->showOnlyMinimized() != (m_ui.showOnlyMinimized->isChecked())) {
        m_groupManager->setShowOnlyMinimized(!m_groupManager->showOnlyMinimized());
        KConfigGroup cg = config();
        cg.writeEntry("showOnlyMinimized", m_groupManager->showOnlyMinimized());
        changed = true;
    }

    if (m_groupManager->groupingStrategy() != (m_ui.groupingStrategy->currentIndex())) {
        m_groupManager->setGroupingStrategy(static_cast<TaskManager::GroupManager::TaskGroupingStrategy>(m_ui.groupingStrategy->itemData(m_ui.groupingStrategy->currentIndex()).toInt()));
        KConfigGroup cg = config();
        cg.writeEntry("groupingStrategy", static_cast<int>(m_groupManager->groupingStrategy()));
        changed = true;
    }

    if (m_groupManager->onlyGroupWhenFull() != m_ui.groupWhenFull->isChecked()) {
        adjustGroupingStrategy();
        m_groupManager->setOnlyGroupWhenFull(m_ui.groupWhenFull->isChecked());
        KConfigGroup cg = config();
        cg.writeEntry("groupWhenFull", m_groupManager->onlyGroupWhenFull());
        changed = true;
    }

    if (m_groupManager->sortingStrategy() != (m_ui.sortingStrategy->currentIndex())) {
        m_groupManager->setSortingStrategy(static_cast<TaskManager::GroupManager::TaskSortingStrategy>(m_ui.sortingStrategy->itemData(m_ui.sortingStrategy->currentIndex()).toInt()));
        KConfigGroup cg = config();
        cg.writeEntry("sortingStrategy", static_cast<int>(m_groupManager->sortingStrategy()));
        changed = true;
    }

    if (m_rootGroupItem->maxRows() != (m_ui.maxRows->value())) {
        m_rootGroupItem->setMaxRows(m_ui.maxRows->value());
        KConfigGroup cg = config();
        cg.writeEntry("maxRows", m_rootGroupItem->maxRows());
        changed = true;
    }

    if (m_rootGroupItem->forceRows() != m_ui.fillRows->isChecked()) {
        m_rootGroupItem->setForceRows(m_ui.fillRows->isChecked());
        KConfigGroup cg = config();
        cg.writeEntry("forceRows", m_rootGroupItem->forceRows());
        changed = true;
    }

    if (m_showTooltip != (m_ui.showTooltip->checkState() == Qt::Checked)) {
        m_showTooltip = !m_showTooltip;
        foreach (AbstractTaskItem *taskItem, m_windowTaskItems) {
            WindowTaskItem *windowTaskItem = dynamic_cast<WindowTaskItem *>(taskItem);
            if (windowTaskItem) {
                windowTaskItem->setShowTooltip(m_showTooltip);
            }
        }
        KConfigGroup cg = config();
        cg.writeEntry("showTooltip", m_showTooltip);
        changed = true;
    }

    if (changed) {
        emit settingsChanged();
        emit configNeedsSaving();
        update();
    }
}



void Tasks::themeRefresh()
{
    delete m_taskItemBackground;
    m_taskItemBackground = 0;

    delete m_colorScheme;
    m_colorScheme = 0;

    foreach (WindowTaskItem *taskItem, m_windowTaskItems) {
        taskItem->update(); //FIXME: do we also have to update the group items? probably yes
    }
}


WindowTaskItem* Tasks::windowItem(TaskPtr task)
{
    if (m_windowTaskItems.contains(task)) {
        return m_windowTaskItems.value(task);
    }
    //kDebug() << "item not found";
    return 0;
}

TaskGroupItem* Tasks::groupItem(GroupPtr group)
{
    if (m_groupTaskItems.contains(group)) {
        return m_groupTaskItems.value(group);
    }
    //kDebug() << "item not found";
    return 0;
}

AbstractTaskItem* Tasks::abstractItem(AbstractItemPtr item)
{
    if (m_items.contains(item)) {
        return m_items.value(item);
    }
    //kDebug() << "item not found";
    return 0;
}

TaskGroupItem* Tasks::rootGroupItem()
{
    return m_rootGroupItem;
}


K_EXPORT_PLASMA_APPLET(tasks, Tasks)

#include "tasks.moc"