#include "wmf_video_syncer.h"
#include "scene/resources/video_stream.h"
#include "scene/gui/video_player.h"
#include "video_stream_wmf.h"


void VideoSyncerWMF::_bind_methods() {

	ClassDB::bind_method(D_METHOD("add_video", "video_player"), &VideoSyncerWMF::add_video);
	ClassDB::bind_method(D_METHOD("present"), &VideoSyncerWMF::present);
	ClassDB::bind_method(D_METHOD("play"), &VideoSyncerWMF::play);
	
	//ClassDB::bind_method(D_METHOD("set_stream", "stream"), &VideoPlayer::set_stream);
	//ClassDB::bind_method(D_METHOD("get_stream"), &VideoPlayer::get_stream);
	//ClassDB::bind_method(D_METHOD("clear"), &VideoSyncerWMF::_update_callback);
	//ClassDB::bind_method(D_METHOD("stop"), &VideoSyncerWMF::_edit_set_state);
	//ClassDB::bind_method(D_METHOD("seek"), &VideoSyncerWMF::_edit_set_state);
}

void VideoSyncerWMF::_notification(int p_what) {
	switch(p_what) {
	case NOTIFICATION_PROCESS:
		//print_line("OKOK");
		break;
	}
	//print_line("Notification");
}

void VideoSyncerWMF::add_video(const Node *p_player) {

	if (p_player) {
		auto player = dynamic_cast<const VideoPlayer*>(p_player);
		auto wmf_playback = (VideoStreamPlaybackWMF*)player->get_playback().ptr();
		playbacks.append(wmf_playback);
	}
}

void VideoSyncerWMF::remove_video(const Node *p_player) {

	if (p_player) {
		/*
		auto wmf_playback = (VideoStreamPlaybackWMF*)p_player->get_playback().ptr();
		for (int i = 0; i < playbacks.size(); ++i) {
			VideoStreamPlaybackWMF* p = playbacks.get(i);
			if (p == wmf_playback) {
				playbacks.remove(i);
				break;
			}
		}
		*/
	}
}

void VideoSyncerWMF::play() {
}

void VideoSyncerWMF::present() {
}

VideoSyncerWMF::VideoSyncerWMF() {
}

VideoSyncerWMF::~VideoSyncerWMF() {
}
