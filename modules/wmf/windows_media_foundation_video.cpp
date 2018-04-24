#include "windows_media_foundation_video.h"

#include <mfapi.h>
#include <mferror.h>
#include "sample_grabber_callback.h"
#include <thirdparty/misc/yuv2rgb.h>


#pragma comment(lib, "mfreadwrite")
#pragma comment(lib, "mfplat")
#pragma comment(lib, "mfuuid")


#define CHECK_HR(h) if (FAILED(h)) { DebugBreak(); }
#define SafeRelease(p) { if (p) { (p)->Release(); (p)=nullptr; } }


HRESULT AddSourceNode(IMFTopology* pTopology, IMFMediaSource* pSource,
					  IMFPresentationDescriptor* pPD, IMFStreamDescriptor* pSD, IMFTopologyNode** ppNode)
{
	IMFTopologyNode* pNode = NULL;

	HRESULT hr = S_OK;
	hr = MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, &pNode);
	CHECK_HR(hr);
	hr = pNode->SetUnknown(MF_TOPONODE_SOURCE, pSource);
	CHECK_HR(hr);
	hr = pNode->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, pPD);
	CHECK_HR(hr);
	hr = pNode->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, pSD);
	CHECK_HR(hr);
	hr = pTopology->AddNode(pNode);
	CHECK_HR(hr);

	*ppNode = pNode;
	(*ppNode)->AddRef();

done:
	SafeRelease(pNode);
	return hr;
}

// Add an output node to a topology.
HRESULT AddOutputNode(
	IMFTopology *pTopology,     // Topology.
	IMFActivate *pActivate,     // Media sink activation object.
	DWORD dwId,                 // Identifier of the stream sink.
	IMFTopologyNode **ppNode)   // Receives the node pointer.
{
	IMFTopologyNode *pNode = NULL;

	HRESULT hr = S_OK;
	hr = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &pNode);
	CHECK_HR(hr);
	hr = pNode->SetObject(pActivate);
	CHECK_HR(hr);
	hr = pNode->SetUINT32(MF_TOPONODE_STREAMID, dwId);
	CHECK_HR(hr);
	hr = pNode->SetUINT32(MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE);
	CHECK_HR(hr);
	hr = pTopology->AddNode(pNode);
	CHECK_HR(hr);

	// Return the pointer to the caller.
	*ppNode = pNode;
	(*ppNode)->AddRef();

done:
	SafeRelease(pNode);
	return hr;
}


// Create the topology.
HRESULT CreateTopology(IMFMediaSource *pSource, IMFActivate *pSinkActivate, IMFTopology **ppTopo)
{
	IMFTopology *pTopology = NULL;
	IMFPresentationDescriptor *pPD = NULL;
	IMFStreamDescriptor *pSD = NULL;
	IMFMediaTypeHandler *pHandler = NULL;
	IMFTopologyNode *pNode1 = NULL;
	IMFTopologyNode *pNode2 = NULL;

	HRESULT hr = S_OK;
	DWORD cStreams = 0;

	hr = MFCreateTopology(&pTopology);
	CHECK_HR(hr);
	hr = pSource->CreatePresentationDescriptor(&pPD);
	CHECK_HR(hr);
	hr = pPD->GetStreamDescriptorCount(&cStreams);
	CHECK_HR(hr);

	printf("%d streams\n", cStreams);

	for (DWORD i = 0; i < cStreams; i++)
	{
		BOOL fSelected = FALSE;
		GUID majorType;

		hr = pPD->GetStreamDescriptorByIndex(i, &fSelected, &pSD);
		CHECK_HR(hr);
		hr = pSD->GetMediaTypeHandler(&pHandler);
		CHECK_HR(hr);
		hr = pHandler->GetMajorType(&majorType);
		CHECK_HR(hr);

		if (fSelected == FALSE) {
			printf("%d stream not selected\n", i);
		}

		IMFMediaType* pType = NULL;
		hr = pHandler->GetMediaTypeByIndex(0, &pType);
		CHECK_HR(hr);

		if (majorType == MFMediaType_Video && fSelected) 
		{
			print_line("Video Stream");
			hr = AddSourceNode(pTopology, pSource, pPD, pSD,&pNode1);
			hr = AddOutputNode(pTopology, pSinkActivate, 0, &pNode2);
			hr = pNode1->ConnectOutput(0, pNode2, 0);

			UINT32 m_uiWidth, m_uiHeight;
			MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &m_uiWidth, &m_uiHeight);

			print_line("Width & Height" + itos(m_uiWidth) + itos(m_uiHeight));

			//m_pSampleBuffer = new BYTE[m_uiWidth * m_uiHeight * 4];
		}
		else if (majorType == MFMediaType_Audio && fSelected)
		{
			print_line("Audio Stream");
			hr = AddSourceNode(pTopology, pSource, pPD, pSD, &pNode1);
			CHECK_HR(hr);
			hr = AddOutputNode(pTopology, pSinkActivate, 0, &pNode2);
			CHECK_HR(hr);
			hr = pNode1->ConnectOutput(0, pNode2, 0);
			CHECK_HR(hr);
		}
		else
		{
			print_line("Stream deselected");
			hr = pPD->DeselectStream(i);
			CHECK_HR(hr);
		}
		SafeRelease(pSD);
		SafeRelease(pHandler);
	}

	*ppTopo = pTopology;
	(*ppTopo)->AddRef();

done:
	SafeRelease(pTopology);
	SafeRelease(pNode1);
	SafeRelease(pNode2);
	SafeRelease(pPD);
	SafeRelease(pSD);
	SafeRelease(pHandler);
	return hr;
}


IMFMediaSource* VideoStreamPlaybackWMF::create_media_source(const String &p_file) {
	print_line(__FUNCTION__);

	IMFSourceResolver* pSourceResolver = nullptr;
	IUnknown* pSource = nullptr;

	// Create the source resolver.
	HRESULT hr = S_OK;
	hr = MFCreateSourceResolver(&pSourceResolver);
	CHECK_HR(hr);

	wchar_t* sFile = L"file://C:\\Users\\Aren\\Documents\\TestGodotProject\\SampleVideo_1280x720_5mb.mp4";
	MF_OBJECT_TYPE ObjectType;
	hr = pSourceResolver->CreateObjectFromURL( sFile, // p_file.c_str(),
											  MF_RESOLUTION_MEDIASOURCE, nullptr, &ObjectType, &pSource);
	CHECK_HR(hr);

	IMFMediaSource* pMediaSource = nullptr;
	hr = pSource->QueryInterface(IID_PPV_ARGS(&pMediaSource));
	CHECK_HR(hr);

	return pMediaSource;
}

void VideoStreamPlaybackWMF::play() {
    print_line(__FUNCTION__);

	HRESULT hr = S_OK;
	PROPVARIANT var;
	PropVariantInit(&var);
	CHECK_HR(hr = m_pSession->Start(&GUID_NULL, &var));
	print_line("Play!");
	isPlaying = true;
}

void VideoStreamPlaybackWMF::stop() {
    print_line(__FUNCTION__);

	HRESULT hr = S_OK;
	PROPVARIANT var;
	PropVariantInit(&var);
	CHECK_HR(hr = m_pSession->Stop());
	isPlaying = false;
}

bool VideoStreamPlaybackWMF::is_playing() const {
    //print_line(__FUNCTION__);
    return isPlaying;
}

void VideoStreamPlaybackWMF::set_paused(bool p_paused) {
    print_line(__FUNCTION__ ": " + itos(p_paused));
	isPlaying = (p_paused);
}

bool VideoStreamPlaybackWMF::is_paused() const {
    //print_line(__FUNCTION__);
    return !isPlaying;
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
    return 40.0f;
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

    IMFMediaType* pType = nullptr;
    HRESULT hr = MFCreateMediaType(&pType);
	CHECK_HR(hr);
    hr = pType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
	CHECK_HR(hr);
    hr = pType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264);
	CHECK_HR(hr);
    hr = pType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_YUY2);
	CHECK_HR(hr);

	IMFActivate* pSinkActivate = nullptr;

	// Create the sample grabber sink.
	hr = SampleGrabberCallback::CreateInstance(&m_pCallback);
	CHECK_HR(hr);

	hr = MFCreateSampleGrabberSinkActivate(pType, m_pCallback, &pSinkActivate);
	CHECK_HR(hr);

	// To run as fast as possible, set this attribute (requires Windows 7):
	hr = pSinkActivate->SetUINT32(MF_SAMPLEGRABBERSINK_IGNORE_CLOCK, TRUE);
	CHECK_HR(hr);

	hr = MFCreateMediaSession(nullptr, &m_pSession);
	CHECK_HR(hr);

	m_pSource = create_media_source(p_file);
	hr = CreateTopology(m_pSource, pSinkActivate, &m_pTopology);
	CHECK_HR(hr);

	PROPVARIANT var;
	PropVariantInit(&var);

	hr = m_pSession->SetTopology(0, m_pTopology);
	CHECK_HR(hr);
	hr = m_pSession->Start(&GUID_NULL, &var);
	CHECK_HR(hr);

	size.x = 1920;
	size.y = 1080;

	texture->create(size.x, size.y, Image::FORMAT_RGBA8, Texture::FLAG_FILTER | Texture::FLAG_VIDEO_SURFACE);
}

Ref<Texture> VideoStreamPlaybackWMF::get_texture() {
    print_line(__FUNCTION__);
    return texture;
}

void VideoStreamPlaybackWMF::update(float p_delta) {
    //print_line(__FUNCTION__ ": " + rtos(p_delta));

	if (m_pSession)
	{
		HRESULT hr = S_OK;
		HRESULT hrStatus = S_OK;
		MediaEventType met;
		IMFMediaEvent* pEvent = nullptr;

		hr = m_pSession->GetEvent(MF_EVENT_FLAG_NO_WAIT, &pEvent);
		if (hr == S_OK) {
			hr = pEvent->GetStatus(&hrStatus);
			if (hr == S_OK) {
				hr = pEvent->GetType(&met);
				if (hr == S_OK) {
					printf("%d\n", met);
				}
			}
		}
		SafeRelease(pEvent);

		int pitch = 4;
		frame_data.resize(size.x * size.y * pitch);
		{
			PoolVector<uint8_t>::Write w = frame_data.write();
			char *dst = (char *)w.ptr();

			for (int i = 0; i < (size.x * size.y * pitch); ++i) {
				dst[i] = 0xff;
			}

			//uv_offset=(ti.pic_x/2)+(yuv[1].stride)*(ti.pic_y/2);
			//yuv420_2_rgb8888((uint8_t *)dst, (uint8_t *)yuv[0].data, (uint8_t *)yuv[2].data, (uint8_t *)yuv[1].data, size.x, size.y, yuv[0].stride, yuv[1].stride, size.x << 2, 0);
			//format = Image::FORMAT_RGBA8;
		}

		Ref<Image> img = memnew(Image(size.x, size.y, 0, Image::FORMAT_RGBA8, frame_data)); //zero copy image creation
		texture->set_data(img); //zero copy send to visual server

		//frames_pending = 1;
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
: m_pSession(nullptr)
, m_pSource(nullptr)
, m_pTopology(nullptr) {
    print_line(__FUNCTION__);

	isPlaying = false;
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
