#ifndef favor_types_include
#define favor_types_include

namespace favor {
    typedef std::string string;
    template<typename T> using shared_ptr = std::shared_ptr<T>;
    template<typename T> using vector = std::vector<T>;
    template<typename T> using list = std::list<T>;
}

#endif