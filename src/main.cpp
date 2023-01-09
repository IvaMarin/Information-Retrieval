#include "query.h"

void SetLocale()
{
    std::locale::global(std::locale("en_US.UTF-8"));
    std::wcin.imbue(std::locale());
    std::wcout.imbue(std::locale());
}

void Help()
{
    std::cout << "usage:" << std::endl;
    std::cout << "./search_engine index --input <input file> --output <index file>" << std::endl;
    std::cout << "./search_engine search --index <index file> --output <output file>" << std::endl;
    std::cout << "./search_engine search --index <index file> --input <input file> --output <output file>" << std::endl;
}

std::string GetFilename(char **begin, char **end, const std::string &flag)
{
    char **it = std::find(begin, end, flag);
    if (it != end && ++it != end)
    {
        return std::string(*it);
    }
    return 0;
}

bool FlagExists(char **begin, char **end, const std::string &flag)
{
    return std::find(begin, end, flag) != end;
}

int main(int argc, char *argv[])
{
    SetLocale();

    std::string inputFile, outputFile, indexFile;
    if (FlagExists(argv, argv + argc, "--input"))
        inputFile = GetFilename(argv, argv + argc, "--input");
    if (FlagExists(argv, argv + argc, "--output"))
    {
        outputFile = GetFilename(argv, argv + argc, "--output");
    }
    else
    {
        Help();
        return 0;
    }
    if (FlagExists(argv, argv + argc, "--index"))
        indexFile = GetFilename(argv, argv + argc, "--index");

    if (argc == 6 && std::string(argv[1]) == std::string("index"))
    {
        Index idx;
        idx.Build(inputFile);
        idx.Save(outputFile);
    }
    else if (argc == 6 && std::string(argv[1]) == std::string("search"))
    {
        Query queries;
        queries.GetIndex(indexFile);
        queries.ParseQueries(outputFile);
    }
    else if (argc == 8 && std::string(argv[1]) == std::string("search"))
    {
        Query queries;
        queries.GetIndex(indexFile);
        queries.ParseQueriesFromFile(inputFile, outputFile);
    }
    else
    {
        Help();
    }

    return 0;
}