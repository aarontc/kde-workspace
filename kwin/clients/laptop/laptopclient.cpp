/*
 * Laptop KWin Decoration
 *
 * Copyright (c) 2005 Sandro Giessl <sandro@giessl.com>
 * Port of this decoration to KDE 3.2, accessibility enhancement are
 * Copyright (c) 2003 Luciano Montanaro <mikelima@cirulla.net>
 */

#include <kconfig.h> // up here to avoid X11 header conflict :P
#include "laptopclient.h"
#include <qdrawutil.h>
#include <kpixmapeffect.h>
#include <kdrawutil.h>
#include <kglobal.h>
#include <klocale.h>
#include <qbitmap.h>

namespace Laptop {

static const unsigned char iconify_bits[] = {
    0xff, 0xff, 0x00, 0xff, 0xff, 0x7e, 0x3c, 0x18};

static const unsigned char close_bits[] = {
    0x42, 0xe7, 0x7e, 0x3c, 0x3c, 0x7e, 0xe7, 0x42};

static const unsigned char maximize_bits[] = {
    0x18, 0x3c, 0x7e, 0xff, 0xff, 0x00, 0xff, 0xff };

static const unsigned char r_minmax_bits[] = {
    0x0c, 0x18, 0x33, 0x67, 0xcf, 0x9f, 0x3f, 0x3f};

static const unsigned char l_minmax_bits[] = {
    0x30, 0x18, 0xcc, 0xe6, 0xf3, 0xf9, 0xfc, 0xfc};

static const unsigned char question_bits[] = {
    0x3c, 0x66, 0x60, 0x30, 0x18, 0x00, 0x18, 0x18};

static const unsigned char unsticky_bits[] = {
   0x3c, 0x42, 0x99, 0xbd, 0xbd, 0x99, 0x42, 0x3c};

static const unsigned char sticky_bits[] = {
   0x3c, 0x42, 0x81, 0x81, 0x81, 0x81, 0x42, 0x3c};

static QPixmap *titlePix;
static KPixmap *aUpperGradient;
static KPixmap *iUpperGradient;
// buttons active, inactive, up, down, and 2 sizes :P
static KPixmap *btnPix1;
static KPixmap *iBtnPix1;
static KPixmap *btnDownPix1;
static KPixmap *iBtnDownPix1;
static KPixmap *btnPix2;
static KPixmap *btnDownPix2;
static KPixmap *iBtnPix2;
static KPixmap *iBtnDownPix2;
static QColor btnForeground;

static int titleHeight = 14;
static int btnWidth1 = 17;
static int btnWidth2 = 27;

static int handleSize = 8;	// the resize handle size in pixels

static bool pixmaps_created = false;

// =====================================

extern "C" KDE_EXPORT KDecorationFactory* create_factory()
{
    return new Laptop::LaptopClientFactory();
}

// =====================================

static inline const KDecorationOptions* options()
{
    return KDecoration::options();
}

static void drawButtonFrame(KPixmap *pix, const QColorGroup &g, bool sunken)
{
    QPainter p;
    int w = pix->width();
    int h = pix->height();
    int x2 = w-1;
    int y2 = h-1;
    p.begin(pix);

    if(sunken){
        qDrawShadePanel(&p, 0, 0, w, h, g, true, 2);
    }
    else{
        p.setPen(g.dark());
        p.drawRect(0, 0, w-1, h-1);
        p.setPen(g.light());
        p.drawLine(x2, 0, x2, y2);
        p.drawLine(0, y2, x2, y2);
        p.drawLine(1, 1, x2-2, 1);
        p.drawLine(1, 1, 1, y2-2);
        p.end();
    }
}

static void create_pixmaps()
{
    if(pixmaps_created)
        return;
    pixmaps_created = true;

    titleHeight = QFontMetrics(options()->font(true)).height() + 2;
    if (titleHeight < handleSize) titleHeight = handleSize;
    titleHeight &= ~1; // Make title height even
    if (titleHeight < 14) titleHeight = 14;

    btnWidth1 = titleHeight + 3;
    btnWidth2 = 3*titleHeight/2 + 6;

    // titlebar
    QPainter p;
    QPainter maskPainter;
    int i, x, y;
    titlePix = new QPixmap(33, 12);
    QBitmap mask(33, 12);
    mask.fill(Qt::color0);

    p.begin(titlePix);
    maskPainter.begin(&mask);
    maskPainter.setPen(Qt::color1);
    for(i=0, y=2; i < 3; ++i, y+=4){
        for(x=1; x <= 33; x+=3){
            p.setPen(options()->color(KDecoration::ColorTitleBar, true).light(150));
            p.drawPoint(x, y);
            maskPainter.drawPoint(x, y);
            p.setPen(options()->color(KDecoration::ColorTitleBar, true).dark(150));
            p.drawPoint(x+1, y+1);
            maskPainter.drawPoint(x+1, y+1);
        }
    }
    p.end();
    maskPainter.end();
    titlePix->setMask(mask);

    if(QPixmap::defaultDepth() > 8){
        aUpperGradient = new KPixmap;
        aUpperGradient->resize(32, titleHeight+2);
        iUpperGradient = new KPixmap;
        iUpperGradient->resize(32, titleHeight+2);
        QColor bgColor = options()->color(KDecoration::ColorTitleBar, true);
        KPixmapEffect::gradient(*aUpperGradient,
                                bgColor.light(120),
                                bgColor.dark(120),
                                KPixmapEffect::VerticalGradient);
        bgColor = options()->color(KDecoration::ColorTitleBar, false);
        KPixmapEffect::gradient(*iUpperGradient,
                                bgColor.light(120),
                                bgColor.dark(120),
                                KPixmapEffect::VerticalGradient);
    }
    // buttons (active/inactive, sunken/unsunken, 2 sizes each)
    QColorGroup g = options()->colorGroup(KDecoration::ColorButtonBg, true);
    QColor c = g.background();
    btnPix1 = new KPixmap;
    btnPix1->resize(btnWidth1, titleHeight);
    btnDownPix1 = new KPixmap;
    btnDownPix1->resize(btnWidth1, titleHeight);
    btnPix2 = new KPixmap;
    btnPix2->resize(btnWidth2, titleHeight);
    btnDownPix2 = new KPixmap;
    btnDownPix2->resize(btnWidth2, titleHeight);
    iBtnPix1 = new KPixmap;
    iBtnPix1->resize(btnWidth1, titleHeight);
    iBtnDownPix1 = new KPixmap;
    iBtnDownPix1->resize(btnWidth1, titleHeight);
    iBtnPix2 = new KPixmap;
    iBtnPix2->resize(btnWidth2, titleHeight);
    iBtnDownPix2 = new KPixmap;
    iBtnDownPix2->resize(btnWidth2, titleHeight);
    if(QPixmap::defaultDepth() > 8){
        KPixmapEffect::gradient(*btnPix1, c.light(120), c.dark(130),
                                KPixmapEffect::DiagonalGradient);
        KPixmapEffect::gradient(*btnDownPix1, c.dark(130), c.light(120),
                                KPixmapEffect::DiagonalGradient);
        KPixmapEffect::gradient(*btnPix2, c.light(120), c.dark(130),
                                KPixmapEffect::DiagonalGradient);
        KPixmapEffect::gradient(*btnDownPix2, c.dark(130), c.light(120),
                                KPixmapEffect::DiagonalGradient);
        g = options()->colorGroup(KDecoration::ColorButtonBg, false);
        c = g.background();
        KPixmapEffect::gradient(*iBtnPix1, c.light(120), c.dark(130),
                                KPixmapEffect::DiagonalGradient);
        KPixmapEffect::gradient(*iBtnDownPix1, c.dark(130), c.light(120),
                                KPixmapEffect::DiagonalGradient);
        KPixmapEffect::gradient(*iBtnPix2, c.light(120), c.dark(130),
                                KPixmapEffect::DiagonalGradient);
        KPixmapEffect::gradient(*iBtnDownPix2, c.dark(130), c.light(120),
                                KPixmapEffect::DiagonalGradient);
    }
    else{
        btnPix1->fill(c.rgb());
        btnDownPix1->fill(c.rgb());
        btnPix2->fill(c.rgb());
        btnDownPix2->fill(c.rgb());
        g = options()->colorGroup(KDecoration::ColorButtonBg, false);
        c = g.background();
        iBtnPix1->fill(c.rgb());
        iBtnDownPix1->fill(c.rgb());
        iBtnPix2->fill(c.rgb());
        iBtnDownPix2->fill(c.rgb());
    }
    g = options()->colorGroup(KDecoration::ColorButtonBg, true);
    c = g.background();
    drawButtonFrame(btnPix1, g, false);
    drawButtonFrame(btnDownPix1, g, true);
    drawButtonFrame(btnPix2, g, false);
    drawButtonFrame(btnDownPix2, g, true);
    g = options()->colorGroup(KDecoration::ColorButtonBg, false);
    c = g.background();
    drawButtonFrame(iBtnPix1, g, false);
    drawButtonFrame(iBtnDownPix1, g, true);
    drawButtonFrame(iBtnPix2, g, false);
    drawButtonFrame(iBtnDownPix2, g, true);

    if(qGray(options()->color(KDecoration::ColorButtonBg, true).rgb()) > 128)
        btnForeground = Qt::black;
    else
        btnForeground = Qt::white;
}

static void delete_pixmaps()
{
    delete titlePix;
    if(aUpperGradient){
        delete aUpperGradient;
        delete iUpperGradient;
        delete btnPix1;
        delete btnDownPix1;
        delete iBtnPix1;
        delete iBtnDownPix1;
        delete btnPix2;
        delete btnDownPix2;
        delete iBtnPix2;
        delete iBtnDownPix2;
    }
    pixmaps_created = false;
}

// =====================================

LaptopButton::LaptopButton(ButtonType type, LaptopClient *parent, const char *name)
    : KCommonDecorationButton(type, parent, name)
{
    setBackgroundMode(QWidget::NoBackground);
}

void LaptopButton::reset(unsigned long changed)
{
    if (changed&DecorationReset || changed&ManualReset || changed&SizeChange || changed&StateChange) {
        switch (type() ) {
            case CloseButton:
                setBitmap(close_bits);
                break;
            case HelpButton:
                setBitmap(question_bits);
                break;
            case MinButton:
                setBitmap(iconify_bits);
                break;
            case MaxButton:
                if (isOn() ) {
                    setBitmap(isLeft() ? l_minmax_bits : r_minmax_bits);
                } else {
                    setBitmap(maximize_bits);
                }
                break;
            case OnAllDesktopsButton:
                setBitmap( isOn() ? unsticky_bits : sticky_bits );
                break;
            default:
                setBitmap(0);
                break;
        }

        this->update();
    }
}

void LaptopButton::setBitmap(const unsigned char *bitmap)
{
    if (bitmap)
        deco = QBitmap(8, 8, bitmap, true);
    else {
        deco = QBitmap(8,8);
        deco.fill(Qt::color0);
    }
    deco.setMask(deco);
    repaint();
}

void LaptopButton::drawButton(QPainter *p)
{
    bool smallBtn = width() == btnWidth1;
    if(btnPix1){
        if(decoration()->isActive()){
            if(isDown())
                p->drawPixmap(0, 0, smallBtn ? *btnDownPix1 : *btnDownPix2);
            else
                p->drawPixmap(0, 0, smallBtn ? *btnPix1 : *btnPix2);
        }
        else{
            if(isDown())
                p->drawPixmap(0, 0, smallBtn ? *iBtnDownPix1 : *iBtnDownPix2);
            else
                p->drawPixmap(0, 0, smallBtn ? *iBtnPix1 : *iBtnPix2);
        }
    }
    else{
        QColorGroup g = options()->colorGroup(KDecoration::ColorButtonBg, decoration()->isActive());
        int w = width();
        int h = height();
        p->fillRect(1, 1, w-2, h-2, isDown() ? g.mid() : g.button());
        p->setPen(isDown() ? g.dark() : g.light());
        p->drawLine(0, 0, w-1, 0);
        p->drawLine(0, 0, 0, w-1);
        p->setPen(isDown() ? g.light() : g.dark());
        p->drawLine(w-1, 0, w-1, h-1);
        p->drawLine(0, h-1, w-1, h-1);
    }

    p->setPen(btnForeground);
    int xOff = (width()-8)/2;
    int yOff = (height()-8)/2;
    p->drawPixmap(isDown() ? xOff+1: xOff, isDown() ? yOff+1 : yOff, deco);
}

// =====================================

void LaptopClient::reset(unsigned long changed)
{
    KCommonDecoration::reset(changed);
}

LaptopClient::LaptopClient(KDecorationBridge *b, KDecorationFactory *f)
    : KCommonDecoration(b, f)
{
}

LaptopClient::~LaptopClient()
{
}

QString LaptopClient::visibleName() const
{
    return i18n("Laptop");
}

QString LaptopClient::defaultButtonsLeft() const
{
    return "X";
}

QString LaptopClient::defaultButtonsRight() const
{
    return "HSIA";
}

bool LaptopClient::decorationBehaviour(DecorationBehaviour behaviour) const
{
    switch (behaviour) {
        case DB_MenuClose:
            return false;

        case DB_WindowMask:
            return true;

        case DB_ButtonHide:
            return true;

        default:
            return KCommonDecoration::decorationBehaviour(behaviour);
    }
}

int LaptopClient::layoutMetric(LayoutMetric lm, bool respectWindowState, const KCommonDecorationButton *btn) const
{
    switch (lm) {
        case LM_TitleEdgeLeft:
        case LM_TitleEdgeRight:
        case LM_BorderLeft:
        case LM_BorderRight:
            return 4;

        case LM_BorderBottom:
            return mustDrawHandle() ? handleSize : 4;

        case LM_TitleEdgeTop:
            return 3;

        case LM_TitleEdgeBottom:
            return 1;

        case LM_TitleBorderLeft:
        case LM_TitleBorderRight:
            return 0;

        case LM_ButtonWidth:
        {
            if (btn && (btn->type()==HelpButton||btn->type()==OnAllDesktopsButton) ) {
                return btnWidth1;
            } else {
                return btnWidth2;
            }
        }

        case LM_ButtonHeight:
        case LM_TitleHeight:
            if (isToolWindow() )
                return titleHeight-2;
            else
                return titleHeight;

        case LM_ButtonSpacing:
            return 0;

        case LM_ExplicitButtonSpacer:
            return 0;

        default:
            return KCommonDecoration::layoutMetric(lm, respectWindowState, btn);
    }
}

KCommonDecorationButton *LaptopClient::createButton(ButtonType type)
{
    switch (type) {
        case OnAllDesktopsButton:
            return new LaptopButton(OnAllDesktopsButton, this, "on_all_desktops");

        case HelpButton:
            return new LaptopButton(HelpButton, this, "help");

        case MinButton:
            return new LaptopButton(MinButton, this, "minimize");

        case MaxButton:
            return new LaptopButton(MaxButton, this, "maximize");

        case CloseButton:
            return new LaptopButton(CloseButton, this, "close");

        default:
            return 0;
    }
}

void LaptopClient::init()
{
    bufferDirty = true;

    KCommonDecoration::init();
}

void LaptopClient::paintEvent( QPaintEvent* )
{
    QPainter p(widget());
    QColorGroup g = options()->colorGroup(KDecoration::ColorFrame, isActive());

    QRect r(widget()->rect());
    p.setPen(Qt::black);
    p.drawRect(r);

    // fill mid frame...
    p.setPen(g.background() );
    p.drawLine(r.x()+2, r.y()+2, r.right()-2, r.y()+2);
    p.drawLine(r.left()+2, r.y()+3, r.left()+2, r.bottom()-layoutMetric(LM_BorderBottom)+1 );
    p.drawLine(r.right()-2, r.y()+3, r.right()-2, r.bottom()-layoutMetric(LM_BorderBottom)+1 );
    p.drawLine(r.left()+3, r.y()+3, r.left()+3, r.y()+layoutMetric(LM_TitleEdgeTop)+layoutMetric(LM_TitleHeight)+layoutMetric(LM_TitleEdgeTop) );
    p.drawLine(r.right()-3, r.y()+3, r.right()-3, r.y()+layoutMetric(LM_TitleEdgeTop)+layoutMetric(LM_TitleHeight)+layoutMetric(LM_TitleEdgeTop) );
    if (!mustDrawHandle() )
        p.drawLine(r.left()+1, r.bottom()-2, r.right()-1, r.bottom()-2);

    // outer frame
    p.setPen(g.light());
    p.drawLine(r.x()+1, r.y()+1, r.right()-1, r.y()+1);
    p.drawLine(r.x()+1, r.y()+1, r.x()+1, r.bottom()-1);
    p.setPen(g.dark());
    p.drawLine(r.right()-1, r.y()+1, r.right()-1, r.bottom()-1);
    p.drawLine(r.x()+1, r.bottom()-1, r.right()-1, r.bottom()-1);

    int th = titleHeight;
    int bb = handleSize + 2; // Bottom border
    int bs = handleSize - 2; // inner size of bottom border
    if (!mustDrawHandle()) {
	bb = 6;
	bs = 0;
    }
    if ( isToolWindow() )
	th -= 2;

    // inner rect
    p.drawRect(r.x() + 3, r.y() + th + 3, r.width() - 6, r.height() - th - bb);

    // handles
    if (mustDrawHandle()) {
	if (r.width() > 3*handleSize + 20) {
	    int range = 8 + 3*handleSize/2;
	    qDrawShadePanel(&p, r.x() + 1, r.bottom() - bs, range, 
		    handleSize - 2, g, false, 1, &g.brush(QColorGroup::Mid));
	    qDrawShadePanel(&p, r.x() + range + 1, r.bottom() - bs,
		    r.width() - 2*range - 2, handleSize - 2, g, false, 1,
		    isActive() ? &g.brush(QColorGroup::Background) :
				 &g.brush(QColorGroup::Mid));
	    qDrawShadePanel(&p, r.right() - range, r.bottom() - bs,
		    range, bs, g, false, 1, &g.brush(QColorGroup::Mid));
	} else {
	    qDrawShadePanel(&p, r.x() + 1, r.bottom() - bs,
		    r.width() - 2, bs, g, false, 1,
		    isActive() ?  &g.brush(QColorGroup::Background) :
				  &g.brush(QColorGroup::Mid));
	}
    }

    r = titleRect();

    if(isActive()){
        updateActiveBuffer();
        p.drawPixmap(r.x(), r.y(), activeBuffer);
        p.setPen(g.background());
        p.drawPoint(r.x(), r.y());
        p.drawPoint(r.right(), r.y());
        p.drawLine(r.right()+1, r.y(), r.right()+1, r.bottom());
    }
    else{
        if(iUpperGradient)
            p.drawTiledPixmap(r.x(), r.y(), r.width(), r.height()-1,
                              *iUpperGradient);
        else
            p.fillRect(r.x(), r.y(), r.width(), r.height()-1,
                       options()->color(KDecoration::ColorTitleBar, false));

        p.setFont(options()->font(false, isToolWindow() ));
        QFontMetrics fm(options()->font(false));
        g = options()->colorGroup(KDecoration::ColorTitleBar, false);
        if(iUpperGradient)
            p.drawTiledPixmap(r.x()+((r.width()-fm.width(caption()))/2)-4,
                              r.y(), fm.width(caption())+8, r.height()-1,
                              *iUpperGradient);
        else
            p.fillRect(r.x()+((r.width()-fm.width(caption()))/2)-4, r.y(),
                       fm.width(caption())+8, r.height()-1,
                       g.brush(QColorGroup::Background));
        p.setPen(g.mid());
        p.drawLine(r.x(), r.y(), r.right(), r.y());
        p.drawLine(r.x(), r.y(), r.x(), r.bottom());
        p.setPen(g.button());
        p.drawLine(r.right(), r.y(), r.right(), r.bottom());
        p.drawLine(r.x(), r.bottom(), r.right(), r.bottom());
        p.setPen(options()->color(KDecoration::ColorFont, false));
        p.drawText(r.x(), r.y(), r.width(), r.height()-1,
                   AlignCenter, caption() );
        g = options()->colorGroup(KDecoration::ColorFrame, true);
        p.setPen(g.background());
        p.drawPoint(r.x(), r.y());
        p.drawPoint(r.right(), r.y());
        p.drawLine(r.right()+1, r.y(), r.right()+1, r.bottom());
    }
}

QRegion LaptopClient::cornerShape(WindowCorner corner)
{
    switch (corner) {
        case WC_TopLeft:
            return QRect(0, 0, 1, 1);

        case WC_TopRight:
            return QRect(width()-1, 0, 1, 1);

        case WC_BottomLeft:
            return QRect(0, height()-1, 1, 1);

        case WC_BottomRight:
            return QRect(width()-1, height()-1, 1, 1);

        default:
            return QRegion();
    }

}

bool LaptopClient::mustDrawHandle() const 
{ 
    bool drawSmallBorders = !options()->moveResizeMaximizedWindows();
    if (drawSmallBorders && (maximizeMode() & MaximizeVertical)) {
	return false;
    } else {
	return isResizable();
    }
}

void LaptopClient::updateActiveBuffer( )
{
    QRect rTitle = titleRect();
    if( !bufferDirty && (lastBufferWidth == rTitle.width()))
        return;
    if ( rTitle.width() <= 0 || rTitle.height() <= 0 )
	return;
    lastBufferWidth = rTitle.width();
    bufferDirty = false;

    activeBuffer.resize(rTitle.width(),
                        rTitle.height());
    QPainter p;
    QRect r(0, 0, activeBuffer.width(), activeBuffer.height());
    p.begin(&activeBuffer);
    if(aUpperGradient){
        p.drawTiledPixmap(r, *aUpperGradient);
    }
    else{
        p.fillRect(r, options()->color(KDecoration::ColorTitleBar, true));
    }
    if(titlePix)
        p.drawTiledPixmap(r, *titlePix);

    p.setFont(options()->font(true, isToolWindow() ));
    QFontMetrics fm(options()->font(true));
    QColorGroup g = options()->colorGroup(KDecoration::ColorTitleBar, true);
    if(aUpperGradient)
        p.drawTiledPixmap(r.x()+((r.width()-fm.width(caption()))/2)-4,
                          r.y(), fm.width(caption())+8, r.height()-1,
                          *aUpperGradient);
    else
        p.fillRect(r.x()+((r.width()-fm.width(caption()))/2)-4, 0,
                   fm.width(caption())+8, r.height(),
                   g.brush(QColorGroup::Background));
    p.setPen(g.mid());
    p.drawLine(r.x(), r.y(), r.right(), r.y());
    p.drawLine(r.x(), r.y(), r.x(), r.bottom());
    p.setPen(g.button());
    p.drawLine(r.right(), r.y(), r.right(), r.bottom());
    p.drawLine(r.x(), r.bottom(), r.right(), r.bottom());
    p.setPen(options()->color(KDecoration::ColorFont, true));
    p.drawText(r.x(), r.y(), r.width(), r.height()-1,
               AlignCenter, caption() );
    g = options()->colorGroup(KDecoration::ColorFrame, true);
    p.setPen(g.background());
    p.drawPoint(r.x(), r.y());
    p.drawPoint(r.right(), r.y());
    p.drawLine(r.right()+1, r.y(), r.right()+1, r.bottom());
    p.end();
}

static const int SUPPORTED_WINDOW_TYPES_MASK = NET::NormalMask |
    NET::DesktopMask | NET::DockMask | NET::ToolbarMask | NET::MenuMask |
    NET::DialogMask | /*NET::OverrideMask |*/ NET::TopMenuMask |
    NET::UtilityMask | NET::SplashMask;

bool LaptopClient::isTransient() const
{
    NET::WindowType type = windowType(SUPPORTED_WINDOW_TYPES_MASK);
    return type == NET::Dialog;
}

// =====================================

LaptopClientFactory::LaptopClientFactory()
{
    create_pixmaps();
}

LaptopClientFactory::~LaptopClientFactory()
{
    delete_pixmaps();
}

KDecoration *LaptopClientFactory::createDecoration(KDecorationBridge *b)
{
    findPreferredHandleSize();
    return new Laptop::LaptopClient(b, this);
}

bool LaptopClientFactory::reset(unsigned long changed)
{
    findPreferredHandleSize();

    // TODO Do not recreate decorations if it is not needed. Look at
    // ModernSystem for how to do that
    Laptop::delete_pixmaps();
    Laptop::create_pixmaps();

    bool needHardReset = true;
    if (changed & SettingButtons) {
		// handled by KCommonDecoration
        needHardReset = false;
    }

    if (needHardReset) {
        return true;
    } else {
        resetDecorations(changed);
        return false;
    }
}

bool LaptopClientFactory::supports( Ability ability )
{
    switch( ability )
    {
        case AbilityAnnounceButtons:
        case AbilityButtonOnAllDesktops:
        case AbilityButtonHelp:
        case AbilityButtonMinimize:
        case AbilityButtonMaximize:
        case AbilityButtonClose:
            return true;
        default:
            return false;
    };
}

QValueList< LaptopClientFactory::BorderSize >
LaptopClientFactory::borderSizes() const
{
    // the list must be sorted
    return QValueList< BorderSize >() << BorderNormal << BorderLarge <<
	BorderVeryLarge <<  BorderHuge << BorderVeryHuge << BorderOversized;
}

void LaptopClientFactory::findPreferredHandleSize()
{
    switch (options()->preferredBorderSize(this)) {
    case KDecoration::BorderLarge:
	handleSize = 11;
	break;
    case KDecoration::BorderVeryLarge:
	handleSize = 16;
	break;
    case KDecoration::BorderHuge:
	handleSize = 24;
	break;
    case KDecoration::BorderVeryHuge:
	handleSize = 32;
	break;
    case KDecoration::BorderOversized:
	handleSize = 40;
	break;
    case KDecoration::BorderTiny:
    case KDecoration::BorderNormal:
    default:
	handleSize = 8;
    }
}

} // Laptop namespace

// vim: sw=4
