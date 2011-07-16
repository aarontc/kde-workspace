#ifndef oxygengenericanimationconfigitem_h
#define oxygengenericanimationconfigitem_h

//////////////////////////////////////////////////////////////////////////////
// oxygengenericanimationconfigitem.h
// animation configuration item
// -------------------
//
// Copyright (c) 2010 Hugo Pereira Da Costa <hugo@oxygen-icons.org>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//////////////////////////////////////////////////////////////////////////////

#include "oxygenanimationconfigitem.h"
#include "ui_oxygengenericanimationconfigbox.h"

#include <QtCore/QWeakPointer>
#include <QtGui/QFrame>

namespace Oxygen
{

    class GenericAnimationConfigBox: public QFrame
    {

        Q_OBJECT

        public:

        //! constructor
        GenericAnimationConfigBox(QWidget* parent):
        QFrame( parent )
        { ui.setupUi( this ); }

        //! duration spin box
        QSpinBox* durationSpinBox( void ) const
        { return ui.durationSpinBox; }

        private:

        Ui_GenericAnimationConfigBox ui;

    };

    //! generic animation config item
    class GenericAnimationConfigItem: public AnimationConfigItem
    {

        Q_OBJECT

        public:

        //! constructor
        explicit GenericAnimationConfigItem( QWidget* parent, const QString& title = QString(), const QString& description = QString() ):
            AnimationConfigItem( parent, title, description )
        {}

        //! configure
        virtual void initializeConfigurationWidget( QWidget* );

        //! configuration widget
        virtual QWidget* configurationWidget( void ) const
        {
            assert( _configurationWidget );
            return _configurationWidget.data();
        }

        //! duration
        virtual int duration( void ) const
        { return (_configurationWidget) ? _configurationWidget.data()->durationSpinBox()->value():0; }

        public slots:

        //! duration
        virtual void setDuration( int value )
        {
            if( _configurationWidget )
            { _configurationWidget.data()->durationSpinBox()->setValue( value ); }
        }

        private:

        //! configuration widget
        QWeakPointer<GenericAnimationConfigBox> _configurationWidget;

    };

}

#endif
