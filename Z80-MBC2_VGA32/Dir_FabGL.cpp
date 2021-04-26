#include "Dir_FabGL.h"

#include <string.h>


DirEntry Dir_FabGL::next()
{
    DirEntry entry;
    if ( index_ < count_ )
    {
        auto *item = fs_.get( index_++ );
        entry.name = item->name;
        entry.size = fs_.fileSize( item->name );
        entry.isDir = item->isDir;
    }
    return entry;
}
