#ifndef FOO_UNIONS_H_
#define FOO_UNIONS_H_

union FOO
{
    int x;
    unsigned char bytes[sizeof(x)];
}FOO;
union FOO1
{
    unsigned char bytes[8];
    long long int l;
}FOO1;
union FOO2
{
    unsigned char bytes[4];
    float f;
}FOO2;

#endif
