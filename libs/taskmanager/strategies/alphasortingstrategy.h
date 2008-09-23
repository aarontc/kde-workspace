/*****************************************************************

Copyright 2008 Christian Mollekopf <chrigi_1@hotmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#ifndef ALPHASORTINGSTRATEGY_H
#define ALPHASORTINGSTRATEGY_H

#include <QtCore/QObject>

#include <taskmanager/abstractsortingstrategy.h>

namespace TaskManager
{

/** Sorts the tasks alphabetically by programname found in Task::classClass()*/
class AlphaSortingStrategy : public AbstractSortingStrategy
{
    Q_OBJECT
public:
    AlphaSortingStrategy(QObject *parent);

private:
    /** Sorts list of items*/
    void sortItems(ItemList&);
};

typedef QHash <AbstractGroupableItem*, int> itemHashTable;
typedef QHash <int, itemHashTable*> desktopHashTable;

} // TaskManager namespace
#endif
