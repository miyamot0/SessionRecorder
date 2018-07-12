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

    This file was adapted from meeting-recorder (MIT).

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

****************************************************************************/

#ifndef CAMERATHREAD_H
#define CAMERATHREAD_H

#include <QThread>
#include <QImage>
#include <QMediaRecorder>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"

/**
#include "opencv/highgui.h"
#include "opencv/cv.h"
#include <opencv/cxcore.h>
 */

using namespace cv;

class CameraThread : public QThread
{
    Q_OBJECT

    void run();

signals:
    void qimgReady(const QImage qimg);
    void resultReady(const QString &s);
    void cameraInfo(int, int, int);
    void errorMessage(const QString &e);
    void cameraConnected(bool);

public slots:
    void setOutputDirectory(const QString &d);
    void onStateChanged(QMediaRecorder::State);
    void setCameraOutput(QString);
    void setCameraFramerate(QString);
    void setCameraPower(int, int);
    void updateSessionConditions(QString id, QString session, QString treatment, QString condition);

    void updateSessionConditions(int index, QString value);

public:
    CameraThread(int i);
    CameraThread(int i, QString wxh);

    void breakLoop();

private:
    QImage Mat2QImage(cv::Mat const& src);

    void resizeAR(cv::Mat &, cv::Size);
    void setDefaultDesiredInputSize();

    int fourcc;
    int idx;

    cv::Size desired_input_size;
    cv::Size input_size;

    bool record_video;
    bool is_active;
    bool was_active;

    cv::VideoWriter video;

    cv::Size output_size;
    cv::Size window_size;

    QString outdir;
    QString filename;

    bool stopLoop;

    QString winId = "";
    QString winSession = "";
    QString winTreatment = "";
    QString winCondition = "";

    // Hard-coded values
    int winSizeH = 640;
    int winSizeV = 360;

    Point topRect1;
    Point topRect2;

    Point topText1;
    Point topText2;
    Point topText3;
    Point topText4;

    double avgload = 0.0;
    size_t nframe = 0;

    // Default annotation values
    Scalar yellowColor = Scalar(255, 255, 255);
    Scalar blackColor = Scalar(0, 0, 0);
    double fontScale = 1.0;

    // TODO: adjustable framerate
    int framerate = 30;
};

#endif // CAMERATHREAD_H
