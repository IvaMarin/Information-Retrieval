#include "index.h"

void Index::Build(std::string &inputFile)
{
    std::cout << "Building index...\n";
    auto start = std::chrono::high_resolution_clock::now();

    std::wifstream input((inputFile).c_str());
    for (size_t id = 1; input; ++id)
    {
        if (GetDocInfo(input))
        {
            GetDocContents(id, input);
        }
    }
    docTotal = docIndex.size();
    input.close();

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Successfully built index in "
              << std::chrono::duration<double, std::milli>(end - start).count() << " ms\n";
}

bool Index::GetDocInfo(std::wifstream &input)
{
    uint32_t length;
    std::wstring line, url, title;
    Document doc;
    if (std::getline(input, line))
    {
        uint32_t url_start = line.find(L"url=");
        if (url_start == std::string::npos)
        {
            return false;
        }
        uint32_t title_start = line.find(L"title=");

        url_start += 4; // len("url=")
        length = title_start - url_start - 1;
        url = line.substr(url_start, length);

        title_start += 6; // len("title=")
        length = line.length() - title_start - 1;
        title = line.substr(title_start, length);

        doc.title = title;
        doc.url = url;
        docIndex.push_back(doc);

        return true;
    }
    else
    {
        return false;
    }
}

void Index::GetDocContents(uint32_t id, std::wifstream &input)
{
    std::wstring line, word;
    std::unordered_set<std::wstring> words;
    uint32_t wordCount = 0;
    while (std::getline(input, line))
    {
        if (line == L"</doc>")
        {
            break;
        }

        for (const auto &c : line)
        {
            if (isalnum(c, std::locale()))
            {
                word += towlower(c);
            }
            else if (word.length())
            {
                words.insert(word);
                wordCount++;

                const auto [it, success] = coordinateIndex.insert(
                    std::make_pair(word + L" " + std::to_wstring(id - 1), std::vector<uint32_t>()));
                it->second.push_back(wordCount);

                word.clear();
            }
        }

        if (word.length())
        {
            words.insert(word);
            wordCount++;

            const auto [it, success] = coordinateIndex.insert(
                std::make_pair(word + L" " + std::to_wstring(id - 1), std::vector<uint32_t>()));
            it->second.push_back(wordCount);

            word.clear();
        }
    }

    for (const auto &w : words)
    {
        const auto [it, success] = invertedIndex.insert(
            std::make_pair(w, std::vector<uint32_t>()));
        it->second.push_back(id - 1);
    }

    docIndex[id - 1].wordCount = wordCount;
}

void Index::Save(std::string &outputFile)
{
    std::cout << "Saving index...\n";
    auto start = std::chrono::high_resolution_clock::now();

    SaveIndex(outputFile);
    SaveInvertedIndex(outputFile);
    SaveCoordinateIndex(outputFile);

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Successfully saved index in "
              << std::chrono::duration<double, std::milli>(end - start).count() << " ms\n";
}

void Index::SaveIndex(std::string &outputFile)
{
    std::wofstream wFileOut(outputFile.c_str());
    if (wFileOut)
    {
        wFileOut << docIndex.size() << L"\n";
        for (const auto &doc : docIndex)
        {
            wFileOut << doc.title << L" " << doc.url << L" \"" << doc.wordCount << L"\"\n";
        }
    }
    wFileOut.close();
}

void Index::SaveInvertedIndex(std::string &outputFile)
{
    std::wofstream wFileOut((outputFile + "_inverted").c_str());
    if (wFileOut)
    {
        wFileOut << docTotal << L"\n";
        for (const auto &it : invertedIndex)
        {
            wFileOut << it.first << L" ";
            wFileOut << it.second.size() << L" ";
            for (size_t i = 0; i < it.second.size(); ++i)
            {
                wFileOut << it.second[i] << L" ";
            }
            wFileOut << L"\n";
        }
    }
    wFileOut.close();
}

void Index::SaveCoordinateIndex(std::string &outputFile)
{
    std::wofstream wFileOut((outputFile + "_coordinate").c_str());
    if (wFileOut)
    {
        for (const auto &it : coordinateIndex)
        {
            wFileOut << it.first << L" ";
            wFileOut << it.second.size() << L" ";
            for (size_t i = 0; i < it.second.size(); ++i)
            {
                wFileOut << it.second[i] - 1 << L" ";
            }
            wFileOut << L"\n";
        }
    }
    wFileOut.close();
}

void Index::Load(std::string &inputFile)
{
    std::cout << "Loading index...\n";
    auto start = std::chrono::high_resolution_clock::now();

    LoadIndex(inputFile);
    LoadInvertedIndex(inputFile);
    LoadCoordinateIndex(inputFile);

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Successfully loaded index in "
              << std::chrono::duration<double, std::milli>(end - start).count() << " ms\n";
}

void Index::LoadIndex(std::string &inputFile)
{
    std::wifstream wFileIn(inputFile.c_str());
    std::wstring docLine, size;
    uint32_t reserveSize;

    std::getline(wFileIn, size);
    std::wstringstream wSizeIn(size);
    wSizeIn >> reserveSize;

    Document doc;
    docIndex.reserve(reserveSize);

    size_t pos = 0;
    std::wstring delimiter = L"\" \"";
    while (std::getline(wFileIn, docLine))
    {
        pos = docLine.find(delimiter);
        doc.title = docLine.substr(0, pos + 1);
        docLine.erase(0, pos + delimiter.length() - 1);

        pos = docLine.find(delimiter);
        doc.url = docLine.substr(0, pos + 1);
        docLine.erase(0, pos + delimiter.length());

        std::wstringstream wWordCountIn(docLine.substr(0, docLine.length() - 1));
        wWordCountIn >> doc.wordCount;

        docIndex.push_back(doc);

        docLine.clear();
    }
    wFileIn.close();
}

void Index::LoadInvertedIndex(std::string &inputFile)
{
    std::wifstream wFileIn((inputFile + "_inverted").c_str());
    uint32_t size;
    std::wstring word;

    wFileIn >> docTotal;
    while (!wFileIn.eof())
    {
        std::vector<uint32_t> postings;
        wFileIn >> word;
        wFileIn >> size;

        uint32_t posting;
        postings.reserve(size);
        for (size_t i = 0; i < size; ++i)
        {
            wFileIn >> posting;
            postings.push_back(posting);
        }

        invertedIndex.insert(std::make_pair(word, postings));
        word.clear();
    }
    wFileIn.close();
}

void Index::LoadCoordinateIndex(std::string &inputFile)
{
    std::wifstream wFileIn((inputFile + "_coordinate").c_str());
    std::wstring word, docID;
    uint32_t size;
    while (!wFileIn.eof())
    {
        std::vector<uint32_t> coordinates;
        wFileIn >> word >> docID >> size;

        uint32_t coordinate;
        coordinates.reserve(size);
        for (size_t i = 0; i < size; ++i)
        {
            wFileIn >> coordinate;
            coordinates.push_back(coordinate);
        }

        coordinateIndex.insert(std::make_pair((word + L" " + docID), coordinates));
        word.clear();
    }
    wFileIn.close();
}