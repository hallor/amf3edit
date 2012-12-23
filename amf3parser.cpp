#include <auto_ptr.h>
#include "amf3parser.h"
#include "Variable.h"
#include "Value.h"
#include "exception.h"
#include "amf3vars.h"

using std::auto_ptr;
using namespace amf3;

Value * Amf3Parser::readValue(QIODevice & dev) const
{
    auto_ptr<Value> ret;
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
    case 0x09: ret.reset(new array_type(*this)); break;
    default:
        throw ReadException(dev, "Unknown data type"); // TODO: varargs
    }
    if (ret.get()) {
        ret->read(dev);
        return ret.release();
    }
    return NULL;
}


Variable *Amf3Parser::readVariable(QIODevice &dev) const
{
    auto_ptr<Variable> v(new Variable(*this));

    if (v.get()) {
        v->read(dev);
        return v.release();
    }
    return NULL;
}
