#ifndef WINDOWS_MEDIA_FOUNDATION_VIDEO_H
#define WINDOWS_MEDIA_FOUNDATION_VIDEO_H

#include "io/resource_loader.h"
#include "scene/resources/video_stream.h"

#include <mfidl.h>

class WindowsMediaFoundationVideo : public VideoStreamPlayback {
    GDCLASS(WindowsMediaFoundationVideo, VideoStreamPlayback);

public:
    virtual void play();
    virtual void stop();
    virtual bool is_playing() const;

    virtual void set_paused(bool p_paused);
    virtual bool is_paused() const;

    virtual void set_loop(bool p_enable);
    virtual bool has_loop() const;

    virtual float get_length() const;
    virtual String get_stream_name() const;

    virtual int get_loop_count() const;

    virtual float get_playback_position() const;
    virtual void seek(float p_time);

    void set_file(const String& p_file);

    virtual Ref<Texture> get_texture();
    virtual void update(float p_delta);

    virtual void set_mix_callback(AudioMixCallback p_callback, void *p_userdata);
    virtual int get_channels() const;
    virtual int get_mix_rate() const;

    virtual void set_audio_track(int p_idx);

    WindowsMediaFoundationVideo();
    ~WindowsMediaFoundationVideo();

private:
    IMFMediaSession* m_pSession;
};

class VideoStreamWMF : public VideoStream {
    GDCLASS(VideoStreamWMF, VideoStream);
    RES_BASE_EXTENSION("wmfstr");

    String file;
    int audio_track;

protected:
    static void _bind_methods();

public:
    Ref<VideoStreamPlayback> instance_playback() {
        Ref<WindowsMediaFoundationVideo> pb = memnew(WindowsMediaFoundationVideo);
        pb->set_audio_track(audio_track);
        pb->set_file(file);
        return pb;
    }

    void set_file(const String &p_file) {
        file = p_file;
    }

    String get_file() {
        return file;
    }

    void set_audio_track(int p_track) {
        audio_track = p_track;
    }

    VideoStreamWMF() {
        audio_track = 0;
    }
};

#endif
