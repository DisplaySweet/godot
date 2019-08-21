/* register_types.cpp */

#include "register_types.h"
#include "stream_peer_unixsocket.h"

void register_unixsocket_types() {
	ClassDB::register_class<StreamPeerUnixSocket>();
}

void unregister_unixsocket_types() {
   //nothing to do here
}
