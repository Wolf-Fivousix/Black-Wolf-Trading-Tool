#ifndef ITEMTREE_H
#define ITEMTREE_H
#include <string>
#include <fstream>
#include <sstream>
extern int screenPosition;

class ItemTree
{
    private:
        // Declaration of Copy Constructor and Assign Operator to prevent the compiler of creating default versions for them. (Error's will be detected at Link time)
        ItemTree (ItemTree &obj);
        void operator= (const ItemTree &obj);

        class ItemNode
        {
            friend class ItemTree;
            int         ID;
            std::string name;
            std::string imageAddress;
            int         buyPrice;
            int         sellPrice;
            int         profit;
            ItemNode    *leftBranch;
            ItemNode    *rightBranch;
            static int  STATIC_ItemOrder;            // Holds the order in which the Images/Texts shall be displayed.
            static int  STATIC_ItemPosition;         // Tells which position of my Client Area the Images/Texts shall be displayed.
            static int  STATIC_ItemsLoaded;          // Counter for the price loading function.

            ItemNode(int itemID, int profitValue = 0, int buyValue = 0, int sellValue = 0, std::string nameValue = ""):
                ID(itemID),     // Variables are intialized rather than declared and assigned.
                name(nameValue),
                imageAddress(BuildImageAddress(itemID)),
                buyPrice(buyValue),
                sellPrice(sellValue),
                profit(profitValue),
                leftBranch(NULL),
                rightBranch(NULL)
            {
                // Empty body for the constructor.
            }
            std::string BuildImageAddress(int itemID)
            {
                std::string address = "";            // ""
                address.append("Images\\");          // "Images\\"
                std::ostringstream IDstring;
                IDstring << ID;
                address.append(IDstring.str());      // "Images\\ID"
                address.append(".png");              // "Images\\ID.png"
                return address;
            }
        };

        ItemNode *root;     // Pointer to the root of the Binary Tree.
        int      treeSize;
        // Helper member functions.
        void CreateOrderedProfitTree(ItemNode *node, ItemTree &emptyPRofitTree);
        void CreateSearchTree(ItemNode *node, ItemTree &emptySearchTree, const std::string &itemName);
        void DestroySubTree(ItemNode *&node);
        void DisplayItems(ItemNode *node, HWND hwnd, HDC hdc);
        void FindProfitValue(ItemNode *node, const int desiredProfit, ItemTree &treeByProfit);
        void Get200IDS(ItemNode *node, std::string &apiLink, const int startPosition, const int endPosition);
        void GetItemNodeAttributes(ItemNode *node, const int IDvalue, int &buyValue, int &sellValue, int &profitValue, std::string &nameValue);
        void InsertItem(ItemNode *&root, const int itemID, const int profitValue, const int buyValue, const int sellValue, const std::string &nameValue);
        void InsertItemByProfit(ItemNode *&root, const int itemID, const int profitValue, const int buyValue, const int sellValue, const std::string &nameValue);
        void LoadImageResources(ItemNode *&node, HWND &hwnd);
        void LoadPrices(ItemNode *node, const HWND hwnd,std::ifstream &BLACKLION_PRICES);
        void SaveToFile(ItemNode *node, std::ofstream &FILE);
        void SetName(ItemNode *node, const int ID, const std::string itemName);

    public:
        ItemTree()
            { root = NULL; treeSize = 0; }
        ~ItemTree()
            { DestroySubTree(root); }
        void CreateOrderedProfitTree(ItemTree &emptyProfitTree)
            { CreateOrderedProfitTree(root, emptyProfitTree); }
        void CreateSearchTree (ItemTree &emptySearchTree, const std::string &itemName)
            { CreateSearchTree (root, emptySearchTree, itemName); }
        void DisplayItems(HWND hwnd, HDC hdc)
            { DisplayItems(root, hwnd, hdc); }
        void FindProfitValue(const int desiredProfit, ItemTree &treeByProfit)
            { FindProfitValue(root, desiredProfit, treeByProfit); }
        void Get200IDS(std::string &apiLink, const int startPosition, const int endPosition)
            { Get200IDS(root, apiLink, startPosition, endPosition); }
        void GetItemNodeAttributes(const int IDvalue, int &buyValue, int &sellValue, int &profitValue, std::string &nameValue)
            { GetItemNodeAttributes(root, IDvalue, buyValue, sellValue, profitValue, nameValue); }
        inline int GetTreeSize()
            { return treeSize; }
        void InsertItem(const int itemID, const int profitValue = 0, const int buyValue = 0, const int sellValue = 0, const std::string &nameValue = "")
            { InsertItem(root, itemID, profitValue, buyValue, sellValue, nameValue); }
        void InsertItemByProfit(const int itemID, const int profitValue, const int buyValue = 0, const int sellValue = 0, const std::string &nameValue = "")
            { InsertItemByProfit(root, itemID, profitValue, buyValue, sellValue, nameValue); }
        void LoadImageResources(HWND &hwnd)
            { LoadImageResources(root, hwnd); }
        void LoadPrices(const HWND hwnd, std::ifstream &BLACKLION_PRICES)
            { LoadPrices(root, hwnd, BLACKLION_PRICES); }
        void SaveToFile(std::ofstream &FILE)
            { SaveToFile(root, FILE); }
        void SetName(const int ID, const std::string itemName)
            { SetName(root, ID, itemName); }
        void SetStaticToZero()
            { root->STATIC_ItemPosition = 0; root->STATIC_ItemOrder = 0; root->STATIC_ItemsLoaded = 0; }
};

#endif
