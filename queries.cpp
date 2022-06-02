#include "queries.h"

std::vector<std::string> db::Database::m_process_insert_query(const tokenizer::Tokenizer &tokens)
{
    university::Student student{};
    for (size_t i = 1; i < tokens.size() - 1; ++i) {
        size_t pos = tokens[i].find('=');
        if (pos == std::string::npos) {
            return {INSERT_FORMAT_ERROR + std::string("invalid field")};
        }
        std::string field = tokens[i].substr(0, pos);
        if (field == "name") {
            if (!student.name().empty()) {
                return {INSERT_FORMAT_ERROR + std::string("multiple name")};
            }
            student.update(tokens[i].substr(pos + 1));
        } else if (field == "group") {
            if (student.group() != -1) {
                return {INSERT_FORMAT_ERROR + std::string("multiple group")};
            }
            std::istringstream stream(tokens[i].substr(pos + 1));
            long long group = -1;
            if (!(stream >> group) || !stream.eof()) {
                return {INSERT_FORMAT_ERROR + std::string("group value is not valid")};
            }
            student.update({}, group);
        } else if (field == "rating") {
            if (student.rating() != -1) {
                return {INSERT_FORMAT_ERROR + std::string("multiple rating")};
            }
            std::istringstream stream(tokens[i].substr(pos + 1));
            double rating = -1;
            if (!(stream >> rating) || !stream.eof()) {
                return {INSERT_FORMAT_ERROR + std::string("rating value is not valid")};
            }
            student.update({}, -1, rating);
        } else if (field == "info") {
            if (!student.info().empty()) {
                return {INSERT_FORMAT_ERROR + std::string("multiple info")};
            }
            student.update({}, -1, -1, tokens[i].substr(pos + 1));
        } else {
            return {INSERT_FORMAT_ERROR + std::string("invalid field")};
        }
    }
    if (student.name().empty() || student.group() == -1 || student.rating() == -1) {
        return {INSERT_FORMAT_ERROR + std::string("student not full")};
    }
    return {m_insert(student)};
}

std::vector<std::string> db::Database::m_process_delete_query(const tokenizer::Tokenizer &tokens)
{
    auto start = tokens.begin();
    start++;
    auto end = tokens.begin();
    std::advance(end, tokens.size() - 1);
    if (end == tokens.begin()) {
        return {DELETE_FORMAT_ERROR + std::string("not enough keys")};
    }
    try {
        return {m_delete(tokenizer::Tokenizer{start, end})};
    } catch (const std::invalid_argument &err) {
        return {std::string(DELETE_FORMAT_ERROR) + err.what()};
    }
}

std::vector<std::string> db::Database::m_process_update_query(const tokenizer::Tokenizer &tokens)
{
    // UPDATE {new_values} WHERE {criteria} END
    if (tokens.size() < 5) {
        return {UPDATE_FORMAT_ERROR + std::string("not enough keys")};
    }
    size_t where_pos = std::string::npos;
    for (size_t i = 1; i < tokens.size() - 1; ++i) {
        if (tokens[i] == "WHERE") {
            where_pos = i;
        }
    }
    if (where_pos == std::string::npos) {
        return {UPDATE_FORMAT_ERROR + std::string("no keyword WHERE")};
    }
    std::string new_name{};
    long long new_group = -1;
    double new_rating = -1;
    std::string new_info{};
    for (size_t i = 1; i < where_pos; ++i) {
        size_t sign = tokens[i].find('=');
        if (sign == std::string::npos) {
            return {UPDATE_FORMAT_ERROR + std::string("invalid field")};
        }
        if (sign == tokens[i].size() - 1) {
            return {UPDATE_FORMAT_ERROR + std::string("no value")};
        }
        std::string field = tokens[i].substr(0, sign);
        if (field == "name") {
            if (!new_name.empty()) {
                return {UPDATE_FORMAT_ERROR + std::string("multiple name")};
            }
            new_name = tokens[i].substr(sign + 1);
        } else if (field == "group") {
            if (new_group != -1) {
                return {UPDATE_FORMAT_ERROR + std::string("multiple group")};
            }
            std::istringstream stream(tokens[i].substr(sign + 1));
            long long group = -1;
            if (!(stream >> group) || !stream.eof()) {
                return {UPDATE_FORMAT_ERROR + std::string("group value is not valid")};
            }
            new_group = group;
        } else if (field == "rating") {
            if (new_rating != -1) {
                return {UPDATE_FORMAT_ERROR + std::string("multiple rating")};
            }
            std::istringstream stream(tokens[i].substr(sign + 1));
            double rating = -1;
            if (!(stream >> rating) || !stream.eof()) {
                return {UPDATE_FORMAT_ERROR + std::string("rating value is not valid")};
            }
            new_rating = rating;
        } else if (field == "info") {
            if (!new_info.empty()) {
                return {UPDATE_FORMAT_ERROR + std::string("multiple info")};
            }
            new_info = tokens[i].substr(sign + 1);
        } else {
            return {UPDATE_FORMAT_ERROR + std::string("invalid field")};
        }
    }
    if (new_name.empty() && new_group == -1 && new_rating == -1 && new_info.empty()) {
        return {UPDATE_FORMAT_ERROR + std::string("no new values")};
    }
    auto start = tokens.begin();
    std::advance(start, where_pos + 1);
    auto end = tokens.begin();
    std::advance(end, tokens.size() - 1);
    try {
        return {m_update(tokenizer::Tokenizer{start, end}, new_name, new_group, new_rating, new_info)};
    } catch (const std::invalid_argument &err) {
        return {std::string(UPDATE_FORMAT_ERROR) + err.what()};
    }
}

std::vector<std::string> db::Database::m_process_select_query(const tokenizer::Tokenizer &tokens, int fd)
{
    auto start = tokens.begin();
    start++;
    auto end = tokens.begin();
    std::advance(end, tokens.size() - 1);
    if (end == tokens.begin()) {
        return {SELECT_FORMAT_ERROR + std::string("not enough keys")};
    }
    try {
        return {m_select(tokenizer::Tokenizer{start, end}, fd)};
    } catch (const std::invalid_argument &err) {
        return {std::string(SELECT_FORMAT_ERROR) + err.what()};
    }
}

std::vector<std::string> db::Database::m_process_reselect_query(const tokenizer::Tokenizer &tokens, int fd)
{
    auto start = tokens.begin();
    start++;
    auto end = tokens.begin();
    std::advance(end, tokens.size() - 1);
    if (end == tokens.begin()) {
        return {RESELECT_FORMAT_ERROR + std::string("not enough keys")};
    }
    try {
        return {m_reselect(tokenizer::Tokenizer{start, end}, fd)};
    } catch (const std::invalid_argument &err) {
        return {std::string(RESELECT_FORMAT_ERROR) + err.what()};
    }
}

std::vector<std::string> db::Database::m_process_print_query(const tokenizer::Tokenizer &tokens, int fd) const
{
    if (tokens.size() < 3) {
        return {PRINT_FORMAT_ERROR + std::string("not enough keys")};
    }
    std::vector<FieldType> fields{};
    SortType type{};
    std::vector<std::string> sort_keys{};
    unsigned cnt = m_selections.find(fd)->second.size();
    size_t sort_pos = std::string::npos;
    for (size_t i = 1; i < tokens.size() - 1; ++i) {
        if (tokens[i] == "SORT") {
            sort_pos = i;
        }
    }
    size_t fields_start = 1;
    if (tokens[fields_start].starts_with("count=")) {
        std::istringstream stream(tokens[fields_start].substr(6));
        if (!(stream >> cnt) || !stream.eof()) {
            return {PRINT_FORMAT_ERROR + std::string("invalid count")};
        }
        if (cnt <= 0) {
            return {PRINT_FORMAT_ERROR + std::string("count must be positive")};
        }
        fields_start++;
    }
    if (tokens[fields_start] == "END") {
        return {PRINT_FORMAT_ERROR + std::string("not enough keys")};
    }
    if (tokens[fields_start] == "*") {
        fields.push_back(FieldType::NAME);
        fields.push_back(FieldType::GROUP);
        fields.push_back(FieldType::RATING);
        fields_start++;
    }
    size_t fields_end = sort_pos == std::string::npos ? tokens.size() - 1 : sort_pos;
    for (size_t i = fields_start; i < fields_end; ++i) {
        if (tokens[i] == "name") {
            if (std::find(fields.begin(), fields.end(), FieldType::NAME) == fields.end()) {
                fields.push_back(FieldType::NAME);
            } else {
                return {PRINT_FORMAT_ERROR + std::string("multiple name")};
            }
        } else if (tokens[i] == "group") {
            if (std::find(fields.begin(), fields.end(), FieldType::GROUP) == fields.end()) {
                fields.push_back(FieldType::GROUP);
            } else {
                return {PRINT_FORMAT_ERROR + std::string("multiple group")};
            }
        } else if (tokens[i] == "rating") {
            if (std::find(fields.begin(), fields.end(), FieldType::RATING) == fields.end()) {
                fields.push_back(FieldType::RATING);
            } else {
                return {PRINT_FORMAT_ERROR + std::string("multiple rating")};
            }
        } else if (tokens[i] == "info") {
            return {PRINT_FORMAT_ERROR
                        + std::string("info field has non-limited length and thus can't be printed in terminal")};
        } else {
            return {PRINT_FORMAT_ERROR + std::string("invalid field")};
        }
    }
    if (sort_pos != std::string::npos) {
        if (tokens[sort_pos + 1] == "NONE") {
            type = SortType::NONE;
        } else if (tokens[sort_pos + 1] == "ASC") {
            type = SortType::ASCENDING;
        } else if (tokens[sort_pos + 1] == "DESC") {
            type = SortType::DESCENDING;
        } else if (tokens[sort_pos + 1] == "END") {
            return {PRINT_FORMAT_ERROR + std::string("sort type not specified")};
        } else {
            return {PRINT_FORMAT_ERROR + std::string("invalid sort type")};
        }
        if (tokens[sort_pos + 2] == "END") {
            return {PRINT_FORMAT_ERROR + std::string("no sort keys specified")};
        }
        for (size_t i = sort_pos + 2; i < tokens.size() - 1; ++i) {
            if (tokens[i] == "name" || tokens[i] == "group" || tokens[i] == "rating") {
                if (std::find(sort_keys.begin(), sort_keys.end(), tokens[i]) != sort_keys.end()) {
                    return {PRINT_FORMAT_ERROR + std::string("multiple sort keys are not allowed")};
                }
                sort_keys.push_back(tokens[i]);
            } else {
                return {PRINT_FORMAT_ERROR + std::string("invalid sort key")};
            }
        }
    }
    return m_print(cnt, fields, type, sort_keys, fd);
}

std::vector<std::string> db::Database::m_process_load_query(const tokenizer::Tokenizer &tokens)
{
    if (tokens.size() < 3) {
        return {LOAD_FORMAT_ERROR + std::string("not enough keys in query")};
    }
    if (tokens.size() > 3) {
        return {LOAD_FORMAT_ERROR + std::string("too much keys in query")};
    }
    size_t pos = tokens[1].find('=');
    if (pos == std::string::npos) {
        return {LOAD_FORMAT_ERROR + std::string("file not specified")};
    }
    if (tokens[1].substr(0, pos) == "file") {
        return {m_from_file(tokens[1].substr(pos + 1))};
    } else {
        return {LOAD_FORMAT_ERROR + std::string("file not specified")};
    }
}

std::vector<std::string> db::Database::m_process_dump_query(const tokenizer::Tokenizer &tokens) const
{
    if (tokens.size() < 3) {
        return {DUMP_FORMAT_ERROR + std::string("not enough keys in query")};
    }
    if (tokens.size() > 3) {
        return {DUMP_FORMAT_ERROR + std::string("too much keys in query")};
    }
    size_t pos = tokens[1].find('=');
    if (pos == std::string::npos) {
        return {DUMP_FORMAT_ERROR + std::string("file not specified")};
    }
    if (tokens[1].substr(0, pos) == "file") {
        return {m_to_file(tokens[1].substr(pos + 1))};
    } else {
        return {DUMP_FORMAT_ERROR + std::string("file not specified")};
    }
}