#ifndef STREAM_PEER_UNIXSOCKET_H
#define STREAM_PEER_UNIXSOCKET_H

#include "core/io/stream_peer.h"

class StreamPeerUnixSocket : public StreamPeer {

	GDCLASS(StreamPeerUnixSocket, StreamPeer);
	OBJ_CATEGORY("Networking");

private:
	int _sock_fd;
	String _sock_path;

	Error write(const uint8_t *p_data, int p_bytes, int &r_sent, bool p_block);
	Error read(uint8_t *p_buffer, int p_bytes, int &r_received, bool p_block);

	static void _bind_methods();

public:
	Error connect_to_path(const String &p_path);
	bool is_connected() const;
	String get_connected_path() const;
	void disconnect_from_path();

	int get_available_bytes() const;

	// Read/Write from StreamPeer
	Error put_data(const uint8_t *p_data, int p_bytes);
	Error put_partial_data(const uint8_t *p_data, int p_bytes, int &r_sent);
	Error get_data(uint8_t *p_buffer, int p_bytes);
	Error get_partial_data(uint8_t *p_buffer, int p_bytes, int &r_received);

	StreamPeerUnixSocket();
	~StreamPeerUnixSocket();
};

#endif
