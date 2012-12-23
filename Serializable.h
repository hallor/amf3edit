#ifndef SERIALIZABLE_H
#define SERIALIZABLE_H

class Serializable {
public:
    virtual bool write(QIODevice & /*dev*/) { return false; }
    virtual bool read(QIODevice & dev) = 0;
    virtual QString toString() const {
        return QString("[serializable]");
    }
};

#endif // SERIALIZABLE_H
