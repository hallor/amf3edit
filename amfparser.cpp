#include "amfparser.h"
#include "exception.h"

Variable *AmfParser::readVariable(QIODevice &dev) const
{
    throw ReadException(dev, "AMF ver 1 not supported.");
    return NULL;
}

Value *AmfParser::readValue(QIODevice &dev) const
{
    throw ReadException(dev, "AMF ver 1 not supported.");
    return NULL;
}
