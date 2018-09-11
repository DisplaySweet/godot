#include "scene/main/node.h"
#include "scene/gui/video_player.h"

class VideoStreamPlayback;
class VideoStreamPlaybackWMF;


class VideoSyncerWMF : public Node {

	GDCLASS(VideoSyncerWMF, Node);

	Vector<VideoStreamPlaybackWMF*> playbacks;
	Vector<VideoPlayer*> players;

	bool is_playing = false;

protected:
	static void _bind_methods();

public:
	void add_video(const Node *p_player);
	void remove_video(const Node *p_player);
	void clear();
	void present();

	void play();
	void stop();
	void set_paused(bool b);

	VideoSyncerWMF();
	~VideoSyncerWMF();
};
