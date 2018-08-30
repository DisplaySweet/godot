#include "sample_grabber_callback.h"
#include <new>
#include <cstdio>
#include <cassert>
#include <Shlwapi.h>
#include <mfapi.h>
#include "print_string.h"
#include <thirdparty/misc/yuv2rgb.h>

SampleGrabberCallback::SampleGrabberCallback(PoolVector<uint8_t>* frame_data, ThreadSafe* mtx)
: frame_data(frame_data)
, mtx(mtx)
, m_cRef(1)
{
}

HRESULT SampleGrabberCallback::CreateInstance(SampleGrabberCallback** ppCB, PoolVector<uint8_t>* frame_data, ThreadSafe* mtx)
{
    print_line(__FUNCTION__);

    *ppCB = new (std::nothrow) SampleGrabberCallback(frame_data, mtx);

    if (ppCB == nullptr)
    {
        return E_OUTOFMEMORY;
    }
    return S_OK;
}

SampleGrabberCallback::~SampleGrabberCallback() {
    print_line(__FUNCTION__);
}

STDMETHODIMP SampleGrabberCallback::QueryInterface(REFIID riid, void** ppv)
{
    print_line(__FUNCTION__);
    static const QITAB qit[] =
    {
        QITABENT(SampleGrabberCallback, IMFSampleGrabberSinkCallback),
        QITABENT(SampleGrabberCallback, IMFClockStateSink),
    { 0 }
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) SampleGrabberCallback::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) SampleGrabberCallback::Release()
{
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if (cRef == 0)
    {
        delete this;
    }
    return cRef;
}

// IMFClockStateSink methods.

// In these example, the IMFClockStateSink methods do not perform any actions. 
// You can use these methods to track the state of the sample grabber sink.

STDMETHODIMP SampleGrabberCallback::OnClockStart(MFTIME hnsSystemTime, LONGLONG llClockStartOffset)
{
    print_line(__FUNCTION__);
    return S_OK;
}

STDMETHODIMP SampleGrabberCallback::OnClockStop(MFTIME hnsSystemTime)
{
    print_line(__FUNCTION__);
    return S_OK;
}

STDMETHODIMP SampleGrabberCallback::OnClockPause(MFTIME hnsSystemTime)
{
    print_line(__FUNCTION__);
    return S_OK;
}

STDMETHODIMP SampleGrabberCallback::OnClockRestart(MFTIME hnsSystemTime)
{
    print_line(__FUNCTION__);
    return S_OK;
}

STDMETHODIMP SampleGrabberCallback::OnClockSetRate(MFTIME hnsSystemTime, float flRate)
{
    print_line(__FUNCTION__);
    return S_OK;
}

// IMFSampleGrabberSink methods.

STDMETHODIMP SampleGrabberCallback::OnSetPresentationClock(IMFPresentationClock* pClock)
{
    print_line(__FUNCTION__);
    return S_OK;
}

STDMETHODIMP SampleGrabberCallback::OnProcessSample(REFGUID guidMajorMediaType,
                                                    DWORD dwSampleFlags,
                                                    LONGLONG llSampleTime,
                                                    LONGLONG llSampleDuration,
                                                    const BYTE* pSampleBuffer,
                                                    DWORD dwSampleSize)
{
	if (guidMajorMediaType == MFMediaType_Video)
	{
        //return S_OK;
	}
    assert(frame_data->size() == width * height * 3);
	mtx->lock();
	{
		uint8_t* dst = frame_data->write().ptr();

		char *rgb_buffer = (char *)dst;
		// convert 4 pixels at once
		for (int i = 0; i < dwSampleSize; i += 12) {

			rgb_buffer[i + 0] = pSampleBuffer[i + 2];
			rgb_buffer[i + 1] = pSampleBuffer[i + 1];
			rgb_buffer[i + 2] = pSampleBuffer[i + 0];

			rgb_buffer[i + 3] = pSampleBuffer[i + 5];
			rgb_buffer[i + 4] = pSampleBuffer[i + 4];
			rgb_buffer[i + 5] = pSampleBuffer[i + 3];

			rgb_buffer[i + 6] = pSampleBuffer[i + 8];
			rgb_buffer[i + 7] = pSampleBuffer[i + 7];
			rgb_buffer[i + 8] = pSampleBuffer[i + 6];

			rgb_buffer[i + 9] = pSampleBuffer[i + 11];
			rgb_buffer[i + 10] = pSampleBuffer[i + 10];
			rgb_buffer[i + 11] = pSampleBuffer[i + 9];
		}
	}
    mtx->unlock();
    return S_OK;
}

STDMETHODIMP SampleGrabberCallback::OnShutdown()
{
    print_line(__FUNCTION__);
    return S_OK;
}

void SampleGrabberCallback::set_frame_size(int w, int h)
{
	width = w;
	height = h;
}
