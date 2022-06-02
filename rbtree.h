#ifndef DATABASEDIMA__RBTREE_H_
#define DATABASEDIMA__RBTREE_H_

#include <list>
#include <vector>

#include "student.h"

#define KEY_NOT_FOUND "Key not found in red black tree"

namespace tree {
    template <typename T, typename V>
    class RBTree {
    private:
        struct Node {
            T key{};
            V value{};
            Node *parent{};
            Node *left{};
            Node *right{};
            int color{};
        };

        Node *m_root{};
        Node *m_TNULL{};

        std::vector<T> m_keys{};

        Node *m_search(Node *node, const T &key) const;

        void m_after_delete(Node *node);

        void m_transplant(Node *a, Node *b);

        void m_erase(Node *node, const T &key);

        void m_after_insert(Node *node);

        Node *m_minimum(Node *node) const;

        Node *m_maximum(Node *node) const;

        void m_left_rotate(Node *node);

        void m_right_rotate(Node *node);

        Node *m_copy_tree(Node *from, Node *fromTNULL, Node *parent);

        void m_delete_tree(Node *node);
    private:
    public:
        RBTree();
        RBTree(const RBTree &tree);

        RBTree &operator=(const RBTree &tree);

        ~RBTree();

        V &search_without_copy(const T &key);

        V search(const T &key) const;

        void insert(const T &key, const V &value);

        void erase(const T &key);

        std::vector<T> keys() const {
            return m_keys;
        }

        void clear()
        {
            m_delete_tree(m_root);
            m_root = m_TNULL;
        }
    };
}

template
class tree::RBTree<double, std::list<unsigned long long>>;
template
class tree::RBTree<std::string, std::list<unsigned long long>>;

#endif //DATABASEDIMA__RBTREE_H_