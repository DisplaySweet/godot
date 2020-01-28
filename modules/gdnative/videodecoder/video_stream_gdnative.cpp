/*************************************************************************/
/*  video_stream_gdnative.cpp                                            */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2020 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2020 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "video_stream_gdnative.h"

#include "core/project_settings.h"
#include "servers/audio_server.h"

#ifdef _WIN32
#include "WinSock2.h"
#include "ws2tcpip.h"
#include <cmath>
// the word interface is reserved by MSVC++ as a non-standard keyword (combaseapi.h)
// unfortunately 'interface' is used as a member variable here.
// we have to undefine it for compiler to be happy
#undef interface
#else
#include <sys/socket.h>
#include <sys/errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#define closesocket close
#endif

//#include "drivers/unix/net_socket_posix.h"
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <string>


VideoDecoderServer *VideoDecoderServer::instance = NULL;

static VideoDecoderServer decoder_server;

const int AUX_BUFFER_SIZE = 1024; // Buffer 1024 samples.

// how far off is still considered in sync
const float SYNC_TIMING_TOLERANCE = 0;
// how far off we need to seek
const float SYNC_SEEK_THRESHOLD = 1.0;

// NOTE: Callbacks for the GDNative libraries.
extern "C" {
godot_int GDAPI godot_videodecoder_file_read(void *ptr, uint8_t *buf, int buf_size) {
	// ptr is a FileAccess
	FileAccess *file = reinterpret_cast<FileAccess *>(ptr);

	// if file exists
	if (file) {
		long bytes_read = file->get_buffer(buf, buf_size);
		// No bytes to read => EOF
		if (bytes_read == 0) {
			return 0;
		}
		return bytes_read;
	}
	return -1;
}

int64_t GDAPI godot_videodecoder_file_seek(void *ptr, int64_t pos, int whence) {
	// file
	FileAccess *file = reinterpret_cast<FileAccess *>(ptr);

	if (file) {
		size_t len = file->get_len();
		switch (whence) {
			case SEEK_SET: {
				// Just for explicitness
				size_t new_pos = static_cast<size_t>(pos);
				if (new_pos > len) {
					return -1;
				}
				file->seek(new_pos);
				pos = static_cast<int64_t>(file->get_position());
				return pos;
			} break;
			case SEEK_CUR: {
				// Just in case it doesn't exist
				if (pos < 0 && (size_t)-pos > file->get_position()) {
					return -1;
				}
				pos = pos + static_cast<int>(file->get_position());
				file->seek(pos);
				pos = static_cast<int64_t>(file->get_position());
				return pos;
			} break;
			case SEEK_END: {
				// Just in case something goes wrong
				if ((size_t)-pos > len) {
					return -1;
				}
				file->seek_end(pos);
				pos = static_cast<int64_t>(file->get_position());
				return pos;
			} break;
			default: {
				// Only 4 possible options, hence default = AVSEEK_SIZE
				// Asks to return the length of file
				return static_cast<int64_t>(len);
			} break;
		}
	}
	// In case nothing works out.
	return -1;
}

void GDAPI godot_videodecoder_register_decoder(const godot_videodecoder_interface_gdnative *p_interface) {

	decoder_server.register_decoder_interface(p_interface);
}
}

// VideoStreamPlaybackGDNative starts here.

bool VideoStreamPlaybackGDNative::open_file(const String &p_file) {
	ERR_FAIL_COND_V(interface == NULL, false);
	file = FileAccess::open(p_file, FileAccess::READ);
	bool file_opened = interface->open_file(data_struct, file);

	if (file_opened) {
		num_channels = interface->get_channels(data_struct);
		mix_rate = interface->get_mix_rate(data_struct);

		godot_vector2 vec = interface->get_texture_size(data_struct);
		texture_size = *(Vector2 *)&vec;
		// Only do memset if num_channels > 0 otherwise it will crash.
		if (num_channels > 0) {
			pcm = (float *)memalloc(num_channels * AUX_BUFFER_SIZE * sizeof(float));
			memset(pcm, 0, num_channels * AUX_BUFFER_SIZE * sizeof(float));
		}

		pcm_write_idx = -1;
		samples_decoded = 0;

		texture->create((int)texture_size.width, (int)texture_size.height, Image::FORMAT_RGBA8, Texture::FLAG_FILTER | Texture::FLAG_VIDEO_SURFACE);
	}

	return file_opened;
}

// below functions: startup(), send_udp(), receive_udp(), set_blocking()
// modified from mplayer udp_sync.c
void VideoStreamPlaybackGDNative::startup(void) {
#ifdef _WIN32
	static int wsa_started;
	if (!wsa_started) {
		WSADATA wd;
		WSAStartup(0x0202, &wd);
		wsa_started = 1;
	}
#endif
}

void VideoStreamPlaybackGDNative::send_udp(const char *ip, int port, char *mesg) {
	static int sockfd = -1;
	static struct sockaddr_in socketinfo;

	// port is invalid
	if (port <= 0) return;

	if (sockfd == -1) {
		static const int one = 1;
		int ip_valid = 0;

		startup();
		sockfd = socket(AF_INET, SOCK_DGRAM, 0);
		if (sockfd == -1) {
			print_line("unable to create socket:" + itos(errno));
			return;
		}

		// enable broadcast
#ifdef _WIN32
		setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<const char*>(&one), sizeof(one));
		socketinfo.sin_addr.s_addr = inet_addr(ip);
		ip_valid = socketinfo.sin_addr.s_addr != INADDR_NONE;
#else
		setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &one, sizeof(one));
		ip_valid = inet_aton(ip, &socketinfo.sin_addr);
#endif

		if (!ip_valid) {
			print_line("invalid UDP ip address. Ignoring...");
			return;
		}

		socketinfo.sin_family = AF_INET;
		socketinfo.sin_port = htons(port);
	}

	sendto(sockfd, mesg, strlen(mesg), 0, reinterpret_cast<struct sockaddr *>(&socketinfo), sizeof(socketinfo));
}

void VideoStreamPlaybackGDNative::set_blocking(int fd, int blocking) {
#ifdef _WIN32
	u_long sock_flags = !blocking;
	ioctlsocket(fd, FIONBIO, &sock_flags);
#else
	long sock_flags;
	sock_flags = fcntl(fd, F_GETFL, 0);
	sock_flags = blocking ? sock_flags & ~O_NONBLOCK: sock_flags | O_NONBLOCK;
	fcntl(fd, F_SETFL, sock_flags);
#endif
}

int VideoStreamPlaybackGDNative::receive_udp(int port, float *time) {
	char mesg[128];

	int chars_received = -1;
	int n;

	static int sockfd = -1;

	// port is invalid
	if (port <= 0) return -1;

	if (sockfd == -1) {
#ifdef _WIN32
		DWORD tv =  30000;
#else
		struct timeval tv = { .tv_sec = 30 };
#endif
		struct sockaddr_in servaddr = { 0 };

		startup();
		sockfd = socket(AF_INET, SOCK_DGRAM, 0);
		if (sockfd == -1) {
			print_line("unable to create socket:" + itos(errno));
			return -1;
		}

		servaddr.sin_family = AF_INET;
		servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
		servaddr.sin_port = htons(port);
		if (bind(sockfd, reinterpret_cast<struct sockaddr *>(&servaddr), sizeof(servaddr)) == -1) {
			closesocket(sockfd);
			sockfd = -1;
			return -1;
		}

#ifdef _WIN32
		setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&tv), sizeof(tv));
#else
		setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
#endif
	}

	// set blocking
	set_blocking(sockfd, 0);

	while ((n = recvfrom(sockfd, mesg, sizeof(mesg) - 1, 0, NULL, NULL)) != -1) {
		char *end;
		if (chars_received == -1)
			set_blocking(sockfd, 0);

		chars_received = n;
		mesg[chars_received] = 0;

		*time = strtof(mesg, &end);
		if (*end) {
			print_line("unable to parse udp string");
			return -1;
		}
	}
	if (chars_received == -1)
		return -1;
	return 0;
}

void VideoStreamPlaybackGDNative::update(float p_delta) {
	if (!playing || paused) {
		return;
	}
	if (!file) {
		return;
	}
	time += p_delta;
	float old_time = time;

	if (is_master) {
		// send master's time
		char current_time[128];
		snprintf(current_time, sizeof(current_time), "%f", time);
		send_udp(udp_ip, udp_port, current_time);
	} else {
		// get master's time and update ours
		float new_time = time;
		receive_udp(udp_port, &new_time);
		bool is_seeked = set_sync_time(new_time);
		// compensate for the synced time only when seek didn't happen
		if (!is_seeked)
			p_delta += time - old_time;
	}

	ERR_FAIL_COND(interface == NULL);
	interface->update(data_struct, p_delta);

	// Don't mix if there's no audio (num_channels == 0).
	if (mix_callback && num_channels > 0) {
		if (pcm_write_idx >= 0) {
			// Previous remains
			int mixed = mix_callback(mix_udata, pcm + pcm_write_idx * num_channels, samples_decoded);
			if (mixed == samples_decoded) {
				pcm_write_idx = -1;
			} else {
				samples_decoded -= mixed;
				pcm_write_idx += mixed;
			}
		}
		if (pcm_write_idx < 0) {
			samples_decoded = interface->get_audioframe(data_struct, pcm, AUX_BUFFER_SIZE);
			pcm_write_idx = mix_callback(mix_udata, pcm, samples_decoded);
			if (pcm_write_idx == samples_decoded) {
				pcm_write_idx = -1;
			} else {
				samples_decoded -= pcm_write_idx;
			}
		}
	}

	if (seek_backward) {
		update_texture();
		seek_backward = false;
	}

	while (interface->get_playback_position(data_struct) < time && playing) {
		update_texture();
	}
}

void VideoStreamPlaybackGDNative::update_texture() {
	PoolByteArray *pba = (PoolByteArray *)interface->get_videoframe(data_struct);

	if (pba == NULL) {
		playing = false;
		return;
	}

	Ref<Image> img = memnew(Image(texture_size.width, texture_size.height, 0, Image::FORMAT_RGBA8, *pba));

	texture->set_data(img);
}

// ctor and dtor

VideoStreamPlaybackGDNative::VideoStreamPlaybackGDNative() :
		texture(Ref<ImageTexture>(memnew(ImageTexture))),
		playing(false),
		paused(false),
		mix_udata(NULL),
		mix_callback(NULL),
		num_channels(-1),
		time(0),
		seek_backward(false),
		mix_rate(0),
		delay_compensation(0),
		pcm(NULL),
		pcm_write_idx(0),
		samples_decoded(0),
		file(NULL),
		interface(NULL),
		data_struct(NULL) {}

VideoStreamPlaybackGDNative::~VideoStreamPlaybackGDNative() {
	cleanup();
}

void VideoStreamPlaybackGDNative::cleanup() {
	if (data_struct)
		interface->destructor(data_struct);
	if (pcm)
		memfree(pcm);
	pcm = NULL;
	time = 0;
	num_channels = -1;
	interface = NULL;
	data_struct = NULL;
}

void VideoStreamPlaybackGDNative::set_interface(const godot_videodecoder_interface_gdnative *p_interface) {
	ERR_FAIL_COND(p_interface == NULL);
	if (interface != NULL) {
		cleanup();
	}
	interface = p_interface;
	data_struct = interface->constructor((godot_object *)this);
}

// controls

bool VideoStreamPlaybackGDNative::is_playing() const {
	return playing;
}

bool VideoStreamPlaybackGDNative::is_paused() const {
	return paused;
}

void VideoStreamPlaybackGDNative::play() {

	stop();

	playing = true;

	delay_compensation = ProjectSettings::get_singleton()->get("audio/video_delay_compensation_ms");
	delay_compensation /= 1000.0;
}

void VideoStreamPlaybackGDNative::stop() {
	if (playing) {
		seek(0);
	}
	playing = false;
}

void VideoStreamPlaybackGDNative::seek(float p_time) {
	ERR_FAIL_COND(interface == NULL);
	interface->seek(data_struct, p_time);
	if (p_time < time)
		seek_backward = true;
	time = p_time;
	// reset audio buffers
	memset(pcm, 0, num_channels * AUX_BUFFER_SIZE * sizeof(float));
	pcm_write_idx = -1;
	samples_decoded = 0;
}

// returns true if sync causes a seek event, false otherwise
bool VideoStreamPlaybackGDNative::set_sync_time(float p_time) {
	float diff = abs(p_time - time);
	if (diff > SYNC_SEEK_THRESHOLD) {
		// we should seek() because the difference is too big
		print_line("video sync seek " + rtos(time) + "->" + rtos(p_time));
		seek(p_time);
		return true;
	} else if (diff > SYNC_TIMING_TOLERANCE) {
		// may be too verbose to print
		//print_line("video sync time " + rtos(time) + "->" + rtos(p_time));
		time = p_time;
	}
	return false;
}

float VideoStreamPlaybackGDNative::get_sync_time() const {
	return time;
}

void VideoStreamPlaybackGDNative::set_paused(bool p_paused) {
	paused = p_paused;
}

Ref<Texture> VideoStreamPlaybackGDNative::get_texture() const {
	return texture;
}

float VideoStreamPlaybackGDNative::get_length() const {
	ERR_FAIL_COND_V(interface == NULL, 0);
	return interface->get_length(data_struct);
}

float VideoStreamPlaybackGDNative::get_playback_position() const {

	ERR_FAIL_COND_V(interface == NULL, 0);
	return interface->get_playback_position(data_struct);
}

bool VideoStreamPlaybackGDNative::has_loop() const {
	// TODO: Implement looping?
	return false;
}

void VideoStreamPlaybackGDNative::set_loop(bool p_enable) {
	// Do nothing
}

void VideoStreamPlaybackGDNative::set_audio_track(int p_idx) {
	ERR_FAIL_COND(interface == NULL);
	interface->set_audio_track(data_struct, p_idx);
}

void VideoStreamPlaybackGDNative::set_mix_callback(AudioMixCallback p_callback, void *p_userdata) {

	mix_udata = p_userdata;
	mix_callback = p_callback;
}

int VideoStreamPlaybackGDNative::get_channels() const {
	ERR_FAIL_COND_V(interface == NULL, 0);

	return (num_channels > 0) ? num_channels : 0;
}

int VideoStreamPlaybackGDNative::get_mix_rate() const {
	ERR_FAIL_COND_V(interface == NULL, 0);

	return mix_rate;
}

void VideoStreamPlaybackGDNative::set_netsync(bool p_master, const String &p_ip, int p_port) {
	ERR_FAIL_COND(interface == NULL);

	is_master = p_master;

	std::wstring ws = p_ip.c_str();
	std::string s(ws.begin(), ws.end());
	strcpy(udp_ip, s.c_str());
	udp_port = p_port;

	if (udp_port > 0)
		print_line("video netsync enabled. Is_master:" + itos(is_master) + " Sync " + udp_ip + ":" + itos(udp_port));
}

/* --- NOTE VideoStreamGDNative starts here. ----- */

Ref<VideoStreamPlayback> VideoStreamGDNative::instance_playback() {
	Ref<VideoStreamPlaybackGDNative> pb = memnew(VideoStreamPlaybackGDNative);
	VideoDecoderGDNative *decoder = decoder_server.get_decoder(file.get_extension().to_lower());
	if (decoder == NULL)
		return NULL;
	pb->set_interface(decoder->interface);
	pb->set_audio_track(audio_track);
	pb->set_netsync(is_master, udp_ip, udp_port);
	if (pb->open_file(file))
		return pb;
	return NULL;
}

void VideoStreamGDNative::set_file(const String &p_file) {

	file = p_file;
}

String VideoStreamGDNative::get_file() {

	return file;
}

void VideoStreamGDNative::set_netsync(bool p_master, const String &p_ip, int p_port) {
	is_master = p_master;
	udp_ip = p_ip;
	udp_port = p_port;
}

void VideoStreamGDNative::_bind_methods() {

	ClassDB::bind_method(D_METHOD("set_netsync", "is_master", "ip", "port"), &VideoStreamGDNative::set_netsync);
	ClassDB::bind_method(D_METHOD("set_file", "file"), &VideoStreamGDNative::set_file);
	ClassDB::bind_method(D_METHOD("get_file"), &VideoStreamGDNative::get_file);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "file", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR | PROPERTY_USAGE_INTERNAL), "set_file", "get_file");
}

void VideoStreamGDNative::set_audio_track(int p_track) {

	audio_track = p_track;
}

/* --- NOTE ResourceFormatLoaderVideoStreamGDNative starts here. ----- */

RES ResourceFormatLoaderVideoStreamGDNative::load(const String &p_path, const String &p_original_path, Error *r_error) {
	FileAccess *f = FileAccess::open(p_path, FileAccess::READ);
	if (!f) {
		if (r_error) {
			*r_error = ERR_CANT_OPEN;
		}
		return RES();
	}
	memdelete(f);
	VideoStreamGDNative *stream = memnew(VideoStreamGDNative);
	stream->set_file(p_path);
	Ref<VideoStreamGDNative> ogv_stream = Ref<VideoStreamGDNative>(stream);
	if (r_error) {
		*r_error = OK;
	}
	return ogv_stream;
}

void ResourceFormatLoaderVideoStreamGDNative::get_recognized_extensions(List<String> *p_extensions) const {
	Map<String, int>::Element *el = VideoDecoderServer::get_instance()->get_extensions().front();
	while (el) {
		p_extensions->push_back(el->key());
		el = el->next();
	}
}

bool ResourceFormatLoaderVideoStreamGDNative::handles_type(const String &p_type) const {
	return ClassDB::is_parent_class(p_type, "VideoStream");
}

String ResourceFormatLoaderVideoStreamGDNative::get_resource_type(const String &p_path) const {
	String el = p_path.get_extension().to_lower();
	if (VideoDecoderServer::get_instance()->get_extensions().has(el))
		return "VideoStreamGDNative";
	return "";
}
