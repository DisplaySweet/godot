#include "sample_grabber_callback.h"
#include <new>
#include <Shlwapi.h>
#include <cstdio>
#include "print_string.h"

SampleGrabberCallback::SampleGrabberCallback() {
    m_cRef = 1;
}

HRESULT SampleGrabberCallback::CreateInstance(SampleGrabberCallback** ppCB)
{
    print_line(__FUNCTION__);

    *ppCB = new (std::nothrow) SampleGrabberCallback();

    if (ppCB == nullptr)
    {
        return E_OUTOFMEMORY;
    }
    return S_OK;
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

STDMETHODIMP SampleGrabberCallback::OnProcessSample(REFGUID guidMajorMediaType, DWORD dwSampleFlags,
                                                    LONGLONG llSampleTime, LONGLONG llSampleDuration, const BYTE * pSampleBuffer,
                                                    DWORD dwSampleSize)
{
    // Display information about the sample.
    print_line("Sample: start = " + itos(llSampleTime) + " duration = " + itos(llSampleDuration) + ", bytes = " + itos(dwSampleSize));
    return S_OK;
}

STDMETHODIMP SampleGrabberCallback::OnShutdown()
{
    print_line(__FUNCTION__);
    return S_OK;
}