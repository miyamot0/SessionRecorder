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

#include <QApplication>
#include <QtWidgets>

#include "initializationdialog.h"
#include "avrecorder.h"
#include "camerathread.h"
#include "enums.h"

//#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("Shawn Gilroy");
    QCoreApplication::setOrganizationDomain("smallnstats.com");
    QCoreApplication::setApplicationName("Session Recorder");

    QApplication a(argc, argv);

    InitializationDialog initDlg;
    initDlg.exec();

#ifdef QT_DEBUG
    qDebug() << "Opening AvRecorder";
#endif

    AvRecorder recorder;
    recorder.show();

#ifdef QT_DEBUG
    qDebug() << "Opening Camera Thread";
#endif

    QList<CameraThread *> cameras;

    CameraThread* cam;
    cam = new CameraThread(initDlg.getSelectedVideoSource(),
                           initDlg.getSelectedResolution());

    QObject::connect(&recorder, SIGNAL(outputDirectory(const QString&)), cam, SLOT(setOutputDirectory(const QString&)));

    QObject::connect(&recorder, SIGNAL(stateChanged(QMediaRecorder::State)), cam, SLOT(onStateChanged(QMediaRecorder::State)));
    QObject::connect(&recorder, SIGNAL(cameraOutput(QString)), cam, SLOT(setCameraOutput(QString)));
    QObject::connect(&recorder, SIGNAL(cameraFramerate(QString)), cam, SLOT(setCameraFramerate(QString)));
    QObject::connect(&recorder, SIGNAL(cameraPowerChanged(int, int)), cam, SLOT(setCameraPower(int, int)));
    QObject::connect(&recorder, SIGNAL(sendSessionDetails(QString,QString,QString,QString)), cam, SLOT(updateSessionConditions(QString,QString,QString,QString)));

    QObject::connect(&recorder, SIGNAL(changeSessionConditionSignal(int,QString)), cam, SLOT(updateSessionConditions(int,QString)));

    QObject::connect(cam, SIGNAL(qimgReady(const QImage)), &recorder, SLOT(processQImage(const QImage)));
    QObject::connect(cam, SIGNAL(errorMessage(const QString&)), &recorder, SLOT(displayErrorMessage(const QString&)));
    QObject::connect(cam, SIGNAL(cameraConnected(bool)), &recorder, SLOT(setCameraStatus(bool)));

    // Start thread, once signals for status are connected
    cam->start();

    // Append to list, for easy shutdown (if more than one)
    cameras.append(cam);

    const int retval = a.exec();

    // Kill off cameras on shutdown
    QList<CameraThread *>::iterator i;
    for (i = cameras.begin(); i != cameras.end(); ++i)
    {
        if ((*i)->isRunning())
        {
            (*i)->breakLoop();
            (*i)->quit();

            if (!(*i)->wait(2000))
            {
                (*i)->terminate();

                if (!(*i)->wait(2000))
                {
                    //qDebug() << "CameraThread failed to terminate!";
                }
            }
        }
    }

    return retval;
}
