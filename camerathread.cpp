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

#ifdef QT_DEBUG
#include <QDebug>
#endif

#include <QDir>
#include <QDateTime>
#include <QTextStream>
#include <QLinkedList>
#include <QSettings>
#include <QStandardPaths>

#include "boost/date_time/posix_time/posix_time.hpp"

#include "camerathread.h"

using namespace boost::posix_time;
using namespace cv;

///
/// \brief CameraThread::CameraThreadB
///
/// Construct with a default resolution
///
/// \param i
///
/// Camera id number
///
CameraThread::CameraThread(int i) : idx(i), is_active(false), was_active(false)
{
#ifdef QT_DEBUG
    qDebug() << "CameraThread::CameraThread(int i)";
#endif

    setDefaultDesiredInputSize();

    window_size = Size(desired_input_size.width, desired_input_size.height);

#ifdef QT_DEBUG
    qDebug() << QString("Width: %1, Height: %2").arg(window_size.width).arg(window_size.height);
#endif

    QSettings settings(QSettings::UserScope, QLatin1String("Session Recorder"));
    settings.beginGroup(QLatin1String("AvRecorder"));

    winId =         settings.value(QLatin1String("lineEditId")).toString();
    winSession =    settings.value(QLatin1String("lineEditSession")).toString().rightJustified(4, '0');
    winTreatment =  settings.value(QLatin1String("lineEditTx")).toString();
    winCondition =  settings.value(QLatin1String("lineEditCond")).toString();

    settings.endGroup();
    settings.sync();

    tempWriteLocation = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
}

///
/// \brief CameraThread::CameraThread
///
/// Construct with supplied resolution
///
/// \param i
///
/// Camera id number
///
/// \param wxh
///
/// QString resolution (width x height)
///
CameraThread::CameraThread(int i, QString wxh) : idx(i), is_active(false), was_active(false)
{
#ifdef QT_DEBUG
    qDebug() << "CameraThread::CameraThread(int i, QString wxh)";
#endif

    if (wxh.contains('x'))
    {
        QStringList wh = wxh.split('x');

        bool ok = true;

        desired_input_size.width = wh.at(0).toInt(&ok);
        desired_input_size.height = wh.at(1).toInt(&ok);

        if (!ok)
        {
            setDefaultDesiredInputSize();
        }
    }
    else
    {
      setDefaultDesiredInputSize();
    }

    window_size = Size(desired_input_size.width, desired_input_size.height);

#ifdef QT_DEBUG
    qDebug() << QString("Width: %1, Height: %2").arg(window_size.width).arg(window_size.height);
#endif

    QSettings settings(QSettings::UserScope, QLatin1String("Session Recorder"));
    settings.beginGroup(QLatin1String("AvRecorder"));

    winId =         settings.value(QLatin1String("lineEditId")).toString();
    winSession =    settings.value(QLatin1String("lineEditSession")).toString().rightJustified(4, '0');
    winTreatment =  settings.value(QLatin1String("lineEditTx")).toString();
    winCondition =  settings.value(QLatin1String("lineEditCond")).toString();

    settings.endGroup();
    settings.sync();

    tempWriteLocation = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
}

///
/// \brief CameraThread::setDefaultDesiredInputSize
///
/// Set desired resolution to default (720p)
///
void CameraThread::setDefaultDesiredInputSize()
{
  desired_input_size.width = 640;
  desired_input_size.height = 480;
}

///
/// \brief CameraThread::updateSessionConditions
///
/// SLOT for updating annotations on camera
///
/// \param id
/// \param session
/// \param treatment
/// \param condition
///
void CameraThread::updateSessionConditions(QString id, QString session, QString treatment, QString condition)
{
    winId = id;
    winSession = session;
    winTreatment = treatment;
    winCondition = condition;
}

///
/// \brief CameraThread::updateSessionConditions
///
/// As stated
///
/// \param index
/// \param value
///
void CameraThread::updateSessionConditions(int index, QString value)
{
    switch (index) {
    case 0:
        winId = value;

        break;
    case 1:
        winSession = value.rightJustified(4, '0');

        break;
    case 2:
        winTreatment = value;

        break;
    case 3:
        winCondition = value;

        break;

    default:
        break;
    }
}

///
/// \brief CameraThread::run
///
/// Core thread
/// Note: Q_DECL_OVERRIDE produces an error on OS X 10.11 / Qt 5.6:
///
///
void CameraThread::run() { //Q_DECL_OVERRIDE
#ifdef QT_DEBUG
    qDebug() << "CameraThread::run()";
#endif

    QString result;

    time_duration td, td1, td2;
    ptime nextFrameTimestamp, currentFrameTimestamp;
    ptime initialLoopTimestamp, processingDoneTimestamp, finalLoopTimestamp;

    // initialize capture on default source
    VideoCapture capture(idx);

    if (!capture.isOpened())
    {
        emit errorMessage(QString("Warning: Failed to initialize camera %1.").arg(idx));

        emit cameraConnected(false);

        return;
    }
    else {
#ifdef QT_DEBUG
        qDebug() << "Camera connected!";
#endif
    }

    emit cameraConnected(true);

    input_size.width =  capture.get(CV_CAP_PROP_FRAME_WIDTH);
    input_size.height = capture.get(CV_CAP_PROP_FRAME_HEIGHT);

#ifdef QT_DEBUG
    qDebug() << "Camera" << idx
             << ": Input size: width:" << input_size.width
             << "height:" << input_size.height;
#endif

    emit cameraInfo(idx, input_size.width, input_size.height);

    outdir = "";

    record_video = false;

    qDebug() << VIDEOSTRING;

    //fourcc = -1;
    fourcc = CV_FOURCC('m','p','4','v');
    //fourcc = CV_FOURCC('M','J','P','G');
    //fourcc = CV_FOURCC('H','2','6','4');

    output_size = Size(input_size.width, input_size.height);

    nextFrameTimestamp = microsec_clock::local_time();
    currentFrameTimestamp = nextFrameTimestamp;
    td = (currentFrameTimestamp - nextFrameTimestamp);

    QLinkedList<time_duration> tdlist;

    stopLoop = false;
    is_active = true;

    topRect1 = Point(2, 4);
    topRect2 = Point(180, 4 + (4 * 14));

    topText1 = Point(8, 4 + 14 - 2);
    topText2 = Point(8, 4 + 28 - 2);
    topText3 = Point(8, 4 + 42 - 2);
    topText4 = Point(8, 4 + 56 - 2);

    QDateTime datetime;
    for (;;)
    {
        if (stopLoop)
        {
#ifdef QT_DEBUG
            qDebug() << "Camera" << idx << "stopping";
#endif

            break;
        }

        // Happens when Stop was pressed:
        if (!record_video && video.isOpened())
        {
            video.release();
        }

        // determine time at start of loop
        initialLoopTimestamp = microsec_clock::local_time();

        Mat frame;
        capture >> frame;
        nframe++;

        if (is_active)
        {
            was_active = true;

          if (frame.cols && frame.rows)
          {
              datetime = QDateTime::currentDateTime();

              rectangle(frame,
                        topRect1,
                        topRect2,
                        blackColor,
                        CV_FILLED);

              putText(frame,
                      QString("ID: %1").arg(winId).toStdString().c_str(),
                      topText1,
                      fontStyle,
                      fontScale,
                      yellowColor);

              putText(frame,
                      QString("Session: %1").arg(winSession).toStdString().c_str(),
                      topText2,
                      fontStyle,
                      fontScale,
                      yellowColor);

              putText(frame,
                      QString("Treatment: %1").arg(winTreatment).toStdString().c_str(),
                      topText3,
                      fontStyle,
                      fontScale,
                      yellowColor);

              putText(frame,
                      QString("Condition: %1").arg(winCondition).toStdString().c_str(),
                      topText4,
                      fontStyle,
                      fontScale,
                      yellowColor);

              // Each line is approx 14

              rectangle(frame,
                        Point(2,frame.rows-22),
                        Point(datetime.toString().toStdString().length() * 10, frame.rows-8),
                        blackColor,
                        CV_FILLED);

              putText(frame,
                      datetime.toString().toStdString().c_str(),
                      Point(10,frame.rows-10),
                      fontStyle,
                      fontScale,
                      yellowColor);

              // Save frame to video
              if (record_video && video.isOpened())
              {
                  video << frame;
              }

              Mat window;

              resize(frame, window, window_size);

              QImage qimg = Mat2QImage(window);

              emit qimgReady(qimg);
          }

#ifdef QT_DEBUG
          else
          {
              qDebug() << "Camera" << idx << ": Skipped frame";
          }
#endif

        }
        else if (was_active)
        {
            was_active = false;
            QImage qimg = Mat2QImage(Mat::zeros(window_size, CV_8UC3));

            emit qimgReady(qimg);
        }

      // determine time when all processing done
      processingDoneTimestamp = microsec_clock::local_time();

      // wait for X microseconds until 1second/framerate time has passed after previous frame write
      while(td.total_microseconds() < 1000000/framerate)
      {
          //determine current elapsed time

          currentFrameTimestamp = microsec_clock::local_time();
          td = (currentFrameTimestamp - nextFrameTimestamp);
      }

      // add 1second/framerate time for next loop pause
      nextFrameTimestamp = nextFrameTimestamp + microsec(1000000/framerate);

      // reset time_duration so while loop engages
      td = (currentFrameTimestamp - nextFrameTimestamp);

      //determine and print out delay in ms, should be less than 1000/FPS
      //occasionally, if delay is larger than said value, correction will occur
      //if delay is consistently larger than said value, then CPU is not powerful
      // enough to capture/decompress/record/compress that fast.
      finalLoopTimestamp = microsec_clock::local_time();
      td1 = (processingDoneTimestamp - initialLoopTimestamp);
      td2 = (finalLoopTimestamp - initialLoopTimestamp);

      tdlist << td1;
      size_t tdlistsize = tdlist.size();

      if (tdlistsize > 100)
      {
          tdlist.removeFirst();
      }

      long total_td = 0;

      QLinkedList<time_duration>::const_iterator it;
      for (it = tdlist.constBegin(); it != tdlist.constEnd(); ++it)
      {
          total_td += it->total_microseconds();
      }

      avgload = total_td*framerate/(tdlistsize*1000000.0);
    }

    emit resultReady(result);
}

///
/// \brief CameraThread::resizeAR
///
/// Resize mat to fit window (keeping aspect ratio)
///
/// \param frame
/// \param osize
///
void CameraThread::resizeAR(Mat &frame, Size osize)
{
    float o_aspect_ratio = float(osize.width)/float(osize.height);
    float f_aspect_ratio = float(frame.cols)/float(frame.rows);

    if (fabs(f_aspect_ratio-o_aspect_ratio)<0.01)
    {
        resize(frame, frame, osize);
        return;
    }

    Mat newframe = Mat::zeros(osize, CV_8UC3);
    size_t roi_width = size_t(f_aspect_ratio*osize.height);
    size_t roi_displacement = (osize.width-roi_width)/2;
    Mat roi(newframe, Rect(roi_displacement, 0, roi_width, osize.height));
    resize(frame, roi, roi.size());
    newframe.copyTo(frame);

    return;
}

///
/// \brief CameraThread::setOutputDirectory
///
/// Sets output directory (really just adds forward slash)
///
/// \param d
///
void CameraThread::setOutputDirectory(const QString &d)
{
#ifdef QT_DEBUG
            qDebug() << QString("setOutputDirectory %1").arg(outdir);
#endif

    outdir = d+"/";
}

///
/// \brief CameraThread::onStateChanged
///
/// SLOT that state of recording has changed
///
/// \param state
///
void CameraThread::onStateChanged(QMediaRecorder::State state)
{
    switch (state) {
    case QMediaRecorder::RecordingState:
        if (!is_active)
        {
            record_video = false;
            break;
        }

        if (!video.isOpened())
        {
#ifdef QT_DEBUG
            qDebug() << QString("CameraThread::onStateChanged(): initializing "
                    "VideoWriter for camera %1; Location %2").arg(idx).arg(tempWriteLocation + "/" + VIDEOSTRING);

            qDebug() << "FourCC: " << fourcc;
#endif



            video.open(QString(tempWriteLocation + "/" + VIDEOSTRING).toStdString(),
                       fourcc,
                       framerate,
                       (output_size.width ? output_size : input_size),
                       true);

#ifdef QT_DEBUG
            qDebug() << "Opened window";
#endif

        }

        if (!video.isOpened())
        {
            emit errorMessage(QString("ERROR: Failed to initialize camera %1").arg(idx));
        }
        else
        {
#ifdef QT_DEBUG
            qDebug() << QString("CameraThread::onStateChanged(): initialization ready for camera %1").arg(idx);
#endif

            record_video = true;
        }

        break;

    case QMediaRecorder::PausedState:
        record_video = false;

        break;

    case QMediaRecorder::StoppedState:
        record_video = false;

        break;
    }
}

///
/// \brief CameraThread::Mat2QImage
///
/// Convert mat to QIMage for display
///
/// \param src
/// \return
///
QImage CameraThread::Mat2QImage(cv::Mat const& src)
{
     cv::Mat temp;

     cvtColor(src, temp,CV_BGR2RGB);

     QImage dest((const uchar *) temp.data,
                 temp.cols,
                 temp.rows,
                 temp.step,
                 QImage::Format_RGB888);

     dest.bits(); // enforce deep copy, see documentation
     return dest;
}

///
/// \brief CameraThread::setCameraOutput
///
/// Set output resultion of recording
///
/// \param wxh
///
void CameraThread::setCameraOutput(QString wxh)
{
#ifdef QT_DEBUG
    qDebug() << "CameraThread::setCameraOutput(): " << wxh;
#endif

    if (wxh == "Original")
    {
        output_size = Size(0,0);
    }
    else
    {
        QStringList wh = wxh.split("x");

        if (wh.length()==2)
        {
            output_size = Size(wh.at(0).toInt(), wh.at(1).toInt());
        }
    }
}

///
/// \brief CameraThread::setCameraFramerate
///
/// Sets hard-coded framerate
///
/// \param fps
///
void CameraThread::setCameraFramerate(QString fps)
{
#ifdef QT_DEBUG
    qDebug() << "CameraThread::setCameraFramerate(): " << fps;
#endif

    framerate  = fps.toInt();
}

///
/// \brief CameraThread::breakLoop
///
/// as stated
///
void CameraThread::breakLoop()
{
    stopLoop = true;
}

///
/// \brief CameraThread::setCameraPower
///
/// As stated
///
/// \param i
/// \param state
///
void CameraThread::setCameraPower(int i, int state)
{
    if (i == idx)
    {
#ifdef QT_DEBUG
        qDebug() << "Camera" << idx << "power now" << state;
#endif

        is_active = state;
    }
}
