#ifndef DATABASEDIMA__DB_H_
#define DATABASEDIMA__DB_H_

#include <map>
#include <vector>
#include <list>
#include <set>
#include <iomanip>
#include <fstream>

#include "student.h"
#include "rbtree.h"
#include "criteria.h"
#include "selection.h"
#include "compare.h"

#define OK "OK"

#define SUCCESSFUL_INSERT "Successful insert"
#define SUCCESSFUL_DELETE "Successful delete: "
#define SUCCESSFUL_UPDATE "Successful update: "
#define SUCCESSFUL_SELECT "Successful select: "
#define SUCCESSFUL_RESELECT "Successful reselect: "
#define SUCCESSFUL_LOAD "Successful load from file: "
#define SUCCESSFUL_DUMP "Successful dump to file: "

#define NOTHING_TO_DELETE "No such students in database"
#define NOTHING_TO_UPDATE "No such students in database"
#define NOTHING_TO_PRINT "No students in selection"

#define CANT_OPEN_FILE "File can't be opened"
#define UNKNOWN_JSON_KEY "Unknown key: "

#define EMPTY_QUERY "Empty query"
#define UNKNOWN_QUERY "Unknown query: "

#define INSERT_FORMAT_ERROR "Insert format error: "
#define DELETE_FORMAT_ERROR "Delete format error: "
#define UPDATE_FORMAT_ERROR "Update format error: "
#define SELECT_FORMAT_ERROR "Select format error: "
#define RESELECT_FORMAT_ERROR "Reselect format error: "
#define PRINT_FORMAT_ERROR "Print format error: "
#define LOAD_FORMAT_ERROR "Load format error: "
#define DUMP_FORMAT_ERROR "Dump format error: "

namespace db {
    class Database {
    private:
        static const long BUF_SIZE = 4096;
        enum FieldType {
            NAME,
            GROUP,
            RATING
        };

        enum SortType {
            NONE,
            ASCENDING,
            DESCENDING
        };

        std::vector<university::Student> m_students{};
        std::vector<unsigned long long> m_free_blocks{};

        // store indexes of elements in main vector to avoid extra copies and support fast search
        std::unordered_map<long long, std::list<unsigned long long>> m_groups{};
        tree::RBTree<std::string, std::list<unsigned long long>> m_names{};
        tree::RBTree<double, std::list<unsigned long long>> m_ratings{};

        // store students, not indexes(!) to allow printing last selection even after changes in database
        std::map<int, selection::Selection> m_selections{};

        static std::vector<unsigned long long> m_intersect_sorted(const std::vector<unsigned long long> &first,
                                                                  const std::vector<unsigned long long> &second);

        std::vector<unsigned long long> m_filter(const criteria::Criteria &criteria);

        std::string m_insert(const university::Student &student);
        std::string m_delete(const criteria::Criteria &criteria);
        std::string m_update(const criteria::Criteria &criteria,
                             const std::string &new_name = {},
                             long long new_group = -1,
                             double new_rating = -1,
                             const std::string &new_info = {});
        std::string m_select(const criteria::Criteria &criteria, int fd);
        std::string m_reselect(const criteria::Criteria &criteria, int fd);

        // return vector of strings
        std::vector<std::string> m_print(unsigned cnt,
                                         const std::vector<FieldType> &fields,
                                         SortType type,
                                         const std::vector<std::string> &sort_keys,
                                         int fd) const;
        std::string m_from_file(const std::string &filename);
        std::string m_to_file(const std::string &filename) const;

        // return vector of strings
        std::vector<std::string> m_process_insert_query(const tokenizer::Tokenizer &tokens);
        std::vector<std::string> m_process_delete_query(const tokenizer::Tokenizer &tokens);
        std::vector<std::string> m_process_update_query(const tokenizer::Tokenizer &tokens);
        std::vector<std::string> m_process_select_query(const tokenizer::Tokenizer &tokens, int fd);
        std::vector<std::string> m_process_reselect_query(const tokenizer::Tokenizer &tokens, int fd);
        std::vector<std::string> m_process_print_query(const tokenizer::Tokenizer &tokens, int fd) const;
        std::vector<std::string> m_process_load_query(const tokenizer::Tokenizer &tokens);
        std::vector<std::string> m_process_dump_query(const tokenizer::Tokenizer &tokens) const;
    public:
        Database() = default;
        Database(const Database &db) = default;

        Database &operator=(const Database &db) = default;

        // return vector of strings
        std::vector<std::string> process_query(const std::string &query, int fd);

        void insert_client(int fd)
        {
            m_selections[fd] = {};
        }
        void delete_client(int fd)
        {
            m_selections.erase(fd);
        }

        static size_t buf_size()
        {
            return BUF_SIZE;
        }
    };
}

#endif //DATABASEDIMA__DB_H_
