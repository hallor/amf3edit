#include "amfparser.h"
#include "readexception.h"
Variable *AmfParser::read(QIODevice &dev)
{
    throw ReadException(dev, "AMF ver 1 not supported.");
    return NULL;
}
