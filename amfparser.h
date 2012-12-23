#ifndef AMFPARSER_H
#define AMFPARSER_H
#include "Parser.h"

class Variable;
class Value;
class AmfParser : public Parser
{
public:
    Variable * readVariable(QIODevice & dev) const throw();
    Value * readValue(QIODevice & dev) const throw();
};

#endif // AMFPARSER_H
