#include <windows.h>
#include "ItemTree.h"
#include <iostream>
#include <wininet.h>        //Don`t forget to put "wininet" in the Project > Build Options > Linker Settings for the Windows library.
#include <sstream>          //For converting from std::string to interger.
#include <gdiplus.h>

#define ITEM_DISPLAY_OFFSET     52

extern int screenPosition = 0;
// Destructor
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ItemTree::DestroySubTree(ItemNode *&node)
{
    if (!node)  // The node is empty, nothing to be deleted.
    {
        return;
    }
    DestroySubTree(node->leftBranch);
    DestroySubTree(node->rightBranch);
    delete node;
    //treeSize--;
}
// Insert Item (using ID)
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ItemTree::InsertItem(ItemNode *&root, const int itemID, const int profitValue, const int buyValue, const int sellValue, const std::string &nameValue)
{
    if (!root)
    {
        root = new ItemNode(itemID, profitValue, buyValue, sellValue, nameValue);
        treeSize++;
        return;
    }
    if (root->ID == itemID)
    {
        return;
    }
    if (itemID < root->ID)
    {
        InsertItem(root->leftBranch, itemID, profitValue, buyValue, sellValue, nameValue);
    }
    else
    {
        InsertItem(root->rightBranch, itemID, profitValue, buyValue, sellValue, nameValue);
    }
}
// Insert Item (using ID)
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ItemTree::InsertItemByProfit(ItemNode *&root, const int itemID, const int profitValue, const int buyValue, const int sellValue, const std::string &nameValue)
{
    if (!root)
    {
        root = new ItemNode(itemID, profitValue, buyValue, sellValue, nameValue);
        treeSize++;
        return;
    }
    if (profitValue < root->profit)
    {
        InsertItemByProfit(root->leftBranch, itemID, profitValue, buyValue, sellValue, nameValue);
    }
    else
    {
        InsertItemByProfit(root->rightBranch, itemID, profitValue, buyValue, sellValue, nameValue);
    }
}
// Load Image Resources
//      Load the PNG image for each node in the Item Tree using a INORDER Traversal.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ItemTree::LoadImageResources(ItemNode *&node, HWND &hwnd)
{
    if (node)
    {
        LoadImageResources(node->leftBranch, hwnd);

        std::ifstream PNGIMAGE (node->imageAddress);
        if (!PNGIMAGE)     // Check if the image has been loaded.
        {
            // Download the image from the server in 4 steps.
            std::cout << "Downloading ID: " << node->ID << " ";
            // 1) Open the link for the item.
            std::string iconLink = "https://api.guildwars2.com/v2/items/";
            std::ostringstream IDstring;
            IDstring << node->ID;
            iconLink.append(IDstring.str());
            HINTERNET linkOpen = InternetOpen(iconLink.c_str(), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);           // For some reason the code stops execution here... PROBABLY Memory Leak
            if(!linkOpen)
            {
                InternetCloseHandle(linkOpen);
                std::cout << "failed to load :( - v2/items/ID 'linkopen' offline." << std::endl;
                PNGIMAGE.close();
                return;
            }
            HINTERNET openUrl = InternetOpenUrl(linkOpen, iconLink.c_str(), NULL, 0, INTERNET_FLAG_PRAGMA_NOCACHE|INTERNET_FLAG_KEEP_CONNECTION, 0);
            if(!openUrl)
            {
              InternetCloseHandle(openUrl);
              InternetCloseHandle(linkOpen);
              std::cout << "failed to load :( - v2/items/ID 'openUrl' offline" << std::endl;
              PNGIMAGE.close();
              return;
            }
            std::cout << ".";      // Step 1 completed, no errors.

            // 2) Get the icon link for downloading the image.
            char dataBuffer[1024 * 1];                      // 1 KB of buffer.
            memset(dataBuffer, 0, sizeof(dataBuffer));      // Zero the entire contents of the dataBuffer to prevent garbage from memmory.
            std::string dataReceived;
            DWORD NumberOfBytesRead = 0;
            while(InternetReadFile(openUrl, dataBuffer, sizeof(dataBuffer), &NumberOfBytesRead) && NumberOfBytesRead != 0)
            {
                dataReceived.append(dataBuffer);
            }
            int iconSubstringPosition = dataReceived.find("\"icon");
            std::string iconLinkAddress = dataReceived.substr(iconSubstringPosition + 8, ((dataReceived.find(".png") + 4) - iconSubstringPosition - 8));
            InternetCloseHandle(openUrl);
            InternetCloseHandle(linkOpen);
            std::cout << ".";      // Step 2 completed, no errors.

            // 3) Open another connection and download the image in .PNG
            HINTERNET iconImageLink = InternetOpen(iconLinkAddress.c_str(), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
            if(!iconImageLink)
            {
              InternetCloseHandle(iconImageLink);
              std::cout << "failed to load :( - Invalid ID." << std::endl;
              PNGIMAGE.close();
              return;
            }
            HINTERNET iconOpenUrl = InternetOpenUrl(iconImageLink, iconLinkAddress.c_str(), NULL, 0, INTERNET_FLAG_PRAGMA_NOCACHE|INTERNET_FLAG_KEEP_CONNECTION, 0);
            if(!iconOpenUrl)
            {
              InternetCloseHandle(iconOpenUrl);
              InternetCloseHandle(iconImageLink);
              std::cout << "failed to download :( - Image not found on the server." << std::endl;
              PNGIMAGE.close();
              return;
            }
            std::cout << ".";      // Step 3 completed, no errors.

            // 4) Open the file for output through binary.
            std::ofstream IMAGE (node->imageAddress, std::ios::out|std::ios::binary);
            char Data_Received;
            NumberOfBytesRead = 0;
            // Write on the file.
            while(InternetReadFile(iconOpenUrl, &Data_Received, 1, &NumberOfBytesRead))
            {
              if (NumberOfBytesRead != 1)
                 break;
              IMAGE.write((char*)&Data_Received, sizeof(char));
            }
            // Closes the file.
            IMAGE.close();
            InternetCloseHandle(iconOpenUrl);
            InternetCloseHandle(iconImageLink);
            std::cout << ". ";      // Step 4 completed, no errors.
            std::cout << "downloaded!" << std::endl;
        }
        PNGIMAGE.close();

        LoadImageResources(node->rightBranch, hwnd);
    }
}
// Set Name
//      Traverse the ID tree INORDER and sets the name of the ItemNode if the ID is found.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ItemTree::SetName(ItemNode *node, const int ID, const std::string itemName)
{
    if (node)
    {
        if (ID == node->ID)
        {
            node->name = itemName;
        }
        else if (ID < node->ID)
        {
            SetName(node->leftBranch, ID, itemName);
        }
        else
        {
            SetName(node->rightBranch, ID, itemName);
        }
    }
}
// Display Items
//      Traverse the Item Tree POSTORDER and display the name of each item.
//      Paints the image of each node on the screen, traversing the list POSTORDER.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ItemTree::DisplayItems(ItemNode *node, HWND hwnd, HDC hdc)
{
    if (node)
    {
        DisplayItems(node->rightBranch, hwnd, hdc);

        if ((node->STATIC_ItemOrder >= (screenPosition / 64)) && (node->STATIC_ItemPosition < 8))
        {
            // Inicialization of GDI+.
            Gdiplus::Graphics graphics(hdc);
            /// Writting Display.
            // Loading default "writting style".
            Gdiplus::SolidBrush brush(Gdiplus::Color(255, 0, 0, 0));
            Gdiplus::FontFamily fontFamily(L"Times New Roman");
            Gdiplus::Font font(&fontFamily, 20, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
            Gdiplus::PointF pointF;             // Printing position.
            Gdiplus::StringFormat stringFormat; // Object for setting text alignment.
            stringFormat.SetAlignment(Gdiplus::StringAlignmentFar);  // Strings will be written from the right to left.

            // Name
            pointF.X = 70.0f;
            pointF.Y = (node->STATIC_ItemPosition * 64) + ITEM_DISPLAY_OFFSET;
            // Conversion from String to WideString for Gdigraphics:: call.
            std::wstring tempNameWString(node->name.length(), L' ');
            std::copy (node->name.begin(), node->name.end(), tempNameWString.begin());
            graphics.DrawString(tempNameWString.c_str(), -1, &font, pointF, &brush);    // Names are written from left to right, so no format is passed to the function.
            // Buy Price
            pointF.X = 600.0f;
            std::wostringstream tempBuy;
            tempBuy << node->buyPrice;
            std::wstring tempBuyWString(tempBuy.str());
            if (tempBuyWString.length() >= 5)
            {
                tempBuyWString.insert(tempBuyWString.length()-4, L"    "); // Insert 4 empty spaces on the string, so the coin icon may fit in between (Gold Coin).
            }
            if (tempBuyWString.length() >= 3)
            {
                tempBuyWString.insert(tempBuyWString.length()-2, L"    "); // Insert 4 empty spaces on the string, so the coin icon may fit in between (Silver Coin).
            }
            graphics.DrawString(tempBuyWString.c_str(), -1, &font, pointF, &stringFormat, &brush);
            // Sell Price
            pointF.X = 750.0f;
            std::wostringstream tempSell;
            tempSell << node->sellPrice;
            std::wstring tempSellWString(tempSell.str());
            if (tempSellWString.length() >= 5)
            {
                tempSellWString.insert(tempSellWString.length()-4, L"    "); // Insert 4 empty spaces on the string, so the coin icon may fit in between (Gold Coin).
            }
            if (tempSellWString.length() >= 3)
            {
                tempSellWString.insert(tempSellWString.length()-2, L"    "); // Insert 4 empty spaces on the string, so the coin icon may fit in between (Silver Coin).
            }
            graphics.DrawString(tempSellWString.c_str(), -1, &font, pointF, &stringFormat, &brush);
            // Profit Value
            pointF.X = 900.0f;
            std::wostringstream tempProfit;
            tempProfit << node->profit;
            std::wstring tempProfitWString(tempProfit.str());
            if (tempProfitWString.length() >= 5)
            {
                tempProfitWString.insert(tempProfitWString.length()-4, L"    "); // Insert 4 empty spaces on the string, so the coin icon may fit in between (Gold Coin).
            }
            if (tempProfitWString.length() >= 3)
            {
                tempProfitWString.insert(tempProfitWString.length()-2, L"    "); // Insert 4 empty spaces on the string, so the coin icon may fit in between (Silver Coin).
            }
            graphics.DrawString(tempProfitWString.c_str(), -1, &font, pointF, &stringFormat, &brush);
            // Profit/Cost Ratio
            pointF.X = 1000.0f;
            int pcRatio;
            if (node->buyPrice != 0)
            {
                pcRatio = (node->profit * 100)/node->buyPrice;
            }
            else
            {
                pcRatio = node->profit * 100;
            }

            std::wostringstream tempRatio;
            tempRatio << pcRatio;
            std::wstring tempRatioWString(tempRatio.str());
            tempRatioWString.append(L"%");
            graphics.DrawString(tempRatioWString.c_str(), -1, &font, pointF, &stringFormat, &brush);

            /// Images Display.
            // Coins.
            std::string tempGoldString("Images\\Gold_Coin.png");
            std::wstring tempGoldWString(tempGoldString.length(), L' ');
            std::copy (tempGoldString.begin(), tempGoldString.end(), tempGoldWString.begin());
            Gdiplus::Image *itemIcon= new Gdiplus::Image(tempGoldWString.c_str());
            graphics.DrawImage(itemIcon, 517, ((node->STATIC_ItemPosition * 64) + ITEM_DISPLAY_OFFSET));
            graphics.DrawImage(itemIcon, 667, ((node->STATIC_ItemPosition * 64) + ITEM_DISPLAY_OFFSET));
            graphics.DrawImage(itemIcon, 817, ((node->STATIC_ItemPosition * 64) + ITEM_DISPLAY_OFFSET));
            delete itemIcon;
            std::string tempSilverString("Images\\Silver_Coin.png");
            std::wstring tempSilverWString(tempSilverString.length(), L' ');
            std::copy (tempSilverString.begin(), tempSilverString.end(), tempSilverWString.begin());
            itemIcon= new Gdiplus::Image(tempSilverWString.c_str());
            graphics.DrawImage(itemIcon, 557, ((node->STATIC_ItemPosition * 64) + ITEM_DISPLAY_OFFSET));
            graphics.DrawImage(itemIcon, 707, ((node->STATIC_ItemPosition * 64) + ITEM_DISPLAY_OFFSET));
            graphics.DrawImage(itemIcon, 857, ((node->STATIC_ItemPosition * 64) + ITEM_DISPLAY_OFFSET));
            delete itemIcon;
            std::string tempCopperString("Images\\Copper_Coin.png");
            std::wstring tempCopperWString(tempCopperString.length(), L' ');
            std::copy (tempCopperString.begin(), tempCopperString.end(), tempCopperWString.begin());
            itemIcon= new Gdiplus::Image(tempCopperWString.c_str());
            graphics.DrawImage(itemIcon, 598, ((node->STATIC_ItemPosition * 64) + ITEM_DISPLAY_OFFSET));
            graphics.DrawImage(itemIcon, 748, ((node->STATIC_ItemPosition * 64) + ITEM_DISPLAY_OFFSET));
            graphics.DrawImage(itemIcon, 898, ((node->STATIC_ItemPosition * 64) + ITEM_DISPLAY_OFFSET));
            delete itemIcon;

            // Item Images.
            // GDI+::Image works with Wide String, so convertions from std::string are ncessary.
            std::wstring tempWString(node->imageAddress.length(), L' ');
            std::copy (node->imageAddress.begin(), node->imageAddress.end(), tempWString.begin());
            //Gdiplus::Image *itemIcon= new Gdiplus::Image(tempWString.c_str());
            itemIcon= new Gdiplus::Image(tempWString.c_str());
            // Unlike the previous images, the item icon needs to "align" to the item name, so it gets a different offset.
            // This offset is directly inverse proporcional with the image scling, if the image is scaled to HALF size, the offset is DOUBLED.
            int imageOffsetPosition = 32;
            if (itemIcon->GetHeight() > 128)        // If the image is over 128 bits, Scale to 25%
            {
                graphics.ScaleTransform(0.25f, 0.25f);
                imageOffsetPosition *=4;
            }
            else if (itemIcon->GetHeight() > 64)    // If the image is over 64 bits, Scale to 50%
            {
                graphics.ScaleTransform(0.5f, 0.5f);
                imageOffsetPosition *=2;
            }
            else if (itemIcon->GetHeight() < 64)    // If the image is under 64 bits, scale to 200%
            {
                graphics.ScaleTransform(2.0f, 2.0f);
                imageOffsetPosition /=2;
            }
            else
            {
                ;// Do nothing, the image needs no scaling.
            }
            graphics.DrawImage(itemIcon, 0, ((node->STATIC_ItemPosition * itemIcon->GetHeight()) + imageOffsetPosition));
            delete itemIcon;


            node->STATIC_ItemPosition++;
        }
        node->STATIC_ItemOrder++;


        DisplayItems(node->leftBranch, hwnd, hdc);
    }
}
//  Load Prices
//      Traverse the itemTree INORDER and load the price for each item from the server.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ItemTree::LoadPrices(ItemNode *node, const HWND hwnd, std::ifstream &BLACKLION_PRICES)
{
    if (node)
    {
        LoadPrices(node->leftBranch, hwnd, BLACKLION_PRICES);

        // Load the whole price line in the buffer.
        std::string pricesBuffer;
        std::getline(BLACKLION_PRICES, pricesBuffer);
        // Treating the data.
        // Divide the buffer em 2 substrings.
        int buySubstringPosition = pricesBuffer.find("\"buys");
        int sellSubstringPosition = pricesBuffer.find("\"sells");
        std::string buyString = pricesBuffer.substr(buySubstringPosition, (sellSubstringPosition - buySubstringPosition - 2));
        std::string sellString = pricesBuffer.substr(sellSubstringPosition, (pricesBuffer.length() - sellSubstringPosition));
        // Find the position of "unit_price", read the following price and store it in the itemTree node.
        std::string tempBuyPrice, tempSellPrice;
        tempBuyPrice = (buyString.substr(buyString.find("\"unit_price") + 13));
        tempSellPrice = (sellString.substr(sellString.find("\"unit_price") + 13));
        // Converts from std::string to integer, makes the calculation and convert it back to a string.
        int tax;
        node->buyPrice = atoi(tempBuyPrice.c_str());
        node->sellPrice = atoi(tempSellPrice.c_str());
        tax = (node->sellPrice * 15)/100;
        node->profit = node->sellPrice - tax - node->buyPrice;

        LoadPrices(node->rightBranch, hwnd, BLACKLION_PRICES);
    }
}
//  Get 200 IDS
//      Traverse the itemTree INORDER and load 200 ID's in the link for future price load. (199 in fact, due to the addition of a ",")
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ItemTree::Get200IDS(ItemNode *node, std::string &apiLink, const int startPosition, const int endPosition)
{
    if (node && node->STATIC_ItemsLoaded < endPosition)         // If the node exists AND the endPosition for my link have not been reached (prevents unecessary recursiveness).
    {
        Get200IDS(node->leftBranch, apiLink, startPosition, endPosition);
        if (node->STATIC_ItemsLoaded < endPosition)             // Prevents the recursive aspect of the function from spitting values out of range due to function calls.
        {
            if (node->STATIC_ItemsLoaded >= startPosition)      // Check to see if the node is inside the range of values to be loaded.
            {
                std::ostringstream IDstring;
                IDstring << node->ID;
                apiLink.append(IDstring.str());
                apiLink.append(",");
            }
            node->STATIC_ItemsLoaded++;
        }
        Get200IDS(node->rightBranch, apiLink, startPosition, endPosition);
    }
}
//  Find Profit Value
//      Traverse the itemTree INORDER and find all nodes that have the profit value higher or equal to the parameter.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ItemTree::FindProfitValue(ItemNode *node, const int desiredProfit, ItemTree &treeByProfit)
{
    if(node)
    {
        FindProfitValue(node->leftBranch, desiredProfit, treeByProfit);

        if (node->profit >= desiredProfit)  // Filter nodes with undesired profit
        {
            std::cout << node->ID <<": " << "\t" << node->profit << std::endl;         // Show that the profit is equal or over desired value.
            //treeByProfit.InsertItemByProfit(node->ID, node->profit);      // Add that node to the profitTree.
        }
        else
        {
            //std::cout << node->ID << ": " << "\t" << node->profit << std::endl;
        }

        FindProfitValue(node->rightBranch, desiredProfit, treeByProfit);
    }
}
//  Create Ordered Profit Tree
//      Traverse the itemTree INORDER and make an itemTree ordered by Profit values.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ItemTree::CreateOrderedProfitTree(ItemNode *node, ItemTree &emptyProfitTree)
{
    if(node)
    {
        CreateOrderedProfitTree(node->leftBranch, emptyProfitTree);

        emptyProfitTree.InsertItemByProfit(node->ID, node->profit);      // Add that node to the profitTree.

        CreateOrderedProfitTree(node->rightBranch, emptyProfitTree);
    }
}
//  Save To File
//      Traverse the itemTree INORDER and save all the items to a file.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ItemTree::SaveToFile(ItemNode *node, std::ofstream &FILE)
{
    if (node)
    {
        SaveToFile(node->leftBranch, FILE);
        FILE << node->ID << std::endl;
        SaveToFile(node->rightBranch, FILE);
    }
}
//  Get Item Node Attributes
//      Traverse the itemTree INORDER, locate a specific ID and return ID, Name, BuyPrice, SellPrice and Profit values.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ItemTree::GetItemNodeAttributes(ItemNode *node, const int IDvalue, int &buyValue, int &sellValue, int &profitValue, std::string &nameValue)
{
    if (node)
    {
        GetItemNodeAttributes(node->leftBranch, IDvalue, buyValue, sellValue, profitValue, nameValue);
        if (node->ID == IDvalue)
        {
            nameValue = node->name;
            buyValue = node->buyPrice;
            sellValue = node->sellPrice;
            profitValue = node->profit;
            return;
        }
        GetItemNodeAttributes(node->rightBranch, IDvalue, buyValue, sellValue, profitValue, nameValue);
    }
}
//  Create Search Tree
//      Traverse the itemTree INORDER, locate an item with a specific name and copy that node to the Search Tree.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ItemTree::CreateSearchTree(ItemNode *node, ItemTree &emptySearchTree, const std::string &itemName)
{
    if (node)
    {
        CreateSearchTree(node->leftBranch, emptySearchTree, itemName);
        if (node->name.find(itemName) == 4294967295)
        {
            ; // Do nothing
            // If itemName is NOT found inside node->name, then the return value equals to 4294967295.
        }
        else
        {
            emptySearchTree.InsertItem(node->ID, node->profit, node->buyPrice, node->sellPrice, node->name);
            // Print the item.
            //std::cout << node->ID << ": " << node->name << std::endl;
            //std::cout << node->buyPrice << "\t" << node->sellPrice << "\t" << node->profit << std::endl;
        }
        CreateSearchTree(node->rightBranch, emptySearchTree, itemName);
    }
}
