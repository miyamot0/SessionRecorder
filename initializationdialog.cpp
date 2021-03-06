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

#include "initializationdialog.h"
#include "ui_initializationdialog.h"

#include <QMessageBox>

InitializationDialog::InitializationDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InitializationDialog)
{
    ui->setupUi(this);

#ifdef QT_DEBUG
    qDebug() << "InitializationDialog::InitializationDialog()";
#endif

    QList<QCameraInfo> cams = QCameraInfo::availableCameras();

    ui->comboBoxVideoDevice->addItem(tr("Default"), QVariant(QString()));

    QStringList videoModel;

    foreach (const QCameraInfo &device, cams) {
        ui->comboBoxVideoDevice->addItem(device.description(), QVariant(device.description()));
        videoModel << device.description();
    }

#ifdef QT_DEBUG
    qDebug() << "Cameras:";
    qDebug() << videoModel;
#endif

    QAudioRecorder* audioRecorder = new QAudioRecorder(this);

    QStringList audioModel;

    //audio devices
    ui->comboBoxAudioDevice->addItem(tr("Default"), QVariant(QString()));
    foreach (const QString &device, audioRecorder->audioInputs()) {
        ui->comboBoxAudioDevice->addItem(device, QVariant(device));
        audioModel << device;
    }

#ifdef QT_DEBUG
    qDebug() << "Microphones:";
    qDebug() << audioModel;
#endif

    //audio codecs, amr by default
    ui->comboBoxAudioCodec->addItem(tr("Default"), QVariant(QString()));
    foreach (const QString &codecName, audioRecorder->supportedAudioCodecs()) {
        ui->comboBoxAudioCodec->addItem(codecName, QVariant(codecName));
    }
    ui->comboBoxAudioCodec->addItem(tr("audio/amr"), QVariant(QString()));
    ui->comboBoxAudioCodec->setCurrentIndex(ui->comboBoxAudioCodec->count() - 1);

    //sample rate
    ui->comboBoxAudioSampling->addItem(tr("Default"), QVariant(0));
    foreach (int sampleRate, audioRecorder->supportedAudioSampleRates()) {
        ui->comboBoxAudioSampling->addItem(QString::number(sampleRate), QVariant(sampleRate));
    }

    ui->comboBoxAspectRatio->addItem(tr("Standard"), QVariant(0));
    ui->comboBoxAspectRatio->addItem(tr("Wide Screen"), QVariant(1));

    // Fire off default 4x3 letterbox
    AspectRatioChanged(0);

    // Load cached options
    LoadPreviousOptions();

    connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(SaveAndValidate(QAbstractButton*)));
    connect(ui->comboBoxAspectRatio, SIGNAL(currentIndexChanged(int)), this, SLOT(AspectRatioChanged(int)));
    connect(ui->pushButtonOutputDirectory, SIGNAL(clicked(bool)), this, SLOT(SelectOutputDirectory(bool)));

#ifdef _WIN32
    connect(ui->pushButtonFFmpegDirectory, SIGNAL(clicked(bool)), this, SLOT(SelectFFmpegDirectory(bool)));
#elif __APPLE__
    ui->pushButtonFFmpegDirectory->setEnabled(false);

    ui->lineEditFFmpegDirectory->setText("/usr/local/bin");
    ui->lineEditFFmpegDirectory->setEnabled(false);
#endif

    setWindowTitle(QString("Session Recorder v" + QString("%1.%2.%3").arg(VERSION_MAJOR).arg(VERSION_MINOR).arg(VERSION_BUILD)));
}

///
/// \brief InitializationDialog::SaveAndValidate
/// \param clicked
///
void InitializationDialog::SaveAndValidate(QAbstractButton* clicked)
{
    Q_UNUSED(clicked);

    if (!QFileInfo (ui->lineEditOutputDirectory->text()).exists() ||
            !QFileInfo (ui->lineEditOutputDirectory->text()).isWritable()) {
        QMessageBox errorBox;
        errorBox.critical(this, "Directory Not Found", "We cannot access the output directory. Reload network file shares?");

        return;
    }

    if (!QFileInfo (ui->lineEditFFmpegDirectory->text()).exists() ||
            !QFileInfo (ui->lineEditFFmpegDirectory->text()).isWritable()) {
        QMessageBox errorBox;
        errorBox.critical(this, "Directory Not Found", "We cannot access the FFMpeg encoding files. Confirm binaries exist?");

        return;
    }

    SaveCurrentOptions();

    QDialog::accept();
}

///
/// \brief InitializationDialog::AspectRatioChanged
///
void InitializationDialog::AspectRatioChanged(int index)
{
    switch (index) {
    case 0:
        ui->comboBoxResolution->clear();

        ui->comboBoxResolution->addItem(tr("320x240"), QVariant("320x240"));
        ui->comboBoxResolution->addItem(tr("640x480"), QVariant("640x480"));
        ui->comboBoxResolution->addItem(tr("1280x960"), QVariant("1280x960"));

        break;

    case 1:
        ui->comboBoxResolution->clear();

        ui->comboBoxResolution->addItem(tr("320x180"), QVariant("320x180"));
        ui->comboBoxResolution->addItem(tr("640x360"), QVariant("640x360"));
        ui->comboBoxResolution->addItem(tr("1280x720"), QVariant("1280x720"));
        break;
    default:
        break;
    }
}

///
/// \brief InitializationDialog::getSelectedVideoSource
/// \return
///
int InitializationDialog::getSelectedVideoSource()
{
    return (ui->comboBoxVideoDevice->currentIndex() != 0) ? ui->comboBoxVideoDevice->currentIndex() - 1 :
                                                            0;
}

///
/// \brief InitializationDialog::getSelectedResolution
/// \return
///
QString InitializationDialog::getSelectedResolution()
{
    return ui->comboBoxResolution->currentText();
}

///
/// \brief InitializationDialog::getRecordingSettings
/// \return
///
RecordSettingsData* InitializationDialog::getRecordingSettings() {
    mSettingsHolder.storeData(
            ui->lineEditFFmpegDirectory->text(),
            ui->lineEditOutputDirectory->text(),

            ui->comboBoxVideoDevice->currentText(),
            ui->lineEditVideoFPS->text(),
            ui->comboBoxResolution->currentText(),

            ui->comboBoxAudioDevice->currentText(),
            ui->comboBoxAudioCodec->currentText(),
            ui->comboBoxAudioSampling->currentText());

    return mSettingsHolder.getData();
}

///
/// \brief InitializationDialog::LoadPreviousOptions
///
void InitializationDialog::LoadPreviousOptions()
{
    QSettings settings(QSettings::UserScope, QLatin1String("Session Recorder"));
    settings.beginGroup(QLatin1String("InitializationDialog"));

    ui->comboBoxVideoDevice->setCurrentText(settings.value(QLatin1String("comboBoxVideoDevice")).toString());
    ui->lineEditVideoFPS->setText(settings.value(QLatin1String("lineEditVideoFPS")).toString());

    ui->comboBoxAudioDevice->setCurrentText(settings.value(QLatin1String("comboBoxAudioDevice")).toString());
    ui->comboBoxAudioCodec->setCurrentText(settings.value(QLatin1String("comboBoxAudioCodec")).toString());
    ui->comboBoxAudioSampling->setCurrentText(settings.value(QLatin1String("comboBoxAudioSampling")).toString());

    ui->lineEditOutputDirectory->setText(settings.value(QLatin1String("lineEditOutputDirectory")).toString());
    ui->lineEditFFmpegDirectory->setText(settings.value(QLatin1String("lineEditFFmpegDirectory")).toString());

    ui->comboBoxAspectRatio->setCurrentText(settings.value(QLatin1String("comboBoxAspectRatio")).toString());
    ui->comboBoxResolution->setCurrentText(settings.value(QLatin1String("comboBoxResolution")).toString());

    AspectRatioChanged(ui->comboBoxAspectRatio->currentIndex());
    ui->comboBoxResolution->setCurrentText(settings.value(QLatin1String("comboBoxResolution")).toString());

    settings.endGroup();
    settings.sync();
}

///
/// \brief InitializationDialog::SaveCurrentOptions
///
void InitializationDialog::SaveCurrentOptions()
{
    QSettings settings(QSettings::UserScope, QLatin1String("Session Recorder"));
    settings.beginGroup(QLatin1String("InitializationDialog"));

    settings.setValue(QLatin1String("comboBoxVideoDevice"), ui->comboBoxVideoDevice->currentText());
    settings.setValue(QLatin1String("lineEditVideoFPS"), ui->lineEditVideoFPS->text());

    settings.setValue(QLatin1String("comboBoxAspectRatio"), ui->comboBoxAspectRatio->currentText());
    settings.setValue(QLatin1String("comboBoxResolution"), ui->comboBoxResolution->currentText());

    settings.setValue(QLatin1String("comboBoxAudioDevice"), ui->comboBoxAudioDevice->currentText());
    settings.setValue(QLatin1String("comboBoxAudioCodec"), ui->comboBoxAudioCodec->currentText());
    settings.setValue(QLatin1String("comboBoxAudioSampling"), ui->comboBoxAudioSampling->currentText());

    settings.setValue(QLatin1String("lineEditOutputDirectory"), ui->lineEditOutputDirectory->text());
    settings.setValue(QLatin1String("lineEditFFmpegDirectory"), ui->lineEditFFmpegDirectory->text());

    settings.endGroup();
    settings.sync();
}

///
/// \brief InitializationDialog::SelectOutputDirectory
///
void InitializationDialog::SelectOutputDirectory(bool)
{
    if (ui->lineEditOutputDirectory->text().isEmpty())
    {
        ui->lineEditOutputDirectory->setText(QDir::homePath());
    }

    QString dirName = QFileDialog::getExistingDirectory(this,
                                                        "Select output directory",
                                                        ui->lineEditOutputDirectory->text(),
                                                        QFileDialog::ShowDirsOnly);

    if (!dirName.isNull() && !dirName.isEmpty())
    {
        ui->lineEditOutputDirectory->setText(dirName);
    }
}

///
/// \brief InitializationDialog::SelectFFmpegDirectory
///
void InitializationDialog::SelectFFmpegDirectory(bool)
{
    if (ui->lineEditFFmpegDirectory->text().isEmpty())
    {
        ui->lineEditFFmpegDirectory->setText(QDir::homePath());
    }

    QString dirName = QFileDialog::getExistingDirectory(this,
                                                        "Select FFmpeg directory",
                                                        ui->lineEditFFmpegDirectory->text(),
                                                        QFileDialog::ShowDirsOnly);

    if (!dirName.isNull() && !dirName.isEmpty())
    {
        ui->lineEditFFmpegDirectory->setText(dirName);
    }
}

///
/// \brief InitializationDialog::~InitializationDialog
///
InitializationDialog::~InitializationDialog()
{
    delete ui;
}
