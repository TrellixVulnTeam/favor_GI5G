#ifndef favor_types_include
#define favor_types_include

namespace favor {
    namespace worker{
        class AccountManager;
    }
    template <typename T> class DataLock;

    typedef std::string string;
    typedef worker::AccountManager AccountManager;
    template<typename T> using shared_ptr = std::shared_ptr<T>;
    template<typename T> using vector = std::vector<T>;
    template<typename T> using list = std::list<T>;
}

#endif