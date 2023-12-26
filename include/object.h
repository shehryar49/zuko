#ifndef OBJECT_H
#define OBJECT_H
extern "C" struct ZObject
{
    union
    {
        double f;
        int64_t l;
        int32_t i;
        void* ptr;
    };
    char type;
};
#endif