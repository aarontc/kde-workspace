/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 1999, 2000 Matthias Ettrich <ettrich@kde.org>
Copyright (C) 2003 Lubos Lunak <l.lunak@kde.org>
Copyright (C) 2009 Martin Gräßlin <mgraesslin@kde.org>

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

#ifndef KWIN_TABBOX_H
#define KWIN_TABBOX_H

#include <QKeySequence>
#include <QTimer>
#include <QModelIndex>
#include "utils.h"
#include "tabbox/tabboxhandler.h"

class KActionCollection;
class KConfigGroup;
class QAction;
class QKeyEvent;

struct xcb_button_press_event_t;
struct xcb_motion_notify_event_t;

namespace KWin
{

class Workspace;
class Client;
namespace TabBox
{
class DesktopChainManager;
class TabBoxConfig;
class TabBox;
class TabBoxHandlerImpl : public TabBoxHandler
{
public:
    explicit TabBoxHandlerImpl(TabBox* tabBox);
    virtual ~TabBoxHandlerImpl();

    virtual int activeScreen() const;
    virtual QWeakPointer< TabBoxClient > activeClient() const;
    virtual int currentDesktop() const;
    virtual QString desktopName(TabBoxClient* client) const;
    virtual QString desktopName(int desktop) const;
    virtual bool isKWinCompositing() const;
    virtual QWeakPointer< TabBoxClient > nextClientFocusChain(TabBoxClient* client) const;
    virtual QWeakPointer< TabBoxClient > firstClientFocusChain() const;
    virtual bool isInFocusChain (TabBoxClient* client) const;
    virtual int nextDesktopFocusChain(int desktop) const;
    virtual int numberOfDesktops() const;
    virtual TabBoxClientList stackingOrder() const;
    virtual void elevateClient(TabBoxClient* c, WId tabbox, bool elevate) const;
    virtual void raiseClient(TabBoxClient *client) const;
    virtual void restack(TabBoxClient *c, TabBoxClient *under);
    virtual QWeakPointer< TabBoxClient > clientToAddToList(KWin::TabBox::TabBoxClient* client, int desktop) const;
    virtual QWeakPointer< TabBoxClient > desktopClient() const;
    virtual void activateAndClose();

private:
    bool checkDesktop(TabBoxClient* client, int desktop) const;
    bool checkActivity(TabBoxClient* client) const;
    bool checkApplications(TabBoxClient* client) const;
    bool checkMinimized(TabBoxClient* client) const;
    bool checkMultiScreen(TabBoxClient* client) const;

    TabBox* m_tabBox;
    DesktopChainManager* m_desktopFocusChain;
};

class TabBoxClientImpl : public TabBoxClient
{
public:
    explicit TabBoxClientImpl(Client *client);
    virtual ~TabBoxClientImpl();

    virtual QString caption() const;
    virtual QPixmap icon(const QSize& size = QSize(32, 32)) const;
    virtual WId window() const;
    virtual bool isMinimized() const;
    virtual int x() const;
    virtual int y() const;
    virtual int width() const;
    virtual int height() const;
    virtual bool isCloseable() const;
    virtual void close();
    virtual bool isFirstInTabBox() const;

    Client* client() const {
        return m_client;
    }

private:
    Client* m_client;
};

class TabBox : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.kwin")
public:
    ~TabBox();

    Client* currentClient();
    ClientList currentClientList();
    int currentDesktop();
    QList< int > currentDesktopList();

    void setCurrentClient(Client* newClient);
    void setCurrentDesktop(int newDesktop);

    void setMode(TabBoxMode mode);
    TabBoxMode mode() const {
        return m_tabBoxMode;
    }

    void reset(bool partial_reset = false);
    void nextPrev(bool next = true);

    void delayedShow();
    void hide(bool abort = false);

    /*!
    Increase the reference count, preventing the default tabbox from showing.

    \sa unreference(), isDisplayed()
    */
    void reference() {
        ++m_displayRefcount;
    }
    /*!
    Decrease the reference count.  Only when the reference count is 0 will
    the default tab box be shown.
    */
    void unreference() {
        --m_displayRefcount;
    }
    /*!
    Returns whether the tab box is being displayed, either natively or by an
    effect.

    \sa reference(), unreference()
    */
    bool isDisplayed() const {
        return m_displayRefcount > 0;
    };

    bool handleMouseEvent(xcb_button_press_event_t *e);
    bool handleMouseEvent(xcb_motion_notify_event_t *e);
    void grabbedKeyEvent(QKeyEvent* event);

    bool isGrabbed() const {
        return m_tabGrab || m_desktopGrab;
    };

    void initShortcuts(KActionCollection* keys);

    Client* nextClientStatic(Client*) const;
    Client* previousClientStatic(Client*) const;
    int nextDesktopStatic(int iDesktop) const;
    int previousDesktopStatic(int iDesktop) const;
    void keyPress(int key);
    void keyRelease(const xcb_key_release_event_t *ev);

    static TabBox *self();
    static TabBox *create(QObject *parent);

public Q_SLOTS:
    void show();
    /**
     * Only for DBus Interface to start primary KDE Walk through windows.
     * @param modal Whether the TabBox should grab keyboard and mouse, that is go into modal
     * mode or whether the TabBox is controlled externally (e.g. through an effect).
     * @param layout The name of the layout to use, if null string (default) the configured layout is used
     **/
    Q_SCRIPTABLE void open(bool modal = true, const QString &layout = QString());
    /**
     * Opens the TabBox view embedded on a different window. This implies non-modal mode.
     * The geometry of the TabBox is determined by offset, size and the alignment flags.
     * If the alignment flags are set to center the view scales with the container. That is if
     * the window where the TabBox is embedded onto resizes, the TabBox resizes, too.
     * The alignment in combination with the offset determines to what border the TabBox is snapped.
     * E.g. if horizontal alignment is right the offset is interpreted as the offset between right
     * corner of TabBox view and the container view. When the container changes its geometry this
     * offset is kept. So the offset on the left side would increase.
     * @param wid The window Id the TabBox should be embedded onto
     * @param offset The offset to one of the size borders
     * @param size The size of the TabBox. To use the same size as the container, set alignment to center
     * @param horizontalAlignment Either Qt::AlignLeft, Qt::AlignHCenter or Qt::AlignRight
     * @param verticalAlignment Either Qt::AlignTop, Qt::AlignVCenter or Qt::AlignBottom
     * @param layout The name of the layout to use, if null string (default) the configured layout is used
     **/
    Q_SCRIPTABLE void openEmbedded(qulonglong wid, QPoint offset, QSize size, int horizontalAlignment, int verticalAlignment, const QString &layout = QString());
    Q_SCRIPTABLE void close(bool abort = false);
    Q_SCRIPTABLE void accept();
    Q_SCRIPTABLE void reject();
    void slotWalkThroughDesktops();
    void slotWalkBackThroughDesktops();
    void slotWalkThroughDesktopList();
    void slotWalkBackThroughDesktopList();
    void slotWalkThroughWindows();
    void slotWalkBackThroughWindows();
    void slotWalkThroughWindowsAlternative();
    void slotWalkBackThroughWindowsAlternative();
    void slotWalkThroughCurrentAppWindows();
    void slotWalkBackThroughCurrentAppWindows();
    void slotWalkThroughCurrentAppWindowsAlternative();
    void slotWalkBackThroughCurrentAppWindowsAlternative();

    void handlerReady();

    bool toggle(ElectricBorder eb);

Q_SIGNALS:
    void tabBoxAdded(int);
    Q_SCRIPTABLE void tabBoxClosed();
    Q_SCRIPTABLE void itemSelected();
    void tabBoxUpdated();
    void tabBoxKeyEvent(QKeyEvent*);

private:
    explicit TabBox(QObject *parent);
    void setCurrentIndex(QModelIndex index, bool notifyEffects = true);
    void loadConfig(const KConfigGroup& config, TabBoxConfig& tabBoxConfig);

    bool startKDEWalkThroughWindows(TabBoxMode mode);   // TabBoxWindowsMode | TabBoxWindowsAlternativeMode
    bool startWalkThroughDesktops(TabBoxMode mode);   // TabBoxDesktopMode | TabBoxDesktopListMode
    bool startWalkThroughDesktops();
    bool startWalkThroughDesktopList();
    void navigatingThroughWindows(bool forward, const QKeySequence &shortcut, TabBoxMode mode);   // TabBoxWindowsMode | TabBoxWindowsAlternativeMode
    void KDEWalkThroughWindows(bool forward);
    void CDEWalkThroughWindows(bool forward);
    void walkThroughDesktops(bool forward);
    void KDEOneStepThroughWindows(bool forward, TabBoxMode mode);   // TabBoxWindowsMode | TabBoxWindowsAlternativeMode
    void oneStepThroughDesktops(bool forward, TabBoxMode mode);   // TabBoxDesktopMode | TabBoxDesktopListMode
    void oneStepThroughDesktops(bool forward);
    void oneStepThroughDesktopList(bool forward);
    bool establishTabBoxGrab();
    void removeTabBoxGrab();
    void modalActionsSwitch(bool enabled);
    template <typename Slot>
    void key(KActionCollection *keys, const char *actionName, Slot slot, const QKeySequence &shortcut = QKeySequence());

private Q_SLOTS:
    void reconfigure();
    void globalShortcutChanged(QAction *action, const QKeySequence &seq);

private:
    TabBoxMode m_tabBoxMode;
    TabBoxHandlerImpl* m_tabBox;
    bool m_delayShow;
    int m_delayShowTime;

    QTimer m_delayedShowTimer;
    int m_displayRefcount;

    TabBoxConfig m_defaultConfig;
    TabBoxConfig m_alternativeConfig;
    TabBoxConfig m_defaultCurrentApplicationConfig;
    TabBoxConfig m_alternativeCurrentApplicationConfig;
    TabBoxConfig m_desktopConfig;
    TabBoxConfig m_desktopListConfig;
    // false if an effect has referenced the tabbox
    // true if tabbox is active (independent on showTabbox setting)
    bool m_isShown;
    bool m_desktopGrab;
    bool m_tabGrab;
    // true if tabbox is in modal mode which does not require holding a modifier
    bool m_noModifierGrab;
    QKeySequence m_cutWalkThroughDesktops, m_cutWalkThroughDesktopsReverse;
    QKeySequence m_cutWalkThroughDesktopList, m_cutWalkThroughDesktopListReverse;
    QKeySequence m_cutWalkThroughWindows, m_cutWalkThroughWindowsReverse;
    QKeySequence m_cutWalkThroughWindowsAlternative, m_cutWalkThroughWindowsAlternativeReverse;
    QKeySequence m_cutWalkThroughCurrentAppWindows, m_cutWalkThroughCurrentAppWindowsReverse;
    QKeySequence m_cutWalkThroughCurrentAppWindowsAlternative, m_cutWalkThroughCurrentAppWindowsAlternativeReverse;
    bool m_forcedGlobalMouseGrab;
    bool m_ready; // indicates whether the config is completely loaded
    QList<ElectricBorder> m_borderActivate, m_borderAlternativeActivate;

    static TabBox *s_self;
};

inline
TabBox *TabBox::self()
{
    return s_self;
}

} // namespace TabBox
} // namespace
#endif
