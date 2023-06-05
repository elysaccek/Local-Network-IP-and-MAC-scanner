/*
    -> g++ -o localNetwork.exe main.cpp -lws2_32 -liphlpapi
*/
#include <iostream>
#include <string>
#include <vector>

// --> Mac address
#include <array>
#include <winsock2.h>
#include <iphlpapi.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")
// <-- Mac adress

// Thread
#include <windows.h>

bool checkMAC;

/*
- This function sends back the mac address of the entered ip address.
*/
std::string getMACAddress(const std::string& ipAddress) {
    ULONG destIpAddress = inet_addr(ipAddress.c_str());

    IPAddr srcIpAddress = 0;
    ULONG macAddress[2];
    ULONG macAddressLen = 6;

    DWORD replyStatus = 0;

    std::array<char, 100> macAddressStr;

    DWORD result = SendARP(destIpAddress, srcIpAddress, macAddress, &macAddressLen);

    if (result == NO_ERROR) {
        snprintf(macAddressStr.data(), macAddressStr.size(), "%02X:%02X:%02X:%02X:%02X:%02X",
                 static_cast<int>(macAddress[0] & 0xFF),
                 static_cast<int>((macAddress[0] >> 8) & 0xFF),
                 static_cast<int>((macAddress[0] >> 16) & 0xFF),
                 static_cast<int>((macAddress[1] >> 0) & 0xFF),
                 static_cast<int>((macAddress[1] >> 8) & 0xFF),
                 static_cast<int>((macAddress[1] >> 16) & 0xFF));

        return std::string(macAddressStr.data());
    }

    return "";
}

/*
- This function returns a vector string that stores all the ip addresses in the range.
*/
std::vector<std::string> getIPRange(const std::string& startIP, const std::string& endIP) {
    std::vector<std::string> ipRange;
    
    std::string currentIP = startIP;
    // Adds an ip to the end of its vector.
    ipRange.push_back(currentIP);
    
    while (currentIP != endIP) {
        // 192.168.1.31
        // 0--------9
        // return 9
        size_t lastDot = currentIP.find_last_of(".");
        // Returns the last digit in ip address
        int lastOctet = std::stoi(currentIP.substr(lastDot + 1));
        
        if (lastOctet < 254) {
            lastOctet++;
            currentIP = currentIP.substr(0, lastDot + 1) + std::to_string(lastOctet);
        }else{
            break;
        }
        
        ipRange.push_back(currentIP);
    }
    return ipRange;
}

/*
- Thread function that pings ip address.
*/
DWORD WINAPI pingIP(LPVOID lpParam) {
    std::string ip = reinterpret_cast<char*>(lpParam);
    
    std::string scan = "ping -n 4 " + ip;
    
    // Sends a command to the command line and returns the returned output.
    FILE* pipe = _popen(scan.data(), "r");
    if (pipe == nullptr) {
        std::cout << "Command error!" << std::endl;
        return 0;
    }
    
    char buffer[128];
    std::string result = "";
    
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    
    _pclose(pipe);
    
    if (result.find("TTL=") != std::string::npos) {
        if (checkMAC){
            std::string macAddress = getMACAddress(ip);
            if (!macAddress.empty()) {
                std::cout << ip << " - " << macAddress << std::endl;
            } else {
                std::cout << ip << " - " << "MAC Address not found." << std::endl;
            }  
        } else {
            std::cout << ip << std::endl;
        }
    }

    return 0;
}

int main(int argc, char* argv[]) {
    // Start default mode
    std::string startIP = "192.168.1.1";
    std::string endIP = "192.168.1.255";

    checkMAC = false;

    try{
        if (argc < 2){
            std::cout << "Starting default mode." << std::endl;
        } else {
            for (int i = 1; i < argc ; i++){
                std::string option = argv[i];

                if (option == "-m"){
                    checkMAC = true;
                }else if(option == "-s"){
                    startIP = argv[i+1];
                    i++;
                }else if(option == "-e"){
                    endIP = argv[i+1];
                    i++;
                }
            }
            if(startIP > endIP){
                std::cout << "The starting ip cannot be greater than the ending ip." << std::endl;
                return 0;
            }
        }  
    }catch(const std::exception& e){
        std::cout << "An input error was encountered." << std::endl;
        return 0;
    }

    std::cout << "Start IP: " << startIP << " End IP: " << endIP << std::endl;
    std::cout << "---------------------------------------------" << std::endl;
    
    std::vector<std::string> ipRange = getIPRange(startIP, endIP);

    std::vector<HANDLE> threads;

    for (const std::string& ip : ipRange) {
        HANDLE hThread = CreateThread(NULL, 0, pingIP, reinterpret_cast<LPVOID>(const_cast<char*>(ip.c_str())), 0, NULL);
        
        if (hThread == NULL) {
            std::cerr << "Thread could not be created." << std::endl;
            break;
        }

        threads.push_back(hThread);
    }

    for (HANDLE hThread : threads) {
        WaitForSingleObject(hThread, INFINITE);
        CloseHandle(hThread);
    }

    std::cout << "---------------------------------------------" << std::endl;

    return 0;
}