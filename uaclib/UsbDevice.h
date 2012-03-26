/*!
#
# Win-Widget. Windows related software for Audio-Widget/SDR-Widget (http://code.google.com/p/sdr-widget/)
# Copyright (C) 2012 Nikolay Kovbasa
#
# Permission to copy, use, modify, sell and distribute this software 
# is granted provided this copyright notice appears in all copies. 
# This software is provided "as is" without express or implied
# warranty, and with no claim as to its suitability for any purpose.
#
#----------------------------------------------------------------------------
# Contact: nikkov@gmail.com
#----------------------------------------------------------------------------
*/
#pragma once
#ifndef __USB_DEVICE_H__
#define __USB_DEVICE_H__

#include "targetver.h"
#include <stdio.h>
#include <tchar.h>
#include <windows.h>

#include <libusbk.h>
#include "usb_audio.h"

#ifdef _DEBUG
extern void debugPrintf(const _TCHAR *szFormat, ...);
#endif


// common part of all types of USB descriptors
struct USB_DESCRIPTOR_HEADER
{
	//! Size of this descriptor (in bytes)
	UCHAR bLength;
	//! Descriptor type
	UCHAR bDescriptorType;
	//! Descriptor type
	UCHAR bDescriptorSubType;
};



//Common USB device data and functions
class USBDevice
{
	//
	USB_DEVICE_DESCRIPTOR			m_deviceDescriptor;
	//
	USB_CONFIGURATION_DESCRIPTOR	m_configDescriptor;
	//
	KUSB_HANDLE						m_usbDeviceHandle;


	//device speed LowSpeed=0x01, FullSpeed=0x02, HighSpeed=0x03
	int								m_deviceSpeed;
	KUSB_HANDLE FindDevice();
	bool ParseDescriptors(BYTE *configDescr, DWORD length);
	void InitDescriptors();
protected:
	DWORD							m_errorCode;

	virtual void FreeDevice();

	bool SendUsbControl(int dir, int type, int recipient, int request, int value, int index,
				   unsigned char *buff, int size, ULONG *lengthTransferred);

	virtual bool ParseDescriptorInternal(USB_DESCRIPTOR_HEADER* uDescriptor) = 0;

public:
	USBDevice();
	virtual ~USBDevice();
	virtual bool InitDevice();

	DWORD GetErrorCode() 
	{
		return m_errorCode;
	}

	void ClearErrorCode()
	{
		m_errorCode = ERROR_SUCCESS;
	}

	bool OvlInit(KOVL_POOL_HANDLE* PoolHandle, LONG MaxOverlappedCount)
	{
		if(OvlK_Init(PoolHandle, m_usbDeviceHandle, MaxOverlappedCount, KOVL_POOL_FLAG_NONE))
			return TRUE;
		m_errorCode = GetLastError();
#ifdef _DEBUG
		debugPrintf("ASIOUAC: OvlK_Init failed. ErrorCode: %08Xh\n", m_errorCode);
#endif
		return FALSE;
	}

    bool OvlAcquire(KOVL_HANDLE* OverlappedK, KOVL_POOL_HANDLE PoolHandle)
	{
		if(OvlK_Acquire(OverlappedK, PoolHandle))
			return TRUE;
		m_errorCode = GetLastError();
#ifdef _DEBUG
		debugPrintf("ASIOUAC: OvlK_Acquire failed. ErrorCode: %08Xh\n", m_errorCode);
#endif
		return FALSE;
	}

	bool OvlWait(KOVL_HANDLE OverlappedK, LONG TimeoutMS, KOVL_WAIT_FLAG WaitFlags, PULONG TransferredLength)
	{
		if(OvlK_Wait(OverlappedK, TimeoutMS, WaitFlags, TransferredLength))
			return TRUE;
		m_errorCode = GetLastError();
#ifdef _DEBUG
		debugPrintf("ASIOUAC: OvlK_Wait failed. ErrorCode: %08Xh\n", m_errorCode);
#endif
		return FALSE;
	}

    bool OvlWaitOrCancel(KOVL_HANDLE OverlappedK, LONG TimeoutMS, PULONG TransferredLength)
	{
		if(OvlK_WaitOrCancel(OverlappedK, TimeoutMS, TransferredLength))
			return TRUE;
		if(TimeoutMS != 0) // we are just cancel request, no errors report
		{
			m_errorCode = GetLastError();
#ifdef _DEBUG
			debugPrintf("ASIOUAC: OvlK_WaitOrCancel failed. ErrorCode: %08Xh\n", m_errorCode);
#endif
		}
		return FALSE;
	}

	bool OvlRelease(KOVL_HANDLE OverlappedK)
	{
		if(OvlK_Release(OverlappedK))
			return TRUE;
		m_errorCode = GetLastError();
#ifdef _DEBUG
		debugPrintf("ASIOUAC: OvlK_Release failed. ErrorCode: %08Xh\n", m_errorCode);
#endif
		return FALSE;
	}

	bool OvlReUse(KOVL_HANDLE OverlappedK)
	{
		if(OvlK_ReUse(OverlappedK))
			return TRUE;
		m_errorCode = GetLastError();
#ifdef _DEBUG
		debugPrintf("ASIOUAC: OvlK_ReUse failed. ErrorCode: %08Xh\n", m_errorCode);
#endif
		return FALSE;
	}


    bool UsbResetPipe(UCHAR PipeId)
	{
		if(UsbK_ResetPipe(m_usbDeviceHandle, PipeId))
			return TRUE;
		m_errorCode = GetLastError();
#ifdef _DEBUG
		debugPrintf("ASIOUAC: UsbK_ResetPipe failed. ErrorCode: %08Xh\n", m_errorCode);
#endif
		return FALSE;
	}

    bool UsbAbortPipe(UCHAR PipeId)
	{
		if(UsbK_AbortPipe(m_usbDeviceHandle, PipeId))
			return TRUE;
		m_errorCode = GetLastError();
#ifdef _DEBUG
		debugPrintf("ASIOUAC: UsbK_AbortPipe failed. ErrorCode: %08Xh\n", m_errorCode);
#endif
		return FALSE;
	}

	bool UsbIsoWritePipe(UCHAR PipeID, PUCHAR Buffer, ULONG BufferLength, LPOVERLAPPED Overlapped, PKISO_CONTEXT IsoContext)
	{
		DWORD lastError;
		if(UsbK_IsoWritePipe (m_usbDeviceHandle, PipeID, Buffer, BufferLength, Overlapped, IsoContext) || 
			ERROR_IO_PENDING == (lastError = GetLastError()))
			return TRUE;
		m_errorCode = lastError;
#ifdef _DEBUG
		debugPrintf("ASIOUAC: UsbK_IsoWritePipe failed. ErrorCode: %08Xh\n", m_errorCode);
#endif
		return FALSE;
	}

	bool UsbIsoReadPipe (UCHAR PipeID, PUCHAR Buffer, ULONG BufferLength, LPOVERLAPPED Overlapped, PKISO_CONTEXT IsoContext)
	{
		DWORD lastError;
		if(UsbK_IsoReadPipe (m_usbDeviceHandle, PipeID, Buffer, BufferLength, Overlapped, IsoContext) || 
			ERROR_IO_PENDING == (lastError = GetLastError()))
			return TRUE;
		m_errorCode = lastError;
#ifdef _DEBUG
		debugPrintf("ASIOUAC: UsbK_IsoReadPipe failed. ErrorCode: %08Xh\n", m_errorCode);
#endif
		return FALSE;
	}

	bool UsbSetPipePolicy(UCHAR PipeID, ULONG PolicyType, ULONG ValueLength, PVOID Value)
	{
		if(UsbK_SetPipePolicy(m_usbDeviceHandle, PipeID, PolicyType, ValueLength,  Value))
			return TRUE;

		return FALSE;
	}

	bool UsbClaimInterface(UCHAR Number)
	{
		if(UsbK_ClaimInterface (m_usbDeviceHandle, Number, FALSE))
			return TRUE;
		m_errorCode = GetLastError();
#ifdef _DEBUG
		debugPrintf("ASIOUAC: UsbK_ClaimInterface %d failed. ErrorCode: %08Xh\n", Number, m_errorCode);
#endif
		return FALSE;
	}

	bool UsbSetAltInterface(UCHAR Number, UCHAR AltSettingNumber)
	{
		if(UsbK_SetAltInterface(m_usbDeviceHandle, Number, FALSE, AltSettingNumber))
			return TRUE;
		m_errorCode = GetLastError();
#ifdef _DEBUG
		debugPrintf("ASIOUAC: UsbK_SetAltInterface %d (alt %d) failed. ErrorCode: %08Xh\n", Number, AltSettingNumber, m_errorCode);
#endif
		return FALSE;
	}
	
	bool UsbReleaseInterface(UCHAR Number)
	{
		if(UsbK_ReleaseInterface (m_usbDeviceHandle, Number, FALSE))
			return TRUE;
		m_errorCode = GetLastError();
#ifdef _DEBUG
		debugPrintf("ASIOUAC: UsbK_ReleaseInterface %d failed. ErrorCode: %08Xh\n", Number, m_errorCode);
#endif
		return FALSE;
	}

};


#endif //__USB_DEVICE_H__