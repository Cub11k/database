#include "db.h"

std::vector<unsigned long long> db::Database::m_intersect_sorted(const std::vector<unsigned long long> &first,
                                                                 const std::vector<unsigned long long> &second)
{
    std::vector<unsigned long long> intersection{};
    size_t i = 0;
    size_t j = 0;
    while (i < first.size() || j < second.size()) {
        if (first[i] == second[j]) {
            intersection.push_back(first[i]);
            if (i < first.size() - 1) {
                i++;
            } else {
                break;
            }
            if (j < second.size() - 1) {
                j++;
            } else {
                break;
            }
        } else if (first[i] < second[j]) {
            if (i < first.size() - 1) {
                i++;
            } else {
                break;
            }
        } else {
            if (j < second.size() - 1) {
                j++;
            } else {
                break;
            }
        }
    }
    return intersection;
}

std::vector<unsigned long long> db::Database::m_filter(const criteria::Criteria &criteria)
{
    bool name_filtered{};
    std::vector<unsigned long long> by_name{};
    if (criteria.isPresent("name")) {
        name_filtered = true;
        for (auto &name : m_names.keys()) {
            if (criteria.check(name)) {
                for (auto &idx : m_names.search_without_copy(name)) {
                    by_name.push_back(idx);
                }
            }
        }
    }
    bool rating_filtered{};
    std::vector<unsigned long long> by_rating{};
    if (criteria.isPresent("rating")) {
        rating_filtered = true;
        for (auto &rating : m_ratings.keys()) {
            if (criteria.check(rating)) {
                for (auto &idx : m_ratings.search_without_copy(rating)) {
                    by_rating.push_back(idx);
                }
            }
        }
    }
    bool group_filtered{};
    std::vector<unsigned long long> by_group{};
    if (criteria.isPresent("group")) {
        group_filtered = true;
        for (auto &group : m_groups) {
            if (criteria.check(group.first)) {
                for (auto &idx : m_groups[group.first]) {
                    by_group.push_back(idx);
                }
            }
        }
    }
    if (!by_name.empty()) {
        std::sort(by_name.begin(), by_name.end());
    }
    if (!by_rating.empty()) {
        std::sort(by_rating.begin(), by_rating.end());
    }
    if (!by_group.empty()) {
        std::sort(by_group.begin(), by_group.end());
    }

    std::vector<unsigned long long> intersection{};

    if ((name_filtered && by_name.empty())
        || (rating_filtered && by_rating.empty())
        || (group_filtered && by_group.empty())) {
        return intersection;
    }

    int flags = by_name.empty() * 100 + by_group.empty() * 10 + by_rating.empty();
    switch (flags) {
        case 0:
            intersection = m_intersect_sorted(by_name, by_group);
            return m_intersect_sorted(intersection, by_rating);
        case 1:
            return m_intersect_sorted(by_name, by_group);
        case 10:
            return m_intersect_sorted(by_name, by_rating);
        case 11:
            return by_name;
        case 100:
            return m_intersect_sorted(by_rating, by_group);
        case 101:
            return by_group;
        case 110:
            return by_rating;
        case 111:
            for (unsigned long long i = 0; i < m_students.size(); ++i) {
                if (!m_students[i].empty()) {
                    intersection.push_back(i);
                }
            }
            return intersection;
        default:
            return intersection;
    }
}

std::string db::Database::m_insert(const university::Student &student)
{
    unsigned long long idx = m_students.size();
    if (!m_free_blocks.empty()) {
        idx = m_free_blocks.back();
        m_free_blocks.pop_back();
        m_students[idx] = student;
    } else {
        m_students.push_back(student);
    }
    m_groups[student.group()].push_back(idx);
    try {
        m_names.search_without_copy(student.name()).push_back(idx);
    } catch (const std::invalid_argument &err) {
        if (std::string(err.what()) == KEY_NOT_FOUND) {
            m_names.insert(student.name(), {idx});
        }
    }
    try {
        m_ratings.search_without_copy(student.rating()).push_back(idx);
    } catch (const std::invalid_argument &err) {
        if (std::string(err.what()) == KEY_NOT_FOUND) {
            m_ratings.insert(student.rating(), {idx});
        }
    }
    return SUCCESSFUL_INSERT;
}

std::string db::Database::m_delete(const criteria::Criteria &criteria)
{
    size_t cnt = 0;
    for (auto &idx : m_filter(criteria)) {
        m_groups[m_students[idx].group()].remove(idx);
        m_names.search_without_copy(m_students[idx].name()).remove(idx);
        m_ratings.search_without_copy(m_students[idx].rating()).remove(idx);
        m_students[idx].clear();
        m_free_blocks.push_back(idx);
        cnt++;
    }
    if (cnt == 0) {
        return NOTHING_TO_DELETE;
    }
    return std::string(SUCCESSFUL_DELETE) + std::to_string(cnt);
}

std::string db::Database::m_update(const criteria::Criteria &criteria,
                                   const std::string &new_name,
                                   long long new_group,
                                   double new_rating,
                                   const std::string &new_info)
{
    size_t cnt = 0;
    for (auto &idx : m_filter(criteria)) {
        if (m_students[idx].group() != new_group) {
            // move from one group to another
            m_groups[m_students[idx].group()].remove(idx);
            m_groups[new_group].push_back(idx);
        }
        if (m_students[idx].name() != new_name) {
            // move from one node to another
            m_names.search_without_copy(m_students[idx].name()).remove(idx);
            m_names.search_without_copy(new_name).push_back(idx);
        }
        if (m_students[idx].rating() != new_rating) {
            // move from one node to another
            m_ratings.search_without_copy(m_students[idx].rating()).remove(idx);
            m_ratings.search_without_copy(new_rating).push_back(idx);
        }
        m_students[idx].update(new_name, new_group, new_rating, new_info);
        cnt++;
    }
    if (cnt == 0) {
        return NOTHING_TO_UPDATE;
    }
    return std::string(SUCCESSFUL_UPDATE) + std::to_string(cnt);
}

std::string db::Database::m_select(const criteria::Criteria &criteria, int fd)
{
    m_selections[fd].clear();
    for (auto &idx : m_filter(criteria)) {
        m_selections[fd].insert(m_students[idx]);
    }
    return SUCCESSFUL_SELECT + std::to_string(m_selections[fd].size());
}

std::string db::Database::m_reselect(const criteria::Criteria &criteria, int fd)
{
    selection::Selection tmp{};
    for (auto &student : m_selections[fd]) {
        if (criteria.check(student)) {
            tmp.insert(student);
        }
    }
    m_selections[fd] = tmp;
    return std::string(SUCCESSFUL_RESELECT) + std::to_string(m_selections[fd].size());
}

std::vector<std::string> db::Database::m_print(unsigned cnt,
                                               const std::vector<FieldType> &fields,
                                               SortType type,
                                               const std::vector<std::string> &sort_keys,
                                               int fd) const
{
    std::vector<university::Student> students = m_selections.find(fd)->second.get();
    if (students.empty()) {
        return {NOTHING_TO_PRINT};
    }
    switch (type) {
        case NONE:
            break;
        case ASCENDING:
            std::sort(students.begin(), students.end(), compare::Compare(sort_keys));
            break;
        case DESCENDING:
            std::sort(students.rbegin(), students.rend(), compare::Compare(sort_keys));
            break;
    }
    std::ostringstream stream{};
    unsigned short max_name = 2;
    unsigned short max_group = 3;
    unsigned short max_rating = 3;
    for (unsigned i = 0; i < cnt; ++i) {
        max_name = std::max(max_name, (unsigned short) (students[i].name().size() / 2 + 1));
        max_group = std::max(max_group, (unsigned short) (std::to_string(students[i].group()).size() / 2 + 1));
        max_rating = std::max(max_rating, (unsigned short) (std::to_string(students[i].rating()).size() / 2 + 1));
    }
    int width{};
    for (auto &field : fields) {
        switch (field) {
            case NAME:
                stream << std::setw(max_name) << " ";
                stream << "name";
                stream << std::setw(max_name) << " ";
                max_name *= 2;
                max_name += 4;
                width += max_name;
                break;
            case GROUP:
                stream << std::setw(max_group) << " ";
                stream << "group";
                stream << std::setw(max_group) << " ";
                max_group *= 2;
                max_group += 5;
                width += max_group;
                break;
            case RATING:
                stream << std::setw(max_rating) << " ";
                stream << "rating";
                stream << std::setw(max_rating) << " ";
                max_rating *= 2;
                max_rating += 6;
                width += max_rating;
                break;
        }
        stream << "|";
        width += 1;
    }
    stream << "\n";
    stream << std::setw(width) << std::setfill('-') << "-";
    stream << "\n";
    stream << std::setfill(' ');

    std::vector<std::string> result{};
    std::string buf{};
    for (unsigned i = 0; i < cnt; ++i) {
        std::ostringstream::pos_type pos{};
        auto stream_size = stream.tellp();
        while (stream_size - pos >= BUF_SIZE) {
            if (!buf.empty()) {
                result.push_back(buf + stream.str().substr(pos, BUF_SIZE - buf.size()));
                buf.clear();
            } else {
                result.push_back(stream.str().substr(pos, BUF_SIZE));
            }
            pos += BUF_SIZE;
        }
        if (pos > 0) {
            buf = stream.str().substr(pos, BUF_SIZE);
            pos = 0;
            stream.str({});
        }
        for (auto &field : fields) {
            switch (field) {
                case NAME:
                    stream << std::setw(max_name - 1) << students[i].name() << " ";
                    break;
                case GROUP:
                    stream << std::setw(max_group - 1) << students[i].group() << " ";
                    break;
                case RATING:
                    stream << std::setw(max_rating - 1) << students[i].rating() << " ";
                    break;
            }
            stream << "|";
        }
        stream << "\n";
    }
    std::ostringstream::pos_type pos{};
    auto stream_size = stream.tellp();
    if (!buf.empty()) {
        result.push_back(buf + stream.str().substr(pos, BUF_SIZE - buf.size()));
        buf.clear();
        pos += BUF_SIZE;
    }
    while (stream_size - pos >= BUF_SIZE) {
        if (!buf.empty()) {
            result.push_back(buf + stream.str().substr(pos, BUF_SIZE - buf.size()));
            buf.clear();
        } else {
            result.push_back(stream.str().substr(pos, BUF_SIZE));
        }
        pos += BUF_SIZE;
    }
    if (pos < stream_size) {
        result.push_back(stream.str().substr(pos, BUF_SIZE));
    }
    return result;
}

std::string db::Database::m_from_file(const std::string &filename)
{
    std::ifstream fin(filename);
    if (!fin.is_open()) {
        return CANT_OPEN_FILE;
    }
    std::string file_str{};
    std::getline(fin, file_str, '\0');
    size_t cnt = 0;
    auto iter = file_str.rbegin();
    while (std::isspace(*iter)) {
        iter++;
        cnt++;
    }
    file_str.resize(file_str.size() - cnt);
    if (!file_str.starts_with('{') || !file_str.ends_with('}')) {
        return LOAD_FORMAT_ERROR + std::string("beginning or the end");
    }
    tokenizer::Tokenizer tokens(file_str, "{}");
    tokens.strip();
    if (tokens.size() == 0) {
        return LOAD_FORMAT_ERROR + std::string("no json-serialized objects");
    }
    auto zero_token = tokenizer::Tokenizer{tokens[0], ":"};
    zero_token.strip();
    if (zero_token.size() < 2 || zero_token[0] != "students") {
        return LOAD_FORMAT_ERROR + std::string("\"students\" key");
    }
    zero_token = tokenizer::Tokenizer{zero_token[1], " \n\t\r\v\f"};
    if ((zero_token.size() == 1 && zero_token[0] != "[")
        || (zero_token.size() == 2 && zero_token[0] != "[" && zero_token[1] != "]")) {
        return LOAD_FORMAT_ERROR + std::string("beginning of students array");
    }

    std::vector<university::Student> students{};

    if (zero_token.size() == 1) {
        for (size_t i = 1; i < tokens.size() - 1; ++i) {
            if (tokens[i] != "," && i % 2 == 0) {
                return LOAD_FORMAT_ERROR + std::string("missing comma");
            }
            if (tokens[i] != ",") {
                tokenizer::Tokenizer comma_tokens(tokens[i], ",");
                comma_tokens.strip();
                university::Student student{};
                if (comma_tokens.size() > 4) {
                    return LOAD_FORMAT_ERROR + std::string("too much values for one student");
                }
                for (const auto &comma_token : comma_tokens) {
                    tokenizer::Tokenizer pair_tokens(comma_token, ":");
                    pair_tokens.strip();
                    if (pair_tokens[0] == "info") {
                        if (pair_tokens.size() == 2) {
                            student.update({}, -1, -1, pair_tokens[1]);
                        } else if (pair_tokens.size() != 1) {
                            return LOAD_FORMAT_ERROR + std::string("multiple \':\' in the object");
                        }
                    } else {
                        if (pair_tokens.size() == 1) {
                            return LOAD_FORMAT_ERROR + std::string("empty value in the object");
                        }
                        if (pair_tokens.size() != 2) {
                            return LOAD_FORMAT_ERROR + std::string("multiple \':\' in the object");
                        }
                        if (pair_tokens[0] == "name") {
                            student.update(pair_tokens[1]);
                        } else if (pair_tokens[0] == "group") {
                            std::istringstream stream(pair_tokens[1]);
                            long long group = -1;
                            if (!(stream >> group) || !stream.eof()) {
                                return LOAD_FORMAT_ERROR + std::string("group number is not valid");
                            }
                            student.update({}, group);
                        } else if (pair_tokens[0] == "rating") {
                            std::istringstream stream(pair_tokens[1]);
                            double rating = -1;
                            if (!(stream >> rating) || !stream.eof()) {
                                return LOAD_FORMAT_ERROR + std::string("rating is not valid");
                            }
                            student.update({}, -1, rating);
                        } else {
                            return UNKNOWN_JSON_KEY + pair_tokens[0];
                        }
                    }
                }
                if (student.name().empty() || student.group() == -1 || student.rating() == -1) {
                    return LOAD_FORMAT_ERROR + std::string("student is not full");
                }
                students.push_back(student);
            }
        }
    }
    m_groups.clear();
    m_names.clear();
    m_ratings.clear();
    m_students.clear();
    for (auto &student : students) {
        m_insert(student);
    }
    return SUCCESSFUL_LOAD + std::to_string(students.size());
}

std::string db::Database::m_to_file(const std::string &filename) const
{
    std::ofstream fout(filename, std::ios::trunc | std::ios::out);
    if (!fout.is_open()) {
        return CANT_OPEN_FILE;
    }
    fout << "{\n"
         << "\t\"students\": [\n";
    size_t cnt{};
    for (const auto &m_student : m_students) {
        if (!m_student.empty()) {
            if (cnt != 0) {
                fout << ",\n";
            }
            cnt++;
            m_student.output(fout, "\t\t");
        }
    }
    fout << "\n\t]"
         << "\n}";
    return SUCCESSFUL_DUMP + std::to_string(cnt);
}

std::vector<std::string> db::Database::process_query(const std::string &query, int fd)
{
    tokenizer::Tokenizer tokens(query, " \n\t\r\v\f");
    tokens.strip();
    if (tokens.size() == 0) {
        return {EMPTY_QUERY};
    }
    if (tokens[0] == "INSERT") {
        return m_process_insert_query(tokens);
    } else if (tokens[0] == "DELETE") {
        return m_process_delete_query(tokens);
    } else if (tokens[0] == "UPDATE") {
        return m_process_update_query(tokens);
    } else if (tokens[0] == "SELECT") {
        return m_process_select_query(tokens, fd);
    } else if (tokens[0] == "RESELECT") {
        return m_process_reselect_query(tokens, fd);
    } else if (tokens[0] == "PRINT") {
        return m_process_print_query(tokens, fd);
    } else if (tokens[0] == "LOAD") {
        return m_process_load_query(tokens);
    } else if (tokens[0] == "DUMP") {
        return m_process_dump_query(tokens);
    }
    return {UNKNOWN_QUERY + tokens[0]};
}