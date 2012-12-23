#ifndef PARSER_H
#define PARSER_H

class Variable;
class Value;
class QIODevice;
class Parser
{
public:
    virtual Variable * readVariable(QIODevice & dev) const  = 0;
    virtual Value * readValue(QIODevice & dev) const  = 0;
};

#endif // PARSER_H
