#ifndef AMF3PARSER_H
#define AMF3PARSER_H
#include "Parser.h"

class Variable;
class Value;
class Amf3Parser : public Parser
{
public:
    Variable * readVariable(QIODevice & dev) const;
    Value * readValue(QIODevice & dev) const;
};

#endif // AMF3PARSER_H
