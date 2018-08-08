#ifndef BONJOUR_H
#define BONJOUR_H

#include "reference.h"
#include <string>
#include "dns_sd.h"

class Bonjour : public Reference {
    GDCLASS(Bonjour, Reference);

	const char *host = "";
	const char *domain = "local";

	DNSServiceRef clientRef = nullptr;

	static void DNSSD_API reg_reply(DNSServiceRef sdref, const DNSServiceFlags flags, DNSServiceErrorType errorCode,
		const char* name, const char* regtype, const char* domain, void* context);
	void handleEvents();

protected:
    static void _bind_methods();

public:
    void registerBonjour(const String &type, const String &name, int port);
    void unregisterBonjour();

    Bonjour() = default;
};

#endif
