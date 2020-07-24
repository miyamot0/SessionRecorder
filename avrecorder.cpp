/****************************************************************************

    Copyright 2018 Shawn Gilroy

    This file is part of Session Recorder.

    Session Recorder is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Session Recorder is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Session Recorder.  If not, see http://www.gnu.org/licenses/.

    The Session Recorder is a tool to assist researchers in clinical behavioral research.

    This file was adapted from meeting-recorder (MIT), which was based on Qt Examples
    provided by Digia (BSD-3)

    Email: shawn(dot)gilroy(at)temple.edu

-----------------------------------------------------------------------------

    Copyright (c) 2015-2016 University of Helsinki

    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation files
    (the "Software"), to deal in the Software without restriction,
    including without limitation the rights to use, copy, modify, merge,
    publish, distribute, sublicense, and/or sell copies of the Software,
    and to permit persons to whom the Software is furnished to do so,
    subject to the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
    BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
    ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
    CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

-----------------------------------------------------------------------------

    Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
    Contact: http://www.qt-project.org/legal

    This file is part of the examples of the Qt Toolkit.

    $QT_BEGIN_LICENSE:BSD$
    You may use this file under the terms of the BSD license as follows:

    "Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:
    * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
    * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
     of its contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.


    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
    A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."

    $QT_END_LICENSE$

****************************************************************************/

#include <QAudioProbe>
#include <QAudioRecorder>
#include <QDir>
#include <QFileDialog>
#include <QMediaRecorder>
#include <QHostInfo>
#include <QMessageBox>
#include <QShortcut>
#include <QTimer>

#ifdef QT_DEBUG
#include <QDebug>
#endif

#include "avrecorder.h"
#include "qaudiolevel.h"

#include "ui_avrecorder.h"

static qreal getPeakValue(const QAudioFormat &format);
static QVector<qreal> getBufferLevels(const QAudioBuffer &buffer);

template <class T>
static QVector<qreal> getBufferLevels(const T *buffer, int frames, int channels);

AvRecorder::AvRecorder(RecordSettingsData *recordSettings, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::AvRecorder)
{
    ui->setupUi(this);

#ifdef QT_DEBUG
    qDebug() << "AvRecorder::AvRecorder(QWidget *parent)";
#endif

    resize(0,0);

    LoadPreviousOptions(recordSettings);

    LoadCurrentOptions();

    // <!-- Setup Audio Recorder -->
    audioRecorder = new QAudioRecorder(this);
    probe = new QAudioProbe;
    connect(probe, SIGNAL(audioBufferProbed(QAudioBuffer)), this, SLOT(processBuffer(QAudioBuffer)));
    probe->setSource(audioRecorder);

    connect(audioRecorder, SIGNAL(durationChanged(qint64)), this, SLOT(updateProgress(qint64)));
    connect(audioRecorder, SIGNAL(statusChanged(QMediaRecorder::Status)), this, SLOT(updateStatus(QMediaRecorder::Status)));
    connect(audioRecorder, SIGNAL(stateChanged(QMediaRecorder::State)), this, SLOT(onStateChanged(QMediaRecorder::State)));
    connect(audioRecorder, SIGNAL(error(QMediaRecorder::Error)), this, SLOT(displayErrorMessage()));

    // <!-- Setup Interaction -->
    connect(ui->recordButton, SIGNAL(clicked(bool)), this, SLOT(toggleRecord()));

    connect(ui->lineEditId, SIGNAL(textChanged(QString)), this, SLOT(changeIdSlot(QString)));
    connect(ui->lineEditSession, SIGNAL(textChanged(QString)), this, SLOT(changeSessionSlot(QString)));
    connect(ui->lineEditTx, SIGNAL(textChanged(QString)), this, SLOT(changeTreatmentSlot(QString)));
    connect(ui->lineEditCond, SIGNAL(textChanged(QString)), this, SLOT(changeConditionSlot(QString)));

    // <!-- Setup Conversion Process -->
    combineStreamProcess = new QProcess(this);
    connect(combineStreamProcess, SIGNAL(started()), this, SLOT(processStarted()));
    connect(combineStreamProcess, SIGNAL(readyReadStandardOutput()), this,SLOT(readyReadStandardOutput()));
    connect(combineStreamProcess, SIGNAL(errorOccurred(QProcess::ProcessError)), this, SLOT(processError(QProcess::ProcessError)));
    connect(combineStreamProcess, SIGNAL(finished(int)), this, SLOT(encodingFinished()));
}

///
/// \brief AvRecorder::LoadPreviousOptions
///
void AvRecorder::LoadPreviousOptions(RecordSettingsData* mSettings)
{
#ifdef QT_DEBUG
    qDebug() << "AvRecorder::LoadPreviousOptions()";
#endif

    comboBoxVideoDevice     = mSettings->mVideoDevice;
    lineEditVideoFPS        = mSettings->mVideoFPS.toDouble();

    comboBoxAudioDevice     = mSettings->mAudioDevice;
    comboBoxAudioCodec      = mSettings->mAudioEncoding;
    comboBoxAudioSampling   = mSettings->mAudioSampling;

    lineEditOutputDirectory = mSettings->fileSaveLocation;
    lineEditFFmpegDirectory = mSettings->ffmpegLocation;

#ifdef QT_DEBUG
    qDebug() << "comboBoxVideoDevice: " << comboBoxVideoDevice;
    qDebug() << "lineEditVideoFPS: " << lineEditVideoFPS;
    qDebug() << "mResolution: " << mSettings->mResolution;

    qDebug() << "comboBoxAudioDevice: " << comboBoxAudioDevice;
    qDebug() << "comboBoxAudioCodec: " << comboBoxAudioCodec;
    qDebug() << "comboBoxAudioSampling: " << comboBoxAudioSampling;

    qDebug() << "lineEditOutputDirectory: " << lineEditOutputDirectory;
    qDebug() << "lineEditFFmpegDirectory: " << lineEditFFmpegDirectory;
#endif

    changeShownResolution(mSettings->mResolution);

#ifdef _WIN32
    QFileInfo ffmpegFile(lineEditFFmpegDirectory+"/ffmpeg.exe");
#elif __APPLE__
    QFileInfo ffmpegFile(lineEditFFmpegDirectory+"/ffmpeg");
#endif

    ui->statusFFmpeg->setText(ffmpegFile.exists() ? "Success" : "Not Found");
    ui->statusFFmpeg->setStyleSheet(ffmpegFile.exists() ? QStringLiteral("QLabel { color: green }") :
                                                          QStringLiteral("QLabel { color: red }"));
}

///
/// \brief AvRecorder::changeShownResolution
/// \param val
///
void AvRecorder::changeShownResolution(QString val)
{
    int x, y;

    if (val.contains('x'))
    {
        QStringList wh = val.split('x');

        bool ok = true;

        x = wh.at(0).toInt(&ok);
        y = wh.at(1).toInt(&ok);

        if (ok)
        {
            ui->viewfinder_0->resize(x, y);
        }
    }
}

///
/// \brief AvRecorder::setCameraStatus
/// \param value
///
void AvRecorder::setCameraStatus(bool value)
{

#ifdef QT_DEBUG
    qDebug() << QString("AvRecorder::setCameraStatus(bool value); value = %1").arg(value);
#endif

    ui->statusCamera->setText(value ? "Success" : "Failed");
    ui->statusCamera->setStyleSheet(value ? QStringLiteral("QLabel { color: green }") :
                                            QStringLiteral("QLabel { color: red }"));
}

///
/// \brief AvRecorder::changeIdSlot
/// \param value
///
void AvRecorder::changeIdSlot(QString value)
{
    emit changeSessionConditionSignal(0, value);
}

///
/// \brief AvRecorder::changeSessionSlot
/// \param value
///
void AvRecorder::changeSessionSlot(QString value)
{
    emit changeSessionConditionSignal(1, value);
}

///
/// \brief AvRecorder::changeTreatmentSlot
/// \param value
///
void AvRecorder::changeTreatmentSlot(QString value)
{
    emit changeSessionConditionSignal(2, value);
}

///
/// \brief AvRecorder::changeConditionSlot
/// \param value
///
void AvRecorder::changeConditionSlot(QString value)
{
    emit changeSessionConditionSignal(3, value);
}

///
/// \brief AvRecorder::~AvRecorder
///
AvRecorder::~AvRecorder()
{
    delete audioRecorder;
    delete probe;
}

///
/// \brief AvRecorder::updateProgress
/// \param duration
///
void AvRecorder::updateProgress(qint64 duration)
{
    if (audioRecorder->error() != QMediaRecorder::NoError || duration < 2000)
    {
        return;
    }

    QFileInfo wavFile(lineEditOutputDirectory+"/audio.wav");
    QFileInfo ca1File(lineEditOutputDirectory+"/capture.avi");

    qint64 duration_human = duration / 1000;
    QString duration_unit = "secs";

    if (duration_human > 100)
    {
        duration_human /= 60;
        duration_unit = "mins";
    }

    ui->statusbar->showMessage(tr("Rec started %1 (%2 %3), audio %4 MB, camera 0: %5 MB")
                               .arg(rec_started.toString("hh:mm:ss"))
                               .arg(duration_human)
                               .arg(duration_unit)
                               .arg(wavFile.size()/1024/1024)
                               .arg(ca1File.size()/1024/1024));
}

///
/// \brief AvRecorder::updateStatus
/// \param status
///
void AvRecorder::updateStatus(QMediaRecorder::Status status)
{
    QString statusMessage;

    QString program = QString(lineEditFFmpegDirectory + "/ffmpeg");

    QString id = ui->lineEditId->text();
    QString sessNumber = QString::number(ui->lineEditSession->text().toInt()).rightJustified(4, '0');

    QDir dirNew(QString("%1/%2/%3").arg(lineEditOutputDirectory)
             .arg(id)
             .arg(ui->lineEditTx->text()));

    switch (status) {
    case QMediaRecorder::RecordingStatus:
        statusMessage = tr("Starting to record...");
        break;

    case QMediaRecorder::PausedStatus:
        statusMessage = tr("Paused");
        break;

    case QMediaRecorder::UnloadedStatus:

#ifdef QT_DEBUG
        qDebug() << "Stopped, QProcess here";
#endif

        QDir::setCurrent(lineEditOutputDirectory);
        combineStreamProcess->setWorkingDirectory(lineEditOutputDirectory);

        if (!dirNew.exists())
        {
            dirNew.mkpath(".");
        }

        /* If users wishes to use compression, apply here */
        if (ui->checkBoxCompression->isChecked())
        {
            combineStreamProcess->start(QString("%1 -y -i capture.avi -i audio.wav -async 1 -vcodec libx264 -crf 24 %2/%3/%4-%5.avi")
                                        .arg(program)
                                        .arg(id)
                                        .arg(ui->lineEditTx->text())
                                        .arg(sessNumber)
                                        .arg(ui->lineEditCond->text()));

            statusMessage = tr("Converting files...");
        }
        else
        {
            combineStreamProcess->start(QString("%1 -y -i capture.avi -i audio.wav -async 1 -c copy %2/%3/%4-%5.avi")
                                        .arg(program)
                                        .arg(id)
                                        .arg(ui->lineEditTx->text())
                                        .arg(sessNumber)
                                        .arg(ui->lineEditCond->text()));

            statusMessage = tr("Combining files...");
        }

        ui->statusbar->showMessage(statusMessage);

        break;

    case QMediaRecorder::LoadedStatus:

#ifdef QT_DEBUG
        qDebug() << "Loaded";
#endif

        break;

    default:
        break;
    }

    if (audioRecorder->error() == QMediaRecorder::NoError)
        ui->statusbar->showMessage(statusMessage);
}

///
/// \brief AvRecorder::readyReadStandardOutput
///
void AvRecorder::readyReadStandardOutput()
{

#ifdef QT_DEBUG
    qDebug() << combineStreamProcess->readAllStandardOutput();
#endif

}

///
/// \brief AvRecorder::processStarted
///
void AvRecorder::processStarted()
{

#ifdef QT_DEBUG
    qDebug() << "processStarted()";
#endif

    ui->recordButton->setEnabled(false);

    /* If user wishes, increment session number */
    if (ui->checkBoxIncrement->isChecked())
    {
        ui->lineEditSession->setText(QString::number(sessionNumber + 1));
    }

    SaveCurrentOptions();
}

///
/// \brief AvRecorder::encodingFinished
///
void AvRecorder::encodingFinished()
{

#ifdef QT_DEBUG
    qDebug() << "encodingFinished()";
#endif

    ui->statusbar->showMessage(tr("Video operations completed."));
    ui->recordButton->setEnabled(true);
}

///
/// \brief AvRecorder::processError
/// \param err
///
void AvRecorder::processError(QProcess::ProcessError err)
{

#ifdef QT_DEBUG
    qDebug() << err;
#endif

}

///
/// \brief AvRecorder::onStateChanged
/// \param state
///
void AvRecorder::onStateChanged(QMediaRecorder::State state)
{
#ifdef QT_DEBUG
    qDebug() << QString("AvRecorder::onStateChanged(QMediaRecorder::State state): %1").arg(state);
#endif

    switch (state) {
    case QMediaRecorder::RecordingState:
        ui->recordButton->setText(tr("Stop"));
        break;
    case QMediaRecorder::PausedState:
        ui->recordButton->setText(tr("Stop"));
        break;
    case QMediaRecorder::StoppedState:
        ui->recordButton->setText(tr("Record"));
        break;
    }

    emit stateChanged(state);
}

///
/// \brief AvRecorder::toggleRecord
///
void AvRecorder::toggleRecord()
{
#ifdef QT_DEBUG
    qDebug() << QString("AvRecorder::toggleRecord()");
#endif

    /* Cast all to upper */
    ui->lineEditId->setText(ui->lineEditId->text().toUpper());
    ui->lineEditTx->setText(ui->lineEditTx->text().toUpper());
    ui->lineEditCond->setText(ui->lineEditCond->text().toUpper());
    qApp->processEvents();

    /* Check here if session is something that can be incremented*/
    if (!isSessionAnInt())
    {
        QMessageBox::warning(this, "Error", "You must enter a session number", QMessageBox::Ok);

        return;
    }

    /*If nagging the user*/
    if(ui->checkBoxNag->isChecked() &&
            QFile::exists(QString("%1/%2/%3/%4-%5.avi")
                          .arg(lineEditOutputDirectory)
                          .arg(ui->lineEditId->text())
                          .arg(ui->lineEditTx->text())
                          .arg(QString::number(ui->lineEditSession->text().toInt()).rightJustified(4, '0'))
                          .arg(ui->lineEditCond->text())) &&
            audioRecorder->state() == QMediaRecorder::StoppedState)
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this,
                                      "Overwrite Existing File",
                                      "Overwrite video?",
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::No)
        {
            return;
        }
    }

    emit outputDirectory(lineEditOutputDirectory);

    if (audioRecorder->state() == QMediaRecorder::StoppedState)
    {

#ifdef QT_DEBUG
        qDebug() << "AvRecorder::toggleRecord() :: Starting State";
#endif

        audioRecorder->setAudioInput(comboBoxAudioDevice);
        audioRecorder->setOutputLocation(QUrl::fromLocalFile(lineEditOutputDirectory+"/audio.wav"));

        emit sendSessionDetails(ui->lineEditId->text(),
                                ui->lineEditSession->text(),
                                ui->lineEditTx->text(),
                                ui->lineEditCond->text());

#ifdef QT_DEBUG
        qDebug() << "AvRecorder::toggleRecord() Audio settings";
#endif

        QAudioEncoderSettings settings;
        settings.setCodec(comboBoxAudioCodec);
        settings.setSampleRate(comboBoxAudioSampling.toInt());
        settings.setChannelCount(1);
        settings.setQuality(QMultimedia::VeryHighQuality);

#ifdef QT_DEBUG
        qDebug() << "AvRecorder::toggleRecord() :: Encoding Settings";
#endif

        audioRecorder->setEncodingSettings(settings,
                                           QVideoEncoderSettings(),
                                           QString("audio/x-wav"));

#ifdef QT_DEBUG
        qDebug() << "AvRecorder::toggleRecord() :: Pre record";
#endif

        audioRecorder->record();

#ifdef QT_DEBUG
        qDebug() << "AvRecorder::toggleRecord() :: Recording...";
#endif

        rec_started = QDateTime::currentDateTime();

        ui->lineEditId->setEnabled(false);
        ui->lineEditSession->setEnabled(false);
        ui->lineEditTx->setEnabled(false);
        ui->lineEditCond->setEnabled(false);
    }
    else
    {
#ifdef QT_DEBUG
        qDebug() << "AvRecorder::toggleRecord() :: Stopping State";
#endif

        ui->lineEditId->setEnabled(true);
        ui->lineEditSession->setEnabled(true);
        ui->lineEditTx->setEnabled(true);
        ui->lineEditCond->setEnabled(true);

        audioRecorder->stop();
    }
}

///
/// \brief AvRecorder::SaveCurrentOptions
///
void AvRecorder::SaveCurrentOptions()
{
    QSettings settings(QSettings::UserScope, QLatin1String("Session Recorder"));
    settings.beginGroup(QLatin1String("AvRecorder"));

    settings.setValue(QLatin1String("lineEditId"), ui->lineEditId->text());
    settings.setValue(QLatin1String("lineEditSession"), ui->lineEditSession->text());
    settings.setValue(QLatin1String("lineEditTx"), ui->lineEditTx->text());
    settings.setValue(QLatin1String("lineEditCond"), ui->lineEditCond->text());


    settings.setValue(QLatin1String("checkBoxCompression"), ui->checkBoxCompression->isChecked());
    settings.setValue(QLatin1String("checkBoxIncrement"), ui->checkBoxIncrement->isChecked());
    settings.setValue(QLatin1String("checkBoxNag"), ui->checkBoxNag->isChecked());

    settings.endGroup();
    settings.sync();
}

///
/// \brief AvRecorder::LoadCurrentOptions
///
void AvRecorder::LoadCurrentOptions()
{
    QSettings settings(QSettings::UserScope, QLatin1String("Session Recorder"));
    settings.beginGroup(QLatin1String("AvRecorder"));

    ui->lineEditId->setText(settings.value(QLatin1String("lineEditId")).toString());
    ui->lineEditSession->setText(settings.value(QLatin1String("lineEditSession")).toString());
    ui->lineEditTx->setText(settings.value(QLatin1String("lineEditTx")).toString());
    ui->lineEditCond->setText(settings.value(QLatin1String("lineEditCond")).toString());

    ui->checkBoxCompression->setChecked(settings.value(QLatin1String("checkBoxCompression")).toBool());
    ui->checkBoxIncrement->setChecked(settings.value(QLatin1String("checkBoxIncrement")).toBool());
    ui->checkBoxNag->setChecked(settings.value(QLatin1String("checkBoxNag")).toBool());

    settings.endGroup();
    settings.sync();
}

///
/// \brief AvRecorder::togglePause
///
void AvRecorder::togglePause()
{
#ifdef QT_DEBUG
        qDebug() << "AvRecorder::togglePause()";
#endif

    if (audioRecorder->state() != QMediaRecorder::PausedState)
    {
        audioRecorder->pause();
    }
    else
    {
        audioRecorder->record();
    }
}

///
/// \brief AvRecorder::displayErrorMessage
///
void AvRecorder::displayErrorMessage()
{
    ui->statusbar->showMessage(audioRecorder->errorString());
}

///
/// \brief AvRecorder::displayErrorMessage
/// \param e
///
void AvRecorder::displayErrorMessage(const QString &e)
{
    ui->statusbar->showMessage(e);
}

///
/// \brief getPeakValue
///
/// This function returns the maximum possible sample value for a given audio format
///
/// \param format
/// \return
///
qreal getPeakValue(const QAudioFormat& format)
{
    // Note: Only the most common sample formats are supported
    if (!format.isValid())
        return qreal(0);

    if (format.codec() != "audio/pcm")
        return qreal(0);

    switch (format.sampleType()) {
    case QAudioFormat::Unknown:
        break;
    case QAudioFormat::Float:
        if (format.sampleSize() != 32) // other sample formats are not supported
            return qreal(0);
        return qreal(1.00003);
    case QAudioFormat::SignedInt:
        if (format.sampleSize() == 32)
            return qreal(INT_MAX);
        if (format.sampleSize() == 16)
            return qreal(SHRT_MAX);
        if (format.sampleSize() == 8)
            return qreal(CHAR_MAX);
        break;
    case QAudioFormat::UnSignedInt:
        if (format.sampleSize() == 32)
            return qreal(UINT_MAX);
        if (format.sampleSize() == 16)
            return qreal(USHRT_MAX);
        if (format.sampleSize() == 8)
            return qreal(UCHAR_MAX);
        break;
    }

    return qreal(0);
}

///
/// \brief getBufferLevels
///
/// returns the audio level for each channel
///
/// \param buffer
/// \return
///
QVector<qreal> getBufferLevels(const QAudioBuffer& buffer)
{
    QVector<qreal> values;

    if (!buffer.format().isValid() || buffer.format().byteOrder() != QAudioFormat::LittleEndian)
        return values;

    if (buffer.format().codec() != "audio/pcm")
        return values;

    int channelCount = buffer.format().channelCount();
    values.fill(0, channelCount);
    qreal peak_value = getPeakValue(buffer.format());
    if (qFuzzyCompare(peak_value, qreal(0)))
        return values;

    switch (buffer.format().sampleType()) {
    case QAudioFormat::Unknown:
    case QAudioFormat::UnSignedInt:
        if (buffer.format().sampleSize() == 32)
            values = getBufferLevels(buffer.constData<quint32>(), buffer.frameCount(), channelCount);
        if (buffer.format().sampleSize() == 16)
            values = getBufferLevels(buffer.constData<quint16>(), buffer.frameCount(), channelCount);
        if (buffer.format().sampleSize() == 8)
            values = getBufferLevels(buffer.constData<quint8>(), buffer.frameCount(), channelCount);
        for (int i = 0; i < values.size(); ++i)
            values[i] = qAbs(values.at(i) - peak_value / 2) / (peak_value / 2);
        break;
    case QAudioFormat::Float:
        if (buffer.format().sampleSize() == 32) {
            values = getBufferLevels(buffer.constData<float>(), buffer.frameCount(), channelCount);
            for (int i = 0; i < values.size(); ++i)
                values[i] /= peak_value;
        }
        break;
    case QAudioFormat::SignedInt:
        if (buffer.format().sampleSize() == 32)
            values = getBufferLevels(buffer.constData<qint32>(), buffer.frameCount(), channelCount);
        if (buffer.format().sampleSize() == 16)
            values = getBufferLevels(buffer.constData<qint16>(), buffer.frameCount(), channelCount);
        if (buffer.format().sampleSize() == 8)
            values = getBufferLevels(buffer.constData<qint8>(), buffer.frameCount(), channelCount);
        for (int i = 0; i < values.size(); ++i)
            values[i] /= peak_value;
        break;
    }

    return values;
}

///
///
///
template <class T> QVector<qreal> getBufferLevels(const T *buffer, int frames, int channels)
{
    QVector<qreal> max_values;
    max_values.fill(0, channels);

    for (int i = 0; i < frames; ++i) {
        for (int j = 0; j < channels; ++j) {
            qreal value = qAbs(qreal(buffer[i * channels + j]));
            if (value > max_values.at(j))
                max_values.replace(j, value);
        }
    }

    return max_values;
}

///
/// \brief AvRecorder::processBuffer
/// \param buffer
///
void AvRecorder::processBuffer(const QAudioBuffer& buffer)
{
    if (audioLevels.count() != buffer.format().channelCount()) {
        qDeleteAll(audioLevels);
        audioLevels.clear();
        for (int i = 0; i < buffer.format().channelCount(); ++i) {
            QAudioLevel *level = new QAudioLevel(ui->centralwidget);
            audioLevels.append(level);
            ui->levelsLayout->addWidget(level);
        }
    }

    QVector<qreal> levels = getBufferLevels(buffer);
    for (int i = 0; i < levels.count(); ++i)
        audioLevels.at(i)->setLevel(levels.at(i));
}

///
/// \brief AvRecorder::processQImage
/// \param qimg
///
void AvRecorder::processQImage(const QImage qimg) {
    ui->viewfinder_0->setPixmap(QPixmap::fromImage(qimg.scaled(ui->viewfinder_0->width(), ui->viewfinder_0->height(),
                                                               Qt::KeepAspectRatio)));
    ui->viewfinder_0->show();
}

///
/// \brief AvRecorder::setCameraOutput
/// \param wxh
///
void AvRecorder::setCameraOutput(QString wxh) {
    emit cameraOutput(wxh);
}

///
/// \brief AvRecorder::setCameraFramerate
/// \param fps
///
void AvRecorder::setCameraFramerate(QString fps) {
    emit cameraFramerate(fps);
}

///
/// \brief AvRecorder::setCamera0State
/// \param state
///
void AvRecorder::setCamera0State(int state) {
    emit cameraPowerChanged(0, state);
}

///
/// \brief AvRecorder::isSessionAnInt
/// \return
///
bool AvRecorder::isSessionAnInt()
{
#ifdef QT_DEBUG
        qDebug() << "AvRecorder::isSessionAnInt()";
#endif

    bool check;

    sessionNumber = ui->lineEditSession->text().toInt(&check);

    return check;
}

