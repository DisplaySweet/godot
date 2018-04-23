#include "windows_media_foundation_video.h"

void VideoStreamPlaybackWMF::play() {
    print_line(__FUNCTION__);
}

void VideoStreamPlaybackWMF::stop() {
    print_line(__FUNCTION__);
}

bool VideoStreamPlaybackWMF::is_playing() const {
    //print_line(__FUNCTION__);
    return false;
}

void VideoStreamPlaybackWMF::set_paused(bool p_paused) {
    print_line(__FUNCTION__ ": " + itos(p_paused));
}

bool VideoStreamPlaybackWMF::is_paused() const {
    print_line(__FUNCTION__);
    return false;
}

void VideoStreamPlaybackWMF::set_loop(bool p_enabled) {
    print_line(__FUNCTION__ ": " + itos(p_enabled));
}

bool VideoStreamPlaybackWMF::has_loop() const {
    print_line(__FUNCTION__);
    return false;
}

float VideoStreamPlaybackWMF::get_length() const {
    print_line(__FUNCTION__);
    return 0.0f;
}

String VideoStreamPlaybackWMF::get_stream_name() const {
    print_line(__FUNCTION__);
    return String("");
}

int VideoStreamPlaybackWMF::get_loop_count() const {
    print_line(__FUNCTION__);
    return 0;
}

float VideoStreamPlaybackWMF::get_playback_position() const {
    print_line(__FUNCTION__);
    return 0.0f;
}

void VideoStreamPlaybackWMF::seek(float p_time) {
    print_line(__FUNCTION__ ": " + rtos(p_time));
}

void VideoStreamPlaybackWMF::set_file(const String &p_file) {
    print_line(__FUNCTION__ ": " + p_file);
}

Ref<Texture> VideoStreamPlaybackWMF::get_texture() {
    print_line(__FUNCTION__);
    return nullptr;
}

void VideoStreamPlaybackWMF::update(float p_delta) {
    print_line(__FUNCTION__ ": " + rtos(p_delta));
}

void VideoStreamPlaybackWMF::set_mix_callback(AudioMixCallback p_callback, void *p_userdata) {
    print_line(__FUNCTION__);
}

int VideoStreamPlaybackWMF::get_channels() const {
    print_line(__FUNCTION__);
    return 0;
}

int VideoStreamPlaybackWMF::get_mix_rate() const {
    print_line(__FUNCTION__);
    return 0;
}

void VideoStreamPlaybackWMF::set_audio_track(int p_idx) {
    print_line(__FUNCTION__ ": " + itos(p_idx));
}

VideoStreamPlaybackWMF::VideoStreamPlaybackWMF()
: m_pSession(nullptr) {
    MFCreateMediaSession(nullptr, &m_pSession);
}

VideoStreamPlaybackWMF::~VideoStreamPlaybackWMF() {

    print_line(__FUNCTION__);
    if (m_pSession != nullptr) {
        m_pSession->Shutdown();
    }
}



void VideoStreamWMF::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_file", "file"), &VideoStreamWMF::set_file);
    ClassDB::bind_method(D_METHOD("get_file"), &VideoStreamWMF::get_file);

    ADD_PROPERTY(PropertyInfo(Variant::STRING, "file", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR | PROPERTY_USAGE_INTERNAL), "set_file", "get_file");
}

Ref<VideoStreamPlayback> VideoStreamWMF::instance_playback() {

    print_line(__FUNCTION__);
    Ref<VideoStreamPlaybackWMF> pb = memnew(VideoStreamPlaybackWMF);
    pb->set_audio_track(audio_track);
    pb->set_file(file);
    return pb;
}

void VideoStreamWMF::set_file(const String& p_file) {
    print_line(__FUNCTION__ ": " + p_file);
    file = p_file;
}
