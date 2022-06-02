#include "student.h"

template <>
std::string university::Student::key() const
{
    return m_name;
}
template <>
double university::Student::key() const
{
    return m_rating;
}
template <typename T>
T university::Student::key() const
{
    throw std::invalid_argument("Unexpected key type for red black tree");
}

std::ostream &university::Student::output(std::ostream &out, const std::string &prefix) const
{
    /*
     * {
     *      "name": "m_name",
     *      "group": m_group,
     *      "rating": m_rating,
     *      "info": "m_info"
     * }
    */
    out << prefix << "{\n";
    out << prefix << "\t\"name\": \"" << m_name << "\",\n";
    out << prefix << "\t\"group\": " << m_group << ",\n";
    out << prefix << "\t\"rating\": " << m_rating << ",\n";
    out << prefix << "\t\"info\": \"" << m_info << "\"\n";
    out << prefix << "}";
    return out;
}

void university::Student::update(const std::string &name,
                                 long long int group,
                                 double rating,
                                 const std::string &info)
{
    if (m_name != name && !name.empty()) {
        m_name = name;
    }
    if (m_group != group && group != -1) {
        m_group = group;
    }
    if (m_rating != rating && rating != -1) {
        m_rating = rating;
    }
    if (m_info != info && !info.empty()) {
        m_info = info;
    }
}
bool university::Student::operator==(const university::Student &student) const
{
    return m_name == student.m_name && m_group == student.m_group && m_rating == student.m_rating;
}

std::ostream &operator<<(std::ostream &out, const university::Student &student)
{
    return student.output(out, {});
}