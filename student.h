#ifndef DATABASEDIMA__STUDENT_H_
#define DATABASEDIMA__STUDENT_H_

#include <string>
#include <stdexcept>
#include <iostream>

namespace university {
    class Student {
    private:
        std::string m_name{};
        long long m_group{};
        double m_rating{};
        std::string m_info{};
    public:
        Student()
        {
            m_group = -1;
            m_rating = -1;
        }
        Student(const std::string &name, long long group, double rating, const std::string &info = {})
            : m_name(name), m_group(group), m_rating(rating), m_info(info)
        {}

        Student(const Student &student) = default;

        Student &operator=(const Student &student) = default;

        bool operator==(const Student &student) const;

        void update(const std::string &name = {},
                    long long group = -1,
                    double rating = -1,
                    const std::string &info = {});

        std::string name() const
        {
            return m_name;
        }
        long long group() const
        {
            return m_group;
        }
        double rating() const
        {
            return m_rating;
        }
        std::string info() const
        {
            return m_info;
        }

        bool empty() const
        {
            return m_name.empty() && m_group == -1 && m_rating == -1 && m_info.empty();
        }

        void clear()
        {
            m_name.clear();
            m_group = -1;
            m_rating = -1;
            m_info.clear();
        }

        template <typename T>
        T key() const;

        std::ostream &output(std::ostream &out, const std::string &prefix) const;
    };
}

std::ostream &operator<<(std::ostream &out, const university::Student &student);

#endif //DATABASEDIMA__STUDENT_H_
