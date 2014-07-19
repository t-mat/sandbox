#include <iostream>
#include <unordered_map>
#include <string>

namespace MyUnorderedMapNs {
    using Key = std::string;
    using Value = int;

    struct Hash {
        std::size_t operator()(const Key& key) {
            return std::hash<Key>()(key);
        }
    };

    struct Equality {
        bool operator()(const Key& lhs, const Key& rhs) {
            return lhs == rhs;
        }
    };

    using MyUnorderedMap = std::unordered_map<Key, Value, Hash, Equality>;
}

using MyUnorderedMap = MyUnorderedMapNs::MyUnorderedMap;

int main() {
    auto umap = MyUnorderedMap {
        { "foo", 314 },
        { "bar", 1 },
    };

    umap["foo"] = 42;

    for(auto it = std::cbegin(umap); it != std::cend(umap); ++it) {
        std::cout << "[" << it->first << "] = [" << it->second << "]\n";
    }
}
