#ifndef PARSER_H
#define PARSER_H

class Variable;

class Parser
{
public:
    virtual Variable * read(QIODevice & dev) const throw() = 0;
};

#endif // PARSER_H
