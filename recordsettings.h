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

#ifndef RECORDSETTINGS_H
#define RECORDSETTINGS_H

#include <QString>

struct RecordSettingsData
{
    QString ffmpegLocation;
    QString fileSaveLocation;

    QString mVideoDevice;
    QString mVideoFPS;
    QString mResolution;

    QString mAudioDevice;
    QString mAudioEncoding;
    QString mAudioSampling;
};

class RecordSettings
{
    public:

        RecordSettings()
        {
            data = new RecordSettingsData;
        }

        RecordSettings(const RecordSettings& other)
        {
            data = new RecordSettingsData(*other.data);
        }

        ~RecordSettings()
        {
            delete data;
        }

        RecordSettings& operator=( const RecordSettings& other )
        {
            *data = *other.data;
            return *this;
        }

        void storeData(const QString &ffmpegLocation, const QString &fileSaveLocation,
                       const QString &mVideoDevice, const QString &mVideoFPS, const QString &mResolution,
                       const QString &mAudioDevice, const QString &mAudioEncoding, const QString &mAudioSampling)
        {
            data->ffmpegLocation   = ffmpegLocation;
            data->fileSaveLocation = fileSaveLocation;

            data->mVideoDevice     = mVideoDevice;
            data->mVideoFPS        = mVideoFPS;
            data->mResolution      = mResolution;

            data->mAudioDevice     = mAudioDevice;
            data->mAudioEncoding   = mAudioEncoding;
            data->mAudioSampling   = mAudioSampling;
        }

        RecordSettingsData* getData()
        {
            return data;
        }

    private:
        RecordSettingsData *data;
};

#endif // RECORDSETTINGS_H
