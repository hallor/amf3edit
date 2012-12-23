#ifndef AMFPARSER_H
#define AMFPARSER_H
#include "parser.h"

class Variable;
class AmfParser : public Parser
{
public:
    Variable * read(QIODevice & dev) throw();
};

#endif // AMFPARSER_H
