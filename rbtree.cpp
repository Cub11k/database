#include "rbtree.h"

template <typename T, typename V>
typename tree::RBTree<T, V>::Node *tree::RBTree<T, V>::m_search(Node *node, const T &key) const
{
    if (node == m_TNULL) {
        return nullptr;
    }
    if (key == node->key) {
        return node;
    }

    if (key < node->key) {
        return m_search(node->left, key);
    }
    return m_search(node->right, key);
}

template <typename T, typename V>
void tree::RBTree<T, V>::m_after_delete(Node *node)
{
    Node *tmp;
    while (node != m_root && node->color == 0) {
        if (node == node->parent->left) {
            tmp = node->parent->right;
            if (tmp->color == 1) {
                tmp->color = 0;
                node->parent->color = 1;
                m_left_rotate(node->parent);
                tmp = node->parent->right;
            }

            if (tmp->left->color == 0 && tmp->right->color == 0) {
                tmp->color = 1;
                node = node->parent;
            } else {
                if (tmp->right->color == 0) {
                    tmp->left->color = 0;
                    tmp->color = 1;
                    m_right_rotate(tmp);
                    tmp = node->parent->right;
                }

                tmp->color = node->parent->color;
                node->parent->color = 0;
                tmp->right->color = 0;
                m_left_rotate(node->parent);
                node = m_root;
            }
        } else {
            tmp = node->parent->left;
            if (tmp->color == 1) {
                tmp->color = 0;
                node->parent->color = 1;
                m_right_rotate(node->parent);
                tmp = node->parent->left;
            }

            if (tmp->right->color == 0 && tmp->right->color == 0) {
                tmp->color = 1;
                node = node->parent;
            } else {
                if (tmp->left->color == 0) {
                    tmp->right->color = 0;
                    tmp->color = 1;
                    m_left_rotate(tmp);
                    tmp = node->parent->left;
                }

                tmp->color = node->parent->color;
                node->parent->color = 0;
                tmp->left->color = 0;
                m_right_rotate(node->parent);
                node = m_root;
            }
        }
    }
    node->color = 0;
}

template <typename T, typename V>
void tree::RBTree<T, V>::m_transplant(Node *a, Node *b)
{
    if (a->parent == nullptr) {
        m_root = b;
    } else if (a == a->parent->left) {
        a->parent->left = b;
    } else {
        a->parent->right = b;
    }
    b->parent = a->parent;
}

template <typename T, typename V>
void tree::RBTree<T, V>::m_erase(Node *node, const T &key)
{
    Node *found = m_TNULL;
    Node *tmp{};
    Node *tmp_son{};
    while (node != m_TNULL) {
        if (node->key == key) {
            found = node;
        }
        if (node->key <= key) {
            node = node->right;
        } else {
            node = node->left;
        }
    }

    if (found == m_TNULL) {
        throw std::invalid_argument("Key not found in the red black tree");
    }

    tmp_son = found;
    int y_original_color = tmp_son->color;
    if (found->left == m_TNULL) {
        tmp = found->right;
        m_transplant(found, found->right);
    } else if (found->right == m_TNULL) {
        tmp = found->left;
        m_transplant(found, found->left);
    } else {
        tmp_son = m_minimum(found->right);
        y_original_color = tmp_son->color;
        tmp = tmp_son->right;
        if (tmp_son->parent == found) {
            tmp->parent = tmp_son;
        } else {
            m_transplant(tmp_son, tmp_son->right);
            tmp_son->right = found->right;
            tmp_son->right->parent = tmp_son;
        }

        m_transplant(found, tmp_son);
        tmp_son->left = found->left;
        tmp_son->left->parent = tmp_son;
        tmp_son->color = found->color;
    }
    delete found;
    if (y_original_color == 0) {
        m_after_delete(tmp);
    }
}

template <typename T, typename V>
void tree::RBTree<T, V>::m_after_insert(Node *node)
{
    Node *tmp;
    while (node->parent->color == 1) {
        if (node->parent == node->parent->parent->right) {
            tmp = node->parent->parent->left;
            if (tmp->color == 1) {
                tmp->color = 0;
                node->parent->color = 0;
                node->parent->parent->color = 1;
                node = node->parent->parent;
            } else {
                if (node == node->parent->left) {
                    node = node->parent;
                    m_right_rotate(node);
                }
                node->parent->color = 0;
                node->parent->parent->color = 1;
                m_left_rotate(node->parent->parent);
            }
        } else {
            tmp = node->parent->parent->right;

            if (tmp->color == 1) {
                tmp->color = 0;
                node->parent->color = 0;
                node->parent->parent->color = 1;
                node = node->parent->parent;
            } else {
                if (node == node->parent->right) {
                    node = node->parent;
                    m_left_rotate(node);
                }
                node->parent->color = 0;
                node->parent->parent->color = 1;
                m_right_rotate(node->parent->parent);
            }
        }
        if (node == m_root) {
            break;
        }
    }
    m_root->color = 0;
}

template <typename T, typename V>
typename tree::RBTree<T, V>::Node *tree::RBTree<T, V>::m_copy_tree(Node *from, Node *fromTNULL, Node *parent)
{
    if (from == fromTNULL) {
        return m_TNULL;
    }
    auto newNode = new Node;
    newNode->key = from->key;
    newNode->value = from->value;
    newNode->parent = parent;
    newNode->left = m_copy_tree(from->left, fromTNULL, newNode);
    newNode->right = m_copy_tree(from->right, fromTNULL, newNode);
    return newNode;
}

template <typename T, typename V>
void tree::RBTree<T, V>::m_delete_tree(Node *node)
{
    if (node == m_TNULL) {
        return;
    }
    m_delete_tree(node->left);
    m_delete_tree(node->right);
    node->value = {};
    node->key = {};
    node->parent = {};
    node->left = {};
    node->right = {};
    node->color = {};
    delete node;
}

template <typename T, typename V>
tree::RBTree<T, V>::RBTree()
{
    m_TNULL = new Node;
    m_TNULL->value = {};
    m_TNULL->color = 0;
    m_TNULL->left = nullptr;
    m_TNULL->right = nullptr;
    m_root = m_TNULL;
}

template <typename T, typename V>
tree::RBTree<T, V>::RBTree(const tree::RBTree<T, V> &tree)
{
    m_TNULL = new Node;
    m_TNULL->value = {};
    m_TNULL->color = 0;
    m_TNULL->left = nullptr;
    m_TNULL->right = nullptr;
    m_root = m_copy_tree(tree.m_root, tree.m_TNULL, nullptr);
}

template <typename T, typename V>
tree::RBTree<T, V> &tree::RBTree<T, V>::operator=(const tree::RBTree<T, V> &tree)
{
    if (this == &tree) {
        return *this;
    }
    m_delete_tree(m_root);
    delete m_TNULL;
    m_TNULL = new Node;
    m_TNULL->value = {};
    m_TNULL->color = 0;
    m_TNULL->left = nullptr;
    m_TNULL->right = nullptr;
    m_root = m_copy_tree(tree.m_root, tree.m_TNULL, nullptr);
    return *this;
}

template <typename T, typename V>
tree::RBTree<T, V>::~RBTree()
{
    m_delete_tree(m_root);
    delete m_TNULL;
}

template <typename T, typename V>
V &tree::RBTree<T, V>::search_without_copy(const T &key)
{
    Node *node = m_search(this->m_root, key);
    if (node) {
        return node->value;
    } else {
        throw std::invalid_argument(KEY_NOT_FOUND);
    }
}

template <typename T, typename V>
V tree::RBTree<T, V>::search(const T &key) const
{
    Node *node = m_search(this->m_root, key);
    if (node) {
        return node->value;
    } else {
        throw std::invalid_argument(KEY_NOT_FOUND);
    }
}

template <typename T, typename V>
typename tree::RBTree<T, V>::Node *tree::RBTree<T, V>::m_minimum(Node *node) const
{
    while (node->left != m_TNULL) {
        node = node->left;
    }
    return node;
}

template <typename T, typename V>
typename tree::RBTree<T, V>::Node *tree::RBTree<T, V>::m_maximum(Node *node) const
{
    while (node->right != m_TNULL) {
        node = node->right;
    }
    return node;
}

template <typename T, typename V>
void tree::RBTree<T, V>::m_left_rotate(Node *node)
{
    Node *tmp_son = node->right;
    node->right = tmp_son->left;
    if (tmp_son->left != m_TNULL) {
        tmp_son->left->parent = node;
    }
    tmp_son->parent = node->parent;
    if (node->parent == nullptr) {
        this->m_root = tmp_son;
    } else if (node == node->parent->left) {
        node->parent->left = tmp_son;
    } else {
        node->parent->right = tmp_son;
    }
    tmp_son->left = node;
    node->parent = tmp_son;
}

template <typename T, typename V>
void tree::RBTree<T, V>::m_right_rotate(Node *node)
{
    Node *tmp_son = node->left;
    node->left = tmp_son->right;
    if (tmp_son->right != m_TNULL) {
        tmp_son->right->parent = node;
    }
    tmp_son->parent = node->parent;
    if (node->parent == nullptr) {
        this->m_root = tmp_son;
    } else if (node == node->parent->right) {
        node->parent->right = tmp_son;
    } else {
        node->parent->left = tmp_son;
    }
    tmp_son->right = node;
    node->parent = tmp_son;
}

template <typename T, typename V>
void tree::RBTree<T, V>::insert(const T &key, const V &value)
{
    auto node = m_search(m_root, key);
    if (node != nullptr) {
        node->value = value;
        return;
    }
    m_keys.push_back(key);
    node = new Node;
    node->parent = nullptr;
    node->key = key;
    node->value = value;
    node->left = m_TNULL;
    node->right = m_TNULL;
    node->color = 1;

    Node *tmp_parent = nullptr;
    Node *tmp_node = this->m_root;

    while (tmp_node != m_TNULL) {
        tmp_parent = tmp_node;
        if (node->key < tmp_node->key) {
            tmp_node = tmp_node->left;
        } else {
            tmp_node = tmp_node->right;
        }
    }

    node->parent = tmp_parent;
    if (tmp_parent == nullptr) {
        m_root = node;
    } else if (node->key < tmp_parent->key) {
        tmp_parent->left = node;
    } else {
        tmp_parent->right = node;
    }

    if (node->parent == nullptr) {
        node->color = 0;
        return;
    }

    if (node->parent->parent == nullptr) {
        return;
    }

    m_after_insert(node);
}

template <typename T, typename V>
void tree::RBTree<T, V>::erase(const T &key)
{
    m_erase(this->m_root, key);
}
