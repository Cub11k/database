#include "compare.h"

db::compare::Compare::Compare(const std::vector<std::string> &keys)
{
    for (auto &key : keys) {
        if (key == "name") {
            m_keys.push_back(NAME);
        } else if (key == "group") {
            m_keys.push_back(GROUP);
        } else if (key == "rating") {
            m_keys.push_back(RATING);
        } else {
            throw std::invalid_argument("Unknown key");
        }
    }
}

bool db::compare::Compare::operator()(const university::Student &a, const university::Student &b)
{
    for (auto &key : m_keys) {
        switch (key) {
            case NAME:
                if (a.name() < b.name()) {
                    return true;
                }
                if (a.name() > b.name()) {
                    return false;
                }
                break;
            case GROUP:
                if (a.group() < b.group()) {
                    return true;
                }
                if (a.group() > b.group()) {
                    return false;
                }
                break;
            case RATING:
                if (a.rating() < b.rating()) {
                    return true;
                }
                if (a.rating() > b.rating()) {
                    return false;
                }
                break;
        }
    }
    return false;
}