#include "windows_media_foundation_video.h"

void VideoStreamPlaybackWMF::play() {

}

void VideoStreamPlaybackWMF::stop() {

}

bool VideoStreamPlaybackWMF::is_playing() const {
    return false;
}

void VideoStreamPlaybackWMF::set_paused(bool p_paused) {

}

bool VideoStreamPlaybackWMF::is_paused() const {
    return false;
}

void VideoStreamPlaybackWMF::set_loop(bool p_enabled) {

}

bool VideoStreamPlaybackWMF::has_loop() const {
    return false;
}

float VideoStreamPlaybackWMF::get_length() const {
    return 0.0f;
}

String VideoStreamPlaybackWMF::get_stream_name() const {
    return String("");
}

int VideoStreamPlaybackWMF::get_loop_count() const {
    return 0;
}

float VideoStreamPlaybackWMF::get_playback_position() const {
    return 0.0f;
}

void VideoStreamPlaybackWMF::seek(float p_time) {

}

void VideoStreamPlaybackWMF::set_file(const String &p_file) {

}

Ref<Texture> VideoStreamPlaybackWMF::get_texture() {
    return nullptr;
}

void VideoStreamPlaybackWMF::update(float p_delta) {

}

void VideoStreamPlaybackWMF::set_mix_callback(AudioMixCallback p_callback, void *p_userdata) {

}

int VideoStreamPlaybackWMF::get_channels() const {
    return 0;
}

int VideoStreamPlaybackWMF::get_mix_rate() const {
    return 0;
}

void VideoStreamPlaybackWMF::set_audio_track(int p_idx) {

}

VideoStreamPlaybackWMF::VideoStreamPlaybackWMF()
: m_pSession(nullptr) {
    MFCreateMediaSession(nullptr, &m_pSession);
}

VideoStreamPlaybackWMF::~VideoStreamPlaybackWMF() {
    if (m_pSession != nullptr) {
        m_pSession->Shutdown();
    }
}



void VideoStreamWMF::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_file", "file"), &VideoStreamWMF::set_file);
    ClassDB::bind_method(D_METHOD("get_file"), &VideoStreamWMF::get_file);

    ADD_PROPERTY(PropertyInfo(Variant::STRING, "file", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR | PROPERTY_USAGE_INTERNAL), "set_file", "get_file");
}
