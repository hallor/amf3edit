#ifndef AMFPARSER_H
#define AMFPARSER_H
#include "Parser.h"

class Variable;
class Value;
class AmfParser : public Parser
{
public:
    Variable * readVariable(QIODevice & dev) const ;
    Value * readValue(QIODevice & dev) const ;
};

#endif // AMFPARSER_H
