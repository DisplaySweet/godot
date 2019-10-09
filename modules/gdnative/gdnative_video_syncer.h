#include "scene/main/node.h"
#include "scene/gui/video_player.h"

class VideoStreamPlayback;
class VideoStreamPlaybackGDNative;

class VideoSyncerGDNative : public Node {

	GDCLASS(VideoSyncerGDNative, Node);

	Vector<VideoStreamPlaybackGDNative*> playbacks;
	Vector<VideoPlayer*> players;

	bool is_playing;

protected:
	static void _bind_methods();

public:
	void add_video(Node *const p_player);
	void remove_video(Node *const p_player);
	void clear();
	void present();

	void play();
	void stop();
	void set_paused(bool b);

	VideoSyncerGDNative();
	~VideoSyncerGDNative();
};
