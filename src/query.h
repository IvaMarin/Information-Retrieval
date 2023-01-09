#include "index.h"

#include <stack>
#include <set>
#include <algorithm>
#include <cmath>

#define AND L'&'
#define OR L'|'
#define NOT L'!'
#define LEFT_BRACKET L'('
#define RIGHT_BRACKET L')'
#define QUOTE L'\"'
#define SLASH L'/'
#define SPACE L' '

struct TFIDF
{
    std::unordered_map<uint32_t, double> ranks;

    TFIDF(std::unordered_map<uint32_t, double> ranks) { this->ranks = ranks; }
    bool operator()(const uint32_t &a, const uint32_t &b) { return ranks[a] > ranks[b]; }
};

class Query
{
public:
    void GetIndex(std::string &inputFile);
    void ParseQueries(std::string &outputFile);
    void ParseQueriesFromFile(std::string &inputFile, std::string &outputFile);

private:
    Index index;
    std::stack<std::shared_ptr<std::vector<uint32_t>>> operands;
    std::stack<wchar_t> operations;

    const std::vector<uint32_t> &GetDocIndices(std::wstring &word);

    void ProcessingQuery(std::wstring &query);
    void ProcessingFuzzyQuery(std::wstring &query);
    void ProcessingQuote(std::wstring &quote);

    bool IsFuzzy(std::wstring &query);
    void Ranking(std::vector<uint32_t> &result, std::wstring &query);

    bool IsOperation(wchar_t op);
    uint32_t Priority(wchar_t op);

    void ExecuteOperation(wchar_t op);
    void Union();
    void Intersection();
    std::vector<uint32_t> IntersectionForQuote(std::wstring word1, std::wstring word2,
                                               std::vector<uint32_t> &l, std::vector<uint32_t> &r,
                                               size_t k);
    void Negation();
};