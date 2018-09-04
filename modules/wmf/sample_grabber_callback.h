#ifndef SAMPLEGRABBERCALLBACK_H
#define SAMPLEGRABBERCALLBACK_H

#include "io/resource_loader.h"
#include "scene/resources/video_stream.h"
#include "os/thread_safe.h"

#include <mfidl.h>

class SampleGrabberCallback : public IMFSampleGrabberSinkCallback {
	
	long m_cRef;
	PoolVector<uint8_t>* frame_data;
	ThreadSafe* mtx;
	int width;
	int height;

	IMFTransform *m_pColorTransform = nullptr;
	IMFSample *m_pSample = nullptr;
	IMFSample *m_pOutSample = nullptr;

	SampleGrabberCallback(PoolVector<uint8_t>* frame_data, ThreadSafe* mtx);

public:
	static HRESULT CreateInstance(SampleGrabberCallback **ppCB, PoolVector<uint8_t>* frame_data, ThreadSafe* mtx);
	~SampleGrabberCallback();

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

	HRESULT CreateMediaSample(DWORD cbData, IMFSample **ppSample);

	// custom methods
	void set_frame_size(int w, int h);
	void set_color_transform(IMFTransform* mft) { m_pColorTransform = mft; }

};

#endif
