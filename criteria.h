#ifndef DATABASEDIMA__CONDITION_H_
#define DATABASEDIMA__CONDITION_H_

#include <string>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <numeric>
#include <cmath>
#include <regex>

#include "student.h"
#include "tokenizer.h"

namespace db::criteria {
    std::string to_regex(const std::string &str);

    class Criteria {
    private:
        enum FieldType {
            NAME,
            GROUP,
            RATING
        };
        enum LogicalOperandType {
            OR,
            AND
        };
        class Entry {
        protected:
            FieldType m_field{};
            std::string m_raw_value{};
        public:
            Entry(const std::string &field, const std::string &value);
            Entry(const Entry &entry) = default;

            virtual ~Entry() = default;

            FieldType field() const
            {
                return m_field;
            }

            virtual bool check(const std::string &value) const
            {
                std::cout << "Check string not derived";
                return false;
            }
        };

        template <typename T>
        class TemplatedEntry : public Entry {
        private:
            enum RelationType {
                LESS_OR_EQUAL,
                EQUAL,
                MORE_OR_EQUAL,
                RANGE,
                REGEXP,
                ANY
            } m_type{};
            T m_left_value{};
            T m_right_value{};
        public:
            TemplatedEntry(const Entry &entry);
            TemplatedEntry(const TemplatedEntry &entry) = default;

            ~TemplatedEntry() override = default;

            bool check(const std::string &value) const override;
        };

        std::vector<std::string> m_fields{};
        std::vector<Entry *> m_entries{};
        std::vector<LogicalOperandType> m_logical_operands{};

        static bool m_valid_field(const std::string &field);
    public:
        Criteria(const std::string &str);
        Criteria(const tokenizer::Tokenizer &tokens);
        Criteria(const Criteria &cond) = default;

        ~Criteria();

        Criteria &operator=(const Criteria &cond) = default;

        bool isPresent(const std::string &field) const
        {
            return std::find(m_fields.begin(), m_fields.end(), field) != m_fields.end();
        }

        bool check(const std::string &name) const;

        bool check(long long group) const;

        bool check(double rating) const;

        bool check(const university::Student &student) const;

        bool empty() const
        {
            return m_entries.empty();
        }
    };
}

#endif //DATABASEDIMA__CONDITION_H_
