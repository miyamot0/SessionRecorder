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

****************************************************************************/

#ifndef INITIALIZATIONDIALOG_H
#define INITIALIZATIONDIALOG_H

#include <QDialog>

#include <QAudioRecorder>
#include <QCameraInfo>
#include <QAbstractButton>
#include <QSettings>
#include <QStandardItemModel>
#include <QFileDialog>

#include "enums.h"

namespace Ui {
class InitializationDialog;
}

class InitializationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InitializationDialog(QWidget *parent = 0);
    int getSelectedVideoSource();
    QString getSelectedResolution();

    ~InitializationDialog();

public slots:
    void SaveAndValidate(QAbstractButton *clicked);

    void SelectOutputDirectory(bool);
    void SelectFFmpegDirectory(bool);

    void AspectRatioChanged(int index);

private:
    Ui::InitializationDialog *ui;

    void LoadPreviousOptions();
    void SaveCurrentOptions();

};

#endif // INITIALIZATIONDIALOG_H
