#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/errno.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "stream_peer_unixsocket.h"

Error StreamPeerUnixSocket::connect_to_path(const String &p_path) {
	ERR_FAIL_COND_V(is_connected(), ERR_ALREADY_IN_USE);
	ERR_FAIL_COND_V(p_path.empty(), ERR_INVALID_PARAMETER);

	//Error err;
	struct sockaddr_un addr;

	if ( (_sock_fd = ::socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		print_line("[UnixDomainSocket] socket creation failed:" + itos(errno));
		return FAILED;
	}
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	const char *c_path = p_path.utf8().get_data();
	::strncpy(addr.sun_path, c_path, sizeof(addr.sun_path)-1);
	print_line("[UnixDomainSocket] connecting to " + p_path);

	if (::connect(_sock_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		print_line("[UnixDomainSocket] connect error:" + itos(errno));
		_sock_fd = 0;
		if (errno == ENOENT)
			return ERR_FILE_NOT_FOUND;
		return FAILED;
	}

	_sock_path = p_path;
	return OK;
}

Error StreamPeerUnixSocket::write(const uint8_t *p_data, int p_bytes, int &r_sent, bool p_block) {
	//print_line("[UnixDomainSocket] write(): " + itos(p_bytes) + " bytes");
	ERR_FAIL_COND_V(!is_connected(), ERR_UNAVAILABLE);

	int data_to_send = p_bytes;
	const uint8_t *offset = p_data;
	int total_sent = 0;

	while (data_to_send) {
		int sent_amount = ::write(_sock_fd, offset, data_to_send);

		if (sent_amount < 0) {
			print_line("[UnixDomainSocket] write failed:" + itos(sent_amount));
			disconnect_from_path();
			return FAILED;
		} else if (sent_amount == data_to_send) {
			// send complete
			total_sent += sent_amount;
			r_sent = total_sent;
			return OK;
		} else {
			// partial wrtie
			data_to_send -= sent_amount;
			offset += sent_amount;
			total_sent += sent_amount;
			// return now if non-blocking
			if (!p_block) {
				break;
			}
		}
	}

	r_sent = total_sent;
	return OK;
}

Error StreamPeerUnixSocket::read(uint8_t *p_buffer, int p_bytes, int &r_received, bool p_block) {
	//print_line("[UnixDomainSocket]: read(): " + itos(p_bytes) + " bytes");
	ERR_FAIL_COND_V(!is_connected(), ERR_UNAVAILABLE);

	int to_read = p_bytes;
	int total_read = 0;
	r_received = 0;

	while (to_read) {

		int read = ::read(_sock_fd, p_buffer + total_read, to_read);

		if (read < 0) {
			print_line("[UnixDomainSocket] read failed:" + itos(read));
			disconnect_from_path();
			return FAILED;
		} else if (read == 0) {
			r_received = total_read;
			return ERR_FILE_EOF;
		} else {
			to_read -= read;
			total_read += read;
			// return now if non-blocking
			if (!p_block) {
				break;
			}
		}
	}

	r_received = total_read;
	return OK;
}

bool StreamPeerUnixSocket::is_connected() const {
	return (_sock_fd > 0) ? true : false;
}

void StreamPeerUnixSocket::disconnect_from_path() {
	if (_sock_fd > 0)
		::close(_sock_fd);

	_sock_fd = 0;
	_sock_path = "";
}

Error StreamPeerUnixSocket::put_data(const uint8_t *p_data, int p_bytes) {
	int total;
	return write(p_data, p_bytes, total, true);
}

Error StreamPeerUnixSocket::put_partial_data(const uint8_t *p_data, int p_bytes, int &r_sent) {
	return write(p_data, p_bytes, r_sent, false);
}

Error StreamPeerUnixSocket::get_data(uint8_t *p_buffer, int p_bytes) {
	int total;
	return read(p_buffer, p_bytes, total, true);
}

Error StreamPeerUnixSocket::get_partial_data(uint8_t *p_buffer, int p_bytes, int &r_received) {
	return read(p_buffer, p_bytes, r_received, false);
}

int StreamPeerUnixSocket::get_available_bytes() const {
	ERR_FAIL_COND_V(!is_connected(), -1);

	int count;
	::ioctl(_sock_fd, FIONREAD, &count);

	return count;
}

String StreamPeerUnixSocket::get_connected_path() const {
	return _sock_path;
}


void StreamPeerUnixSocket::_bind_methods() {
	ClassDB::bind_method(D_METHOD("connect_to_path", "path"), &StreamPeerUnixSocket::connect_to_path);
	ClassDB::bind_method(D_METHOD("is_connected_to_path"), &StreamPeerUnixSocket::is_connected);
	ClassDB::bind_method(D_METHOD("get_connected_path"), &StreamPeerUnixSocket::get_connected_path);
	ClassDB::bind_method(D_METHOD("disconnect_from_path"), &StreamPeerUnixSocket::disconnect_from_path);
}

StreamPeerUnixSocket::StreamPeerUnixSocket() :
		_sock_fd(0) {
}

StreamPeerUnixSocket::~StreamPeerUnixSocket() {
	disconnect_from_path();
}
