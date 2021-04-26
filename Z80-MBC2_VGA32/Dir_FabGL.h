#pragma once

#include "Dir_I.h"

#include "fabutils.h"

#include "stdio_undefs.h"

class Dir_FabGL :
    public Dir_I
{
public:
    Dir_FabGL( fabgl::FileBrowser &fs ) : fs_( fs ), count_( fs.count() ), index_( 1 ) // 0 = is always parent directory
    {        
    }

    virtual DirEntry next();

    virtual int close()
    {
        return 0;
    }

    virtual const char *error() const
    {
        return "";
    }

    virtual operator bool() const
    {
        return true;
    }
private:
    fabgl::FileBrowser &fs_;
    int count_;
    int index_;
};
