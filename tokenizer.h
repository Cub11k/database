#ifndef DATABASEDIMA__TOKENIZER_H_
#define DATABASEDIMA__TOKENIZER_H_

#include <iostream>
#include <string>
#include <vector>

namespace tokenizer {
    class Tokenizer {
    private:
        typedef __gnu_cxx::__normal_iterator<const std::basic_string<char> *, std::vector<std::basic_string<char>>>
            tokenizer_iterator;
        std::vector<std::string> m_words{};
    public:
        Tokenizer(const std::string &str, const std::string &delimiters);
        Tokenizer(const tokenizer_iterator &first, const tokenizer_iterator &last);

        const std::string &operator[](size_t idx) const
        {
            return m_words[idx];
        }

        size_t size() const
        {
            return m_words.size();
        }

        auto begin() const
        {
            return m_words.begin();
        }

        auto end() const
        {
            return m_words.end();
        }

        void strip();
    };
}

#endif //DATABASEDIMA__TOKENIZER_H_
