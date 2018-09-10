#include "scene/main/node.h"
#include "scene/gui/video_player.h"

class VideoStreamPlayback;
class VideoStreamPlaybackWMF;


class VideoSyncerWMF : public Node {

	GDCLASS(VideoSyncerWMF, Node);

	PoolVector<VideoStreamPlaybackWMF*> playbacks;

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	void add_video(const Node *p_player);
	void remove_video(const Node *p_player);
	void present();

	void play();

	VideoSyncerWMF();
	~VideoSyncerWMF();
};
