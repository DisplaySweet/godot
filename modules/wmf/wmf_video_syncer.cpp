#include "wmf_video_syncer.h"
#include "scene/resources/video_stream.h"
#include "scene/gui/video_player.h"
#include "video_stream_wmf.h"


void VideoSyncerWMF::_bind_methods() {

	ClassDB::bind_method(D_METHOD("add_video", "video_player"), &VideoSyncerWMF::add_video);
	ClassDB::bind_method(D_METHOD("clear"), &VideoSyncerWMF::clear);
	ClassDB::bind_method(D_METHOD("present"), &VideoSyncerWMF::present);
	ClassDB::bind_method(D_METHOD("play"), &VideoSyncerWMF::play);
	ClassDB::bind_method(D_METHOD("stop"), &VideoSyncerWMF::stop);
	ClassDB::bind_method(D_METHOD("set_paused", "paused"), &VideoSyncerWMF::set_paused);
}

void VideoSyncerWMF::add_video(const Node *p_player) {

	auto player = dynamic_cast<const VideoPlayer*>(p_player);
	if (player) {
		if (players.find(player) == -1)
			players.push_back((VideoPlayer*)player);
	}
}

void VideoSyncerWMF::remove_video(const Node *p_player) {

	stop();

	auto player = dynamic_cast<const VideoPlayer*>(p_player);
	if (player) {
		int idx = players.find(player);
		if (idx != -1)
			players.remove(idx);
	}
}

void VideoSyncerWMF::clear() {

	if (is_playing) {
		stop();
	}

	for (int i = 0; i < playbacks.size(); ++i) {
		playbacks.get(i)->enable_sync(false);
	}
	players.clear();
	playbacks.clear();
}

void VideoSyncerWMF::play() {

	playbacks.clear();
	for (int i = 0; i < players.size(); ++i) {
		VideoPlayer* player = players.get(i);
		auto wmf_playback = (VideoStreamPlaybackWMF*)player->get_playback().ptr();
		wmf_playback->enable_sync(true);
		playbacks.push_back(wmf_playback);
	}

	for (int i = 0; i < players.size(); ++i) {
		players.get(i)->play();
	}

	is_playing = true;
}

void VideoSyncerWMF::stop() {

	for (int i = 0; i < players.size(); ++i) {
		players.get(i)->stop();
	}
	playbacks.clear();

	is_playing = false;
}

void VideoSyncerWMF::set_paused(bool b) {
	for (int i = 0; i < players.size(); ++i) {
		players.get(i)->set_paused(b);
	}

	is_playing = !b;
}

void VideoSyncerWMF::present() {

	if (!is_playing) return;

	int64_t min_sample_time = INT64_MAX;
	for (int i = 0; i < playbacks.size(); ++i) {
		int64_t sample_time = playbacks.get(i)->next_sample_time();
		if (sample_time < min_sample_time)
			min_sample_time = sample_time;
	}

	for (int i = 0; i < playbacks.size(); ++i) {
		int64_t sample_time = playbacks.get(i)->next_sample_time();

		if (sample_time <= (min_sample_time + 30))
			playbacks.get(i)->present();
	}
}

VideoSyncerWMF::VideoSyncerWMF() {
}

VideoSyncerWMF::~VideoSyncerWMF() {
	playbacks.clear();
	players.clear();
}
