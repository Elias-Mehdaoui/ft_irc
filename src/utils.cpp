#include "Client.hpp"

std::vector<std::string> ft_split(std::string str, std::string delimiter)
{
    std::vector<std::string> tokens;
    size_t pos;
    std::string token;

    while((pos = str.find(delimiter)) != std::string::npos)
    {
        token = str.substr(0, pos);
        tokens.push_back(token);
        str.erase(0, pos + 1);
    }
    tokens.push_back(str);

    return tokens;
}

std::string ft_tolower(std::string str)
{
    std::string res = "";

    for (int i = 0; str[i]; i++)
        res += std::tolower(str[i]);
    
    return res;
}
