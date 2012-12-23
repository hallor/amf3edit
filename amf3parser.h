#ifndef AMF3PARSER_H
#define AMF3PARSER_H
#include "parser.h"

class Variable;
class Amf3Parser : public Parser
{
public:
    Amf3Parser();
    Variable * read(QIODevice & dev) throw();
};

#endif // AMF3PARSER_H
