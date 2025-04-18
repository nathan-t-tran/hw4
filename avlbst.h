#ifndef AVLBST_H
#define AVLBST_H

#include <iostream>
#include <exception>
#include <cstdlib>
#include <cstdint>
#include <algorithm>
#include "bst.h"

struct KeyError { };

/**
* A special kind of node for an AVL tree, which adds the balance as a data member, plus
* other additional helper functions. You do NOT need to implement any functionality or
* add additional data members or helper functions.
*/
template <typename Key, typename Value>
class AVLNode : public Node<Key, Value>
{
public:
    // Constructor/destructor.
    AVLNode(const Key& key, const Value& value, AVLNode<Key, Value>* parent);
    virtual ~AVLNode();

    // Getter/setter for the node's height.
    int8_t getBalance () const;
    void setBalance (int8_t balance);
    void updateBalance(int8_t diff);

    // Getters for parent, left, and right. These need to be redefined since they
    // return pointers to AVLNodes - not plain Nodes. See the Node class in bst.h
    // for more information.
    virtual AVLNode<Key, Value>* getParent() const override;
    virtual AVLNode<Key, Value>* getLeft() const override;
    virtual AVLNode<Key, Value>* getRight() const override;

protected:
    int8_t balance_;    // effectively a signed char
    
    

};

/*
  -------------------------------------------------
  Begin implementations for the AVLNode class.
  -------------------------------------------------
*/

/**
* An explicit constructor to initialize the elements by calling the base class constructor
*/
template<class Key, class Value>
AVLNode<Key, Value>::AVLNode(const Key& key, const Value& value, AVLNode<Key, Value> *parent) :
    Node<Key, Value>(key, value, parent), balance_(0)
{

}

/**
* A destructor which does nothing.
*/
template<class Key, class Value>
AVLNode<Key, Value>::~AVLNode()
{

}

/**
* A getter for the balance of a AVLNode.
*/
template<class Key, class Value>
int8_t AVLNode<Key, Value>::getBalance() const
{
    return balance_;
}

/**
* A setter for the balance of a AVLNode.
*/
template<class Key, class Value>
void AVLNode<Key, Value>::setBalance(int8_t balance)
{
    balance_ = balance;
}

/**
* Adds diff to the balance of a AVLNode.
*/
template<class Key, class Value>
void AVLNode<Key, Value>::updateBalance(int8_t diff)
{
    balance_ += diff;
}

/**
* An overridden function for getting the parent since a static_cast is necessary to make sure
* that our node is a AVLNode.
*/
template<class Key, class Value>
AVLNode<Key, Value> *AVLNode<Key, Value>::getParent() const
{
    return static_cast<AVLNode<Key, Value>*>(this->parent_);
}

/**
* Overridden for the same reasons as above.
*/
template<class Key, class Value>
AVLNode<Key, Value> *AVLNode<Key, Value>::getLeft() const
{
    return static_cast<AVLNode<Key, Value>*>(this->left_);
}

/**
* Overridden for the same reasons as above.
*/
template<class Key, class Value>
AVLNode<Key, Value> *AVLNode<Key, Value>::getRight() const
{
    return static_cast<AVLNode<Key, Value>*>(this->right_);
}


/*
  -----------------------------------------------
  End implementations for the AVLNode class.
  -----------------------------------------------
*/


template <class Key, class Value>
class AVLTree : public BinarySearchTree<Key, Value>
{
public:
    virtual void insert (const std::pair<const Key, Value> &new_item); // TODO
    virtual void remove(const Key& key);  // TODO
protected:
    virtual void nodeSwap( AVLNode<Key,Value>* n1, AVLNode<Key,Value>* n2);

    // Add helper functions here
    //helper functions
    void insertFix(AVLNode<Key, Value>* parent, AVLNode<Key, Value>* child);
    void rotateLeft(AVLNode<Key, Value>* node);
    void rotateRight(AVLNode<Key, Value>* node);
    void removeFix(AVLNode<Key, Value>* node, int diff);
    AVLNode<Key, Value>* predecessor(AVLNode<Key, Value>* current);
};

/*
 * Recall: If key is already in the tree, you should 
 * overwrite the current value with the updated value.
 */
template<class Key, class Value>
void AVLTree<Key, Value>::insert(const std::pair<const Key, Value> &new_item)
{
    // If the tree is empty
    if(this->root_ == nullptr) {
        this->root_ = new AVLNode<Key, Value>(new_item.first, new_item.second, nullptr);
        return;
    }

    AVLNode<Key, Value>* curr = static_cast<AVLNode<Key, Value>*>(this->root_);
    AVLNode<Key, Value>* parent = nullptr;

    // Traverse to insert like in a normal BST
    while(curr != nullptr) {
        if(new_item.first == curr->getKey()) {
            curr->setValue(new_item.second);
            return;
        }
        parent = curr;
        if(new_item.first < curr->getKey()) {
            curr = curr->getLeft();
        } else {
            curr = curr->getRight();
        }
    }

    // Create the new node
    AVLNode<Key, Value>* newNode = new AVLNode<Key, Value>(new_item.first, new_item.second, parent);
    if(new_item.first < parent->getKey()) {
        parent->setLeft(newNode);
    } else {
        parent->setRight(newNode);
    }

    // Start rebalancing
    if(parent->getLeft() == newNode) {
        parent->updateBalance(-1);
    } else {
        parent->updateBalance(1);
    }

    insertFix(parent, newNode);
}


/*
 * Recall: The writeup specifies that if a node has 2 children you
 * should swap with the predecessor and then remove.
 */
template<class Key, class Value>
void AVLTree<Key, Value>::remove(const Key& key)
{
    AVLNode<Key, Value>* node = static_cast<AVLNode<Key, Value>*>(this->internalFind(key));
    if(node == nullptr) return;

    // Case: Two children — swap with predecessor
    if(node->getLeft() != nullptr && node->getRight() != nullptr) {
        AVLNode<Key, Value>* pred = predecessor(node);
        nodeSwap(pred, node);  // already provided
    }

    // Now node has at most one child
    AVLNode<Key, Value>* parent = node->getParent();
    AVLNode<Key, Value>* child = node->getLeft() ? node->getLeft() : node->getRight();

    int diff = 0;

    if(parent) {
        if(parent->getLeft() == node)
            diff = 1;
        else
            diff = -1;
    }

    if(child != nullptr) {
        child->setParent(parent);
    }

    if(parent == nullptr) {
        this->root_ = child;
    } else {
        if(parent->getLeft() == node)
            parent->setLeft(child);
        else
            parent->setRight(child);
    }

    delete node;

    // Start fixing balance from parent
    removeFix(parent, diff);
}


template<class Key, class Value>
void AVLTree<Key, Value>::insertFix(AVLNode<Key, Value>* parent, AVLNode<Key, Value>* child)
{
    if(parent == nullptr || parent->getParent() == nullptr) return;

    AVLNode<Key, Value>* grandparent = parent->getParent();

    if(parent == grandparent->getLeft()) {
        grandparent->updateBalance(-1);
        if(grandparent->getBalance() == 0) return;
        else if(grandparent->getBalance() == -1)
            insertFix(grandparent, parent);
        else if(grandparent->getBalance() == -2) {
            if(child == parent->getLeft()) {
                rotateRight(grandparent);
                parent->setBalance(0);
                grandparent->setBalance(0);
            } else {
                rotateLeft(parent);
                rotateRight(grandparent);
                int8_t childBalance = child->getBalance();
                if(childBalance == -1) {
                    parent->setBalance(0);
                    grandparent->setBalance(1);
                } else if(childBalance == 0) {
                    parent->setBalance(0);
                    grandparent->setBalance(0);
                } else {
                    parent->setBalance(-1);
                    grandparent->setBalance(0);
                }
                child->setBalance(0);
            }
        }
    } else {
        grandparent->updateBalance(1);
        if(grandparent->getBalance() == 0) return;
        else if(grandparent->getBalance() == 1)
            insertFix(grandparent, parent);
        else if(grandparent->getBalance() == 2) {
            if(child == parent->getRight()) {
                rotateLeft(grandparent);
                parent->setBalance(0);
                grandparent->setBalance(0);
            } else {
                rotateRight(parent);
                rotateLeft(grandparent);
                int8_t childBalance = child->getBalance();
                if(childBalance == 1) {
                    parent->setBalance(0);
                    grandparent->setBalance(-1);
                } else if(childBalance == 0) {
                    parent->setBalance(0);
                    grandparent->setBalance(0);
                } else {
                    parent->setBalance(1);
                    grandparent->setBalance(0);
                }
                child->setBalance(0);
            }
        }
    }
}

template<class Key, class Value>
void AVLTree<Key, Value>::rotateLeft(AVLNode<Key, Value>* x)
{
    AVLNode<Key, Value>* y = x->getRight();
    x->setRight(y->getLeft());
    if(y->getLeft() != nullptr) y->getLeft()->setParent(x);
    y->setParent(x->getParent());

    if(x->getParent() == nullptr)
        this->root_ = y;
    else if(x == x->getParent()->getLeft())
        x->getParent()->setLeft(y);
    else
        x->getParent()->setRight(y);

    y->setLeft(x);
    x->setParent(y);
}

template<class Key, class Value>
void AVLTree<Key, Value>::rotateRight(AVLNode<Key, Value>* x)
{
    AVLNode<Key, Value>* y = x->getLeft();
    x->setLeft(y->getRight());
    if(y->getRight() != nullptr) y->getRight()->setParent(x);
    y->setParent(x->getParent());

    if(x->getParent() == nullptr)
        this->root_ = y;
    else if(x == x->getParent()->getLeft())
        x->getParent()->setLeft(y);
    else
        x->getParent()->setRight(y);

    y->setRight(x);
    x->setParent(y);
}

template<class Key, class Value>
void AVLTree<Key, Value>::removeFix(AVLNode<Key, Value>* node, int diff)
{
    while(node != nullptr) {
        node->updateBalance(diff);
        int bal = node->getBalance();

        if(bal == -1 || bal == 1) break;

        if(bal == -2) {
            AVLNode<Key, Value>* left = node->getLeft();
            int leftBal = left->getBalance();

            if(leftBal <= 0) {
                rotateRight(node);
                if(leftBal == 0) {
                    node->setBalance(-1);
                    left->setBalance(1);
                    break;
                } else {
                    node->setBalance(0);
                    left->setBalance(0);
                    node = left->getParent();
                    diff = 1;
                    continue;
                }
            } else {
                AVLNode<Key, Value>* child = left->getRight();
                int childBal = child->getBalance();

                rotateLeft(left);
                rotateRight(node);

                if(childBal == -1) {
                    node->setBalance(1);
                    left->setBalance(0);
                } else if(childBal == 0) {
                    node->setBalance(0);
                    left->setBalance(0);
                } else {
                    node->setBalance(0);
                    left->setBalance(-1);
                }

                child->setBalance(0);
                node = child->getParent();
                diff = 1;
                continue;
            }
        }

        if(bal == 2) {
            AVLNode<Key, Value>* right = node->getRight();
            int rightBal = right->getBalance();

            if(rightBal >= 0) {
                rotateLeft(node);
                if(rightBal == 0) {
                    node->setBalance(1);
                    right->setBalance(-1);
                    break;
                } else {
                    node->setBalance(0);
                    right->setBalance(0);
                    node = right->getParent();
                    diff = -1;
                    continue;
                }
            } else {
                AVLNode<Key, Value>* child = right->getLeft();
                int childBal = child->getBalance();

                rotateRight(right);
                rotateLeft(node);

                if(childBal == 1) {
                    node->setBalance(-1);
                    right->setBalance(0);
                } else if(childBal == 0) {
                    node->setBalance(0);
                    right->setBalance(0);
                } else {
                    node->setBalance(0);
                    right->setBalance(1);
                }

                child->setBalance(0);
                node = child->getParent();
                diff = -1;
                continue;
            }
        }

        // If balance is 0, continue upward
        if(node->getParent() != nullptr) {
            if(node == node->getParent()->getLeft())
                diff = 1;
            else
                diff = -1;
        }

        node = node->getParent();
    }
}

template<class Key, class Value>
AVLNode<Key, Value>* AVLTree<Key, Value>::predecessor(AVLNode<Key, Value>* current)
{
    if(current->getLeft() != nullptr) {
        AVLNode<Key, Value>* pred = current->getLeft();
        while(pred->getRight() != nullptr)
            pred = pred->getRight();
        return pred;
    }

    AVLNode<Key, Value>* parent = current->getParent();
    while(parent != nullptr && current == parent->getLeft()) {
        current = parent;
        parent = parent->getParent();
    }
    return parent;
}



template<class Key, class Value>
void AVLTree<Key, Value>::nodeSwap( AVLNode<Key,Value>* n1, AVLNode<Key,Value>* n2)
{
    BinarySearchTree<Key, Value>::nodeSwap(n1, n2);
    int8_t tempB = n1->getBalance();
    n1->setBalance(n2->getBalance());
    n2->setBalance(tempB);
}


#endif
