#include "sample_grabber_callback.h"
#include <new>
#include <cstdio>
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
        return S_OK;
	}
    frame_data2.resize(frame_data->size());

    PoolVector<uint8_t>::Write w = frame_data2.write();
    char *rgb_buffer = (char *)w.ptr();

	BYTE* pSampleBufferY = (BYTE*)pSampleBuffer;

	size_t UOffset2 = (((height + 15) & ~15) * width);
	BYTE* pSampleBufferU = pSampleBufferY + UOffset2;

	size_t VOffset2 = ((((height / 2) + 7) & ~7) * (width / 2));
	BYTE* pSampleBufferV = pSampleBufferU + VOffset2;

	/*
	void yuv420_2_rgb8888(
		uint8_t  *dst_ptr_,
		const uint8_t  *y_ptr,
		const uint8_t  *u_ptr,
		const uint8_t  *v_ptr,
		int32_t   width,
		int32_t   height,
		int32_t   y_span,
		int32_t   uv_span,
		int32_t   dst_span,
		int32_t   dither)
	*/
	yuv420_2_rgb8888((uint8_t *)rgb_buffer,
				     (uint8_t *)pSampleBufferY,
					 (uint8_t *)pSampleBufferV,
					 (uint8_t *)pSampleBufferU,
					 width, height,
					 width,
					 width / 2,
					 width * 4, 0);

	mtx->lock();
	{
		uint8_t* dst = frame_data->write().ptr();
		const uint8_t* src = frame_data2.read().ptr();
		memcpy(dst, src, frame_data2.size());
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
