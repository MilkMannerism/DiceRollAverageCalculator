// DiceRollAverageCalculator.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "DiceRollAverageCalculator.h"
#include "resource.h"
#include <numeric>
#include <vector>
#include <string>
#include <map>
#include <sstream>
#include <fstream>
#include <shlwapi.h>
#include <CommCtrl.h>
#pragma comment(lib, "shlwapi.lib")

#define MAX_LOADSTRING 100
#define WM_USER_CALCULATE (WM_USER + 1)
#define WM_USER_INPUT_DONE (WM_USER + 2)

#define ID_BUTTON_CALCULATE 1001
#define ID_EDIT_PEOPLE 1002
#define ID_EDIT_ROLLS 1003
#define ID_TEXT_RESULT 1004

#define ID_BUTTON_ADD 2001
#define IDM_SAVE 2002
#define IDC_GLOBAL_DICE_TYPE 2003

#define ID_BUTTON_SUBMIT_BASE 6000
#define ID_BUTTON_REMOVE_BASE 7000

#define ID_BUTTON_HISTORY_BASE 8000

std::vector<std::map<std::wstring, std::vector<int>>> playerRolls;

std::vector<HWND> playerEdits; 
std::vector<HWND> playerLabels;

struct PlayerStats {
    HWND hAvg;
    HWND hMostFreq;
    HWND hLatest;
    HWND hComparison;
};
std::vector<PlayerStats> playerStats;
int playerCount = 0;

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    RollHistoryProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void                InitializeControls(HWND hWnd);
void                RemovePlayer(HWND hWnd, int playerIndex);
void                AddPlayer(HWND hWnd, const std::wstring& name = L"", const std::map<std::wstring, std::vector<int>>& rolls = {});
double              calculateAverage(const std::vector<int>& rolls);
std::string         findMostFrequent(const std::vector<int>& rolls);
double              calculateOverallPartyAverage(const std::vector<std::map<std::wstring, std::vector<int>>>& playerRolls, const std::wstring& diceType);
void                UpdatePlayerStatistics(HWND hWnd, int playerIndex);
void                SetStatisticsEmpty(HWND hWnd, int playerIndex);
void                UpdateAllPlayerStatistics(HWND hWnd);
void                SaveData(const std::wstring& filePath, const std::vector<std::map<std::wstring, std::vector<int>>>& playerRolls, const std::vector<HWND>& playerLabels);
std::wstring        GetSaveFilePath();
void                LoadData(const std::wstring& filePath, HWND hWnd);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_DICEROLLAVERAGECALCULATOR, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DICEROLLAVERAGECALCULATOR));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DICEROLLAVERAGECALCULATOR));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_DICEROLLAVERAGECALCULATOR);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)*
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        InitializeControls(hWnd);
        std::wstring saveFilePath = GetSaveFilePath(); // Function to get the path
        if (!saveFilePath.empty()) {
            LoadData(saveFilePath, hWnd);
        }
        UpdateAllPlayerStatistics(hWnd);
    }
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
            {
                int result = MessageBox(hWnd, L"Would you like to save before leaving?", L"Confirm Exit", MB_YESNOCANCEL | MB_ICONQUESTION);
                if (result == IDYES) {
                    SaveData(GetSaveFilePath(), playerRolls, playerLabels);
                    DestroyWindow(hWnd); // Proceed to destroy the window after saving
                }
                else if (result == IDNO) {
                    DestroyWindow(hWnd); // Close the window without saving
                }
                else if (result == IDCANCEL) {
                    break;
                }
            }
                break;
            case IDC_GLOBAL_DICE_TYPE:
                if (HIWORD(wParam) == CBN_SELCHANGE) {
                    // Dice type has changed, update all statistics displays
                    UpdateAllPlayerStatistics(hWnd);
                }
                break;
            case ID_BUTTON_ADD:
                AddPlayer(hWnd);
                break;
            case IDM_SAVE:
                SaveData(GetSaveFilePath(), playerRolls, playerLabels);
                break;
            default:
                // Handling dynamic button commands for submit, remove, and history
                if (wmId >= ID_BUTTON_SUBMIT_BASE && wmId < ID_BUTTON_SUBMIT_BASE + 100) {
                    int playerIndex = wmId - ID_BUTTON_SUBMIT_BASE;
                    HWND hGlobalDiceType = GetDlgItem(hWnd, IDC_GLOBAL_DICE_TYPE);
                    int diceIndex = SendMessage(hGlobalDiceType, CB_GETCURSEL, 0, 0);
                    WCHAR diceType[10];
                    SendMessage(hGlobalDiceType, CB_GETLBTEXT, diceIndex, (LPARAM)diceType);

                    WCHAR buffer[256];
                    GetDlgItemText(hWnd, 5000 + playerIndex, buffer, 256);
                    int roll = _wtoi(buffer);  // Convert input to integer

                    if (roll != 0 || wcslen(buffer) > 0) {
                        playerRolls[playerIndex - 1][diceType].push_back(roll);
                        SetDlgItemText(hWnd, 5000 + playerIndex, L"");

                        UpdateAllPlayerStatistics(hWnd);
                    }
                    else {
                        MessageBox(hWnd, L"Please enter a valid roll number.", L"Input Error", MB_ICONERROR);
                    }
                }
                else if (wmId >= ID_BUTTON_REMOVE_BASE && wmId < ID_BUTTON_REMOVE_BASE + 100) {
                    int playerIndex = wmId - ID_BUTTON_REMOVE_BASE;
                    int msgboxID = MessageBox(
                        hWnd,
                        L"Are you sure you want to remove this player?",
                        L"Confirm",
                        MB_ICONWARNING | MB_YESNO
                    );

                    if (msgboxID == IDYES) {
                        RemovePlayer(hWnd, playerIndex);
                    }
                }
                else if (wmId >= ID_BUTTON_HISTORY_BASE && wmId < ID_BUTTON_HISTORY_BASE + 100) {
                    int playerIndex = wmId - ID_BUTTON_HISTORY_BASE - 1;  // Why did the -1 fix this :( NOTE::Player index starts at 1 which is why -1 is needed
                    // Ensure playerIndex is within range
                    if (playerIndex >= 0 && playerIndex < playerCount) {
                        DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_ROLL_HISTORY), hWnd, RollHistoryProc, (LPARAM)playerIndex);
                    }
                    else {
                        MessageBox(hWnd, L"Invalid player index.", L"Error", MB_OK);
                    }
                }
                break;
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_CLOSE:
    {
        int result = MessageBox(hWnd, L"Would you like to save before leaving?", L"Confirm Exit", MB_YESNOCANCEL | MB_ICONQUESTION);
        if (result == IDYES) {
            SaveData(GetSaveFilePath(), playerRolls, playerLabels);
            DestroyWindow(hWnd); // Proceed to destroy the window after saving
        }
        else if (result == IDNO) {
            DestroyWindow(hWnd); // Close the window without saving
        }
        // If result is IDCANCEL, do nothing, effectively cancelling the close.
        //This does not currently work, hitting the red X will always close the app after this popup
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

INT_PTR CALLBACK RollHistoryProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    static int playerIndex;
    static std::wstring diceType;

    switch (message) {
    case WM_INITDIALOG:
        playerIndex = (int)lParam;
        if (playerIndex >= 0 && playerIndex < playerRolls.size()) {
            HWND hGlobalDiceType = GetDlgItem(GetParent(hDlg), IDC_GLOBAL_DICE_TYPE);
            int diceIndex = SendMessage(hGlobalDiceType, CB_GETCURSEL, 0, 0);
            WCHAR dtBuffer[10];
            SendMessage(hGlobalDiceType, CB_GETLBTEXT, diceIndex, (LPARAM)dtBuffer);
            diceType = dtBuffer;

            for (int roll : playerRolls[playerIndex][diceType]) {
                WCHAR buffer[32];
                wsprintf(buffer, L"Roll: %d", roll);
                SendDlgItemMessage(hDlg, IDC_ROLL_LIST, LB_ADDSTRING, 0, (LPARAM)buffer);
            }
        }
        return (INT_PTR)TRUE;


    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_REMOVE_ROLL:
        {
            int selectedIdx = SendDlgItemMessage(hDlg, IDC_ROLL_LIST, LB_GETCURSEL, 0, 0);
            if (selectedIdx != LB_ERR) {
                SendDlgItemMessage(hDlg, IDC_ROLL_LIST, LB_DELETESTRING, selectedIdx, 0);
                auto& rolls = playerRolls[playerIndex][diceType];
                if (selectedIdx < rolls.size()) {
                    rolls.erase(rolls.begin() + selectedIdx);
                    UpdateAllPlayerStatistics(GetParent(hDlg));
                }
            }
        }
        break;
        case IDC_CLOSE_BUTTON:
            // Close the dialog when the 'Cancel' button is pressed
            EndDialog(hDlg, 0);
            return (INT_PTR)TRUE;
        }
        break;

    case WM_CLOSE:
        // Also close the dialog on 'X' button click
        EndDialog(hDlg, 0);
        return (INT_PTR)TRUE;
    }

    return (INT_PTR)FALSE;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}


void InitializeControls(HWND hWnd) {
    // Create "Add Player" button
    CreateWindow(L"BUTTON", L"Add Player",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        50, 10, 100, 25, hWnd, (HMENU)ID_BUTTON_ADD, GetModuleHandle(NULL), NULL);

    // Create "Remove Player" button
    CreateWindow(L"BUTTON", L"Save",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        160, 10, 120, 25, hWnd, (HMENU)IDM_SAVE, GetModuleHandle(NULL), NULL);

    // Create the dice type drop down menu
    HWND hGlobalDiceType = CreateWindow(WC_COMBOBOX, L"",
        CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_VISIBLE,
        290, 10, 100, 300, hWnd, (HMENU)IDC_GLOBAL_DICE_TYPE, GetModuleHandle(NULL), NULL);

    // Define dice types
    const wchar_t* diceTypes[] = { L"d2", L"d4", L"d6", L"d8", L"d10", L"d12", L"d20", L"d100" };
    for (const auto& type : diceTypes) {
        SendMessage(hGlobalDiceType, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(type));
    }
    SendMessage(hGlobalDiceType, CB_SETCURSEL, 6, 0);
}

//Function to add players to the system
void AddPlayer(HWND hWnd, const std::wstring& name, const std::map<std::wstring, std::vector<int>>& rolls) {
    playerCount++;
    int baseY = 40 + 30 * (playerCount - 1);
    std::map<std::wstring, std::vector<int>> diceRolls;
    const wchar_t* diceTypes[] = { L"d2", L"d4", L"d6", L"d8", L"d10", L"d12", L"d20", L"d100" };
    WCHAR displayName[30];
    if (name.empty()) {
        wsprintf(displayName, L"Player %d", playerCount);
    }
    else {
        wcsncpy_s(displayName, name.c_str(), sizeof(displayName) / sizeof(displayName[0]) - 1);
        displayName[sizeof(displayName) / sizeof(displayName[0]) - 1] = 0; // Ensure null termination
    }

    // Player label for name
    HWND hLabel = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", displayName,
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL,
        50, baseY, 100, 20, hWnd, (HMENU)(5100 + playerCount), NULL, NULL);
    playerLabels.push_back(hLabel);

    // Edit control for inputting rolls
    HWND hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL,
        160, baseY, 100, 20, hWnd, (HMENU)(5000 + playerCount), NULL, NULL);
    playerEdits.push_back(hEdit);

    // Submit button
    HWND hSubmit = CreateWindow(L"BUTTON", L"Submit",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        270, baseY, 60, 20, hWnd, (HMENU)(ID_BUTTON_SUBMIT_BASE + playerCount), NULL, NULL);

    // Remove button
    HWND hRemove = CreateWindow(L"BUTTON", L"X",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        340, baseY, 20, 20, hWnd, (HMENU)(ID_BUTTON_REMOVE_BASE + playerCount), NULL, NULL);

    // History button
    HWND hHistory = CreateWindow(L"BUTTON", L"History",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        370, baseY, 60, 20, hWnd, (HMENU)(ID_BUTTON_HISTORY_BASE + playerCount), NULL, NULL);

    // Additional stat displays
    PlayerStats stats;
    stats.hAvg = CreateWindow(L"STATIC", L"Avg: 0", WS_CHILD | WS_VISIBLE,
        440, baseY, 100, 20, hWnd, NULL, hInst, NULL);
    stats.hMostFreq = CreateWindow(L"STATIC", L"Most Freq: None", WS_CHILD | WS_VISIBLE,
        550, baseY, 150, 20, hWnd, NULL, hInst, NULL);
    stats.hLatest = CreateWindow(L"STATIC", L"Latest: None", WS_CHILD | WS_VISIBLE,
        710, baseY, 100, 20, hWnd, NULL, hInst, NULL);
    stats.hComparison = CreateWindow(L"STATIC", L"Comp: 0%", WS_CHILD | WS_VISIBLE,
        820, baseY, 100, 20, hWnd, NULL, hInst, NULL);

    playerStats.push_back(stats); // Add the structure to the vector

    if (rolls.empty()) {
        for (const auto& type : diceTypes) {
            diceRolls[type] = std::vector<int>();  // Initialize an empty vector for each dice type
        }
        playerRolls.push_back(diceRolls);
    }
    else {
        playerRolls.push_back(rolls);
    }
}

// Function to remove players from the system
void RemovePlayer(HWND hWnd, int playerIndex) {
    int arrayIndex = playerIndex - 1;

    // Destroy controls for the player being removed
    DestroyWindow(playerLabels[arrayIndex]);
    DestroyWindow(playerEdits[arrayIndex]);
    DestroyWindow(GetDlgItem(hWnd, ID_BUTTON_SUBMIT_BASE + playerIndex));
    DestroyWindow(GetDlgItem(hWnd, ID_BUTTON_REMOVE_BASE + playerIndex));
    DestroyWindow(GetDlgItem(hWnd, ID_BUTTON_HISTORY_BASE + playerIndex));

    DestroyWindow(playerStats[arrayIndex].hAvg);
    DestroyWindow(playerStats[arrayIndex].hMostFreq);
    DestroyWindow(playerStats[arrayIndex].hLatest);
    DestroyWindow(playerStats[arrayIndex].hComparison);

    // Erase entries from vectors
    playerLabels.erase(playerLabels.begin() + arrayIndex);
    playerEdits.erase(playerEdits.begin() + arrayIndex);
    playerRolls.erase(playerRolls.begin() + arrayIndex);
    playerStats.erase(playerStats.begin() + arrayIndex);

    // Decrease IDs and positions of all subsequent players
    for (size_t i = arrayIndex; i < playerLabels.size(); ++i) {
        int newY = 40 + 30 * i;

        SetWindowPos(playerLabels[i], HWND_TOP, 50, newY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        SetWindowPos(playerEdits[i], HWND_TOP, 160, newY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

        // Move and update the submit, remove, and history buttons
        HWND submitButton = GetDlgItem(hWnd, ID_BUTTON_SUBMIT_BASE + i + 2);
        HWND removeButton = GetDlgItem(hWnd, ID_BUTTON_REMOVE_BASE + i + 2);
        HWND historyButton = GetDlgItem(hWnd, ID_BUTTON_HISTORY_BASE + i + 2);

        SetWindowPos(submitButton, HWND_TOP, 270, newY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        SetWindowPos(removeButton, HWND_TOP, 340, newY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        SetWindowPos(historyButton, HWND_TOP, 370, newY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

        // Correctly update control IDs
        SetWindowLong(submitButton, GWL_ID, ID_BUTTON_SUBMIT_BASE + i + 1);
        SetWindowLong(removeButton, GWL_ID, ID_BUTTON_REMOVE_BASE + i + 1);
        SetWindowLong(historyButton, GWL_ID, ID_BUTTON_HISTORY_BASE + i + 1);

        SetWindowLong(playerLabels[i], GWL_ID, 5100 + i + 1);
        SetWindowLong(playerEdits[i], GWL_ID, 5000 + i + 1);

        // Move and update statistic controls
        SetWindowPos(playerStats[i].hAvg, HWND_TOP, 440, newY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        SetWindowPos(playerStats[i].hMostFreq, HWND_TOP, 550, newY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        SetWindowPos(playerStats[i].hLatest, HWND_TOP, 710, newY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        SetWindowPos(playerStats[i].hComparison, HWND_TOP, 820, newY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    }

    // Update player count
    playerCount--;
}


double calculateAverage(const std::vector<int>& rolls) {
    if (rolls.empty()) return 0.0;
    int sum = std::accumulate(rolls.begin(), rolls.end(), 0);
    return static_cast<double>(sum) / rolls.size();
}

std::string findMostFrequent(const std::vector<int>& rolls) {
    std::map<int, int> freq;
    for (int roll : rolls) {
        freq[roll]++;
    }

    int maxFreq = 0;
    std::vector<int> mostFreq;
    for (auto& p : freq) {
        if (p.second > maxFreq) {
            mostFreq.clear();
            maxFreq = p.second;
            mostFreq.push_back(p.first);
        }
        else if (p.second == maxFreq) {
            mostFreq.push_back(p.first);
        }
    }

    std::string result;
    for (int num : mostFreq) {
        if (!result.empty()) result += ", ";
        result += std::to_string(num);
    }
    return result;
}

double calculateOverallPartyAverage(const std::vector<std::map<std::wstring, std::vector<int>>>& playerRolls, const std::wstring& diceType) {
    double totalSum = 0;
    int totalCount = 0;

    for (const auto& player : playerRolls) {
        auto it = player.find(diceType);
        if (it != player.end()) {
            totalSum += std::accumulate(it->second.begin(), it->second.end(), 0);
            totalCount += it->second.size();
        }
    }

    return totalCount > 0 ? totalSum / totalCount : 0.0;
}

void UpdatePlayerStatistics(HWND hWnd, int playerIndex) {
    if (playerIndex < 0 || playerIndex >= playerRolls.size()) return;

    HWND hGlobalDiceType = GetDlgItem(hWnd, IDC_GLOBAL_DICE_TYPE);
    int diceIndex = SendMessage(hGlobalDiceType, CB_GETCURSEL, 0, 0);
    WCHAR diceType[10];
    SendMessage(hGlobalDiceType, CB_GETLBTEXT, diceIndex, (LPARAM)diceType);

    // Check if the selected dice type has any rolls
    if (playerRolls[playerIndex].count(diceType) == 0 || playerRolls[playerIndex][diceType].empty()) {
        SetStatisticsEmpty(hWnd, playerIndex); // Set statistics displays to "empty" or similar message
        return; // Exit if there are no rolls for the selected type
    }

    const std::vector<int>& rolls = playerRolls[playerIndex][diceType];

    // Calculating statistics
    double avg = calculateAverage(rolls);
    std::string mostFreq = findMostFrequent(rolls);
    int latestRoll = !rolls.empty() ? rolls.back() : 0;
    double compAvg = calculateOverallPartyAverage(playerRolls, diceType);

    // Formatting and updating the text displays
    WCHAR buffer[256];
    swprintf(buffer, sizeof(buffer) / sizeof(buffer[0]), L"Avg: %.2f", avg);
    SetWindowText(playerStats[playerIndex].hAvg, buffer);

    swprintf(buffer, sizeof(buffer) / sizeof(buffer[0]), L"Most Freq: %S", mostFreq.c_str());
    SetWindowText(playerStats[playerIndex].hMostFreq, buffer);

    swprintf(buffer, sizeof(buffer) / sizeof(buffer[0]), L"Latest: %d", latestRoll);
    SetWindowText(playerStats[playerIndex].hLatest, buffer);

    if (compAvg != 0) {
        swprintf(buffer, sizeof(buffer) / sizeof(buffer[0]), L"Comp: %.2f%%", (avg - compAvg) / compAvg * 100);
    }
    else {
        wcscpy_s(buffer, L"Comp: N/A");
    }
    SetWindowText(playerStats[playerIndex].hComparison, buffer);
}

void SetStatisticsEmpty(HWND hWnd, int playerIndex) {
    SetWindowText(playerStats[playerIndex].hAvg, L"N/A");
    SetWindowText(playerStats[playerIndex].hMostFreq, L"N/A");
    SetWindowText(playerStats[playerIndex].hLatest, L"N/A");
    SetWindowText(playerStats[playerIndex].hComparison, L"N/A");
}

void UpdateAllPlayerStatistics(HWND hWnd) {
    for (size_t i = 0; i < playerRolls.size(); ++i) {
        UpdatePlayerStatistics(hWnd, i);
    }
}
void SaveData(const std::wstring& filePath, const std::vector<std::map<std::wstring, std::vector<int>>>& playerRolls, const std::vector<HWND>& playerLabels) {
    std::wofstream outFile(filePath);
    if (!outFile.is_open()) {
        MessageBox(NULL, L"Failed to open file for writing.", L"Error", MB_ICONERROR);
        return;
    }

    wchar_t buffer[256];
    wchar_t debugOutput[512];  // Make sure this buffer is large enough to hold the final string. Could work with 256 but after the roll history builds up enough it would fall out of range
    for (size_t i = 0; i < playerRolls.size(); ++i) {
        GetWindowText(playerLabels[i], buffer, 256);

        // Format the debug string
        swprintf(debugOutput, sizeof(debugOutput) / sizeof(wchar_t), L"\n%s \n", buffer);

        // Send the formatted string to the debug output
        OutputDebugString(debugOutput);
        outFile << buffer; // Save player name

        for (const auto& pair : playerRolls[i]) {
            const auto& diceType = pair.first;
            const auto& rolls = pair.second;
            for (int roll : rolls) {
                std::wstring debugOutput = L"Saving " + diceType + L":" + std::to_wstring(roll) + L"\n";
                OutputDebugString(debugOutput.c_str());
                outFile << L"," << diceType << L":" << roll;
            }
        }
        outFile << std::endl;
    }

    outFile.close();
    MessageBox(NULL, L"Data saved successfully.", L"Success", MB_ICONINFORMATION);
}

std::wstring GetSaveFilePath() {
    WCHAR path[MAX_PATH];
    if (GetModuleFileName(NULL, path, MAX_PATH)) {
        PathRemoveFileSpec(path); // Remove executable name from path
        PathAppend(path, L"\PlayerAndRollStatsSave.txt"); // Append folder and file name
        return std::wstring(path);
    }
    return L"";
}

void LoadData(const std::wstring& filePath, HWND hWnd) {
    std::wifstream inFile(filePath);
    if (!inFile.is_open()) {
        MessageBox(hWnd, L"Failed to open save file.", L"Error", MB_ICONERROR);
        return;
    }

    std::wstring line;
    const wchar_t* diceTypes[] = { L"d2", L"d4", L"d6", L"d8", L"d10", L"d12", L"d20", L"d100" };
    while (std::getline(inFile, line)) {
        std::wistringstream iss(line);
        std::wstring name, part;
        std::map<std::wstring, std::vector<int>> rollsForTypes;

        // Initialize empty vectors for each dice type
        for (const wchar_t* type : diceTypes) {
            rollsForTypes[type];
        }

        std::getline(iss, name, L',');  // First entry is the player name
        while (std::getline(iss, part, L',')) {
            size_t pos = part.find(':');
            if (pos != std::wstring::npos) {
                std::wstring diceType = part.substr(0, pos);
                int roll = std::stoi(part.substr(pos + 1));
                rollsForTypes[diceType].push_back(roll);
            }
        }
        AddPlayer(hWnd, name, rollsForTypes);
    }
}