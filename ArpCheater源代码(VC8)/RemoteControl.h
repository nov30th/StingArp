#pragma once
static UINT SendAvailableData(LPVOID lpParam);
class CRemoteControl
{
public:
    CRemoteControl(CString localip, CString localmac, CString gateip, CString gatemac, CString netmask);
    int FullScanFind(CString ipAddress, CString macAddress, CString scanDestination, CString scanTime);
    int CheatGateFound(CString ipAddress, CString macAddress, CString scanTime);
    int SendData(CString packetContext);
    CString FindIPFromMAC(CString macString,CString ipAddress);
    ~CRemoteControl(void);
    bool SaveConfig(void);
    bool LoadConfig(void);
private:
    int SendUdpPacket(CString packetContext);
private:
    bool enableControl;
    bool acceptControl;
    CString strPassword;
    unsigned int warningLevel;
    unsigned int messageType;
    CString destAddress;
    unsigned int destPort;
    CString localIPAddress;
    CString netMask;
    CString gateIPAddress;
    CString localMAC;
    CString gateMAC;
    CString protocolType;
};
