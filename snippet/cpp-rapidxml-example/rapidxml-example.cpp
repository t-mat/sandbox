// RapidXml : http://rapidxml.sourceforge.net/
#include <cstddef>
#include <cassert>
#define RAPIDXML_NO_STDLIB
#define RAPIDXML_NO_EXCEPTIONS
#include "rapidxml-1.13/rapidxml.hpp"

#include <stdio.h>
#include <string>
#include <vector>
#include <fstream>
#include <cstdint>

#if defined(RAPIDXML_NO_EXCEPTIONS)
void rapidxml::parse_error_handler(const char* what, void* where) {
    printf("Parse error(@%p): %s\n", where, what);
    std::abort();
}
#endif


void walk(const rapidxml::xml_node<>* node, int indent = 0) {
    const auto ind = std::string(indent * 4, ' ');
    printf("%s", ind.c_str());

    const rapidxml::node_type t = node->type();
    switch(t) {
    case rapidxml::node_element:
        {
            printf("<%.*s", node->name_size(), node->name());
            for(const rapidxml::xml_attribute<>* a = node->first_attribute()
                ; a
                ; a = a->next_attribute()
            ) {
                printf(" %.*s", a->name_size(), a->name());
                printf("='%.*s'", a->value_size(), a->value());
            }
            printf(">\n");

            for(const rapidxml::xml_node<>* n = node->first_node()
                ; n
                ; n = n->next_sibling()
            ) {
                walk(n, indent+1);
            }
            printf("%s</%.*s>\n", ind.c_str(), node->name_size(), node->name());
        }
        break;

    case rapidxml::node_data:
        printf("DATA:[%.*s]\n", node->value_size(), node->value());
        break;

    default:
        printf("NODE-TYPE:%d\n", t);
        break;
    }
}


void processXmlFile(const char* data) {
    enum {
        PARSE_FLAGS = rapidxml::parse_non_destructive
    };

    // NOTE : There is a `const_cast<>`, but `rapidxml::parse_non_destructive`
    //        guarantees `data` is not overwritten.
    rapidxml::xml_document<> xmlDoc;
    xmlDoc.parse<PARSE_FLAGS>(const_cast<char*>(data));
    walk(xmlDoc.first_node());
}


template<class String>
std::vector<char> loadFile(const String& filename) {
    std::vector<char> file;
    if(std::ifstream is { filename, is.binary | is.ate }) {
        file.resize(static_cast<size_t>(is.tellg()));
        is.seekg(0);
        is.read(file.data(), file.size());
        file.push_back(0);
    }
    return file;
}


int main(int argc, const char* argv[]) {
    const auto args = std::vector<std::string>(argv+1, argv+argc);
    for(const auto& arg : args) {
        const auto xmlFile = loadFile(arg);
        if(! xmlFile.empty()) {
            const auto tmp = xmlFile;
            processXmlFile(xmlFile.data());

            // check for const correctness
            if(memcmp(xmlFile.data(), tmp.data(), tmp.size()) != 0) {
                printf("ERROR: xmlFile is overwritten.\n");
            }
        }
    }
}
