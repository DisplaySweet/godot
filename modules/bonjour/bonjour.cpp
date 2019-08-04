#include "bonjour.h"

#ifdef _WIN32
#include "WinSock2.h"
#elif __APPLE__
#include <sys/select.h>
#include <sys/errno.h>
#elif __linux__
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/errno.h>
#endif


void Bonjour::handleEvents() {
	bool stopNow = false;
	int dns_sd_fd = clientRef ? DNSServiceRefSockFD(clientRef) : -1;
	fd_set readfds;
	struct timeval tv;
	int result;

	while (!stopNow) {
		FD_ZERO(&readfds);
		FD_SET(dns_sd_fd, &readfds);
		tv.tv_sec = 100000000;
		tv.tv_usec = 0;
		result = select(dns_sd_fd + 1, &readfds, (fd_set*)NULL, (fd_set*)NULL, &tv);

		if (result > 0) {
			DNSServiceErrorType err = kDNSServiceErr_NoError;

			if (FD_ISSET(dns_sd_fd, &readfds))
				err = DNSServiceProcessResult(clientRef);

			if (err)
				print_line("Error. DNSServiceProcessResult returned:" + itos(err));
		}
		else {
			print_line("select() returned:" + itos(result) + " errono:" + itos(errno));
			if (errno != EINTR)
				stopNow = true;
		}
        // exit anyway
        stopNow = true;
	}
}

bool Bonjour::registerBonjour(const String &type, const String &name, int port) {
	// convert godot String to const char*
	std::wstring ws;

	ws = name.c_str();
	std::string sname(ws.begin(), ws.end());
	const char* c_name = sname.c_str();

	ws = type.c_str();
	std::string stype(ws.begin(), ws.end());
	const char* c_type = stype.c_str();

	// registering
	DNSServiceErrorType err = DNSServiceRegister(&clientRef, 0, kDNSServiceInterfaceIndexAny,
		c_name, c_type, domain, host, htons(port), 0, NULL, reg_reply, NULL);
	if (err != kDNSServiceErr_NoError) {
		print_line("Bonjour: registration failed with error:" + itos(err));
		return false;
	}
	print_line("Bonjour: registration done!");

	handleEvents();
	return true;
}

void DNSSD_API Bonjour::reg_reply(DNSServiceRef sdref, const DNSServiceFlags flags, DNSServiceErrorType errorCode,
		const char* name, const char* regtype, const char* domain, void* context) {
	String rName = name, rRegType = regtype, rDomain = domain;

	if (errorCode == kDNSServiceErr_NoError) {
		if (flags & kDNSServiceFlagsAdd)
			print_line("Bonjour: Registered name: \"" + rName + "\" type: \"" + rRegType + "\" on domain: \"" + rDomain + "\"");
		else
			print_line("Bonjour: Name registration removed");
	}
	else if (errorCode == kDNSServiceErr_NameConflict) {
		print_line("Bonjour: Name in use, please choose another\n");
	}
	else {
		print_line("Bonjour: Error:" + itos(errorCode));
	}

	if (!(flags & kDNSServiceFlagsMoreComing))
		fflush(stdout);
}

void Bonjour::unregisterBonjour() {
	DNSServiceRefDeallocate(clientRef);
	clientRef = nullptr;
}

void Bonjour::_bind_methods() {
    ClassDB::bind_method(D_METHOD("register", "type", "name", "port"), &Bonjour::registerBonjour);
    ClassDB::bind_method(D_METHOD("unregister"), &Bonjour::unregisterBonjour);
}
