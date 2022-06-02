#ifndef DATABASEDIMA__COMPARE_H_
#define DATABASEDIMA__COMPARE_H_

#include <vector>
#include <string>
#include <stdexcept>

#include "student.h"

namespace db::compare {
    class Compare {
    private:
        enum SortKey {
            NAME,
            GROUP,
            RATING
        };

        std::vector<SortKey> m_keys{};
    public:
        Compare(const std::vector<std::string> &keys);

        bool operator()(const university::Student &a, const university::Student &b);
    };
}

#endif //DATABASEDIMA__COMPARE_H_
