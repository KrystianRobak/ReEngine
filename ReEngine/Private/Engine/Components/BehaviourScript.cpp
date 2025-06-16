#include "Engine/Components/BehaviourScript.h"

std::vector<std::string> SSplitString(const std::string& str, char delimiter)
{
    std::vector<std::string> parts;
    size_t start = 0;
    size_t end = str.find(delimiter);

    while (end != std::string::npos)
    {
        parts.push_back(str.substr(start, end - start));
        start = end + 1;
        end = str.find(delimiter, start);
    }

    parts.push_back(str.substr(start));
    return parts;
}