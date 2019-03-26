#ifndef DEFINE_H
#define DEFINE_H

#define FOREACH(cls, begin, end, var, content) \
    do { \
        for (int _tmp_foreach_val = (int)begin; _tmp_foreach_val <= (int)end; ++_tmp_foreach_val) \
        { \
            cls var = (cls)_tmp_foreach_val; \
            content \
        } \
    } while(false)

#endif