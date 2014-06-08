#include <QDebug>

#include <QFile>
#include <QAudioOutput>
#include <QAudioDeviceInfo>
#include "audiooutput.h"
#include "libmodplug/modplug.h"

#ifdef HAVE_UNZIP_H
#include <QBuffer>
#include "../zip/unzip.h"
#endif

const int DataFrequencyHz = 44100;
const int BufferSize      = 32768;

ModPlugGenerator::ModPlugGenerator(const QString &fileName, const QAudioFormat &format, QObject *parent)
    //:   QIODevice(parent)
:   m_pos(0)
{
 moveToThread(parent->thread());
 setParent(parent);
 generateData(fileName, format);
}

ModPlugGenerator::~ModPlugGenerator()
{
 m_buffer.clear();
}

void ModPlugGenerator::generateData(const QString &fileName, const QAudioFormat &format)
{
// if(thread()->isFinished()) return;

 bool opened=0;
 ModPlugFile *modFile;
 QByteArray m_input_buf;

#ifdef HAVE_UNZIP_H
 QString ext=fileName.right(4).toLower();
 if(ext==".mdz" || ext==".s3z" || ext==".xmz" || ext==".itz")
 {
  QBuffer writeBuf(&m_input_buf);
  writeBuf.open(QBuffer::WriteOnly);
  QDataStream out(&writeBuf);
  UnZip uz;
  if(uz.openArchive(fileName)) { qWarning("ModPlug error: unpacking moplug file"); return; }
  uz.extractFile(((UnZip::ZipEntry)uz.entryList().first()).filename,out.device());
  opened=1;
 }
#endif

// if(thread()->isFinished()) return;

 if(!opened)
 {
  QFile file(fileName);
  if(!file.open(QIODevice::ReadOnly)) { qWarning("ModPlug error: %s", qPrintable(file.errorString())); return; }
  m_input_buf = file.readAll();
  file.close();
 }

 if(m_input_buf.isEmpty()) { qWarning("ModPlug error: reading moplug file"); return; }
 modFile = ModPlug_Load(m_input_buf.data(), m_input_buf.size());
 ModPlug_Settings modSetting;
 ModPlug_GetSettings(&modSetting);
 modSetting.mFlags = modSetting.mFlags | MODPLUG_ENABLE_SURROUND;
 modSetting.mResamplingMode = MODPLUG_RESAMPLE_FIR;
 ModPlug_SetSettings(&modSetting);
 //ModPlug_SetMasterVolume(modFile,ModPlug_GetMasterVolume(modFile)*1.25);
#if QT_VERSION >= 0x050000
 qint64 length = (format.sampleRate() * format.channelCount() * (format.sampleSize() / 8))
                     * (ModPlug_GetLength(modFile)/1000);
#else
 qint64 length = (format.frequency() * format.channels() * (format.sampleSize() / 8))
                     * (ModPlug_GetLength(modFile)/1000);
#endif

// if(thread()->isFinished()) return;

 m_buffer.resize(length);
 ModPlug_Read(modFile, m_buffer.data(), m_buffer.size());
 ModPlug_Unload(modFile);
}

qint64 ModPlugGenerator::readData(char *data, qint64 len)
{
 if(/*thread()->isFinished() ||*/ m_buffer.isEmpty()) return 0;
 qint64 total = 0;
 while (len - total > 0) {
  const qint64 chunk = qMin((m_buffer.size() - m_pos), len - total);
  memcpy(data + total, m_buffer.constData() + m_pos, chunk);
  m_pos = (m_pos + chunk) % m_buffer.size();
  total += chunk;
//  if(!m_pos) stop(); // don't loop
 }
 return total;
}

qint64 ModPlugGenerator::writeData(const char *data, qint64 len)
{
 Q_UNUSED(data);
 Q_UNUSED(len);

 return 0;
}

qint64 ModPlugGenerator::bytesAvailable() const
{
 return m_buffer.size() + QIODevice::bytesAvailable();
}

AudioPlay::AudioPlay(const QString &fileName,const bool &paused)
    :
//    ,   m_pullTimer(new QTimer(this))
        m_device(QAudioDeviceInfo::defaultOutputDevice())
    ,   m_generator(0)
    ,   m_audioOutput(0)
    ,   m_output(0)
    ,   m_fileName(fileName)
    ,   m_buffer(BufferSize, 0)
    ,   m_paused(paused)
{
 AudioThread = new QThread;
 this->moveToThread(AudioThread);
 connect(AudioThread,SIGNAL(started()),this,SLOT(init()));
 connect(this,SIGNAL(onReleasePlayer()),AudioThread,SLOT(quit()));
 connect(this,SIGNAL(onReleasePlayer()),this,SLOT(deleteLater()));
 connect(AudioThread,SIGNAL(finished()),AudioThread,SLOT(deleteLater()));

 QTimer::singleShot(2000,AudioThread,SLOT(start()));

// connect(m_pullTimer, SIGNAL(timeout()), SLOT(pullTimerExpired()));
}

AudioPlay::~AudioPlay()
{
 AudioThread->quit();
 m_generator->stop();
 m_audioOutput->stop();
 delete m_generator;
 delete m_audioOutput;

 m_buffer.clear();
 delete m_output;

//// quit();
//// if(isRunning()) wait();

//// m_pullTimer->stop();
// m_generator->stop();
//// delete m_pullTimer;
// delete m_generator;
//#ifndef Q_OS_SYMBIAN // Thread has panicked
// m_audioOutput->stop();
// delete m_audioOutput;
// delete m_output;
//#endif
//// terminate();
}

void AudioPlay::initializeAudio(const QString &fileName)
{
 if(!fileName.isEmpty()) m_fileName=fileName;

 m_pullMode = true;

#if QT_VERSION >= 0x050000
 m_format.setSampleRate(DataFrequencyHz);
 m_format.setChannelCount(2);
#else
 m_format.setFrequency(DataFrequencyHz);
 m_format.setChannels(2);
#endif
 m_format.setSampleSize(16);
 m_format.setCodec("audio/pcm");
 m_format.setByteOrder(QAudioFormat::LittleEndian);
 m_format.setSampleType(QAudioFormat::SignedInt);

 QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
 if (!info.isFormatSupported(m_format)) {
  qWarning() << "Default format not supported - trying to use nearest";
  m_format = info.nearestFormat(m_format);
 }

 m_generator = new ModPlugGenerator(m_fileName,m_format,this);
 connect(this,SIGNAL(onReleasePlayer()),m_generator,SLOT(stop()));
 connect(this,SIGNAL(onReleasePlayer()),m_generator,SLOT(deleteLater()));
}

void AudioPlay::loadAudioFile(const QString &fileName)
{
// if(isRunning())
 {
  delete m_generator;
  m_buffer.clear();
  initializeAudio(fileName);
 }
// else m_fileName=fileName;
}

void AudioPlay::createAudioOutput()
{
 delete m_audioOutput;
 m_audioOutput = 0;
 m_audioOutput = new QAudioOutput(m_device, m_format);
 connect(m_audioOutput, SIGNAL(stateChanged(QAudio::State)), SLOT(stateChanged(QAudio::State)));
 connect(this,SIGNAL(onReleasePlayer()),m_audioOutput,SLOT(deleteLater()));
 m_generator->start();
 if(!m_paused) m_audioOutput->start(m_generator);

}
void AudioPlay::init()
{
 initializeAudio();
// if(isFinished()) return;
 createAudioOutput();
// exec();
}

void AudioPlay::deviceChanged(int index)
{
// m_pullTimer->stop();
 m_generator->stop();
 m_audioOutput->stop();
 m_audioOutput->disconnect(this);
 m_device = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput).first();
 createAudioOutput();
}

void AudioPlay::stateChanged(QAudio::State state)
{
//     qDebug() << "state = " << state;
}


//void AudioPlay::pullTimerExpired()
//{
// if (m_audioOutput && m_audioOutput->state() != QAudio::StoppedState) {
//  int chunks = m_audioOutput->bytesFree()/m_audioOutput->periodSize();
//  while (chunks) {
//   const qint64 len = m_generator->read(m_buffer.data(), m_audioOutput->periodSize());
//   if (len)
//    m_output->write(m_buffer.data(), len);
//   if (len != m_audioOutput->periodSize())  break;
//   --chunks;
//  }

// }
//}

//void AudioPlay::toggleMode()
//{
// m_pullTimer->stop();
// m_audioOutput->stop();

// if (m_pullMode) {
//  //        m_modeButton->setText(PullModeLabel);
//  m_output = m_audioOutput->start();
//  m_pullMode = false;
//  m_pullTimer->start(20);
// } else {
//  //        m_modeButton->setText(PushModeLabel);
//  m_pullMode = true;
//  m_audioOutput->start(m_generator);
// }

// //    m_suspendResumeButton->setText(SuspendLabel);
//}

// It's don't working on symbian (thread has panicked).
//void AudioPlay::toggleSuspendResume()
//{
// if (m_audioOutput->state() == QAudio::SuspendedState) {
//  //        qWarning() << "status: Suspended, resume()";
//  m_audioOutput->resume();
//  //        m_suspendResumeButton->setText(SuspendLabel);
// } else if (m_audioOutput->state() == QAudio::ActiveState) {
//  //        qWarning() << "status: Active, suspend()";
//  m_audioOutput->suspend();
//  //        m_suspendResumeButton->setText(ResumeLabel);
// } else if (m_audioOutput->state() == QAudio::StoppedState) {
//  //        qWarning() << "status: Stopped, resume()";
//  m_audioOutput->resume();
//  //        m_suspendResumeButton->setText(SuspendLabel);
// } else if (m_audioOutput->state() == QAudio::IdleState) {
//  //        qWarning() << "status: IdleState";
// }
//}


