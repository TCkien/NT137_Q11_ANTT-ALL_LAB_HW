/* Theo hướng dẫn của 
Kết nối đến self-signed: https://stackoverflow.com/questions/19338395/how-do-you-use-winhttp-to-do-ssl-with-a-self-signed-cert 
Tải file xuống: https://stackoverflow.com/questions/822714/how-to-download-a-file-with-winhttp-in-c-c
Tìm file APPDATA: https://learn.microsoft.com/en-us/windows/win32/api/shlobj_core/nf-shlobj_core-shgetfolderpatha?redirectedfrom=MSDN
Tạo thông báo messageBox: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-messageboxw
Sleep: https://www.geeksforgeeks.org/cpp/sleep-function-in-cpp/
Add registry key: https://stackoverflow.com/questions/41317224/how-can-i-make-my-program-run-on-startup-by-adding-it-to-the-registry
Theo hướng dẫn tìm system folder: https://learn.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getsystemdirectoryw
Open registry key: https://learn.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regopenkeyexw
Create registry key: https://learn.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regcreatekeyexw
Mở ảnh trong cmd: https://stackoverflow.com/questions/3400884/how-do-i-open-an-explorer-window-in-a-given-directory-from-cmd-exe
Lệnh compile: g++ harmless_download.cpp -o harmless_download.exe -static -static-libgcc -static-libstdc++ -lwinhttp -municode
*/


#include <iostream>
#include <string>
#include <windows.h>
#include <winhttp.h>
#include <shlobj.h>
using namespace std;

// Hàm tìm thư mục APPDATA
wstring getAPPDATApath();

// Hàm tạo presistent
bool setRunAtStartup_OpenImage(const wstring &imageFullPath, const wstring &regValueName = L"OpenMyImage");

// Hàm tải file ảnh
bool download_file(wstring &url, wstring &outpath, wstring &host);

// Hàm đóng các handle 
void cleanup(HINTERNET &hOpen,HINTERNET &hConnect,HINTERNET &hRequest);


int wmain(int argc, wchar_t* argv[]) {
    wstring host = L"192.168.106.131";
    wstring url  = L"downloads/pic.png";
    wstring AppDatapath = getAPPDATApath();
    wstring outpath = AppDatapath + L"\\downloaded.png";

    bool ok = download_file(url, outpath, host);

    if (ok) {
        wcout << L"Downloaded successfully to: " << outpath << std::endl;
        if (setRunAtStartup_OpenImage(outpath)) {
        wcout << L"Registry key added. mspaint will open the image on startup." << std::endl;
        } else {
            wcerr << L"Failed to add registry key!" << std::endl;
        }
    } else {
        wcerr << L"Download failed!" << std::endl;
    }
    Sleep(10000);
    return 0;
}

wstring getAPPDATApath(){
    TCHAR szPath[MAX_PATH];

    if(SUCCEEDED(SHGetFolderPath(NULL, 
                                CSIDL_PROFILE, 
                                NULL, 
                                0, 
                                szPath))) 
    {
       return wstring(szPath) + L"\\AppData";
    }
    return L"."; // Trả về thư mục hiện tại nếu thất bại
}

void cleanup(HINTERNET &hOpen,HINTERNET &hConnect,HINTERNET &hRequest){
    if (hRequest) {
        WinHttpCloseHandle(hRequest);
        hRequest = nullptr;
    }
    if (hConnect) {
        WinHttpCloseHandle(hConnect);
        hConnect = nullptr;
    }
    if (hOpen) {
        WinHttpCloseHandle(hOpen);
        hOpen = nullptr;
    }
    return ;
}


bool download_file(wstring &url, wstring &outpath, wstring &host) {
    HINTERNET hOpen = nullptr, hConnect = nullptr, hRequest = nullptr;
    DWORD dwSize = 0, dwDownloaded = 0;
    LPSTR pszOutBuffer = nullptr;
    FILE *pFile = nullptr;

    // Mở session
    hOpen = WinHttpOpen(L"DownloaderApp",
                        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                        WINHTTP_NO_PROXY_NAME,
                        WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hOpen) {
        wprintf(L"WinHttpOpen failed (0x%.8X)\n", GetLastError());
        cleanup(hOpen, hConnect, hRequest);
        return false;
    }

    // Tạo kết nối
    hConnect = WinHttpConnect(hOpen, host.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) {
        wprintf(L"WinHttpConnect failed (0x%.8X)\n", GetLastError());
        cleanup(hOpen, hConnect, hRequest);
        return false;
    }

    // Tạo request
    hRequest = WinHttpOpenRequest(hConnect, L"GET", url.c_str(),
                                  NULL, WINHTTP_NO_REFERER,
                                  WINHTTP_DEFAULT_ACCEPT_TYPES,
                                  WINHTTP_FLAG_SECURE);
    if (!hRequest) {
        wprintf(L"WinHttpOpenRequest failed (0x%.8X)\n", GetLastError());
        cleanup(hOpen, hConnect, hRequest);
        return false;
    }

    // Xử lí request self-signed
    if (!WinHttpSendRequest(hRequest,
                            WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                            nullptr, 0, 0, 0)) {
        // Sau khi request đầu thất bại thì có thể chỉnh lại để nhận máy chủ self-signed
        if (GetLastError() == ERROR_WINHTTP_SECURE_FAILURE) {
            DWORD dwFlags =
                SECURITY_FLAG_IGNORE_UNKNOWN_CA |
                SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE |
                SECURITY_FLAG_IGNORE_CERT_CN_INVALID |
                SECURITY_FLAG_IGNORE_CERT_DATE_INVALID;
            WinHttpSetOption(hRequest, WINHTTP_OPTION_SECURITY_FLAGS, &dwFlags, sizeof(dwFlags));
            // Sau đó request lại
            if (!WinHttpSendRequest(hRequest,
                                    WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                    nullptr, 0, 0, 0)) {
                cleanup(hOpen, hConnect, hRequest);
                return false;
            }
        } else {
            cleanup(hOpen, hConnect, hRequest);
            return false;
        }
    }

    // Nhận response
    if (!WinHttpReceiveResponse(hRequest, nullptr)) {
        cleanup(hOpen, hConnect, hRequest);
        return false;
    }

    // Mở file
    pFile = _wfopen(outpath.c_str(), L"wb");
    if (!pFile) {
        cleanup(hOpen, hConnect, hRequest);
        return false;
    }

    // Nhận và ghi vào file
    do {
        if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) {
            printf("Error %u in WinHttpQueryDataAvailable.\n", GetLastError());
            break;
        }
        if (dwSize == 0) break;
        // Tạo biến chứa dữ liệu nhận được
        pszOutBuffer = new char[dwSize + 1];
        if (!pszOutBuffer) {
            printf("Out of memory\n");
            break;
        }
        // Nhận dữ liệu
        ZeroMemory(pszOutBuffer, dwSize + 1);

        //Ghi vào file
        if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer, dwSize, &dwDownloaded)) {
            printf("Error %u in WinHttpReadData.\n", GetLastError());
        } else if (dwDownloaded > 0) {
            fwrite(pszOutBuffer, 1, dwDownloaded, pFile);
        }
        // Giải phóng biến
        delete[] pszOutBuffer;
    } while (dwSize > 0);

    // Đóng file và các handle
    fclose(pFile);
    cleanup(hOpen, hConnect, hRequest);
    return true;
}

bool setRunAtStartup_OpenImage(const wstring &imageFullPath, const wstring &regValueName)
{
    // Tìm vị trí của thư mục system
    wstring systemFolder;
    {
        WCHAR buf[MAX_PATH];
        UINT len = GetSystemDirectoryW(buf, MAX_PATH);
        if (len == 0 || len >= MAX_PATH) return false;
        systemFolder = buf;
    }
    // Truy xuất đường dẫn đến chương trình mspaint
    wstring paintExe = systemFolder + L"\\mspaint.exe";

    // Tạo câu lệnh: "C:\Windows\System32\mspaint.exe" "Đường dẫn đến downloaded.png"
    wstring cmd = L"\"" + paintExe + L"\" \"" + imageFullPath + L"\"";

    // Mở key run để chuẩn bị thêm câu lệnh trên vào
    HKEY hKey = NULL;
    LONG lRes = RegOpenKeyExW(
        HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        0,
        KEY_SET_VALUE,
        &hKey
    );


    if (lRes != ERROR_SUCCESS) {
        // Nếu thất bại sẽ cố tạo key trong registry
        lRes = RegCreateKeyExW(
            HKEY_CURRENT_USER,
            L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
            0, NULL, 0, KEY_SET_VALUE, NULL, &hKey, NULL
        );
        if (lRes != ERROR_SUCCESS) {
            return false;
        }
    }

    // Thêm câu lệnh trên vào registry run key để chạy mspaint mở tệp ảnh
    lRes = RegSetValueExW(
        hKey,
        regValueName.c_str(),
        0,
        REG_SZ,
        (const BYTE*)cmd.c_str(),
        (DWORD)((cmd.size() + 1) * sizeof(wchar_t))
    );
    RegCloseKey(hKey);

    if (lRes != ERROR_SUCCESS) {
        return false;
    }
    return true;
}