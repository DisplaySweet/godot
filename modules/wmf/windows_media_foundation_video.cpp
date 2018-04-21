#include "windows_media_foundation_video.h"

void WindowsMediaFoundationVideo::play() {

}

void WindowsMediaFoundationVideo::stop() {

}

bool WindowsMediaFoundationVideo::is_playing() const {
    return false;
}

void WindowsMediaFoundationVideo::set_paused(bool p_paused) {

}

bool WindowsMediaFoundationVideo::is_paused() const {
    return false;
}

void WindowsMediaFoundationVideo::set_loop(bool p_enabled) {

}

bool WindowsMediaFoundationVideo::has_loop() const {
    return false;
}

float WindowsMediaFoundationVideo::get_length() const {
    return 0.0f;
}

String WindowsMediaFoundationVideo::get_stream_name() const {
    return String("");
}

int WindowsMediaFoundationVideo::get_loop_count() const {
    return 0;
}

float WindowsMediaFoundationVideo::get_playback_position() const {
    return 0.0f;
}

void WindowsMediaFoundationVideo::seek(float p_time) {

}

void WindowsMediaFoundationVideo::set_file(const String &p_file) {

}

Ref<Texture> WindowsMediaFoundationVideo::get_texture() {
    return nullptr;
}

void WindowsMediaFoundationVideo::update(float p_delta) {

}

void WindowsMediaFoundationVideo::set_mix_callback(AudioMixCallback p_callback, void *p_userdata) {

}

int WindowsMediaFoundationVideo::get_channels() const {
    return 0;
}

int WindowsMediaFoundationVideo::get_mix_rate() const {
    return 0;
}

void WindowsMediaFoundationVideo::set_audio_track(int p_idx) {

}

WindowsMediaFoundationVideo::WindowsMediaFoundationVideo()
: m_pSession(nullptr) {
    MFCreateMediaSession(nullptr, &m_pSession);
}

WindowsMediaFoundationVideo::~WindowsMediaFoundationVideo() {
    if (m_pSession != nullptr) {
        m_pSession->Shutdown();
    }
}



void VideoStreamWMF::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_file", "file"), &VideoStreamWMF::set_file);
    ClassDB::bind_method(D_METHOD("get_file"), &VideoStreamWMF::get_file);

    ADD_PROPERTY(PropertyInfo(Variant::STRING, "file", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR | PROPERTY_USAGE_INTERNAL), "set_file", "get_file");
}
