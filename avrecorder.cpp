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

AvRecorder::AvRecorder(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::AvRecorder),
    outputLocationSet(false)
{
    ui->setupUi(this);
    resize(0,0);

    audioRecorder = new QAudioRecorder(this);
    probe = new QAudioProbe;
    connect(probe, SIGNAL(audioBufferProbed(QAudioBuffer)),
            this, SLOT(processBuffer(QAudioBuffer)));
    probe->setSource(audioRecorder);

    // Video devices
    ui->videoDeviceBox->addItem(tr("Default"), QVariant(QString()));

    // Video codecs
    ui->comboBoxCodec->addItem(tr("Default"), QVariant(QString()));

    //audio devices
    ui->audioDeviceBox->addItem(tr("Default"), QVariant(QString()));
    foreach (const QString &device, audioRecorder->audioInputs()) {
        ui->audioDeviceBox->addItem(device, QVariant(device));
    }

    //audio codecs
    ui->audioCodecBox->addItem(tr("Default"), QVariant(QString()));
    foreach (const QString &codecName, audioRecorder->supportedAudioCodecs()) {
        ui->audioCodecBox->addItem(codecName, QVariant(codecName));
    }

    //containers
    //ui->containerBox->addItem(tr("Default"), QVariant(QString()));
    //foreach (const QString &containerName, audioRecorder->supportedContainers()) {
    //    ui->containerBox->addItem(containerName, QVariant(containerName));
    //}

    //sample rate
    //ui->sampleRateBox->addItem(tr("Default"), QVariant(0));
    //foreach (int sampleRate, audioRecorder->supportedAudioSampleRates()) {
    //    ui->sampleRateBox->addItem(QString::number(sampleRate), QVariant(
    //            sampleRate));
    //}

    //channels
    //ui->channelsBox->addItem(tr("Default"), QVariant(-1));
    //ui->channelsBox->addItem(QStringLiteral("1"), QVariant(1));
    //ui->channelsBox->addItem(QStringLiteral("2"), QVariant(2));
    //ui->channelsBox->addItem(QStringLiteral("4"), QVariant(4));

    //quality
    //ui->qualitySlider->setRange(0, int(QMultimedia::VeryHighQuality));
    //ui->qualitySlider->setValue(int(QMultimedia::NormalQuality));

    //bitrates:
    //ui->bitrateBox->addItem(tr("Default"), QVariant(0));
    //ui->bitrateBox->addItem(QStringLiteral("32000"), QVariant(32000));
    //ui->bitrateBox->addItem(QStringLiteral("64000"), QVariant(64000));
    //ui->bitrateBox->addItem(QStringLiteral("96000"), QVariant(96000));
    //ui->bitrateBox->addItem(QStringLiteral("128000"), QVariant(128000));

    //camera output sizes:
    //ui->cameraOutBox->addItem("Original");
    //ui->cameraOutBox->addItem("960x540");
    //ui->cameraOutBox->addItem("768x432");
    //ui->cameraOutBox->addItem("640x360");
    //ui->cameraOutBox->addItem("480x270");
    //ui->cameraOutBox->setCurrentIndex(3); // Needs to match CameraThread::output_size

    //camera framerates:
    //ui->frameRateBox->addItem("30");
    //ui->frameRateBox->addItem("25");
    //ui->frameRateBox->addItem("15");
    //ui->frameRateBox->addItem("10");
    //ui->frameRateBox->addItem("5");
    //ui->frameRateBox->setCurrentIndex(1);  // Needs to match CameraThread::framerate

    connect(audioRecorder, SIGNAL(durationChanged(qint64)), this, SLOT(updateProgress(qint64)));
    connect(audioRecorder, SIGNAL(statusChanged(QMediaRecorder::Status)), this, SLOT(updateStatus(QMediaRecorder::Status)));
    connect(audioRecorder, SIGNAL(stateChanged(QMediaRecorder::State)), this, SLOT(onStateChanged(QMediaRecorder::State)));
    connect(audioRecorder, SIGNAL(error(QMediaRecorder::Error)), this, SLOT(displayErrorMessage()));

    connect(ui->recordButton, SIGNAL(clicked(bool)), this, SLOT(toggleRecord()));
    connect(ui->outputButton, SIGNAL(clicked(bool)), this, SLOT(setOutputLocation()));

    connect(ui->lineEditId, SIGNAL(textChanged(QString)), this, SLOT(changeIdSlot(QString)));
    connect(ui->lineEditSession, SIGNAL(textChanged(QString)), this, SLOT(changeSessionSlot(QString)));
    connect(ui->lineEditTx, SIGNAL(textChanged(QString)), this, SLOT(changeTreatmentSlot(QString)));
    connect(ui->lineEditCond, SIGNAL(textChanged(QString)), this, SLOT(changeConditionSlot(QString)));

    defaultDir = QDir::homePath() + "/SessionRecordings";

    setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);

    this->statusBar()->setSizeGripEnabled(false);

    setFixedSize(this->maximumSize());

    QSettings settings(QSettings::UserScope, QLatin1String("Session Recorder"));
    settings.beginGroup(QLatin1String("AvRecorder"));

    dirName = settings.value(QLatin1String("StoredOutputDirectory")).toString();

    ui->lineEditOutputFolder->setText(dirName);

    settings.endGroup();
    settings.sync();

    QDir dir(dirName);
    if (!dir.exists())
    {
        if (!dir.mkpath("."))
        {

#ifdef QT_DEBUG
            qWarning() << "WARNING: Failed to create directory" << defaultDir;
#endif

            defaultDir = QDir::homePath();

            dirName = defaultDir;
        }

        outputLocationSet = false;
    }
    else
    {
        outputLocationSet = true;
    }

    setOutputLocation();
    emit outputDirectory(dirName);

    combineStreamProcess = new QProcess(this);
    connect(combineStreamProcess, SIGNAL(started()), this, SLOT(processStarted()));

#ifdef QT_DEBUG
    connect(combineStreamProcess, SIGNAL(readyReadStandardOutput()), this,SLOT(readyReadStandardOutput()));
    connect(combineStreamProcess, SIGNAL(errorOccurred(QProcess::ProcessError)), this, SLOT(processError(QProcess::ProcessError)));
#endif

    //connect(combineStreamProcess, SIGNAL(finished(int)), this, SLOT(encodingFinished()));
}

///
/// \brief AvRecorder::setCameraStatus
/// \param value
///
void AvRecorder::setCameraStatus(bool value)
{
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

    QFileInfo wavFile(dirName+"/audio.wav");
    QFileInfo ca1File(dirName+"/capture0.avi");

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
    QString program = "/usr/local/bin/ffmpeg";

    QString idNumber = QString::number(ui->lineEditId->text().toInt()).rightJustified(4, '0');
    QString sessNumber = QString::number(ui->lineEditSession->text().toInt()).rightJustified(4, '0');

    QDir dirNew(QString("%1/%2/%3").arg(dirName)
             .arg(idNumber)
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

        QDir::setCurrent(dirName);
        combineStreamProcess->setWorkingDirectory(dirName);

        if (!dirNew.exists())
        {
            dirNew.mkpath(".");
        }

        combineStreamProcess->start(QString("%1 -y -i capture.avi -i audio.wav -async 1 -c copy %2/%3/%4-output.avi")
                                    .arg(program)
                                    .arg(idNumber)
                                    .arg(ui->lineEditTx->text())
                                    .arg(sessNumber));

        statusMessage = tr("Recording stopped");

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
/// \brief boxValue
/// \param box
/// \return
///
static QVariant boxValue(const QComboBox *box)
{
    int idx = box->currentIndex();

    if (idx == -1)
    {
        return QVariant();
    }

    return box->itemData(idx);
}

///
/// \brief AvRecorder::toggleRecord
///
void AvRecorder::toggleRecord()
{
    if (!outputLocationSet)
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Session recorder");
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setText("Create target directory first");
        msgBox.setInformativeText("Before recording, you need to create a directory "
                      "to store the media files.");
        msgBox.exec();
            setOutputLocation();
    }

    if (!outputLocationSet)
        return;

    emit outputDirectory(dirName);

    if (audioRecorder->state() == QMediaRecorder::StoppedState) {
        audioRecorder->setAudioInput(boxValue(ui->audioDeviceBox).toString());

    if (!OutputLocationEmptyOrOk())
        return;

    emit sendSessionDetails(ui->lineEditId->text(),
                            ui->lineEditSession->text(),
                            ui->lineEditTx->text(),
                            ui->lineEditCond->text());

/*
    //sample rate
    ui->sampleRateBox->addItem(tr("Default"), QVariant(0));
    foreach (int sampleRate, audioRecorder->supportedAudioSampleRates()) {
        ui->sampleRateBox->addItem(QString::number(sampleRate), QVariant(
                sampleRate));
    }

    //channels
    ui->channelsBox->addItem(tr("Default"), QVariant(-1));
    ui->channelsBox->addItem(QStringLiteral("1"), QVariant(1));
    ui->channelsBox->addItem(QStringLiteral("2"), QVariant(2));
    ui->channelsBox->addItem(QStringLiteral("4"), QVariant(4));

    //quality
    ui->qualitySlider->setRange(0, int(QMultimedia::VeryHighQuality));
    ui->qualitySlider->setValue(int(QMultimedia::NormalQuality));

    //bitrates:
    ui->bitrateBox->addItem(tr("Default"), QVariant(0));
    ui->bitrateBox->addItem(QStringLiteral("32000"), QVariant(32000));
    ui->bitrateBox->addItem(QStringLiteral("64000"), QVariant(64000));
    ui->bitrateBox->addItem(QStringLiteral("96000"), QVariant(96000));
    ui->bitrateBox->addItem(QStringLiteral("128000"), QVariant(128000));
*/

        QAudioEncoderSettings settings;
        settings.setCodec(boxValue(ui->audioCodecBox).toString());

        //settings.setSampleRate(boxValue(ui->sampleRateBox).toInt());
        settings.setSampleRate(sampleRate);

        //settings.setBitRate(boxValue(ui->bitrateBox).toInt());

        //settings.setChannelCount(boxValue(ui->channelsBox).toInt());
        settings.setChannelCount(channelCount);

        //settings.setQuality(QMultimedia::EncodingQuality(ui->qualitySlider->value()));
        settings.setQuality(QMultimedia::EncodingQuality(1));

        //settings.setEncodingMode(ui->constantQualityRadioButton->isChecked() ?
        //                         QMultimedia::ConstantQualityEncoding :
        //                         QMultimedia::ConstantBitRateEncoding);

        settings.setEncodingMode(QMultimedia::ConstantQualityEncoding);

        //QString container = boxValue(ui->containerBox).toString();

        audioRecorder->setEncodingSettings(settings, QVideoEncoderSettings(), QString());
        audioRecorder->record();

        rec_started = QDateTime::currentDateTime();

        //QFile timefile(dirName+"/starttime.txt");
        //if (timefile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        //    QTextStream out(&timefile);
        //    out << rec_started.toString("yyyy-MM-dd'T'hh:mm:sst") << "\n";
        //    timefile.close();
        //}

        //QFile hostfile(dirName+"/hostname.txt");
        //if (hostfile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        //    QTextStream out(&hostfile);
        //    out << QHostInfo::localHostName() << "\n";
        //    hostfile.close();
        //}

        ui->lineEditId->setEnabled(false);
        ui->lineEditSession->setEnabled(false);
        ui->lineEditTx->setEnabled(false);
        ui->lineEditCond->setEnabled(false);

        ui->audioCodecBox->setEnabled(false);
        ui->audioDeviceBox->setEnabled(false);
    }
    else {
        ui->lineEditId->setEnabled(true);
        ui->lineEditSession->setEnabled(true);
        ui->lineEditTx->setEnabled(true);
        ui->lineEditCond->setEnabled(true);

        ui->audioCodecBox->setEnabled(true);
        ui->audioDeviceBox->setEnabled(true);

        audioRecorder->stop();
    }
}

///
/// \brief AvRecorder::togglePause
///
void AvRecorder::togglePause()
{
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
/// \brief AvRecorder::setOutputLocation
///
void AvRecorder::setOutputLocation() {
    if (!dirName.isNull() && !dirName.isEmpty())
    {
        ui->statusbar->showMessage("Output directory: " + dirName);

        audioRecorder->setOutputLocation(QUrl::fromLocalFile(dirName + "/audio.wav"));

        emit outputDirectory(dirName);

        ui->lineEditOutputFolder->setText(dirName);

        outputLocationSet = true;

        QSettings settings(QSettings::UserScope, QLatin1String("Session Recorder"));
        settings.beginGroup(QLatin1String("AvRecorder"));

        settings.setValue(QLatin1String("StoredOutputDirectory"), dirName);

        settings.endGroup();
        settings.sync();
    }
    else
    {
        dirName = QFileDialog::getExistingDirectory(this,
                                                    "Select or create a directory",
                                                    defaultDir,
                                                    QFileDialog::ShowDirsOnly);


        if (!dirName.isNull() && !dirName.isEmpty())
        {
            ui->statusbar->showMessage("Output directory: "+dirName);

            audioRecorder->setOutputLocation(QUrl::fromLocalFile(dirName+"/audio.wav"));

            emit outputDirectory(dirName);

            ui->lineEditOutputFolder->setText(defaultDir);

            outputLocationSet = true;

            QSettings settings(QSettings::UserScope, QLatin1String("Session Recorder"));
            settings.beginGroup(QLatin1String("AvRecorder"));

            settings.setValue(QLatin1String("StoredOutputDirectory"), dirName);

            settings.endGroup();
            settings.sync();
        }
        else
        {
            outputLocationSet = false;
        }
    }
}

///
/// \brief AvRecorder::OutputLocationEmptyOrOk
/// \return
///
bool AvRecorder::OutputLocationEmptyOrOk() {
    QDir dir(dirName);

    if (!dir.exists())
    {

#ifdef QT_DEBUG
        qDebug() << "OutputLocationEmptyOrOk(): Directory"
                 << dirName
                 << "does not exist, this should not happen";
#endif

        return false;
    }

    if (dir.entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries).count())
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Session Recorder");
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("Directory not empty.");
        msgBox.setInformativeText("The selected output directory for recording "
                      "is not empty. The old recording will be "
                      "overwritten.");

        return (msgBox.exec() == QMessageBox::Ok);
    }

    return true;
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
    ui->viewfinder_0->setPixmap(QPixmap::fromImage(qimg));
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
