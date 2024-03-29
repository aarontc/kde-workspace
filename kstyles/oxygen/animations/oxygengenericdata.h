#ifndef oxygengeneric_datah
#define oxygengeneric_datah

//////////////////////////////////////////////////////////////////////////////
// oxygengenericdata.h
// generic data container for animations
// -------------------
//
// Copyright (c) 2009 Hugo Pereira Da Costa <hugo@oxygen-icons.org>
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

#include "oxygenanimationdata.h"
#include "oxygenanimation.h"

#include <QObject>
#include <QTextStream>
namespace Oxygen
{


    //! generic data
    class GenericData: public AnimationData
    {

        Q_OBJECT

        //! declare opacity property
        Q_PROPERTY( qreal opacity READ opacity WRITE setOpacity )

        public:

        //! constructor
        GenericData( QObject* parent, QWidget* widget, int duration );

        //! destructor
        virtual ~GenericData( void )
        {}

        //! return animation object
        virtual const Animation::Pointer& animation() const
        { return _animation; }

        //! duration
        virtual void setDuration( int duration )
        { _animation.data()->setDuration( duration ); }

        //! opacity
        virtual qreal opacity( void ) const
        { return _opacity; }

        //! opacity
        virtual void setOpacity( qreal value )
        {

            value = digitize( value );
            if( _opacity == value ) return;

            _opacity = value;
            setDirty();

        }

        private:

        //! animation handling
        Animation::Pointer _animation;

        //! opacity variable
        qreal _opacity;

    };

}

#endif
