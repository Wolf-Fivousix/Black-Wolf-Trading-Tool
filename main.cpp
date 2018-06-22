/**********************************************************************************************
 *                         Black Wolf Trading Tool                                            *
 *                       Created by Diego "Wolf Fivousix"   -   2014~2015                     *
 * Credtis: Mayra de Barros Gallo (Yuki) for the Black Wolf icon.                             *
 *          theForger and his tutorial: http://www.winprog.org/tutorial/                      *
 **********************************************************************************************/

/**
This program uses Guild Wars 2 API to craw through the Black Lion Trading Company offers and retrieve the price of every listing.
    OVERVIEW:
        As the program starts it requests the current listings from the server, with that information,
it builds the Data Base and requests the prices for every item. From there it just displays them from the last
to first, based on ID, so the most recently added item to the game are showed first. There is a search field that
allows for display of specific items (case sensitive), but requires a minimum of 2 letters. The profit button will
organize the display from the most profitable trade to the least.

13) Future Improvement: Make a splash screen that will tell what the program is loading before showing up on the screen. (Then re-work the cmd output for it.)
            Depending on how much improvement #30 can make, this item might become irrelevant.
18) Future Improvement: Make the ID/Profit buttons invert the display of items (From "low to high" to "high to low").
22) Future Improvement: Make the Search Field work with a "enter key" press.
23) Future Improvement: Make the scroll bar not "flicker" the screen when dragged after using the search button with a small sample, like Tormented/Mordrem/etc...
29) Future Improvement: Skill Point transformation profits. (Skill points are about to be removed from the game with the introduction of Heart of Thorns expansion).
29) Future Improvement: Create a "not yet available" icon for itens recently introduced and not yet displayed on the Items API (which also causes "empty" name display).
30) Future Improvement: Optimize the way requests for Names and Prices are made (This is the highest priority em terms of bottle neck).
31) Future Improvement: Make items searchable by ID using a paramenter in the search box (if starts with '-' search for the spcified ID).
32) Future Improvement: Account for ALL buy orders in the whole market and track how much money is invested in the market.
33) Future Improvement: Something similar to #32, but only regarding a specific Weapon Skin, like Chaos, to see how the overall price behaves through time.
            (Further considerations, as how often and where the program would back up such information are needed - and might render this useless.)
34)
Hot fix: Potent Superior Sharpening Stone has a wrong icon, type "Pot" and look for it"
**/
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if defined(UNICODE) && !defined(_UNICODE)
    #define _UNICODE
#elif defined(_UNICODE) && !defined(UNICODE)
    #define UNICODE
#endif

#include "Resources.h"      // Icon, menu definitions AND more definitions, lots of them.
#include <tchar.h>
#include <windows.h>
#include <fstream>
#include <iostream>
#include <wininet.h>        //Don`t forget to put "wininet" in the Project > Build Options > Linker Settings for the Windows library.
#include "ItemTree.h"
#include <gdiplus.h>
#include <sstream>

// Global Variables
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ItemTree    dataBaseTree;
ItemTree    *searchTree;
ItemTree    *profitTree;
ItemTree    *paintTree;
extern int screenPosition;
SCROLLINFO GLOBAL_ScrollBarInfo;
HWND GLOBAL_NameSearchField;
int ItemTree::ItemNode::STATIC_ItemOrder = 0;
int ItemTree::ItemNode::STATIC_ItemPosition = 0;
int ItemTree::ItemNode::STATIC_ItemsLoaded = 0;
        // Windows class name
TCHAR szClassName[ ] = _T("Black Wolf Trading Tool Software");

// Function Declarations
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK AboutDialogProcedure(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
void BuildBalancedProfitTree(int itemsBuffer[], int startPosition, int endPosition);
void BuildProfitTreeUsingDataBaseTree();
void CreateItemTree(HWND hwnd);
void CreateBalancedTree(int itemsBuffer[], int startPosition, int endPosition);
void DownloadListingsFile(HWND hwnd);
void DownloadAndLoadImagesFiles(HWND hwnd);
void DownloadAndLoadPricesFile(const HWND hwnd);
void Make199NamesRequests(const HWND hwnd, std::ofstream &BLACKLION_FILE, const int itemsDownloaded);
void Make199PricesRequests(const HWND hwnd, std::ofstream &BLACKLION_FILE, const int itemsDownloaded);
void NamesCheckFile(const HWND hwnd);
void NamesCreateFile(const HWND hwnd);
void NamesLoadFile(const HWND hwnd);
void PricesLoader(HWND hwnd);
void SaveTreeInFile(ItemTree &treeToSave);

int WINAPI WinMain (HINSTANCE hThisInstance, HINSTANCE hPrevInstance, LPSTR lpszArgument, int nCmdShow)
{
    HWND hwnd;               /* This is the handle for our window */
    MSG messages;            /* Here messages to the application are saved */
    WNDCLASSEX wincl;        /* Data structure for the windowclass */


    Gdiplus::GdiplusStartupInput gdiplusStartUpInput;                       // For Image display.
    ULONG_PTR           gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartUpInput, NULL);     // For Image display.

    /* The Window structure */
    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
    wincl.style = CS_DBLCLKS;                 /* Catch double-clicks */
    wincl.cbSize = sizeof (WNDCLASSEX);

    // Use customized icon and default mouse-pointer
    wincl.hIcon = LoadIcon (GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON));
    wincl.hIconSm = (HICON) LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON), IMAGE_ICON, 16, 16, 0);
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;                 /* No menu */
    wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
    wincl.cbWndExtra = 0;                      /* structure or the window instance */
    /* Use Windows's default colour as the background of the window */
    wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;

    /* Register the window class, and if it fails quit the program */
    if (!RegisterClassEx (&wincl))
        return 0;

    /* The class is registered, let's create the program*/
    hwnd = CreateWindowEx (0,                   /* Extended possibilites for variation */
                           szClassName,         // Classname */
                           _T("Black Wolf Trading Tool"),       // Title Text
                           WS_MINIMIZEBOX | WS_SYSMENU | WS_VSCROLL, // Window options (Enables minimizing and blocks resizing).
                           4, 4,                // Screen Position
                           MAIN_WINDOW_WIDTH, MAIN_WINDOW_HEIGHT,  // The programs Width and Height (Scroll Bar takes 20 width, and Menu Bar takes X height)
                           HWND_DESKTOP,        /* Parent Window (The window is a child-window to desktop) */
                           NULL,                /* Menu (No automatic menu) */
                           hThisInstance,       /* Program Instance handler */
                           NULL);                 /* Additional Creation Data (No Window Creation data) */


    /* Make the window visible on the screen */
    ShowWindow (hwnd, nCmdShow);

    // GetMessage() will return -1 if it encounters an error, but we will NOT be handling erros at this point.
    while (GetMessage (&messages, NULL, 0, 0) > 0)
    {
        /* Translate virtual-key messages into character messages */
        TranslateMessage(&messages);
        /* Send message to WindowProcedure */
        DispatchMessage(&messages);
    }


    Gdiplus::GdiplusShutdown(gdiplusToken);                         // For Image display.
    /* The program return-value is 0 - The value that PostQuitMessage() gave */
    return messages.wParam;
}

//      Windows Procedure
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    //SCROLLINFO GLOBAL_ScrollBarInfo;
    switch (message)                  /* handle the messages */
    {
        case WM_CREATE:
            {
                /// Creating the MENU
                HMENU mainMenu;
                mainMenu = CreateMenu();
                AppendMenu(mainMenu, MF_STRING, MENU_ABOUT_BUTTON, "About");
                SetMenu(hwnd, mainMenu);

                /// Updating the Data Base
                DownloadListingsFile(hwnd);     // Communicate with GW2 server and creates the Listings File.
                CreateItemTree(hwnd);           // Read Listings file and create the ItemTree based on ID.
                DownloadAndLoadImagesFiles(hwnd); // Images check.
                NamesCheckFile(hwnd);   // Names check.
                DownloadAndLoadPricesFile(hwnd);  // Prices check.
                std::cout << "Items in the Listings: " << dataBaseTree.GetTreeSize() << std::endl;
                // At this point the DataBaseTree is full and all variables filled.

                /// Building the Profit Tree.
                // Profit Tree.
                profitTree = new ItemTree;
                dataBaseTree.CreateOrderedProfitTree(*profitTree);
                // Profit Tree created, now balance this tree (save ID's to a file, load the file and compare each ID with the DataBase).
                SaveTreeInFile(*profitTree);
                delete profitTree;          // Delete the old tree used to create the balanced tree.
                profitTree = new ItemTree;  // Creates the new (empty) tree. This "new" has NO DELETE, as it is going to be my profit tree.
                // Build the new profitTree with the values in the DataBaseTree.
                BuildProfitTreeUsingDataBaseTree();

                /// Creating the Window.
                paintTree = &dataBaseTree;      // Default screen loads the DBT (ordered by ID).
                // Buttons.
                HWND buttonID = CreateWindow("BUTTON", "ID", WS_VISIBLE | WS_CHILD | WS_BORDER,
                                                 0, 0, 48, 24, hwnd, (HMENU) WINDOW_ID_BUTTON, NULL, NULL);
                GLOBAL_NameSearchField = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
                                                 100, 0, 250, 24, hwnd, NULL , NULL, NULL);
                HWND buttonSearch = CreateWindow("BUTTON", "Search", WS_VISIBLE | WS_CHILD | WS_BORDER,
                                                 360, 0, 52, 24, hwnd, (HMENU) WINDOW_SEARCH_BUTTON, NULL, NULL);

                HWND buttonProfit = CreateWindow("BUTTON", "Profit", WS_VISIBLE | WS_CHILD | WS_BORDER,
                                                 835, 7, 48, 24, hwnd, (HMENU) WINDOW_PROFIT_BUTTON, NULL, NULL);
                // Scroll bar.
                GLOBAL_ScrollBarInfo = {sizeof(GLOBAL_ScrollBarInfo)};
                GLOBAL_ScrollBarInfo.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
                GLOBAL_ScrollBarInfo.nMin = 0;
                // Use the number of items in the tree to calculate the maximum value for the scroll bar, -8 Itens on the screen.
                // (-7 so the scroll bar can reach the last item in the list with the drag and drop function)
                GLOBAL_ScrollBarInfo.nMax = (dataBaseTree.GetTreeSize()-7) * 64;
                GLOBAL_ScrollBarInfo.nPage = 64;  // Size of each "scroll".
                GLOBAL_ScrollBarInfo.nPos = 0;    // Starting position for the Thumb.
                SetScrollInfo(hwnd, SB_VERT, &GLOBAL_ScrollBarInfo , TRUE);

                // Hides the console window (this can be removed once a splash screen is created).
                ShowWindow(GetConsoleWindow(), SW_HIDE);
                break;
            }
        case WM_COMMAND:
            {
                switch (LOWORD(wParam))
                {
                    case MENU_ABOUT_BUTTON:
                        {
                            DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ABOUT_WINDOW), hwnd, AboutDialogProcedure);
                            break;
                        }
                    case WINDOW_ID_BUTTON:
                        {
                            paintTree = &dataBaseTree;
                            // Update Scroll Barr.
                            GLOBAL_ScrollBarInfo = {sizeof(GLOBAL_ScrollBarInfo)};
                            GLOBAL_ScrollBarInfo.fMask = SIF_RANGE;
                            GLOBAL_ScrollBarInfo.nMin = 0;
                            GLOBAL_ScrollBarInfo.nMax = (dataBaseTree.GetTreeSize()-7) * 64;
                            SetScrollInfo(hwnd, SB_VERT, &GLOBAL_ScrollBarInfo , TRUE);

                            InvalidateRect(hwnd, NULL, TRUE);
                            break;
                        }
                    case WINDOW_SEARCH_BUTTON:
                        {
                            if (searchTree == NULL)     // First time the search tree is used, there is no object associated with it.
                            {
                                searchTree = new ItemTree;
                            }
                            else        // SearchTree was already used, therefore points to an object that must be deleted.
                            {
                                delete searchTree;
                                searchTree = new ItemTree;
                            }
                            // Process the input.
                            char tempText[50];
                            GetWindowText(GLOBAL_NameSearchField, tempText, 49);
                            std::string textFromInput = tempText;
                            if (textFromInput.size() <= 1)
                            {
                                // If the search field is empty it generates SIGSEV due to the size of the tree produced.
                                // If the search field is "a" it generates SIGSEV due to the size of the tree produced.
                                // By doing this I also loose the ability to search with one letter (which doesn't happen offen anyway).
                                break;
                            }
                            dataBaseTree.CreateSearchTree(*searchTree, textFromInput);
                            paintTree = searchTree;

                            // Resize the Scroll Bar due to the change in the display tree.
                            GLOBAL_ScrollBarInfo = {sizeof(GLOBAL_ScrollBarInfo)};
                            GLOBAL_ScrollBarInfo.fMask = SIF_RANGE| SIF_POS;
                            GLOBAL_ScrollBarInfo.nMin = 0;
                            GLOBAL_ScrollBarInfo.nMax = (searchTree->GetTreeSize()-7) * 64;
                            GLOBAL_ScrollBarInfo.nPos = 0;
                            SetScrollInfo(hwnd, SB_VERT, &GLOBAL_ScrollBarInfo , TRUE);
                            screenPosition = 0;

                            InvalidateRect(hwnd, NULL, TRUE);
                            break;
                        }
                    case WINDOW_PROFIT_BUTTON:
                        {
                            paintTree = profitTree;
                            // Update Scroll Barr.
                            GLOBAL_ScrollBarInfo = {sizeof(GLOBAL_ScrollBarInfo)};
                            GLOBAL_ScrollBarInfo.fMask = SIF_RANGE;
                            GLOBAL_ScrollBarInfo.nMin = 0;
                            GLOBAL_ScrollBarInfo.nMax = (profitTree->GetTreeSize()-7) * 64;
                            SetScrollInfo(hwnd, SB_VERT, &GLOBAL_ScrollBarInfo , TRUE);

                            InvalidateRect(hwnd, NULL, TRUE);
                            break;
                        }
                    default:
                        ;   // do nothing.
                }
                break;
            }
        case WM_DESTROY:
            {
                PostQuitMessage (0);       /* send a WM_QUIT to the message queue */
                break;
            }
        case WM_VSCROLL:
            {
                switch (LOWORD(wParam))
                {
                    case SB_LINEUP:
                        {
                            if (screenPosition > GLOBAL_ScrollBarInfo.nMin)
                            {
                                screenPosition -= 64;
                                InvalidateRect(hwnd, NULL, TRUE);
                            }
                            break;
                        }
                    case SB_LINEDOWN:
                        {
                            if (screenPosition < GLOBAL_ScrollBarInfo.nMax-64)    // 64 pixels off due to the scrolling bar range.
                            {
                                screenPosition += 64;
                                InvalidateRect(hwnd, NULL, TRUE);
                            }
                            break;
                        }
                    case SB_THUMBPOSITION:
                        {
                            GLOBAL_ScrollBarInfo.cbSize = sizeof(GLOBAL_ScrollBarInfo);
                            GLOBAL_ScrollBarInfo.fMask = SIF_TRACKPOS;
                            GetScrollInfo(hwnd, SB_VERT, &GLOBAL_ScrollBarInfo);
                            screenPosition = GLOBAL_ScrollBarInfo.nTrackPos;
                            InvalidateRect(hwnd, NULL, TRUE);
                            break;
                        }
                    case SB_THUMBTRACK:
                        {
                            GLOBAL_ScrollBarInfo.cbSize = sizeof(GLOBAL_ScrollBarInfo);
                            GLOBAL_ScrollBarInfo.fMask = SIF_TRACKPOS;
                            GetScrollInfo(hwnd, SB_VERT, &GLOBAL_ScrollBarInfo);
                            screenPosition = GLOBAL_ScrollBarInfo.nTrackPos;
                            InvalidateRect(hwnd, NULL, TRUE);
                            break;
                        }
                    default:
                        ; // Do nothing.
                }
                GLOBAL_ScrollBarInfo.cbSize = sizeof(GLOBAL_ScrollBarInfo);
                GLOBAL_ScrollBarInfo.fMask = SIF_POS;
                GLOBAL_ScrollBarInfo.nPos = screenPosition;
                SetScrollInfo(hwnd, SB_VERT, &GLOBAL_ScrollBarInfo, TRUE);

                break;
            }
            /* Attempt to make the Edit Search Box read the input when "enter key" is pressed.
        case WM_CHAR:
            {
                // this doesn't work because of te "window focus" (alt+tab would solve it, but that is not good).
                std::cout << "test" << std::endl;
                if (wParam == 13)
                {
                    std::cout << "Enter..." << std::endl;
                }
                break;
            }
            */

        case WM_MOUSEWHEEL:
            {
                /* std::cout << "wParam:" << wParam << std::endl;
                 In my tests the wParam is either:      Moving Down              Moving Up
                                   (most of the time) 4.287.102.976              78.644.320 (most of the time)
                                                      4.279.238.656              15.728.640
                                                      4.255.645.696              31.457.280 (rarely)
                 So even when the "non common" values do happen, ignoring them seems to have no effect whatsoever.
                //*/
                if (screenPosition < GLOBAL_ScrollBarInfo.nMax-64 && wParam > 4000000000)   // Moving Down
                {
                    screenPosition += 64;
                    InvalidateRect(hwnd, NULL, TRUE);
                }
                else if (screenPosition > GLOBAL_ScrollBarInfo.nMin && wParam < 100000000)  // Moving Up
                {
                    screenPosition -= 64;
                    InvalidateRect(hwnd, NULL, TRUE);
                }
                // Update the scroll bar.
                GLOBAL_ScrollBarInfo.cbSize = sizeof(GLOBAL_ScrollBarInfo);
                GLOBAL_ScrollBarInfo.fMask = SIF_POS;
                GLOBAL_ScrollBarInfo.nPos = screenPosition;
                SetScrollInfo(hwnd, SB_VERT, &GLOBAL_ScrollBarInfo, TRUE);
                break;
            }
        case WM_PAINT:
            {
                PAINTSTRUCT ps;                             // Needed for BeginPaing.

                HDC hdc = BeginPaint(hwnd, &ps);

                // Inicializtion of GDC+ for drawing text.
                Gdiplus::Graphics graphics(hdc);
                Gdiplus::SolidBrush brush(Gdiplus::Color(255, 0, 0, 0));
                Gdiplus::FontFamily fontFamily(L"Times New Roman");
                Gdiplus::Font font(&fontFamily, 20, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
                Gdiplus::PointF pointF;             // Printing position.
                Gdiplus::StringFormat stringFormat; // Object for setting text alignment.
                pointF.X = 517;
                pointF.Y = 5;
                graphics.DrawString(L"Buy Price", -1, &font, pointF, &brush);
                pointF.X = 667;
                pointF.Y = 5;
                graphics.DrawString(L"Sell Price", -1, &font, pointF, &brush);

                // Draw of the items.
                paintTree->SetStaticToZero();
                paintTree->DisplayItems(hwnd, hdc);

                EndPaint(hwnd, &ps);
                break;
            }
        default:                      /* for messages that we don't deal with */
            return DefWindowProc (hwnd, message, wParam, lParam);
    }
    return 0;
}
//      About Dialog Procedure
//          Opens a new window with information.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK AboutDialogProcedure(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch(Message)
	{
		case WM_COMMAND:
            {
                switch(LOWORD(wParam))
                {
                    case IDOK:
                        {
                            EndDialog(hwnd, IDOK);
                            break;
                        }
                }
            }
		default:
            {
                return FALSE;
            }
	}
	return TRUE;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////   FUNCTIONS  ///////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Build Balanced Profit Tree
//      Reads the Data Base Tree and creates a Balanced Profit Tree.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void BuildBalancedProfitTree(int itemsBuffer[], int startPosition, int endPosition)
{
    if (startPosition > endPosition)
    {
        return; // Do nothing, base case.
    }
    int middlePosition = (startPosition + endPosition)/2;

    // Get the Item values from the Data Base.
    int itemID = itemsBuffer[middlePosition];
    int itemBuyPrice;
    int itemSellPrice;
    int itemProfit;
    std::string itemName;
    dataBaseTree.GetItemNodeAttributes(itemID, itemBuyPrice, itemSellPrice, itemProfit, itemName);
    // Add one item in the newTree.
    profitTree->InsertItemByProfit(itemsBuffer[middlePosition], itemProfit, itemBuyPrice, itemSellPrice, itemName);
    // Populate left branch.
    BuildBalancedProfitTree(itemsBuffer, startPosition, middlePosition-1);
    // Populate right branch.
    BuildBalancedProfitTree(itemsBuffer, middlePosition+1, endPosition);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Build Profit Tree Using Data Base Tree
//      Create profitTree using the DataBase.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void BuildProfitTreeUsingDataBaseTree()
{
    std::cout << "Reading player level: ";
    std::ifstream NEW_ORDERED_TREE_FILE ("ProfitOrder.txt");      // Open my just created Listings File.
    if (!NEW_ORDERED_TREE_FILE)
    {
        std::cout << "NewTree building FAILED!" << std::endl;
        return;
    }
    std::string stringBuffer;
    int itemsInFile = 0;
    int itemsBuffer [100000];
    memset(itemsBuffer, 0, sizeof(itemsBuffer));
    while (std::getline(NEW_ORDERED_TREE_FILE, stringBuffer))
    {
        itemsBuffer[itemsInFile] = atoi(stringBuffer.c_str());      // Convert the string buffer to integer
        itemsInFile++;
    }
    NEW_ORDERED_TREE_FILE.close();                             // Close the Listings File file.
    BuildBalancedProfitTree(itemsBuffer, 0, itemsInFile-1);  //ItemsInFile-1 so I don't step outside of my array!
    std::cout << "Over 9.000 !!" << std::endl;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//      Create Item Tree
//          Open the Listings file and populate the dataBaseTree with all ID's.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CreateItemTree(HWND hwnd)
{
    std::cout << "Creating Item Tree...";
    std::ifstream BLACKLION_LISTINGS ("Listings.txt");      // Open my just created Listings File.
    if (!BLACKLION_LISTINGS)
    {
        MessageBox(hwnd, "BLACKLION_LISTINGS Failled to open!", "Listings Error", MB_OK | MB_ICONEXCLAMATION);
        DestroyWindow(hwnd);
        return;
    }
    std::string stringBuffer;
    int itemsInFile = 0;
    int itemsBuffer [100000];
    memset(itemsBuffer, 0, sizeof(itemsBuffer));
    while (std::getline(BLACKLION_LISTINGS, stringBuffer))
    {
        itemsBuffer[itemsInFile] = atoi(stringBuffer.c_str());      // Convert the string buffer to integer
        itemsInFile++;
    }
    BLACKLION_LISTINGS.close();                             // Close the Listings File file.

    CreateBalancedTree(itemsBuffer, 0, itemsInFile-1);  //ItemsInFile-1 so I don't step outside of my array!
    std::cout << " Item Tree Created!" << std::endl;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Create Balanced Tree
//      This recursive function will give the Binary Tree the correct values in order to create a Balanced Binary Tree
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CreateBalancedTree(int itemsBuffer[], int startPosition, int endPosition)
{
    if (startPosition > endPosition)
    {
        return; // Do nothing, base case.
    }
    int middlePosition = (startPosition + endPosition)/2;
    dataBaseTree.InsertItem(itemsBuffer[middlePosition]);
    // Populate left branch.
    CreateBalancedTree(itemsBuffer, startPosition, middlePosition-1);
    // Populate right branch.
    CreateBalancedTree(itemsBuffer, middlePosition+1, endPosition);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  DownloadListingsFile
//      Get the listings from the server and creates the "Listings.txt" file in the right format.
//       - Use the perfected version of IternetReadFile and teste it reading a smaller buffer multiple times!!
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DownloadListingsFile(HWND hwnd)
{
    std::cout << "Creating Listings File...";
    std::ofstream BLACKLION_LISTINGS ("Listings.txt");

    std::string listingsLink = "https://api.guildwars2.com/v2/commerce/listings/";
    HINTERNET linkOpen = InternetOpen(listingsLink.c_str(), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if(!linkOpen)
    {
      MessageBox(hwnd, "Listings Link failed!", "Database Error", MB_OK | MB_ICONEXCLAMATION);
      InternetCloseHandle(linkOpen);
      return;
    }
    HINTERNET openUrl = InternetOpenUrl(linkOpen, listingsLink.c_str(), NULL, 0, INTERNET_FLAG_PRAGMA_NOCACHE|INTERNET_FLAG_KEEP_CONNECTION, 0);
    if(!openUrl)
    {
      MessageBox(hwnd, "Open Listings URL failed!", "Database Error", MB_OK | MB_ICONEXCLAMATION);
      InternetCloseHandle(openUrl);
      InternetCloseHandle(linkOpen);
      return;
    }
    char dataBuffer[1024 * 1];                      // 1 KB of buffer
    memset(dataBuffer, 0, sizeof(dataBuffer));      // Zero the entire contents of the dataBuffer to prevent garbage from memmory.
    std::string dataReceived;
    std::string newID;
    DWORD NumberOfBytesRead = 0;
    // Receives the data from the web page.
    while(InternetReadFile(openUrl, dataBuffer, sizeof(dataBuffer), &NumberOfBytesRead) && NumberOfBytesRead != 0)
    {
        dataReceived.append(dataBuffer);
        memset(dataBuffer, 0, sizeof(dataBuffer));
    }
    // Treating the data.
    newID = dataReceived.substr(1, dataReceived.find(",") - 1);     // Removes the '[' starter and get the first ID.
    while (dataReceived.length() > 6)                               // The last ID is over 68000 + ']', that secures that the last item will never have a .length() shorter than 6.
    {
        // These items were removed from the TP and should not be in the Listings, but up to this point they are returned.
        // Comparing them with the v2/items listings WILL give a propper valid comparison (with the exeption of 29978 - Karma Item).
        if (   newID == "9102"
            || newID == "29952"
            || newID == "29960"
            || newID == "29964"
            || newID == "29974"
            || newID == "29978"     // Karma item!
            || newID == "31113"
            || newID == "31120"
            || newID == "31124"
            || newID == "31127"
            || newID == "31130"
            || newID == "31131"
            || newID == "31132"
            || newID == "31133"
            || newID == "31135"
            || newID == "31136"
            || newID == "31137"
            || newID == "31139"
            || newID == "31140"
            || newID == "31144"
            || newID == "31167"
            || newID == "31168"
            || newID == "31169"
            || newID == "31175"
            || newID == "31178"
            || newID == "46743")
            // 16th of March update: the v2/items/ API has some lag and so new items need to be manually added at this point, for further information:
            // https://forum-en.guildwars2.com/forum/community/api/Orders-for-new-items/first
        {
            ; // If one of this items is read, do nothing about it (it will be skipped at the end of this loop
        }
        else
        {
            BLACKLION_LISTINGS << newID << std::endl;
        }
        dataReceived.erase(0, dataReceived.find(",")+1);   // From the start of string, finds the first ',', jumps it, and erases the just analysed portion.
        newID = dataReceived.substr(0, dataReceived.find(","));
    }
    // So far the last ID is 68672 and is a valid one, so I don't need to Validate it.
    newID = dataReceived.substr(0, dataReceived.length() -1);       // Removes the ']' ending and get the last ID.
    BLACKLION_LISTINGS << newID << std::endl;
    BLACKLION_LISTINGS.close();
    InternetCloseHandle(openUrl);
    InternetCloseHandle(linkOpen);
    std::cout << " All Listings Created!" << std::endl;
}
//  Download And Load Images Files
//      Check if the static images of the program are present, if not, give a warning.
//   Also uses DatabaseTree to check if all itens have their corresponding image on the computer, if not, it will be downloaded.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DownloadAndLoadImagesFiles(HWND hwnd)
{
    std::cout << "Loading Images...";
    // Checking for static images.
    std::ifstream GOLDCOIN ("Images\\Gold_Coin.png");
    if (!GOLDCOIN)
    {
        MessageBox(hwnd, "Gold_Coin.png image missing", "Image Load Error", MB_OK | MB_ICONEXCLAMATION);
    }
    std::ifstream SILVERCOIN ("Images\\Silver_Coin.png");
    if (!SILVERCOIN)
    {
        MessageBox(hwnd, "Silver_Coin.png image missing", "Image Load Error", MB_OK | MB_ICONEXCLAMATION);
    }
    std::ifstream COPPERCOIN ("Images\\Copper_Coin.png");
    if (!COPPERCOIN)
    {
        MessageBox(hwnd, "Copper_Coin.png image missing", "Image Load Error", MB_OK | MB_ICONEXCLAMATION);
    }

    // Checking for variable images.
    dataBaseTree.LoadImageResources(hwnd);
    std::cout << " All Images Loaded!" << std::endl;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Download And Load Prices File
//      Get the prices for all the items, 200 at a time.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DownloadAndLoadPricesFile(const HWND hwnd)
{
    //*
    // Downloading
    std::cout << "Downloading Prices... " << std::endl;
    std::ofstream BLACKLION_PRICES ("Prices.txt");
    static int itemsOnTheTree = dataBaseTree.GetTreeSize();
    int itemsDownloaded = 0;
    while (itemsDownloaded < itemsOnTheTree)
    {
        Make199PricesRequests(hwnd, BLACKLION_PRICES, itemsDownloaded);
        std::cout << "   " << itemsDownloaded+199 << " prices downloaded..." << std::endl;
        itemsDownloaded +=199;
    }
    BLACKLION_PRICES.close();
    std::cout << "All Prices Downloaded." << std::endl;
    //*/
    // Loading
    std::cout << "Loading Prices...";
    std::ifstream BLACKLION_PRICES_INPUT ("Prices.txt");
    dataBaseTree.LoadPrices(hwnd, BLACKLION_PRICES_INPUT);
    BLACKLION_PRICES_INPUT.close();
    std::cout << " All Prices Loaded!" << std::endl;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Make 199 Names Requests
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Uma possibilidade de melhorar a eficiência dessa função seria criar um arquivo para receber todo o output (no lugar da minha string) e, após puxar todos ID's (que é o mais demorado)
//fazer uma segunda interação cortando todos os ID's.
void Make199NamesRequests(const HWND hwnd, std::ofstream &BLACKLION_FILE, const int itemsDownloaded)
{
    // Open the link for the item.
    std::string apiLink = "https://api.guildwars2.com/v2/items?ids=";
    dataBaseTree.Get200IDS(apiLink, itemsDownloaded, itemsDownloaded + 199);
    dataBaseTree.SetStaticToZero();
    // Initializes an application's use of the WinINet functions. (Creates the handle for the function to work.)
    HINTERNET linkOpen = InternetOpen(apiLink.c_str(), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    // Checks if InternetOpen() got a correct handle.
    if(!linkOpen)
    {
      MessageBox(hwnd, "Open Link failed!", "Download Prices File Failed!", MB_OK | MB_ICONEXCLAMATION);
      InternetCloseHandle(linkOpen);
      return;
    }
    // Opens a resource specified by a complete FTP or HTTP URL.
    HINTERNET openUrl = InternetOpenUrl(linkOpen, apiLink.c_str(), NULL, 0, INTERNET_FLAG_PRAGMA_NOCACHE|INTERNET_FLAG_KEEP_CONNECTION, 0);
    if(!openUrl)
    {
      MessageBox(hwnd, "Open URL failed!\nCreate Item Name Error", "Download Prices File Failed!", MB_OK | MB_ICONEXCLAMATION);
      InternetCloseHandle(openUrl);
      InternetCloseHandle(linkOpen);
      return;
    }
    // Get the item link for downloading the name.
    char dataBuffer[1024 * 1];                         // 1 KB of buffer.
    memset(dataBuffer, 0, sizeof(dataBuffer));         // Zero the entire contents of the dataBuffer to prevent garbage from memmory.
    std::string dataReceived;
    DWORD NumberOfBytesRead = 0;
    // Receives the data from the web page.
    while(InternetReadFile(openUrl, dataBuffer, sizeof(dataBuffer), &NumberOfBytesRead) && NumberOfBytesRead != 0)
    {
        dataReceived.append(dataBuffer);
        memset(dataBuffer, 0, sizeof(dataBuffer));
    }
    // Treating the data.
    while (dataReceived.find("\"name\"") != 4294967295)     // While there is a "name" in the buffer, keep working.
    {
        // ID first.
        // Tem alguma coisa errada com o "find("icon")"... a partir do 8700 ele está encontrando um valor muito baixo, 21...
        // Talvez algo errado com a função de erase?
        std::cout << "id :" << dataReceived.find("\"id\"") << "  /  " << dataReceived.find("\"icon\"")<< std::endl;
        //BLACKLION_FILE << dataReceived.substr((dataReceived.find("\"id\"")), (dataReceived.find("\"icon\"") - dataReceived.find("\"id\""))) << "\t";
        BLACKLION_FILE << dataReceived.substr((dataReceived.find("\"id\"")), 10) << std::endl;
        BLACKLION_FILE << "id :" << dataReceived.find("\"id\"") << "  /  Icon: " << (dataReceived.find("\"icon")) << std::endl;

        // Now the name.
        /* Esse cara não tem nada a ver, o problema está na metade de cima.
        std::cout << "id :" << dataReceived.substr(dataReceived.find("\"id\""), 10) << "  /  " << dataReceived.find("\"id\"") << std::endl;
        std::cout << "name :" << dataReceived.substr(dataReceived.find("\"name\"") + 7, 30) << "  /  " << dataReceived.find("\"name\"") << std::endl;
        std::cout << "type :" << dataReceived.find("\"type\"") << std::endl;
        BLACKLION_FILE << dataReceived.substr((dataReceived.find("\"name\"")), (dataReceived.find("\"type\"") + 1)) << std::endl; // Coppy the ["] character from "type".
        */
        // Erases the copied item.
        dataReceived.erase(0, (dataReceived.find("\"type\"")));     // Erases what was copied.
        dataReceived.erase(0, dataReceived.find("\"name\""));       // Erases the string until the next item.
    }
    InternetCloseHandle(openUrl);
    InternetCloseHandle(linkOpen);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Make 199 Prices Requests
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Make199PricesRequests(const HWND hwnd, std::ofstream &BLACKLION_FILE, const int itemsDownloaded)
{
    // Open the link for the item.
    std::string apiLink = "https://api.guildwars2.com/v2/commerce/prices?ids=";
    dataBaseTree.Get200IDS(apiLink, itemsDownloaded, itemsDownloaded + 199);
    dataBaseTree.SetStaticToZero();
    // Initializes an application's use of the WinINet functions. (Creates the handle for the function to work.)
    HINTERNET linkOpen = InternetOpen(apiLink.c_str(), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    // Checks if InternetOpen() got a correct handle.
    if(!linkOpen)
    {
      MessageBox(hwnd, "Open Link failed!", "Download Prices File Failed!", MB_OK | MB_ICONEXCLAMATION);
      InternetCloseHandle(linkOpen);
      return;
    }
    // Opens a resource specified by a complete FTP or HTTP URL.
    HINTERNET openUrl = InternetOpenUrl(linkOpen, apiLink.c_str(), NULL, 0, INTERNET_FLAG_PRAGMA_NOCACHE|INTERNET_FLAG_KEEP_CONNECTION, 0);
    if(!openUrl)
    {
      MessageBox(hwnd, "Open URL failed!\nCreate Item Name Error", "Download Prices File Failed!", MB_OK | MB_ICONEXCLAMATION);
      InternetCloseHandle(openUrl);
      InternetCloseHandle(linkOpen);
      return;
    }
    // Get the item link for downloading the name.
    char dataBuffer[1024 * 1];                         // 1 KB of buffer.
    memset(dataBuffer, 0, sizeof(dataBuffer));         // Zero the entire contents of the dataBuffer to prevent garbage from memmory.
    std::string dataReceived;
    DWORD NumberOfBytesRead = 0;
    // Receives the data from the web page.
    while(InternetReadFile(openUrl, dataBuffer, sizeof(dataBuffer), &NumberOfBytesRead) && NumberOfBytesRead != 0)
    {
        dataReceived.append(dataBuffer);
        memset(dataBuffer, 0, sizeof(dataBuffer));
    }
    // Treating the data.
    while (dataReceived.find("]") > 1)
    {
        //std::cout << dataReceived << std::endl;
        BLACKLION_FILE << dataReceived.substr(0, dataReceived.find("}}")) << std::endl;      // Send one item per line, delimitated by "}}".
        dataReceived.erase(0, (dataReceived.find("}}")+2));         // Erases the just sent part of the string AND the folowing "}}".
    }
    InternetCloseHandle(openUrl);
    InternetCloseHandle(linkOpen);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Names Check File
//      Perform a check in the Names.txt file
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void NamesCheckFile(const HWND hwnd)
{
    std::ifstream NAMES_CHECK ("Names.txt");
    if (!NAMES_CHECK)   // File is NOT present, create it.
    {
        NamesCreateFile(hwnd);
    }
    else    // File is present, proceed with names check.
    {
        std::string tempNameCheckBuffer ("");
        int nameCounter = 0;
        while(std::getline(NAMES_CHECK, tempNameCheckBuffer))
        {
            nameCounter++;
        }
        NAMES_CHECK.close();    // Closes the file before opening for load.
        if (nameCounter != dataBaseTree.GetTreeSize())    // NOT as many names as items in the tree, proceed to file overwrite.
        {
            NamesCreateFile(hwnd);
        }
        // If the file is present AND has as many names as items in the Data Base Tree, then no need to download.
    }
    NamesLoadFile(hwnd);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Names Create File
//      Create the Names.txt file using all ID's stored in the Data Base Tree.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void NamesCreateFile(const HWND hwnd)
{
    std::cout << "You are missing some item names! I'll download them for you." << std::endl;
    std::cout << "Creating Names File... " << std::endl;
    std::ofstream BLACKLION_NAMES ("Names.txt");
    static int itemsOnTheTree = dataBaseTree.GetTreeSize();
    static int itemsDownloaded = 0;
    while (itemsDownloaded < itemsOnTheTree)
    {
        Make199NamesRequests(hwnd, BLACKLION_NAMES, itemsDownloaded);
        std::cout << "   " << itemsDownloaded+199 << " names downloaded..." << std::endl;
        itemsDownloaded +=199;
    }
    BLACKLION_NAMES.close();
    std::cout << "Names File Created!" << std::endl;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Names Load File
//      Load all the names from Names.txt into the Data Base Tree.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void NamesLoadFile(const HWND hwnd)
{
    std::cout << "Loading Names...";
    std::ifstream ITEM_DATABASE ("Names.txt");
    if (!ITEM_DATABASE)
    {
        MessageBox(hwnd, "ITEM_DATABSE Failed to open!", "Database Error", MB_OK | MB_ICONEXCLAMATION);
        DestroyWindow(hwnd);
        return;
    }
    std::string itemNameBuffer("");
    while(std::getline(ITEM_DATABASE, itemNameBuffer))
    {
        std::string tempBuffer(itemNameBuffer.substr(5, itemNameBuffer.find(",") - 5));  // Get the ID for the item in the line.
        int tempIntID = atoi(tempBuffer.c_str());       // Convert String to Int.
        tempBuffer = itemNameBuffer.substr( (itemNameBuffer.find("\"name\"") + 8), (itemNameBuffer.find("\",\"") - itemNameBuffer.find("\"name\"") - 8));   // Get the Name of the Item.
        dataBaseTree.SetName(tempIntID, tempBuffer);
    }
    ITEM_DATABASE.close();
    std::cout << "All Names Loaded!" << std::endl;
}
//  Save Tree In File
//      Create a file with all ID's from the tree.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SaveTreeInFile(ItemTree &treeToSave)
{
    std::cout << "Sorting Profit... ";
    std::ofstream ORGANIZINGTREEFILE ("ProfitOrder.txt");
    treeToSave.SaveToFile(ORGANIZINGTREEFILE);
    std::cout << "Profit sorted." << std::endl;
}
