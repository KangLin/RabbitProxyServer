//! @author Kang Lin(kl222@126.com)

#ifndef CSIGNAL_H
#define CSIGNAL_H

#include <QObject>
#include <string>

#include "rtc/rtc.hpp"

class CIceSignal : public QObject
{
    Q_OBJECT

public:
    explicit CIceSignal(QObject *parent = nullptr);
    virtual ~CIceSignal();

    virtual int Open(const std::string& szServer, quint16 nPort,
                     const std::string& user, const std::string& password) = 0;
    virtual int Open(const std::string &szUrl) = 0;
    virtual int Close() = 0;
    virtual bool IsOpen() = 0;


    virtual int SendDescription(const QString& user,
                                const QString& id,
                                const rtc::Description& description) = 0;
    virtual int SendCandiate(const QString& user,
                             const QString& id,
                             const rtc::Candidate& candidate) = 0;

    /**
     * @brief Write
     * @param buf
     * @param nLen
     * @return If success, return count bytes of be send
     *         If fail, return -1
     */
    virtual int Write(const char* buf, int nLen) = 0;
    /**
     * @brief Read
     * @param buf
     * @param nLen
     * @return If success, return count bytes of be read
     *         If fail, return -1
     */
    virtual int Read(char* buf, int nLen) = 0;

Q_SIGNALS:
    void sigConnected();
    void sigDisconnected();
    void sigReadyRead();
    void sigError(int nError, const QString& szError);

    /**
     * @brief sigOffer
     * @param user
     * @param id: channel id
     */
    void sigOffer(const QString& user, const QString& id);
    /**
     * @brief sigCandiate
     * @param user
     * @param id: channel id
     * @param mid
     * @param sdp
     */
    void sigCandiate(const QString& user,
                     const QString& id,
                     const QString& mid,
                     const QString& sdp);
    /**
     * @brief sigDescription
     * @param user
     * @param id: channel id
     * @param type
     * @param sdp
     */
    void sigDescription(const QString& user,
                        const QString& id,
                        const QString& type,
                        const QString& sdp);
};

#endif // CSIGNAL_H
