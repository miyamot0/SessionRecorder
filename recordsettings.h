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

        RecordSettings::RecordSettings()
        {
            data = new RecordSettingsData;
        }

        RecordSettings::RecordSettings(const RecordSettings& other)
        {
            data = new RecordSettingsData(*other.data);
        }

        RecordSettings::~RecordSettings()
        {
            delete data;
        }

        RecordSettings& RecordSettings::operator=( const RecordSettings& other )
        {
            *data = *other.data;
            return *this;
        }

        void RecordSettings::storeData(const QString &ffmpegLocation, const QString &fileSaveLocation,
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

        RecordSettingsData* RecordSettings::getData()
        {
            return data;
        }

    private:
        RecordSettingsData *data;
};

#endif // RECORDSETTINGS_H
