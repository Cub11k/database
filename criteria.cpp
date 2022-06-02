#include "criteria.h"

std::string db::criteria::to_regex(const std::string &str)
{
    std::string res{};
    for (auto &c : str) {
        if (c == '*') {
            res.push_back('.');
        }
        res.push_back(c);
    }
    return res;
}

db::criteria::Criteria::Entry::Entry(const std::string &field, const std::string &value)
{
    if (field == "name") {
        m_field = NAME;
    } else if (field == "group") {
        m_field = GROUP;
    } else if (field == "rating") {
        m_field = RATING;
    }
    m_raw_value = value;
}

template <>
db::criteria::Criteria::TemplatedEntry<std::string>::TemplatedEntry(const Entry &entry)
    : Entry(entry)
{
    if (m_raw_value == "*") {
        m_type = ANY;
    } else if (m_raw_value.find('*') != std::string::npos) {
        m_type = REGEXP;
    } else {
        m_type = EQUAL;
    }
    m_left_value = m_raw_value;
}

template <typename T>
db::criteria::Criteria::TemplatedEntry<T>::TemplatedEntry(const Entry &entry)
    : Entry(entry)
{
    if (m_raw_value == "*") {
        m_type = ANY;
    } else {
        if (std::count(m_raw_value.begin(), m_raw_value.end(), '-') > 1) {
            throw std::invalid_argument("Value is incorrect: " + m_raw_value);
        }
        size_t minus = m_raw_value.find('-');
        size_t asterisk = m_raw_value.find('*');
        std::istringstream stream{};
        if (minus != std::string::npos) {
            if (asterisk == std::string::npos) {
                stream.clear();
                stream.seekg(0);
                stream.str(m_raw_value.substr(0, minus));
                if (!(stream >> m_left_value) || !stream.eof()) {
                    throw std::invalid_argument("Value is incorrect: " + m_raw_value);
                }
                stream.str(m_raw_value.substr(minus + 1));
                stream.clear();
                stream.seekg(0);
                if (!(stream >> m_right_value) || !stream.eof()) {
                    throw std::invalid_argument("Value is incorrect: " + m_raw_value);
                }
                m_type = RANGE;
            } else if (asterisk == 0 || asterisk == m_raw_value.size() - 1) {
                if (asterisk == minus - 1) {
                    stream.str(m_raw_value.substr(minus + 1));
                    stream.clear();
                    stream.seekg(0);
                    if (!(stream >> m_right_value) || !stream.eof()) {
                        throw std::invalid_argument("Value is incorrect: " + m_raw_value);
                    }
                    m_type = LESS_OR_EQUAL;
                } else if (asterisk == minus + 1) {
                    stream.str(m_raw_value.substr(0, minus));
                    stream.clear();
                    stream.seekg(0);
                    if (!(stream >> m_left_value) || !stream.eof()) {
                        throw std::invalid_argument("Value is incorrect: " + m_raw_value);
                    }
                    m_type = MORE_OR_EQUAL;
                } else {
                    throw std::invalid_argument("Value is incorrect: " + m_raw_value);
                }
            } else {
                throw std::invalid_argument("Value is incorrect: " + m_raw_value);
            }
        } else {
            stream.str(m_raw_value);
            stream.clear();
            stream.seekg(0);
            if (!(stream >> m_left_value) || !stream.eof()) {
                throw std::invalid_argument("Value is incorrect: " + m_raw_value);
            }
            m_type = EQUAL;
        }
    }
}

template <>
bool db::criteria::Criteria::TemplatedEntry<std::string>::check(const std::string &value) const
{
    switch (m_type) {
        case ANY:
            return true;
        case EQUAL:
            return value == m_left_value;
        case REGEXP:
            return std::regex_match(value, std::regex(to_regex(m_left_value)));
        default:
            return false;
    }
}

template <typename T>
bool db::criteria::Criteria::TemplatedEntry<T>::check(const std::string &value) const
{
    std::istringstream stream(value);
    T val{};
    stream >> val;
    switch (m_type) {
        case ANY:
            return true;
        case LESS_OR_EQUAL:
            return val <= m_right_value;
        case EQUAL:
            if (typeid(T) == typeid(double)) {
                return std::fabs(val - m_left_value) < std::numeric_limits<double>::epsilon();
            }
            return val == m_left_value;
        case MORE_OR_EQUAL:
            return val >= m_left_value;
        case RANGE:
            return val >= m_left_value && val <= m_right_value;
        default:
            return false;
    }
}

bool db::criteria::Criteria::m_valid_field(const std::string &field)
{
    return field == "name" || field == "group" || field == "rating";
}

db::criteria::Criteria::Criteria(const std::string &str)
{
    if (str.empty()) {
        throw std::invalid_argument("Invalid criteria: no criteria");
    }
    if (str == "*") {
        return;
    }
    m_logical_operands.push_back(OR); // first criteria is always considered as (false || criteria)
    tokenizer::Tokenizer criteria(str, " \n\t\r\v\f");
    for (size_t i = 0; i < criteria.size(); ++i) {
        if (i % 2 == 0) {
            size_t sign = criteria[i].find('=');
            if (sign == std::string::npos) {
                throw std::invalid_argument("Invalid criteria: " + criteria[i]);
            }
            std::string field = criteria[i].substr(0, sign);
            if (!m_valid_field(field)) {
                throw std::invalid_argument("Invalid field: " + field);
            }
            if (std::find(m_fields.begin(), m_fields.end(), field) == m_fields.end()) {
                m_fields.push_back(field);
            }
            tokenizer::Tokenizer values(criteria[i].substr(sign + 1), ",");
            for (auto &value : values) {
                Entry *entry{};
                if (field == "name") {
                    entry = new TemplatedEntry<std::string>({field, value});
                } else if (field == "group") {
                    try {
                        entry = new TemplatedEntry<long long>({field, value});
                    } catch (const std::invalid_argument &err) {
                        delete entry;
                        throw err;
                    }
                } else if (field == "rating") {
                    try {
                        entry = new TemplatedEntry<double>({field, value});
                    } catch (const std::invalid_argument &err) {
                        delete entry;
                        throw err;
                    }
                }
                m_entries.push_back(entry);
                if (value != values[values.size() - 1]) {
                    m_logical_operands.push_back(OR);
                }
            }
        } else {
            if (criteria[i] == "AND") {
                m_logical_operands.push_back(AND);
            } else if (criteria[i] == "OR") {
                m_logical_operands.push_back(OR);
            } else {
                throw std::invalid_argument("Unknown logical operand: " + criteria[i]);
            }
        }
    }
}

db::criteria::Criteria::Criteria(const tokenizer::Tokenizer &tokens)
{
    if (tokens.size() == 0) {
        throw std::invalid_argument("Invalid criteria: no criteria");
    }
    if (tokens.size() == 1 && tokens[0] == "*") {
        return;
    }
    m_logical_operands.push_back(OR); // first criteria is always considered as (false || criteria)
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (i % 2 == 0) {
            size_t sign = tokens[i].find('=');
            if (sign == std::string::npos) {
                throw std::invalid_argument("Invalid criteria: " + tokens[i]);
            }
            std::string field = tokens[i].substr(0, sign);
            if (!m_valid_field(field)) {
                throw std::invalid_argument("Invalid field: " + field);
            }
            if (std::find(m_fields.begin(), m_fields.end(), field) == m_fields.end()) {
                m_fields.push_back(field);
            }
            tokenizer::Tokenizer values(tokens[i].substr(sign + 1), ",");
            if (values.size() == 0) {
                throw std::invalid_argument("Invalid value: no value");
            }
            for (auto &value : values) {
                Entry *entry{};
                if (field == "name") {
                    entry = new TemplatedEntry<std::string>({field, value});
                } else if (field == "group") {
                    try {
                        entry = new TemplatedEntry<long long>({field, value});
                    } catch (const std::invalid_argument &err) {
                        delete entry;
                        throw err;
                    }
                } else if (field == "rating") {
                    try {
                        entry = new TemplatedEntry<double>({field, value});
                    } catch (const std::invalid_argument &err) {
                        delete entry;
                        throw err;
                    }
                }
                m_entries.push_back(entry);
                if (value != values[values.size() - 1]) {
                    m_logical_operands.push_back(OR);
                }
            }
        } else {
            if (tokens[i] == "AND") {
                m_logical_operands.push_back(AND);
            } else if (tokens[i] == "OR") {
                m_logical_operands.push_back(OR);
            } else {
                throw std::invalid_argument("Unknown logical operand: " + tokens[i]);
            }
        }
    }
}

db::criteria::Criteria::~Criteria()
{
    for (auto &entry : m_entries) {
        delete entry;
    }
}

bool db::criteria::Criteria::check(const std::string &name) const
{
    if (m_entries.empty()) {
        return true;
    }
    bool flag{};
    size_t start{};
    size_t cnt{};
    for (auto &m_entry : m_entries) {
        start++;
        if (m_entry->field() == NAME) {
            cnt++;
            flag = m_entry->check(name);
            break;
        }
    }
    for (size_t i = start; i < m_entries.size(); ++i) {
        if (m_entries[i]->field() == NAME) {
            cnt++;
            switch (m_logical_operands[i]) {
                case AND:
                    flag = flag && m_entries[i]->check(name);
                    break;
                case OR:
                    flag = flag || m_entries[i]->check(name);
                    break;
            }
        }
    }
    return cnt == 0 || flag;
}

bool db::criteria::Criteria::check(long long group) const
{
    if (m_entries.empty()) {
        return true;
    }
    bool flag{};
    size_t start{};
    size_t cnt{};
    for (auto m_entry : m_entries) {
        start++;
        if (m_entry->field() == GROUP) {
            cnt++;
            flag = m_entry->check(std::to_string(group));
            break;
        }
    }
    for (size_t i = start; i < m_entries.size(); ++i) {
        if (m_entries[i]->field() == GROUP) {
            cnt++;
            switch (m_logical_operands[i]) {
                case AND:
                    flag = flag && m_entries[i]->check(std::to_string(group));
                    break;
                case OR:
                    flag = flag || m_entries[i]->check(std::to_string(group));
                    break;
            }
        }
    }
    return cnt == 0 || flag;
}

bool db::criteria::Criteria::check(double rating) const
{
    if (m_entries.empty()) {
        return true;
    }
    bool flag{};
    size_t start{};
    size_t cnt{};
    for (auto &m_entry : m_entries) {
        start++;
        if (m_entry->field() == RATING) {
            cnt++;
            flag = m_entry->check(std::to_string(rating));
            break;
        }
    }
    for (size_t i = start; i < m_entries.size(); ++i) {
        if (m_entries[i]->field() == RATING) {
            cnt++;
            switch (m_logical_operands[i]) {
                case AND:
                    flag = flag && m_entries[i]->check(std::to_string(rating));
                    break;
                case OR:
                    flag = flag || m_entries[i]->check(std::to_string(rating));
                    break;
            }
        }
    }
    return cnt == 0 || flag;
}

bool db::criteria::Criteria::check(const university::Student &student) const
{
    if (m_entries.empty()) {
        return true;
    }
    bool flag{};
    for (size_t i = 0; i < m_entries.size(); ++i) {
        switch (m_logical_operands[i]) {
            case AND:
                switch (m_entries[i]->field()) {
                    case NAME:
                        flag = flag && m_entries[i]->check(student.name());
                        break;
                    case GROUP:
                        flag = flag && m_entries[i]->check(std::to_string(student.group()));
                        break;
                    case RATING:
                        flag = flag && m_entries[i]->check(std::to_string(student.rating()));
                        break;
                }
                break;
            case OR:
                switch (m_entries[i]->field()) {
                    case NAME:
                        flag = flag || m_entries[i]->check(student.name());
                        break;
                    case GROUP:
                        flag = flag || m_entries[i]->check(std::to_string(student.group()));
                        break;
                    case RATING:
                        flag = flag || m_entries[i]->check(std::to_string(student.rating()));
                        break;
                }
                break;
        }
    }
    return flag;
}