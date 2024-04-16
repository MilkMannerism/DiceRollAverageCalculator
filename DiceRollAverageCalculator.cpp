// DiceRollAverageCalculator.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "DiceRollAverageCalculator.h"
#include <numeric>
#include <vector>
#include <string>
#include <map>
#include <sstream>
#include <fstream>
#include <shlwapi.h>
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

#define ID_BUTTON_SUBMIT_BASE 6000
#define ID_BUTTON_REMOVE_BASE 7000

#define ID_BUTTON_HISTORY_BASE 8000

std::vector<std::vector<int>> playerRolls;

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
void                InitializeControls(HWND hWnd); //Custom
//void                CalculateAndDisplayResults(HWND hWnd); //Custom
void                RemovePlayer(HWND hWnd, int playerIndex);
void                AddPlayer(HWND hWnd, const std::wstring& name = L"", const std::vector<int>& rolls = {});
double              calculateAverage(const std::vector<int>& rolls);
std::string         findMostFrequent(const std::vector<int>& rolls);
double              groupAverageExcludingPlayer(const std::vector<std::vector<int>>& allRolls, int excludedPlayerIndex);
void                UpdatePlayerStatistics(HWND hWnd, int playerIndex);
void                UpdateAllPlayerStatistics(HWND hWnd);
void                SaveData(const std::wstring& filePath, const std::vector<std::vector<int>>& playerRolls, const std::vector<HWND>& playerLabels);
std::wstring        GetSaveFilePath();
void                LoadData(const std::wstring& filePath, HWND hWnd);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

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
                    // Call your save function here
                    SaveData(L"\PlayerAndRollStatsSave.txt", playerRolls, playerLabels);
                    DestroyWindow(hWnd); // Proceed to destroy the window after saving
                }
                else if (result == IDNO) {
                    DestroyWindow(hWnd); // Close the window without saving
                }
            }
                break;
            break;
            case ID_BUTTON_CALCULATE:
                // Here you can retrieve inputs, perform calculations, and set output
                WCHAR buffer[256];
                GetDlgItemText(hWnd, ID_EDIT_PEOPLE, buffer, 256);
                // Convert buffer to an integer or process text as needed
                SetDlgItemText(hWnd, ID_TEXT_RESULT, L"Result displayed here after processing input.");
                break;
            case ID_BUTTON_ADD:
                AddPlayer(hWnd);
                break;
            case IDM_SAVE:  // Assume IDM_SAVE is defined and linked to a "Save" menu item or button
                SaveData(L"\PlayerAndRollStatsSave.txt", playerRolls, playerLabels);
                break;
            default:
                // Handling dynamic button commands for submit and remove
                if (wmId >= ID_BUTTON_SUBMIT_BASE && wmId < ID_BUTTON_SUBMIT_BASE + 100) {
                    int playerIndex = wmId - ID_BUTTON_SUBMIT_BASE;
                    WCHAR buffer[256];

                    // Retrieve the current text from the input box
                    GetDlgItemText(hWnd, 5000 + playerIndex, buffer, 256);
                    int roll = _wtoi(buffer);  // Convert input to integer

                    if (roll != 0 || wcslen(buffer) > 0) {  // Check to make sure the buffer had a number
                        playerRolls[playerIndex - 1].push_back(roll);  // Store the roll
                        //MessageBox(hWnd, L"Roll submitted!", L"Success", MB_OK);

                        // Clear the input box after submission
                        SetDlgItemText(hWnd, 5000 + playerIndex, L"");

                        // Update statistics for all players
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
                    int playerIndex = (wmId - ID_BUTTON_HISTORY_BASE) - 1;  // This gives 1 for first player, 2 for second, etc.
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
            // Assuming SaveData takes care of getting the path and saving all data.
            SaveData(GetSaveFilePath(), playerRolls, playerLabels);
            DestroyWindow(hWnd); // Proceed to destroy the window after saving
        }
        else if (result == IDNO) {
            DestroyWindow(hWnd); // Close the window without saving
        }
        // If result is IDCANCEL, do nothing, effectively cancelling the close.
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
    static int playerIndex;  // Keep track of which player's rolls we're editing

    switch (message) {
    case WM_INITDIALOG:
        playerIndex = (int)lParam; // Cast is necessary as lParam is LPARAM (long pointer)
        if (playerIndex >= 0 && playerIndex < playerRolls.size()) {
            for (int roll : playerRolls[playerIndex]) {
                WCHAR buffer[32];
                wsprintf(buffer, L"Roll: %d", roll);
                SendDlgItemMessage(hDlg, IDC_ROLL_LIST, LB_ADDSTRING, 0, (LPARAM)buffer);
            }
        }
        else {
            MessageBox(hDlg, L"Player index is out of range.", L"Error", MB_OK);
        }
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_REMOVE_ROLL:
        {
            // New block to ensure selectedIdx initialization isn't skipped
            int selectedIdx = SendDlgItemMessage(hDlg, IDC_ROLL_LIST, LB_GETCURSEL, 0, 0);
            if (selectedIdx != LB_ERR) {
                SendDlgItemMessage(hDlg, IDC_ROLL_LIST, LB_DELETESTRING, selectedIdx, 0);
                playerRolls[playerIndex].erase(playerRolls[playerIndex].begin() + selectedIdx);
                UpdateAllPlayerStatistics(hDlg);
            }
        }
        break;
        case IDCANCEL:
            // Close the dialog when the 'Cancel' or 'X' button is pressed
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



//CODE FROM HERE DOWN IS CUSTOM CLASSES

void InitializeControls(HWND hWnd) {
    // Create "Add Player" button
    CreateWindow(L"BUTTON", L"Add Player",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        50, 10, 100, 25, hWnd, (HMENU)ID_BUTTON_ADD, GetModuleHandle(NULL), NULL);

    // Create "Remove Player" button
    CreateWindow(L"BUTTON", L"Save",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        160, 10, 120, 25, hWnd, (HMENU)IDM_SAVE, GetModuleHandle(NULL), NULL);
}

//Function to add players to the system
void AddPlayer(HWND hWnd, const std::wstring& name, const std::vector<int>& rolls) {
    playerCount++;
    int baseY = 40 + 30 * (playerCount - 1);

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
        playerRolls.push_back(std::vector<int>());
    }
    else {
        playerRolls.push_back(rolls);
        // Update the statistics to reflect the loaded data
        UpdatePlayerStatistics(hWnd, playerCount - 1);
    }
}

// Function to remove players from the system
void RemovePlayer(HWND hWnd, int playerIndex) {
    int arrayIndex = playerIndex - 1;

    // Destroy controls for player details
    DestroyWindow(playerLabels[arrayIndex]);
    DestroyWindow(playerEdits[arrayIndex]);
    DestroyWindow(GetDlgItem(hWnd, ID_BUTTON_SUBMIT_BASE + playerIndex));
    DestroyWindow(GetDlgItem(hWnd, ID_BUTTON_REMOVE_BASE + playerIndex));
    DestroyWindow(GetDlgItem(hWnd, ID_BUTTON_HISTORY_BASE + playerIndex));

    // Destroy statistic controls
    DestroyWindow(playerStats[arrayIndex].hAvg);
    DestroyWindow(playerStats[arrayIndex].hMostFreq);
    DestroyWindow(playerStats[arrayIndex].hLatest);
    DestroyWindow(playerStats[arrayIndex].hComparison);

    // Erase entries from vectors
    playerLabels.erase(playerLabels.begin() + arrayIndex);
    playerEdits.erase(playerEdits.begin() + arrayIndex);
    playerRolls.erase(playerRolls.begin() + arrayIndex);
    playerStats.erase(playerStats.begin() + arrayIndex);

    // Decrease IDs of all subsequent players and move their controls up
    for (size_t i = arrayIndex; i < playerLabels.size(); ++i) {
        int newY = 40 + 30 * i;

        // Move player details controls
        SetWindowPos(playerLabels[i], HWND_TOP, 50, newY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        SetWindowPos(playerEdits[i], HWND_TOP, 160, newY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        SetWindowPos(GetDlgItem(hWnd, ID_BUTTON_SUBMIT_BASE + i + 1), HWND_TOP, 270, newY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        SetWindowPos(GetDlgItem(hWnd, ID_BUTTON_REMOVE_BASE + i + 1), HWND_TOP, 340, newY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        SetWindowPos(GetDlgItem(hWnd, ID_BUTTON_HISTORY_BASE + i + 1), HWND_TOP, 370, newY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

        // Move statistic controls
        SetWindowPos(playerStats[i].hAvg, HWND_TOP, 440, newY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        SetWindowPos(playerStats[i].hMostFreq, HWND_TOP, 550, newY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        SetWindowPos(playerStats[i].hLatest, HWND_TOP, 710, newY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        SetWindowPos(playerStats[i].hComparison, HWND_TOP, 820, newY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

        // Update control IDs as needed
        SetWindowLong(playerLabels[i], GWL_ID, 5100 + i);
        SetWindowLong(playerEdits[i], GWL_ID, 5000 + i);
        SetWindowLong(GetDlgItem(hWnd, ID_BUTTON_SUBMIT_BASE + i + 1), GWL_ID, ID_BUTTON_SUBMIT_BASE + i + 1);
        SetWindowLong(GetDlgItem(hWnd, ID_BUTTON_REMOVE_BASE + i + 1), GWL_ID, ID_BUTTON_REMOVE_BASE + i + 1);
        SetWindowLong(GetDlgItem(hWnd, ID_BUTTON_HISTORY_BASE + i + 1), GWL_ID, ID_BUTTON_HISTORY_BASE + i + 1);
    }

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

double groupAverageExcludingPlayer(const std::vector<std::vector<int>>& allRolls, int excludedPlayerIndex) {
    double totalSum = 0;
    int totalCount = 0;
    for (size_t i = 0; i < allRolls.size(); ++i) {
        if (i != excludedPlayerIndex) {
            for (int roll : allRolls[i]) {
                totalSum += roll;
                totalCount++;
            }
        }
    }
    return totalCount > 0 ? totalSum / totalCount : 0;
}

void UpdatePlayerStatistics(HWND hWnd, int playerIndex) {
    if (playerIndex < 0 || playerIndex >= playerStats.size()) return;

    // Calculating statistics
    double avg = calculateAverage(playerRolls[playerIndex]);
    std::string mostFreq = findMostFrequent(playerRolls[playerIndex]);
    int latestRoll = !playerRolls[playerIndex].empty() ? playerRolls[playerIndex].back() : 0;
    double compAvg = groupAverageExcludingPlayer(playerRolls, playerIndex);

    // Formatting and updating the text displays
    WCHAR buffer[256];
    swprintf(buffer, 256, L"Avg: %.2f", avg);
    SetWindowText(playerStats[playerIndex].hAvg, buffer);

    swprintf(buffer, 256, L"Most Freq: %S", mostFreq.c_str());
    SetWindowText(playerStats[playerIndex].hMostFreq, buffer);

    swprintf(buffer, 256, L"Latest: %d", latestRoll);
    SetWindowText(playerStats[playerIndex].hLatest, buffer);

    if (compAvg != 0) {
        swprintf(buffer, 256, L"Comp: %.2f%%", (avg - compAvg) / compAvg * 100);
    }
    else {
        wcscpy_s(buffer, L"Comp: N/A");  // Handle division by zero or undefined comparison
    }
    SetWindowText(playerStats[playerIndex].hComparison, buffer);
}

void UpdateAllPlayerStatistics(HWND hWnd) {
    for (size_t i = 0; i < playerRolls.size(); ++i) {
        UpdatePlayerStatistics(hWnd, i);
    }
}

void SaveData(const std::wstring& filePath, const std::vector<std::vector<int>>& playerRolls, const std::vector<HWND>& playerLabels) {
    std::wofstream outFile(filePath);
    if (!outFile.is_open()) {
        MessageBox(NULL, L"Failed to open file for writing.", L"Error", MB_ICONERROR);
        return;
    }

    wchar_t buffer[256];
    for (size_t i = 0; i < playerRolls.size(); ++i) {
        GetWindowText(playerLabels[i], buffer, 256);  // Get the player name from the edit control
        outFile << buffer << L",";  // Save player name

        for (int roll : playerRolls[i]) {
            outFile << roll << L",";  // Save each roll
        }
        outFile << std::endl;  // New line for next player
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
    while (std::getline(inFile, line)) {
        std::wistringstream iss(line);
        std::wstring part;
        std::vector<int> rolls;

        std::getline(iss, part, L',');
        std::wstring name = part;

        while (std::getline(iss, part, L',')) {
            if (part.empty()) continue;
            std::wistringstream rollStream(part);
            int roll;
            if (rollStream >> roll) { 
                rolls.push_back(roll);
            }
        }

        AddPlayer(hWnd, name, rolls);
    }
}