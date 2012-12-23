#include <auto_ptr.h>
#include "amf3parser.h"
#include "variable.h"
#include "amf3vars.h"
#include "readexception.h"

using std::auto_ptr;
using namespace amf3;
Variable *Amf3Parser::read(QIODevice &dev) const
{
    auto_ptr<Serializable *> ret;
    quint8 code;
    if (dev.read((char*)&code, 1) <= 0)
        throw ReadException(dev);

    switch (code)
    {
    case 0x00: ret.reset(new undefined_type()); break;
    case 0x01: ret.reset(new null_type()); break;
    case 0x02: ret.reset(new false_type()); break;
    case 0x03: ret.reset(new true_type()); break;
    case 0x04: ret.reset(new integer_type()); break;
    case 0x06: ret.reset(new string_type()); break;
    case 0x09: ret.reset(new array_type(this, dev)); break;
    default:
        throw ReadException(dev, "Unknown data type: %02x", code);
    }
    if (ret)
        if (ret->read(dev)) {
            return ret.release();
        }
    return NULL;
}
