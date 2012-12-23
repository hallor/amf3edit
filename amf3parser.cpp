#include "amf3parser.h"

Amf3Parser::Amf3Parser()
{
}

Variable *Amf3Parser::read(QIODevice &dev)
{
    Serializable * ret = NULL;
    quint8 code;
    quint64 start_pos = dev.pos();
    if (!dev.read((char*)&code, 1))
        return NULL;
    switch (code)
    {
    case 0x00: ret = new undefined_type(); break;
    case 0x01: ret = new null_type(); break;
    case 0x02: ret = new false_type(); break;
    case 0x03: ret = new true_type(); break;
    case 0x04: ret = new integer_type(); break;
    case 0x06: ret = new string_type(); break;
    case 0x09: ret = new array_type(); break;
    default:
        return ret;
    }
    if (ret)
        if (ret->read(dev))
            return ret;
    delete ret;
    printf("Error reading token %X @%X, failed @%X\n", code, start_pos, dev.pos());
    return NULL;
}
