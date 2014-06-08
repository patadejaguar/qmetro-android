#ifndef AUDIOOUTPUT_H
#define AUDIOOUTPUT_H

#include <QThread>
#include <QIODevice>
#include <QTimer>
#include <QByteArray>
#include <QAudioOutput>

class ModPlugGenerator : public QIODevice
{
    Q_OBJECT
public:
    ModPlugGenerator(const QString &fileName, const QAudioFormat &format, QObject *parent=0);
    ~ModPlugGenerator();

    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);
    qint64 bytesAvailable() const;

    qint64 m_pos;
    QByteArray m_buffer;

public slots:
    void start() { open(QIODevice::ReadOnly); }
    void stop() { m_pos=0; close(); }
    //void pause() { close(); }

private:
    void generateData(const QString &fileName, const QAudioFormat &format);


};

class AudioPlay : public QObject
{
    Q_OBJECT
public:
    AudioPlay(const QString &fileName="",const bool &paused=0);
    ~AudioPlay();
    void loadAudioFile(const QString &fileName);

private:
    void initializeAudio(const QString &fileName="");
    void createAudioOutput();

    QThread *AudioThread;

private:
    QTimer*          m_pullTimer;

    QAudioDeviceInfo m_device;
    ModPlugGenerator*       m_generator;
    QAudioOutput*    m_audioOutput;
    QIODevice*       m_output; // not owned
    QAudioFormat     m_format;
    QString          m_fileName;

    bool             m_pullMode;
    QByteArray       m_buffer;

    bool             m_paused;


private slots:
//    void pullTimerExpired();
    void stateChanged(QAudio::State state);
    void deviceChanged(int index);
    void init();



public slots:
    void releasePlayer(){ emit onReleasePlayer(); deleteLater(); }
    void pausePlay(){ m_audioOutput->stop(); }
    void resumePlay(){ m_audioOutput->start(m_generator); }

//    void toggleMode();
//    void toggleSuspendResume();

signals:
    void onReleasePlayer();
};

#endif

