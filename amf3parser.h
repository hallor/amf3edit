#ifndef AMF3PARSER_H
#define AMF3PARSER_H
#include "Parser.h"

class Variable;
class Value;
class Amf3Parser : public Parser
{
public:
    Amf3Parser();
    Variable * readVariable(QIODevice & dev) const throw();
    Value * readValue(QIODevice & dev) const throw();
};

#endif // AMF3PARSER_H
