#include "query.h"

void Query::GetIndex(std::string &inputFile)
{
    index.Load(inputFile);
}

void Query::ParseQueries(std::string &outputFile)
{
    std::wstring query;
    bool isFuzzy = false;
    while (std::getline(std::wcin, query))
    {
        if (!query.length())
        {
            break;
        }

        std::wofstream wFileOut(outputFile.c_str());

        isFuzzy = IsFuzzy(query);
        auto start = std::chrono::high_resolution_clock::now();
        if (isFuzzy)
        {
            ProcessingFuzzyQuery(query);
        }
        else
        {
            ProcessingQuery(query);
        }
        std::shared_ptr<std::vector<uint32_t>> result_ptr = operands.top();
        operands.pop();
        if (isFuzzy)
        {
            Ranking(*result_ptr, query);
        }
        auto end = std::chrono::high_resolution_clock::now();

        std::cout << "Found " << (*result_ptr).size() << " result(s) in "
                  << std::chrono::duration<double, std::milli>(end - start).count() << " ms\n";

        wFileOut << (*result_ptr).size() << L'\n';
        for (const auto &i : (*result_ptr))
        {
            wFileOut << index.docIndex[i].title << L' ' << index.docIndex[i].url << L'\n';
        }
        wFileOut.close();
    }
}

void Query::ParseQueriesFromFile(std::string &inputFile, std::string &outputFile)
{
    std::wifstream wFileIn(inputFile.c_str());
    std::wofstream wFileOut(outputFile.c_str());

    std::wstring query;
    bool isFuzzy = false;
    while (std::getline(wFileIn, query))
    {
        if (!query.length())
        {
            break;
        }

        isFuzzy = IsFuzzy(query);
        auto start = std::chrono::high_resolution_clock::now();
        if (isFuzzy)
        {
            ProcessingFuzzyQuery(query);
        }
        else
        {
            ProcessingQuery(query);
        }
        std::shared_ptr<std::vector<uint32_t>> result_ptr = operands.top();
        operands.pop();
        if (isFuzzy)
        {
            Ranking(*result_ptr, query);
        }
        auto end = std::chrono::high_resolution_clock::now();

        std::cout << "Found " << (*result_ptr).size() << " result(s) in "
                  << std::chrono::duration<double, std::milli>(end - start).count() << " ms\n";

        wFileOut << (*result_ptr).size() << L'\n';
        for (const auto &i : (*result_ptr))
        {
            wFileOut << index.docIndex[i].title << L' ' << index.docIndex[i].url << L'\n';
        }
    }
    wFileIn.close();
    wFileOut.close();
}

const std::vector<uint32_t> &Query::GetDocIndices(std::wstring &word)
{
    static const std::vector<uint32_t> empty(0);

    auto it = index.invertedIndex.find(word);
    if (it != index.invertedIndex.end())
    {
        return it->second;
    }
    else
    {
        return empty;
    }
}

void Query::ProcessingQuery(std::wstring &query)
{
    std::wstring word;
    std::wstring quote;
    for (size_t i = 0; i < query.length(); ++i)
    {
        wchar_t c = query[i];

        if (query.substr(i, 4) == L" && ")
        {
            c = AND;
            i += 3;
        }
        else if (query.substr(i, 4) == L" || ")
        {
            c = OR;
            i += 3;
        }
        else if (c == SPACE)
        {
            c = AND;
        }

        if (c == QUOTE)
        {
            do
            {
                i++;
                c = query[i];
                quote += towlower(c);
            } while (query[i + 1] != QUOTE);
            i++;
            ProcessingQuote(quote);
            quote.clear();
            continue;
        }

        if ((IsOperation(c) || c == LEFT_BRACKET || c == RIGHT_BRACKET) && word.length())
        {
            auto postings = std::make_shared<std::vector<uint32_t>>(GetDocIndices(word));
            operands.push(postings);
            word.clear();
        }

        if (c == LEFT_BRACKET)
        {
            operations.push(LEFT_BRACKET);
        }
        else if (c == RIGHT_BRACKET)
        {
            while (operations.top() != LEFT_BRACKET)
            {
                ExecuteOperation(operations.top());
                operations.pop();
            }
            operations.pop();
        }
        else if (IsOperation(c))
        {
            while (!operations.empty() && (((c != NOT) && (Priority(operations.top()) >= Priority(c))) ||
                                           ((c == NOT) && (Priority(operations.top()) > Priority(c)))))
            {
                ExecuteOperation(operations.top());
                operations.pop();
            }
            operations.push(c);
        }
        else
        {
            word += towlower(c);
        }
    }

    if (word.length())
    {
        auto postings = std::make_shared<std::vector<uint32_t>>(GetDocIndices(word));
        operands.push(postings);
    }

    while (!operations.empty())
    {
        wchar_t op = operations.top();
        ExecuteOperation(op);
        operations.pop();
    }
}

void Query::ProcessingFuzzyQuery(std::wstring &query)
{
    std::wstring word;
    for (size_t i = 0; i < query.length(); ++i)
    {
        if ((query[i] == SPACE) && word.length())
        {
            auto postings = std::make_shared<std::vector<uint32_t>>(GetDocIndices(word));
            operands.push(postings);
            word.clear();
        }

        if ((query[i] == SPACE))
        {
            while (!operations.empty())
            {
                Union();
                operations.pop();
            }
            operations.push(OR);
        }
        else
        {
            word += towlower(query[i]);
        }
    }

    if (word.length())
    {
        auto postings = std::make_shared<std::vector<uint32_t>>(GetDocIndices(word));
        operands.push(postings);
    }

    while (!operations.empty())
    {
        Union();
        operations.pop();
    }
}

void Query::ProcessingQuote(std::wstring &quote)
{
    std::wstring word = L"";
    std::wstring wordPrevious;
    std::stack<std::vector<uint32_t>> result;

    size_t i = 0;
    size_t k = 1;
    while (i <= quote.size())
    {
        if ((i == quote.size()) or (quote[i] == SPACE))
        {
            if ((word.size() != 0) and (result.size() == 0))
            {
                wordPrevious = word;
                result.push(GetDocIndices(word));
                word = L"";
            }
            if (word.size())
            {
                std::vector<uint32_t> postings1 = result.top();
                result.pop();
                std::vector<uint32_t> postings2 = GetDocIndices(word);

                result.push(IntersectionForQuote(wordPrevious, word, postings1, postings2, k));
                k = 1;
                wordPrevious = word;
                word = L"";
            }
        }
        else if (quote[i] == SLASH)
        {
            k = 0;
            i++;
            while ('0' <= quote[i] and quote[i] <= '9')
            {
                k *= 10;
                k += (size_t)(quote[i] - '0');
                i++;
            }
            i--;
        }
        else
        {
            word += quote[i];
        }
        i++;
    }

    auto result_ptr = std::make_shared<std::vector<uint32_t>>(
        result.size() == 1 ? result.top() : std::vector<uint32_t>());
    operands.push(result_ptr);
}

bool Query::IsFuzzy(std::wstring &query)
{
    for (size_t i = 0; i < query.size(); i++)
    {
        if (query[i] == AND or query[i] == OR or query[i] == NOT or
            query[i] == LEFT_BRACKET or query[i] == RIGHT_BRACKET or
            query[i] == QUOTE)
        {
            return false;
        }
    }
    return true;
}

void Query::Ranking(std::vector<uint32_t> &result, std::wstring &query)
{
    std::unordered_map<uint32_t, double> ranks;
    for (const auto &i : result)
    {
        ranks[i] = 0;
        std::wstringstream wQueryIn(query);
        std::wstring word;
        while (getline(wQueryIn, word, L' '))
        {
            // double N_d = static_cast<double>(index.docIndex[i].wordCount);
            double tf_d = static_cast<double>(index.coordinateIndex[word + L' ' + std::to_wstring(i)].size());
            double N = static_cast<double>(index.docTotal);
            double df_t = static_cast<double>(index.invertedIndex[word].size());
            // ranks[i] += (tf_d / N_d) * std::log10(N / df_t);
            if (tf_d > 0)
            {
                ranks[i] += (1 + std::log10(tf_d)) * std::log10(N / df_t);
            }
        }
    }
    std::sort(result.begin(), result.end(), TFIDF(ranks));
}

bool Query::IsOperation(wchar_t op)
{
    return (op == AND) || (op == OR) || (op == NOT);
}

uint32_t Query::Priority(wchar_t op)
{
    switch (op)
    {
    case OR:
        return 1;
    case AND:
        return 2;
    case NOT:
        return 3;
    default:
        return 0;
    }
}

void Query::ExecuteOperation(wchar_t op)
{
    switch (op)
    {
    case OR:
        Union();
        break;
    case AND:
        Intersection();
        break;
    case NOT:
        Negation();
        break;
    default:
        break;
    }
}

void Query::Union()
{
    std::vector<uint32_t> result;
    std::shared_ptr<std::vector<uint32_t>> postings1_ptr = operands.top();
    operands.pop();
    std::shared_ptr<std::vector<uint32_t>> postings2_ptr = operands.top();
    operands.pop();
    std::set_union((*postings1_ptr).begin(), (*postings1_ptr).end(),
                   (*postings2_ptr).begin(), (*postings2_ptr).end(),
                   std::back_inserter(result));
    auto result_ptr = std::make_shared<std::vector<uint32_t>>(result);
    operands.push(result_ptr);
}

void Query::Intersection()
{
    std::vector<uint32_t> result;
    std::shared_ptr<std::vector<uint32_t>> postings1_ptr = operands.top();
    operands.pop();
    std::shared_ptr<std::vector<uint32_t>> postings2_ptr = operands.top();
    operands.pop();
    std::set_intersection((*postings1_ptr).begin(), (*postings1_ptr).end(),
                          (*postings2_ptr).begin(), (*postings2_ptr).end(),
                          std::back_inserter(result));
    auto result_ptr = std::make_shared<std::vector<uint32_t>>(result);
    operands.push(result_ptr);
}

std::vector<uint32_t> Query::IntersectionForQuote(std::wstring word1, std::wstring word2,
                                                  std::vector<uint32_t> &l, std::vector<uint32_t> &r,
                                                  size_t k)
{
    std::set<uint32_t> result;
    size_t i = 0, j = 0;
    while (i < l.size() && j < r.size())
    {
        if (l[i] == r[j])
        {
            std::vector<uint32_t> coordinates1 = index.coordinateIndex[word1 + L' ' + std::to_wstring(l[i])];
            std::vector<uint32_t> coordinates2 = index.coordinateIndex[word2 + L' ' + std::to_wstring(r[j])];

            size_t ii = 0, jj = 0;
            while (ii < coordinates1.size())
            {
                while (jj < coordinates2.size())
                {
                    if ((coordinates2[jj] - coordinates1[ii]) > 0 and
                        (coordinates2[jj] - coordinates1[ii]) <= k)
                    {
                        result.insert(l[i]);
                    }
                    else if (coordinates2[jj] > coordinates1[ii])
                    {
                        break;
                    }
                    jj++;
                }
                ii++;
            }
            i++;
            j++;
        }
        else if (l[i] < r[j])
        {
            i++;
        }
        else
        {
            j++;
        }
    }

    return std::vector<uint32_t>(result.begin(), result.end());
}

void Query::Negation()
{
    std::vector<uint32_t> result;
    std::shared_ptr<std::vector<uint32_t>> postings_ptr = operands.top();
    operands.pop();
    size_t j = 0;
    for (uint32_t i = 0; i < index.docTotal; ++i)
    {
        if (j == (*postings_ptr).size() || i < (*postings_ptr)[j])
        {
            result.push_back(i);
        }
        else if (i == (*postings_ptr)[j])
        {
            ++j;
        }
    }
    auto result_ptr = std::make_shared<std::vector<uint32_t>>(result);
    operands.push(result_ptr);
}