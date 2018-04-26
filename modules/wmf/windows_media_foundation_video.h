#ifndef WINDOWS_MEDIA_FOUNDATION_VIDEO_H
#define WINDOWS_MEDIA_FOUNDATION_VIDEO_H

#include "io/resource_loader.h"
#include "scene/resources/video_stream.h"
#include "os/thread_safe.h"
#include <mfidl.h>


class SampleGrabberCallback;
class IMFMediaSession;
class IMFMediaSource;
class IMFTopology;

class VideoStreamPlaybackWMF : public VideoStreamPlayback {
    GDCLASS(VideoStreamPlaybackWMF, VideoStreamPlayback);

	IMFMediaSession *m_pSession;
	IMFMediaSource *m_pSource;
	IMFTopology *m_pTopology;
	SampleGrabberCallback* m_pCallback;

	PoolVector<uint8_t> frame_data;
	Ref<ImageTexture> texture;
    ThreadSafe mtx;

	Point2i size;
	bool is_video_playing;
	bool is_video_paused;
	int64_t video_duration;

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

    VideoStreamPlaybackWMF();
    ~VideoStreamPlaybackWMF();    
};

class VideoStreamWMF : public VideoStream {
    GDCLASS(VideoStreamWMF, VideoStream);
    RES_BASE_EXTENSION("wmfvstr");

    String file;
    int audio_track;

protected:
    static void _bind_methods();

public:
    Ref<VideoStreamPlayback> instance_playback();

    void set_file(const String &p_file);

    String get_file() {
        return file;
    }

    void set_audio_track(int p_track) {
        audio_track = p_track;
    }

    VideoStreamWMF();
    ~VideoStreamWMF();
};

#endif
