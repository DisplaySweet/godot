#ifndef SAMPLEGRABBERCALLBACK_H
#define SAMPLEGRABBERCALLBACK_H

#include "io/resource_loader.h"
#include "scene/resources/video_stream.h"
#include "os/thread_safe.h"

#include <mfidl.h>

class SampleGrabberCallback : public IMFSampleGrabberSinkCallback {
	
	long m_cRef;
	PoolVector<uint8_t>* frame_data;
	PoolVector<uint8_t> frame_data2;
	ThreadSafe* mtx;

	SampleGrabberCallback(PoolVector<uint8_t>* frame_data, ThreadSafe* mtx);

public:
	static HRESULT CreateInstance(SampleGrabberCallback **ppCB, PoolVector<uint8_t>* frame_data, ThreadSafe* mtx);

	// IUnknown methods
	STDMETHODIMP QueryInterface(REFIID iid, void **ppv);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	// IMFClockStateSink methods
	STDMETHODIMP OnClockStart(MFTIME hnsSystemTime, LONGLONG llClockStartOffset);
	STDMETHODIMP OnClockStop(MFTIME hnsSystemTime);
	STDMETHODIMP OnClockPause(MFTIME hnsSystemTime);
	STDMETHODIMP OnClockRestart(MFTIME hnsSystemTime);
	STDMETHODIMP OnClockSetRate(MFTIME hnsSystemTime, float flRate);

	// IMFSampleGrabberSinkCallback methods
	STDMETHODIMP OnSetPresentationClock(IMFPresentationClock *pClock);
	STDMETHODIMP OnProcessSample(REFGUID guidMajorMediaType, DWORD dwSampleFlags,
	                             LONGLONG llSampleTime, LONGLONG llSampleDuration, const BYTE *pSampleBuffer,
	                             DWORD dwSampleSize);
	STDMETHODIMP OnShutdown();
};

#endif
