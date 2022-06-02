#include "tokenizer.h"

tokenizer::Tokenizer::Tokenizer(const std::string &str, const std::string &delimiters)
{
    std::string word{};
    bool quoted{};
    for (auto &c : str) {
        if (c == '\"') {
            quoted = !quoted;
        } else {
            if (!quoted && delimiters.find(c) != std::string::npos) {
                if (!word.empty()) {
                    m_words.push_back(word);
                    word = "";
                }
            } else {
                word.push_back(c);
            }
        }
    }
    if (!word.empty()) {
        m_words.push_back(word);
    }
}

tokenizer::Tokenizer::Tokenizer(const tokenizer_iterator &first, const tokenizer_iterator &last)
{
    if (first == last) {
        m_words.push_back(*first);
        return;
    }
    tokenizer_iterator iter = first;
    while (iter != last) {
        m_words.push_back(*iter);
        iter++;
    }
}

void tokenizer::Tokenizer::strip()
{
    for (auto &word: m_words) {
        auto begin = word.begin();
        while (begin != word.end()) {
            if (std::isspace(*begin)) {
                begin++;
            } else {
                break;
            }
        }
        if (begin == word.end()) {
            word = {};
            break;
        }
        auto end = word.rbegin();
        while (end.base() != begin) {
            if (std::isspace(*end)) {
                end++;
            } else {
                break;
            }
        }
        word = std::string(begin, end.base());
    }
    for (size_t i = 0; i < m_words.size(); ++i) {
        if (m_words[i].empty()) {
            auto iter = m_words.begin();
            std::advance(iter, i);
            m_words.erase(iter);
            i--;
        }
    }
}