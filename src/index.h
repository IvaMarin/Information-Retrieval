#include <iostream>
#include <fstream>
#include <sstream>
#include <locale>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <chrono>

struct Document
{
    std::wstring title;
    std::wstring url;
    uint32_t wordCount;
};

class Index
{
public:
    void Build(std::string &inputFile);

    void Save(std::string &outputFile);
    void SaveIndex(std::string &outputFile);
    void SaveInvertedIndex(std::string &outputFile);
    void SaveCoordinateIndex(std::string &outputFile);

    void Load(std::string &inputFile);
    void LoadIndex(std::string &inputFile);
    void LoadInvertedIndex(std::string &inputFile);
    void LoadCoordinateIndex(std::string &inputFile);

    friend class Query;

private:
    uint32_t docTotal;
    std::vector<Document> docIndex;
    std::unordered_map<std::wstring, std::vector<uint32_t>> invertedIndex;
    std::unordered_map<std::wstring, std::vector<uint32_t>> coordinateIndex;

    bool GetDocInfo(std::wifstream &input);
    void GetDocContents(uint32_t id, std::wifstream &input);
};