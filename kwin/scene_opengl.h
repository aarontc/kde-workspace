/*****************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2006 Lubos Lunak <l.lunak@kde.org>

You can Freely distribute this program under the GNU General Public
License. See the file "COPYING" for the exact licensing terms.
******************************************************************/

#ifndef KWIN_SCENE_OPENGL_H
#define KWIN_SCENE_OPENGL_H

#include "scene.h"

#include <GL/gl.h>
#include <GL/glx.h>

namespace KWinInternal
{

class SceneOpenGL
    : public Scene
    {
    public:
        SceneOpenGL( Workspace* ws );
        virtual ~SceneOpenGL();
        virtual void paint( QRegion damage, ToplevelList windows );
        virtual void postPaint();
        virtual void windowGeometryShapeChanged( Toplevel* );
        virtual void windowOpacityChanged( Toplevel* );
        virtual void windowAdded( Toplevel* );
        virtual void windowDeleted( Toplevel* );
    protected:
        virtual void paintGenericScreen( int mask, ScreenPaintData data );
        virtual void paintSimpleScreen( int mask, QRegion region );
        virtual void paintBackground( QRegion region );
    private:
        void initBuffer();
        bool findConfig( const int* attrs, GLXFBConfig& config, VisualID visual = None );
        typedef GLuint Texture;
        GC gcroot;
        Drawable buffer;
        GLXFBConfig fbcroot;
        static bool root_db;
        static GLXFBConfig fbcdrawable;
        static GLXDrawable glxroot;
        static GLXContext context;
        static bool tfp_mode;
        static bool copy_buffer_hack;
        class Window;
        QMap< Toplevel*, Window > windows;
    };

class SceneOpenGL::Window
    : public Scene::Window
    {
    public:
        Window( Toplevel* c );
        virtual void free();
        virtual void performPaint( int mask, QRegion region, WindowPaintData data );
        void bindTexture();
        void discardTexture();
        Window() {} // QMap sucks even in Qt4
    private:
        Texture texture;
        bool texture_y_inverted; // texture has y inverted
        Pixmap bound_pixmap; // the pixmap the texture is bound to, only for tfp_mode
        GLXPixmap bound_glxpixmap; // the glx pixmap the texture is bound to, only for tfp_mode
    };

} // namespace

#endif
