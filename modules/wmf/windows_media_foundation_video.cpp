#include "windows_media_foundation_video.h"

#include <mfapi.h>
#include <mferror.h>
#include <mfidl.h>
#include "sample_grabber_callback.h"
#include <thirdparty/misc/yuv2rgb.h>


#pragma comment(lib, "mfreadwrite")
#pragma comment(lib, "mfplat")
#pragma comment(lib, "mfuuid")


#define CHECK_HR(func) if (SUCCEEDED(hr)) { hr = (func); if (FAILED(hr)) { DebugBreak(); } }
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

	*ppNode = pNode;
	(*ppNode)->AddRef();

done:
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
done:
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

	*ppTopo = pTopology;
	(*ppTopo)->AddRef();

done:
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

	wchar_t* sFile = L"file://D:\\Godot\\Workspace\\VideoPlayback\\OneBarangaroo_Film_TV.mp4";
	MF_OBJECT_TYPE ObjectType;
	CHECK_HR(pSourceResolver->CreateObjectFromURL( sFile, // p_file.c_str(),
											  MF_RESOLUTION_MEDIASOURCE, nullptr, &ObjectType, &pSource));

	CHECK_HR(pSource->QueryInterface(IID_PPV_ARGS(pMediaSource)));

	SafeRelease(pSourceResolver);
	SafeRelease(pSource);

	return hr;
}

void VideoStreamPlaybackWMF::play() {
    print_line(__FUNCTION__);

	HRESULT hr = S_OK;

	PROPVARIANT var;
	PropVariantInit(&var);
	CHECK_HR(m_pSession->Start(&GUID_NULL, &var));

	if (SUCCEEDED(hr))
	{
		print_line("Play!");
		is_video_playing = true;
	}
}

void VideoStreamPlaybackWMF::stop() {
    print_line(__FUNCTION__);

	HRESULT hr = S_OK;
	PROPVARIANT var;
	PropVariantInit(&var);
	CHECK_HR(m_pSession->Stop());

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
	if (p_paused)
	{
		HRESULT hr = S_OK;
		CHECK_HR(m_pSession->Pause());
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
    return String("The Movie!");
}

int VideoStreamPlaybackWMF::get_loop_count() const {
    print_line(__FUNCTION__);
    return 0;
}

float VideoStreamPlaybackWMF::get_playback_position() const {
    print_line(__FUNCTION__);
    return 0.0f;
}

void VideoStreamPlaybackWMF::seek(float p_time) {
    print_line(__FUNCTION__ ": " + rtos(p_time));
}

void VideoStreamPlaybackWMF::set_file(const String &p_file) {
    print_line(__FUNCTION__ ": " + p_file);

	HRESULT hr = S_OK;

    IMFMediaType* pType = nullptr;
	
	CHECK_HR(MFCreateMediaType(&pType));
    CHECK_HR(pType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video));
	CHECK_HR(pType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264));
	CHECK_HR(pType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_I420));

	IMFActivate* pSinkActivate = nullptr;

	// Create the sample grabber sink.
	CHECK_HR(SampleGrabberCallback::CreateInstance(&m_pCallback, &frame_data, &mtx));
	CHECK_HR(MFCreateSampleGrabberSinkActivate(pType, m_pCallback, &pSinkActivate));

	CHECK_HR(MFCreateMediaSession(nullptr, &m_pSession));

	CHECK_HR(CreateMediaSource(p_file, &m_pSource));
	CHECK_HR(CreateTopology(m_pSource, pSinkActivate, &m_pTopology, &stream_info));

	PROPVARIANT var;
	PropVariantInit(&var);

	CHECK_HR(m_pSession->SetTopology(0, m_pTopology));

	if (SUCCEEDED(hr))
	{
		IMFRateControl* m_pRate;
		HRESULT hrTmp = MFGetService(m_pSession, MF_RATE_CONTROL_SERVICE, IID_PPV_ARGS(&m_pRate));

		BOOL bThin = false;
		float fRate = 0.f;
		CHECK_HR(m_pRate->GetRate(&bThin, &fRate));
		print_line("Thin = " + itos(bThin) + ", Playback Rate:" + rtos(fRate));
		//CHECK_HR(m_pRate->SetRate(TRUE, 1));

		m_pCallback->SetFrameSize(stream_info.size.x, stream_info.size.y);

		frame_data.resize(stream_info.size.x * stream_info.size.y * 4);
		texture->create(stream_info.size.x, stream_info.size.y, Image::FORMAT_RGBA8, Texture::FLAG_FILTER | Texture::FLAG_VIDEO_SURFACE);
	}
	else
	{
		SafeRelease(m_pSession);
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

	if (m_pSession) {

		HRESULT hr = S_OK;
		HRESULT hrStatus = S_OK;
		MediaEventType met;
		IMFMediaEvent* pEvent = nullptr;

		hr = m_pSession->GetEvent(MF_EVENT_FLAG_NO_WAIT, &pEvent);
		if ( hr == S_OK) {
			hr = pEvent->GetStatus(&hrStatus);
			if (hr == S_OK) {
				hr = pEvent->GetType(&met);
				if (hr == S_OK) {
					if (met == MESessionEnded) {
						// We're done playing
						SafeRelease(pEvent);
						m_pSession->Shutdown();
						SafeRelease(m_pSession);
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
: m_pSession(NULL)
, m_pSource(NULL)
, m_pTopology(NULL)
, is_video_playing(false)
, is_video_paused(false)
, video_duration(0) {
    print_line(__FUNCTION__);

	texture = Ref<ImageTexture>(memnew(ImageTexture));
}

VideoStreamPlaybackWMF::~VideoStreamPlaybackWMF() {

    print_line(__FUNCTION__);
    if (m_pSession != nullptr) {
        m_pSession->Shutdown();
    }
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

	HRESULT hr = S_OK;
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
