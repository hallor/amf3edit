#ifndef PARSER_H
#define PARSER_H

class Variable;
class Value;
class Parser
{
public:
    virtual Variable * readVariable(QIODevice & dev) const throw() = 0;
    virtual Value * readValue(QIODevice & dev) const throw() = 0;
};

#endif // PARSER_H
