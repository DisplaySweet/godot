#include "gdnative_video_syncer.h"
#include "scene/resources/video_stream.h"
#include "scene/gui/video_player.h"
#include "videodecoder/video_stream_gdnative.h"

void VideoSyncerGDNative::_bind_methods() {

	ClassDB::bind_method(D_METHOD("add_video", "video_player"), &VideoSyncerGDNative::add_video);
	ClassDB::bind_method(D_METHOD("clear"), &VideoSyncerGDNative::clear);
	ClassDB::bind_method(D_METHOD("present"), &VideoSyncerGDNative::present);
	ClassDB::bind_method(D_METHOD("play"), &VideoSyncerGDNative::play);
	ClassDB::bind_method(D_METHOD("stop"), &VideoSyncerGDNative::stop);
	ClassDB::bind_method(D_METHOD("set_paused", "paused"), &VideoSyncerGDNative::set_paused);
}

void VideoSyncerGDNative::add_video(Node *const p_player) {

	VideoPlayer *const player = dynamic_cast<VideoPlayer *const>(p_player);
	if (player) {
		if (players.find(player) == -1)
			players.push_back((VideoPlayer*)player);
	}
}

void VideoSyncerGDNative::remove_video(Node *const p_player) {

	stop();

	VideoPlayer *const player = dynamic_cast<VideoPlayer *const>(p_player);
	if (player) {
		int idx = players.find(player);
		if (idx != -1)
			players.remove(idx);
	}
}

void VideoSyncerGDNative::clear() {

	if (is_playing) {
		stop();
	}

	players.clear();
	playbacks.clear();
}

void VideoSyncerGDNative::play() {

	playbacks.clear();
	for (int i = 0; i < players.size(); ++i) {
		VideoPlayer* player = players.get(i);
		VideoStreamPlaybackGDNative *playback = (VideoStreamPlaybackGDNative*)player->get_playback().ptr();
		playbacks.push_back(playback);
	}

	for (int i = 0; i < players.size(); ++i) {
		players.get(i)->play();
	}

	is_playing = true;
}

void VideoSyncerGDNative::stop() {

	for (int i = 0; i < players.size(); ++i) {
		players.get(i)->stop();
	}
	playbacks.clear();

	is_playing = false;
}

void VideoSyncerGDNative::set_paused(bool b) {
	for (int i = 0; i < players.size(); ++i) {
		players.get(i)->set_paused(b);
	}

	is_playing = !b;
}

void VideoSyncerGDNative::present() {

	if (!is_playing) return;
	if (playbacks.size() <= 1) return;

	// sync to the first video
	float time = playbacks.get(0)->get_sync_time();

	for (int i = 1; i < playbacks.size(); ++i) {
		playbacks.get(i)->set_sync_time(time);
	}
}

VideoSyncerGDNative::VideoSyncerGDNative() : is_playing(false) {
}

VideoSyncerGDNative::~VideoSyncerGDNative() {
	playbacks.clear();
	players.clear();
}
