// UVCExtensionApp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <mfapi.h>
#include <mfplay.h>
#include <mfreadwrite.h>
#include <vector>
#include <string>
#include <ks.h>
#include <ksproxy.h>
#include <vidcap.h>
#include "UVCExtensionApp.h"

// GUID of the extension unit, {C987A729 - 59D3 - 4569 - 8467 - FF0849FC19E8} for Nova UVC
static const GUID XU_GUID = { 0xc987a729, 0x59d3, 0x4569, {0x84, 0x67, 0xff, 0x08, 0x49, 0xfc, 0x19, 0xe8} };

//Media foundation and DSHOW specific structures, class and variables
IMFMediaSource *pVideoSource = NULL;
IMFAttributes *pVideoConfig = NULL;
IMFActivate **ppVideoDevices = NULL;
IMFSourceReader *pVideoReader = NULL;

//Other variables
UINT32 noOfVideoDevices = 0;
UINT32 noDeviceID = 0;
WCHAR *szFriendlyName = NULL;

#define NUM_PACK 152
#define SIZE_PACK 20480ULL
int main(int argc, char *argv[])
{
	HRESULT hr = S_OK;
	UINT32 selectedVal = 0xFFFFFFFF;
	ULONG readCount;
	UINT32 XU_node = 0;	//variable for UVC
	FILE* RawFile;

	//Get all video devices
	GetVideoDevices();
	if (noOfVideoDevices == 0) {
		printf("Can not detect any camera or UVC devices\n");
		system("pause");
		return 0;
	}

	printf("Video Devices connected:\n");
	for (UINT32 i = 0; i < noOfVideoDevices; i++){
		GetDeviceID(i);
		//compare PID/VID
		if (noDeviceID == 0x06038612 || noDeviceID == 0x045E0840) {
			//Get the device names
			GetVideoDeviceFriendlyNames(i);
			printf("%ws found!\n", szFriendlyName);
			InitVideoDevice(i);
			break;
		} else {
			printf("No Nova cam detected! Force to start test mode\n");
			system("pause");
			return 0;
		}
	}
#if 0
	printf("\nSelect\n1. To Set XU \n2. To Get XU\nAnd press Enter\n");
	scanf_s("%d", &selectedVal);
	if (selectedVal == 1){
		flags = KSPROPERTY_TYPE_SET | KSPROPERTY_TYPE_TOPOLOGY;
		printf("Enter input value: ");
		scanf_s("%d", (UINT32*)&data[0]);
	} else if (selectedVal == 2){
		flags = KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_TOPOLOGY;
	} else {
		printf("value not defined. exit program.\n");
		return 0;
	}
	printf("Enter XU node number (Default=4): ");
	scanf_s("%u", &XU_node);
	printf("Enter CS: ");
	scanf_s("%u", &CS);

	printf("\nTrying to invoke UVC extension unit...\n");
	if (!SetGetExtensionUnit(XU_GUID, XU_node, CS, flags, (void*)data, sizeof(data), &readCount)) {
		if (flags == (KSPROPERTY_TYPE_SET | KSPROPERTY_TYPE_TOPOLOGY)) {
			printf("Set OK! data[] = %02x%02x%02x%02x...\n", data[0], data[1], data[2], data[3]);
		} else {
			printf("Get OK! data[] = %02x%02x%02x%02x...\n", data[0], data[1], data[2], data[3]);
		}
	}
#else
	XU_node = 4;
	static UINT8 RawData[1920*1080*3/2];
	UINT8 buf_seq[2] = {0};
	UINT8 buf[SIZE_PACK] = {0};
	memset(buf, 0x00, SIZE_PACK);
	//reset seq
	printf("reset sequence numnber...\n");
	buf_seq[0] = NUM_PACK & 0xff;
	buf_seq[1] = (NUM_PACK>>8) & 0xff;
	SetGetExtensionUnit(XU_GUID, XU_node, 6, KSPROPERTY_TYPE_SET | KSPROPERTY_TYPE_TOPOLOGY, buf_seq, 2, &readCount);
	for (UINT32 i=0; i<NUM_PACK; i++){
		SetGetExtensionUnit(XU_GUID, XU_node, 5, KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_TOPOLOGY, buf, SIZE_PACK, &readCount);
		if (i==NUM_PACK-1){
			memcpy((void*)(RawData + (i * SIZE_PACK)), (void*)buf, (1920*1080*3/2)-(SIZE_PACK*i));
		} else {
			memcpy((void*)(RawData + (i * SIZE_PACK)), (void*)buf, SIZE_PACK);
		}
	}
	if (fopen_s(&RawFile, "1.raw", "wb+") != 0) {
		printf("Open RawFile failed!\n");
		goto EXIT;
	}
	if (RawFile && fwrite(RawData, sizeof(UINT8), sizeof(RawData)/sizeof(UINT8), RawFile) != (sizeof(RawData)/sizeof(UINT8))){
		printf("Write RawFile failed\n");
		goto EXIT;
	}
#endif
EXIT:
	//Release all the video device resources
	for (UINT32 i = 0; i < noOfVideoDevices; i++)
	{
		SafeRelease(&ppVideoDevices[i]);
	}
	CoTaskMemFree(ppVideoDevices);
	SafeRelease(&pVideoConfig);
	SafeRelease(&pVideoSource);
	CoTaskMemFree(szFriendlyName);

	if (RawFile) {
		fclose(RawFile);
	}

	system("pause");
    return 0;
}

//Function to get UVC video devices
HRESULT GetVideoDevices()
{
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	MFStartup(MF_VERSION);

	// Create an attribute store to specify the enumeration parameters.
	HRESULT hr = MFCreateAttributes(&pVideoConfig, 1);
	CHECK_HR_RESULT(hr, "Create attribute store");

	// Source type: video capture devices
	hr = pVideoConfig->SetGUID(
		MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
		MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID
	);
	CHECK_HR_RESULT(hr, "Video capture device SetGUID");

	// Enumerate devices.
	hr = MFEnumDeviceSources(pVideoConfig, &ppVideoDevices, &noOfVideoDevices);
	CHECK_HR_RESULT(hr, "Device enumeration");

done:
	return hr;
}

//Function to get UVC video devices ID
HRESULT GetDeviceID(UINT32 i) {
	TCHAR* pwszSymbolicLink_enum = NULL;
	TCHAR* temp = NULL;
	UINT32 cchSymbolicLink_enum;
	UINT32 pid = 0, vid = 0;
	HRESULT hr = (ppVideoDevices[i])->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, &pwszSymbolicLink_enum, &cchSymbolicLink_enum);
	CHECK_HR_RESULT(hr, "ppDevices->GetAllocatedString(...)");
	_tcstok_s(pwszSymbolicLink_enum, TEXT("VID_"), &temp);
	vid = _tcstol(temp, NULL, 16);
	_tcstok_s(temp, TEXT("PID_"), &temp);
	pid = _tcstol(temp, NULL, 16);
	noDeviceID = (vid << 16) | (pid);
done:
	CoTaskMemFree(pwszSymbolicLink_enum);
	return hr;
}

//Function to get device friendly name
HRESULT GetVideoDeviceFriendlyNames(int deviceIndex)
{
	// Get the the device friendly name.
	UINT32 cchName;

	HRESULT hr = ppVideoDevices[deviceIndex]->GetAllocatedString(
		MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME,
		&szFriendlyName, &cchName);
	CHECK_HR_RESULT(hr, "Get video device friendly name");

done:
	return hr;
}

//Function to initialize video device
HRESULT InitVideoDevice(int deviceIndex)
{
	IKsTopologyInfo* pKsTopologyInfo = NULL;
	DWORD dwNumNodes;
	WCHAR pwchNodeName[128];
	DWORD dwNameLen;

	HRESULT hr = ppVideoDevices[deviceIndex]->ActivateObject(IID_PPV_ARGS(&pVideoSource));
	CHECK_HR_RESULT(hr, "Activating video device");

	// Create a source reader.
	hr = MFCreateSourceReaderFromMediaSource(pVideoSource, pVideoConfig, &pVideoReader);
	CHECK_HR_RESULT(hr, "Creating video source reader");

	//list nodes in each device
	hr = pVideoSource->QueryInterface(__uuidof(IKsTopologyInfo), (void**)&pKsTopologyInfo);
	CHECK_HR_RESULT(hr, "Query node interface");

	hr = pKsTopologyInfo->get_NumNodes(&dwNumNodes);
	CHECK_HR_RESULT(hr, "IKsTopologyInfo->get_NumNodes(...)");

	for (DWORD i = 0; i < dwNumNodes; i++) {
		hr = pKsTopologyInfo->get_NodeName(i, pwchNodeName, 128, &dwNameLen);
		CHECK_HR_RESULT(hr, "IKsTopologyInfo->get_NodeName(...)");
		printf("Node[%u]: NodeName=%S, dwNameLen=%u\n", i, pwchNodeName, dwNameLen);
	}

done:
	return hr;
}

//Function to set/get parameters of UVC extension unit
HRESULT SetGetExtensionUnit(GUID xuGuid, DWORD dwExtensionNode, ULONG xuPropertyId, ULONG flags, void* data, int len, ULONG* readCount)
{
	IUnknown* unKnown;
	IKsControl* ks_control = NULL;
	IKsTopologyInfo* pKsTopologyInfo = NULL;
	KSP_NODE kspNode;

	HRESULT hr = pVideoSource->QueryInterface(__uuidof(IKsTopologyInfo), (void**)&pKsTopologyInfo);
	CHECK_HR_RESULT(hr, "IMFMediaSource::QueryInterface(IKsTopologyInfo)");

	hr = pKsTopologyInfo->CreateNodeInstance(dwExtensionNode, IID_IUnknown, (LPVOID *)&unKnown);
	CHECK_HR_RESULT(hr, "ks_topology_info->CreateNodeInstance(...)");

	hr = unKnown->QueryInterface(__uuidof(IKsControl), (void **)&ks_control);
	CHECK_HR_RESULT(hr, "ks_topology_info->QueryInterface(...)");

	kspNode.Property.Set = xuGuid;              // XU GUID = Extension Unit GUID from USBtreeviewer
	kspNode.NodeId = (ULONG)dwExtensionNode;   // XU Node ID = index to extension unit of this device
	kspNode.Property.Id = xuPropertyId;         // XU control ID = control code = CS
	kspNode.Property.Flags = flags;             // Set/Get request = SET_CUR or GET_CUR

	hr = ks_control->KsProperty((PKSPROPERTY)&kspNode, sizeof(kspNode), (PVOID)data, len, readCount);
	CHECK_HR_RESULT(hr, "ks_control->KsProperty(...)");

done:
	SafeRelease(&ks_control);
	return hr;
}
