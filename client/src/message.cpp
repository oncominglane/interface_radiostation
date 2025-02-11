#include <regex>

#include "message.h"
#include "screen.h"
#include <string_view>

void message(std::string &data, std::vector<std::string> *texts) {
    (*texts).clear();

    std::string ascii_data;
    for (auto &symbol : data)
        if (std::isprint(symbol))
            ascii_data += symbol;

    std::cout << "ASCII Data: `" << ascii_data << "`\n";
    std::string_view input = static_cast<std::string_view>(ascii_data);

    char delim = '^';
    for (auto found = input.find(delim); found != std::string_view::npos; found = input.find(delim)) {
        (*texts).emplace_back(input, 0, found);
        input.remove_prefix(found + 1);
    }

    if (not input.empty())
        (*texts).emplace_back(input);
}