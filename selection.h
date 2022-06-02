#ifndef DATABASEDIMA__SELECTION_H_
#define DATABASEDIMA__SELECTION_H_

#include <vector>

#include "student.h"

namespace db::selection {
    class Selection {
    private:
        std::vector<university::Student> m_students{};
    public:
        Selection() = default;

        void insert(const university::Student &student)
        {
            m_students.push_back(student);
        }

        size_t size() const {
            return m_students.size();
        }

        auto begin() const {
            return m_students.begin();
        }

        auto end() const {
            return m_students.end();
        }

        void clear()
        {
            m_students.clear();
        }

        const std::vector<university::Student> &get() const
        {
            return m_students;
        }
    };
}

#endif //DATABASEDIMA__SELECTION_H_
