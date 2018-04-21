#include "register_types.h"
#include "windows_media_foundation_video.h"
#include "resource_importer_wmf_video.h"

#include <mfapi.h>
#include <stdio.h>

void register_wmf_types() {
    MFStartup(MF_VERSION, MFSTARTUP_FULL);

#ifdef TOOLS_ENABLED
	Ref<ResourceImporterWMFVideo> wmfv_import;
	wmfv_import.instance();
	ResourceFormatImporter::get_singleton()->add_importer(wmfv_import);
#endif
    ClassDB::register_class<VideoStreamWMF>();
//    ClassDB::register_class<WindowsMediaFoundationVideo>();
}

void unregister_wmf_types() {
    MFShutdown();
}