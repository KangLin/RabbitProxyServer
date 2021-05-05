#ifndef CSIGNAL_H
#define CSIGNAL_H

#include <QObject>
#include <string>

class CSignal : public QObject
{
    Q_OBJECT

public:
    explicit CSignal(QObject *parent = nullptr);
    
    virtual int Open(const std::string &szUrl) = 0;
    virtual int Close() = 0;
    virtual bool IsOpen() = 0;
    virtual int Write(const char* buf, int nLen) = 0;
    virtual int Read(char* buf, int nLen) = 0;

signals:
    void sigConnected();
    void sigDisconnected();
    void sigReadyRead();
};

#endif // CSIGNAL_H
