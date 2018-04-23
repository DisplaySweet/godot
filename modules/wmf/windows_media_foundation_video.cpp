#include "windows_media_foundation_video.h"

#include <mfapi.h>
#include <mferror.h>
#include <mfidl.h>
#include "sample_grabber_callback.h"
#include <thirdparty/misc/yuv2rgb.h>
#include "os/file_access.h"


#pragma comment(lib, "mfreadwrite")
#pragma comment(lib, "mfplat")
#pragma comment(lib, "mfuuid")


#define CHECK_HR(func) if (SUCCEEDED(hr)) { hr = (func); if (FAILED(hr)) { print_line("failed, return:" + itos(hr)); } }
#define SafeRelease(p) { if (p) { (p)->Release(); (p)=nullptr; } }


HRESULT AddSourceNode(IMFTopology* pTopology, IMFMediaSource* pSource,
					  IMFPresentationDescriptor* pPD, IMFStreamDescriptor* pSD,
					  IMFTopologyNode** ppNode)
{
	IMFTopologyNode* pNode = NULL;

	HRESULT hr = S_OK;
	CHECK_HR(MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, &pNode));
	CHECK_HR(pNode->SetUnknown(MF_TOPONODE_SOURCE, pSource));
	CHECK_HR(pNode->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, pPD));
	CHECK_HR(pNode->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, pSD));
	CHECK_HR(pTopology->AddNode(pNode));

	if (SUCCEEDED(hr))
	{
		*ppNode = pNode;
		(*ppNode)->AddRef();
	}
	SafeRelease(pNode);
	return hr;
}

// Add an output node to a topology.
HRESULT AddOutputNode(IMFTopology *pTopology,     // Topology.
					  IMFActivate *pActivate,     // Media sink activation object.
					  DWORD dwId,                 // Identifier of the stream sink.
					  IMFTopologyNode **ppNode)   // Receives the node pointer.
{
	IMFTopologyNode *pNode = NULL;

	HRESULT hr = S_OK;
	CHECK_HR(MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &pNode));
	CHECK_HR(pNode->SetObject(pActivate));
	CHECK_HR(pNode->SetUINT32(MF_TOPONODE_STREAMID, dwId));
	CHECK_HR(pNode->SetUINT32(MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE));
	CHECK_HR(pTopology->AddNode(pNode));

	// Return the pointer to the caller.
	if (SUCCEEDED(hr))
	{
		*ppNode = pNode;
		(*ppNode)->AddRef();
	}

	SafeRelease(pNode);
	return hr;
}


// Create the topology.
HRESULT CreateTopology(IMFMediaSource *pSource, IMFActivate *pSinkActivate, IMFTopology **ppTopo, VideoStreamPlaybackWMF::StreamInfo *info)
{
	IMFTopology *pTopology = NULL;
	IMFPresentationDescriptor *pPD = NULL;
	IMFStreamDescriptor *pSD = NULL;
	IMFMediaTypeHandler *pHandler = NULL;
	IMFTopologyNode *inputNode = NULL;
	IMFTopologyNode *outputNode = NULL;
	IMFTopologyNode *inputNodeAudio = NULL;
	IMFTopologyNode *outputNodeAudio = NULL;
	IMFActivate* audioActivate = NULL;

	HRESULT hr = S_OK;

	CHECK_HR(MFCreateTopology(&pTopology));
	CHECK_HR(pSource->CreatePresentationDescriptor(&pPD));

	DWORD cStreams = 0;
	CHECK_HR(pPD->GetStreamDescriptorCount(&cStreams));

	print_line(itos(cStreams) + " streams");

	for (DWORD i = 0; i < cStreams; i++)
	{
		BOOL bSelected = FALSE;
		GUID majorType;

		CHECK_HR(pPD->GetStreamDescriptorByIndex(i, &bSelected, &pSD));
		CHECK_HR(pSD->GetMediaTypeHandler(&pHandler));
		CHECK_HR(pHandler->GetMajorType(&majorType));

		if (majorType == MFMediaType_Video && bSelected) 
		{
			print_line("Video Stream");

			IMFMediaType* pType = NULL;
			CHECK_HR(pHandler->GetMediaTypeByIndex(0, &pType));
			CHECK_HR(AddSourceNode(pTopology, pSource, pPD, pSD,&inputNode));
			CHECK_HR(AddOutputNode(pTopology, pSinkActivate, 0, &outputNode));
			CHECK_HR(inputNode->ConnectOutput(0, outputNode, 0));

			UINT32 width, height;
			MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &width, &height);

			info->size.x = width;
			info->size.y = height;
			print_line("Width & Height " + itos(width) + "x" + itos(height));

			UINT32 numerator, denominator;
			MFGetAttributeRatio(pType, MF_MT_FRAME_RATE, &numerator, &denominator);
			print_line("Frame Rate: " + itos(numerator) + "/" + itos(denominator));

			UINT64 duration;
			pPD->GetUINT64(MF_PD_DURATION, &duration);
			info->duration = duration / 10000000.f;
			print_line("Duration: " + rtos(info->duration) + " secs");

			break;
		}
		else if (majorType == MFMediaType_Audio && bSelected)
		{
			CHECK_HR(MFCreateAudioRendererActivate(&audioActivate));
			CHECK_HR(AddSourceNode(pTopology, pSource, pPD, pSD, &inputNodeAudio));
			CHECK_HR(AddOutputNode(pTopology, audioActivate, 0, &outputNodeAudio));
			CHECK_HR(inputNodeAudio->ConnectOutput(0, outputNodeAudio, 0));
		}
		else
		{
			print_line("Stream deselected");
			CHECK_HR(pPD->DeselectStream(i));
		}
		SafeRelease(pSD);
		SafeRelease(pHandler);
	}

	if (SUCCEEDED(hr))
	{
		*ppTopo = pTopology;
		(*ppTopo)->AddRef();
	}
	SafeRelease(pTopology);
	SafeRelease(inputNode);
	SafeRelease(outputNode);
	SafeRelease(pPD);
	SafeRelease(pHandler);
	SafeRelease(audioActivate);
	return hr;
}


HRESULT CreateMediaSource(const String &p_file, IMFMediaSource** pMediaSource) {
	print_line(__FUNCTION__);

	IMFSourceResolver* pSourceResolver = nullptr;
	IUnknown* pSource = nullptr;

	// Create the source resolver.
	HRESULT hr = S_OK;
	CHECK_HR(MFCreateSourceResolver(&pSourceResolver));

	print_line("Original File:" + p_file);

	Error e;
	FileAccess* fa = FileAccess::open(p_file, FileAccess::READ, &e);

	if (e != OK) {
		return E_FAIL;
	}

	String absolute_path = fa->get_path_absolute();
	fa->close();
	print_line("Absolute Path: " + absolute_path);

	MF_OBJECT_TYPE ObjectType;
	CHECK_HR(pSourceResolver->CreateObjectFromURL(absolute_path.c_str(),
												  MF_RESOLUTION_MEDIASOURCE, nullptr, &ObjectType, &pSource));
	CHECK_HR(pSource->QueryInterface(IID_PPV_ARGS(pMediaSource)));

	SafeRelease(pSourceResolver);
	SafeRelease(pSource);

	return hr;
}

void VideoStreamPlaybackWMF::shutdown_stream() {

	if (media_source) {
		media_source->Stop();
		media_source->Shutdown();
	}
	if (media_session) {
		media_session->Stop();
		media_session->Shutdown();
	}

	SafeRelease(topology);
	SafeRelease(media_source);
	SafeRelease(media_session);
	SafeRelease(presentation_clock);
	//SafeRelease(sample_grabber_callback);

	is_video_playing = false;
	is_video_paused = false;
	is_video_seekable = false;

	stream_info.size = Point2i(0, 0);
	stream_info.fps = 0;
	stream_info.duration = 0;
}

void VideoStreamPlaybackWMF::play() {
    print_line(__FUNCTION__);

	if (is_video_playing) return;

	HRESULT hr = S_OK;

	PROPVARIANT var;
	PropVariantInit(&var);
	CHECK_HR(media_session->Start(&GUID_NULL, &var));

	if (SUCCEEDED(hr))
	{
		is_video_playing = true;
	}
}

void VideoStreamPlaybackWMF::stop() {
    print_line(__FUNCTION__);

	HRESULT hr = S_OK;
	CHECK_HR(media_session->Stop());

	if (SUCCEEDED(hr))
	{
		is_video_playing = false;
	}
}

bool VideoStreamPlaybackWMF::is_playing() const {
    //print_line(__FUNCTION__);
    return is_video_playing;
}

void VideoStreamPlaybackWMF::set_paused(bool p_paused) {
    print_line(__FUNCTION__ ": " + itos(p_paused));
	is_video_paused = p_paused;

	HRESULT hr = S_OK;
	if (p_paused)
	{
		CHECK_HR(media_session->Pause());
	}
	else
	{
		PROPVARIANT var;
		PropVariantInit(&var);
		CHECK_HR(media_session->Start(&GUID_NULL, &var));
	}
}

bool VideoStreamPlaybackWMF::is_paused() const {
    //print_line(__FUNCTION__);
    return !is_video_paused;
}

void VideoStreamPlaybackWMF::set_loop(bool p_enabled) {
    print_line(__FUNCTION__ ": " + itos(p_enabled));
}

bool VideoStreamPlaybackWMF::has_loop() const {
    print_line(__FUNCTION__);
    return false;
}

float VideoStreamPlaybackWMF::get_length() const {
    print_line(__FUNCTION__);
    return stream_info.duration;
}

String VideoStreamPlaybackWMF::get_stream_name() const {
    print_line(__FUNCTION__);
    return String("A mp4 video");
}

int VideoStreamPlaybackWMF::get_loop_count() const {
    print_line(__FUNCTION__);
    return 0;
}

float VideoStreamPlaybackWMF::get_playback_position() const {
    //print_line(__FUNCTION__);

	HRESULT hr = S_OK;
	if (presentation_clock) {
		MFTIME position = 0;
		CHECK_HR(presentation_clock->GetTime(&position));
		return position / 10000000.0;
	}
    return 0.0f;
}

void VideoStreamPlaybackWMF::seek(float p_time) {
    print_line(__FUNCTION__ ": " + rtos(p_time));

	if (media_session) {
		
		p_time *= 10000000.0;

		HRESULT hr = S_OK;
		PROPVARIANT varStart;
		varStart.vt = VT_I8;
		varStart.hVal.QuadPart = (MFTIME)p_time;
		CHECK_HR(media_session->Start(NULL, &varStart));

		if (is_video_paused) {
			media_session->Pause();
		}
	}
}

void VideoStreamPlaybackWMF::set_file(const String &p_file) {
    print_line(__FUNCTION__ ": " + p_file);

	shutdown_stream();

	HRESULT hr = S_OK;

    IMFMediaType* pType = nullptr;
	
	CHECK_HR(MFCreateMediaType(&pType));
    CHECK_HR(pType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video));
	CHECK_HR(pType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264));
	CHECK_HR(pType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_I420));

	IMFActivate* pSinkActivate = nullptr;

	// Create the sample grabber sink.
	CHECK_HR(SampleGrabberCallback::CreateInstance(&sample_grabber_callback, &frame_data, &mtx));
	CHECK_HR(MFCreateSampleGrabberSinkActivate(pType, sample_grabber_callback, &pSinkActivate));

	CHECK_HR(MFCreateMediaSession(nullptr, &media_session));

	CHECK_HR(CreateMediaSource(p_file, &media_source));
	CHECK_HR(CreateTopology(media_source, pSinkActivate, &topology, &stream_info));

	CHECK_HR(media_session->SetTopology(0, topology));

	if (SUCCEEDED(hr)) {

		IMFRateControl* m_pRate;
		HRESULT hrTmp = MFGetService(media_session, MF_RATE_CONTROL_SERVICE, IID_PPV_ARGS(&m_pRate));

		BOOL bThin = false;
		float fRate = 0.f;
		CHECK_HR(m_pRate->GetRate(&bThin, &fRate));
		print_line("Thin = " + itos(bThin) + ", Playback Rate:" + rtos(fRate));

		DWORD caps = 0;
		CHECK_HR(media_session->GetSessionCapabilities(&caps));
		if ((caps & MFSESSIONCAP_SEEK) != 0) {
			is_video_seekable = true;
		}

		IMFClock* clock;
		if (SUCCEEDED(media_session->GetClock(&clock))) {
			CHECK_HR(clock->QueryInterface(IID_PPV_ARGS(&presentation_clock)));
		}

		sample_grabber_callback->set_frame_size(stream_info.size.x, stream_info.size.y);
		frame_data.resize(stream_info.size.x * stream_info.size.y * 4);
		texture->create(stream_info.size.x, stream_info.size.y, Image::FORMAT_RGBA8, Texture::FLAG_FILTER | Texture::FLAG_VIDEO_SURFACE);
	}
	else
	{
		SafeRelease(media_session);
	}
}

Ref<Texture> VideoStreamPlaybackWMF::get_texture() {
    print_line(__FUNCTION__);
    return texture;
}

void VideoStreamPlaybackWMF::update(float p_delta) {
    //print_line(__FUNCTION__ ": " + rtos(p_delta));

	if (!is_video_playing || is_video_paused) {
		return;
	}

	if (media_session)
	{
		HRESULT hr = S_OK;
		HRESULT hrStatus = S_OK;
		MediaEventType met = 0;
		IMFMediaEvent* pEvent = nullptr;

		hr = media_session->GetEvent(MF_EVENT_FLAG_NO_WAIT, &pEvent);
		if (SUCCEEDED(hr))
		{
			hr = pEvent->GetStatus(&hrStatus);
			if (SUCCEEDED(hr))
			{
				hr = pEvent->GetType(&met);
				if (SUCCEEDED(hr))
				{
					if (met == MESessionEnded)
					{
						// We're done playing
						media_session->Stop();
						is_video_playing = false;
						SafeRelease(pEvent);
						return;
					}
				}
			}
		}
		SafeRelease(pEvent);

		mtx.lock();
		Ref<Image> img = memnew(Image(stream_info.size.x, stream_info.size.y, 0, Image::FORMAT_RGBA8, frame_data)); //zero copy image creation
		mtx.unlock();
		
		texture->set_data(img); //zero copy send to visual server
	}
}

void VideoStreamPlaybackWMF::set_mix_callback(AudioMixCallback p_callback, void *p_userdata) {
    print_line(__FUNCTION__);
}

int VideoStreamPlaybackWMF::get_channels() const {
    print_line(__FUNCTION__);
    return 0;
}

int VideoStreamPlaybackWMF::get_mix_rate() const {
    print_line(__FUNCTION__);
    return 0;
}

void VideoStreamPlaybackWMF::set_audio_track(int p_idx) {
    print_line(__FUNCTION__ ": " + itos(p_idx));
}

VideoStreamPlaybackWMF::VideoStreamPlaybackWMF()
: media_session(NULL)
, media_source(NULL)
, topology(NULL)
, presentation_clock(NULL)
, is_video_playing(false)
, is_video_paused(false)
, is_video_seekable(false) {
    print_line(__FUNCTION__);

	texture = Ref<ImageTexture>(memnew(ImageTexture));
}

VideoStreamPlaybackWMF::~VideoStreamPlaybackWMF() {
    print_line(__FUNCTION__);

	shutdown_stream();
}



void VideoStreamWMF::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_file", "file"), &VideoStreamWMF::set_file);
    ClassDB::bind_method(D_METHOD("get_file"), &VideoStreamWMF::get_file);

    ADD_PROPERTY(PropertyInfo(Variant::STRING, "file", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR | PROPERTY_USAGE_INTERNAL), "set_file", "get_file");
}

Ref<VideoStreamPlayback> VideoStreamWMF::instance_playback() {
    print_line(__FUNCTION__);

    Ref<VideoStreamPlaybackWMF> pb = memnew(VideoStreamPlaybackWMF);
    pb->set_audio_track(audio_track);
    pb->set_file(file);
    return pb;
}

void VideoStreamWMF::set_file(const String& p_file) {
    print_line(__FUNCTION__ ": " + p_file);

    file = p_file;
}

VideoStreamWMF::VideoStreamWMF() {
    print_line(__FUNCTION__);
    audio_track = 0;

    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    MFStartup(MF_VERSION);
}

VideoStreamWMF::~VideoStreamWMF() {
    print_line(__FUNCTION__);

    MFShutdown();
}
