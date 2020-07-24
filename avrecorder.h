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

#ifndef AVRECORDER_H
#define AVRECORDER_H

#include <QMainWindow>
#include <QMediaRecorder>
#include <QDateTime>
#include <QSettings>
#include <QProcess>
#include <QMessageBox>

#include "recordsettings.h"

QT_BEGIN_NAMESPACE
namespace Ui { class AvRecorder; }
class QAudioRecorder;
class QAudioProbe;
class QAudioBuffer;
QT_END_NAMESPACE

class QAudioLevel;

class AvRecorder : public QMainWindow
{
    Q_OBJECT

public:
    AvRecorder(RecordSettingsData* recordSettings, QWidget *parent = 0);
    void LoadPreviousOptions(RecordSettingsData *mSettings);
    ~AvRecorder();

signals:
    void outputDirectory(const QString&);
    void stateChanged(QMediaRecorder::State);
    void cameraOutput(QString);
    void cameraFramerate(QString);
    void cameraPowerChanged(int, int);
    void sendSessionDetails(QString, QString, QString, QString);

    void changeSessionConditionSignal(int, QString);

public slots:
    void processBuffer(const QAudioBuffer&);
    void processQImage(const QImage qimg);
    void displayErrorMessage(const QString&);
    void setCameraStatus(bool value);

    void changeIdSlot(QString);
    void changeSessionSlot(QString);
    void changeTreatmentSlot(QString);
    void changeConditionSlot(QString);

    void processStarted();
    void readyReadStandardOutput();
    void encodingFinished();

private slots:
    void togglePause();
    void toggleRecord();

    void setCameraOutput(QString);
    void setCameraFramerate(QString);
    void setCamera0State(int);

    void updateStatus(QMediaRecorder::Status);
    void onStateChanged(QMediaRecorder::State);
    void updateProgress(qint64 pos);

    void displayErrorMessage();

    void processError(QProcess::ProcessError err);

private:
    void changeShownResolution(QString val);

    bool isSessionAnInt();

    void SaveCurrentOptions();
    void LoadCurrentOptions();

    Ui::AvRecorder *ui;

    QProcess *combineStreamProcess;

    QAudioRecorder *audioRecorder;
    QAudioProbe *probe;
    QList<QAudioLevel*> audioLevels;

    QDateTime rec_started;

    int sessionNumber;

    // TODO: change hard-coded values
    int channelCount = 1;

    QString comboBoxVideoDevice;
    double lineEditVideoFPS;

    QString comboBoxAudioDevice;
    QString comboBoxAudioCodec;
    QString comboBoxAudioSampling;

    QString lineEditOutputDirectory;
    QString lineEditFFmpegDirectory;
};

#endif // AVRECORDER_H
