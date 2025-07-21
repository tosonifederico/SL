#pragma once


#include "./internals.h"


typedef struct tree_node {
    int key;
    void *data;
    size_t type_size;

    struct tree_node *parent;
    struct tree_node *left;
    struct tree_node *right;
} tree_node;


typedef struct AVL_Tree {
    struct AVL_Tree *self;

    tree_node *root;

    pthread_mutex_t mutex;
    pthread_mutexattr_t mutex_attr;

    void (*insert)(struct AVL_Tree *self, int key, void *data, size_t type_size);
    void (*delete)(struct AVL_Tree *self, int key);
    tree_node* (*lookup)(struct AVL_Tree *self, int key);
    void (*free)(struct AVL_Tree *self);
} AVL_Tree;


AVL_Tree* New_AVL_Tree();
static size_t tree_node_height(tree_node *node);
static tree_node* tree_node_left_rotate(tree_node *node);
static tree_node* tree_node_right_rotate(tree_node *node);
static int8_t tree_node_balance(tree_node *node);
static tree_node* init_avl_node(int key, void *data, size_t type_size);
static tree_node* avl_insert_node(tree_node *node, int key, void *data, size_t type_size);
static void avl_insert(AVL_Tree *self, int key, void *data, size_t type_size);
static tree_node* get_min_node(tree_node* node);
static tree_node* avl_delete_node(tree_node* node, int key);
static void avl_delete(AVL_Tree *self, int key);
static tree_node* avl_search_node(tree_node *node, int key);
static inline tree_node* avl_lookup(struct AVL_Tree *self, int key);
static void avl_free_subtree(tree_node *node);
static void avl_free(AVL_Tree *self);


AVL_Tree* New_AVL_Tree() {
    AVL_Tree *self = (AVL_Tree*) malloc(sizeof(AVL_Tree));
    
    if (!self)
        throw_memory_allocation_error();

    self->self = self;

    self->root = NULL;

    pthread_mutexattr_init(&self->mutex_attr);
    pthread_mutexattr_settype(&self->mutex_attr, PTHREAD_MUTEX_RECURSIVE);

    pthread_mutex_init(&self->mutex, &self->mutex_attr);

    pthread_mutexattr_destroy(&self->mutex_attr);

    self->insert = avl_insert;
    self->delete = avl_delete;
    self->lookup = avl_lookup;
    self->free = avl_free;

    return self;
}


static size_t tree_node_height(tree_node *node) {
    if (!node)
        return 0;
    return 1 + MAX(tree_node_height(node->left), tree_node_height(node->right));
}


static tree_node* tree_node_left_rotate(tree_node *node) {
    tree_node *b = node->right;
    tree_node *y = b->left;

    b->left = node;
    node->right = y;

    if (y)
        y->parent = node;

    b->parent = node->parent;
    node->parent = b;

    return b;
}


static tree_node* tree_node_right_rotate(tree_node *node) {
    tree_node *b = node->left;
    tree_node *y = b->right;

    b->right = node;
    node->left = y;

    if (y) 
        y->parent = node;

    b->parent = node->parent;
    node->parent = b;

    return b;
}


static int8_t tree_node_balance(tree_node *node) {
    if (!node) 
        return 0;
    return tree_node_height(node->left) - tree_node_height(node->right);
}


static tree_node* init_avl_node(int key, void *data, size_t type_size) {
    tree_node *node = (tree_node*) malloc(sizeof(tree_node));
    
    if (!node)
        throw_memory_allocation_error();

    node->key = key;
    node->data = copy_from_void_ptr(data, type_size);
    node->type_size = type_size;
    node->parent = node->left = node->right = NULL;

    return node;
}


static tree_node* avl_insert_node(tree_node *node, int key, void *data, size_t type_size) {
    if (!node)
        return init_avl_node(key, data, type_size);

    if (key < node->key) {
        node->left = avl_insert_node(node->left, key, data, type_size);
        if (node->left)
            node->left->parent = node;
    } else if (key > node->key) {
        node->right = avl_insert_node(node->right, key, data, type_size);
        if (node->right)
            node->right->parent = node;
    } else {
        node->data = copy_from_void_ptr(data, type_size);
        return node;
    }

    int balance = tree_node_balance(node);

    if (balance > 1 && key < node->left->key)
        return tree_node_right_rotate(node);

    if (balance < -1 && key > node->right->key)
        return tree_node_left_rotate(node);

    if (balance > 1 && key > node->left->key) {
        node->left = tree_node_left_rotate(node->left);
        if (node->left) 
            node->left->parent = node;
        return tree_node_right_rotate(node);
    }

    if (balance < -1 && key < node->right->key) {
        node->right = tree_node_right_rotate(node->right);
        if (node->right) 
            node->right->parent = node;
        return tree_node_left_rotate(node);
    }

    return node;
}


static void avl_insert(AVL_Tree *self, int key, void *data, size_t type_size) {
    LOCK(self->mutex);

    self->root = avl_insert_node(self->root, key, data, type_size);
    if (self->root)
        self->root->parent = NULL;

    UNLOCK(self->mutex);
}


static tree_node* get_min_node(tree_node* node) {
    while (node && node->left)
        node = node->left;
    return node;
}


static tree_node* avl_delete_node(tree_node* node, int key) {
    if (!node)
        return NULL;

    if (key < node->key) {
        node->left = avl_delete_node(node->left, key);
        if (node->left)
            node->left->parent = node;
    } else if (key > node->key) {
        node->right = avl_delete_node(node->right, key);
        if (node->right)
            node->right->parent = node;
    } else {
        if (!node->left || !node->right) {
            tree_node *temp = node->left ? node->left : node->right;

            if (!temp) {
                free(node->data);
                free(node);
                return NULL;
            } else {
                temp->parent = node->parent;
                free(node->data);
                free(node);
                return temp;
            }
        } else {
            tree_node *successor = get_min_node(node->right);
            node->key = successor->key;
            node->data = copy_from_void_ptr(successor->data, successor->type_size);

            node->right = avl_delete_node(node->right, successor->key);
            if (node->right)
                node->right->parent = node;
        }
    }

    int balance = tree_node_balance(node);

    if (balance > 1) {
        if (tree_node_balance(node->left) >= 0)
            return tree_node_right_rotate(node);
        else {
            node->left = tree_node_left_rotate(node->left);
            if (node->left) 
                node->left->parent = node;
            return tree_node_right_rotate(node);
        }
    }

    if (balance < -1) {
        if (tree_node_balance(node->right) <= 0)
            return tree_node_left_rotate(node);
        else {
            node->right = tree_node_right_rotate(node->right);
            if (node->right)
                node->right->parent = node;
            return tree_node_left_rotate(node);
        }
    }

    return node;
}


static void avl_delete(AVL_Tree *self, int key) {
    LOCK(self->mutex);

    if (!self || !self->root) 
        goto un;

    self->root = avl_delete_node(self->root, key);

    if (self->root)
        self->root->parent = NULL;

    un:
        UNLOCK(self->mutex);
}


static tree_node* avl_search_node(tree_node *node, int key) {
    if (!node || node->key == key)
        return node;
    if (key > node->key)
        return avl_search_node(node->right, key);
    else
        return avl_search_node(node->left, key);
}


static inline tree_node* avl_lookup(AVL_Tree *self, int key) {
    LOCK(self->mutex);

    tree_node *res = avl_search_node(self->root, key);
    
    UNLOCK(self->mutex);
    
    return res;
}


static void avl_free_subtree(tree_node *node) {
    if (!node)
        return;

    avl_free_subtree(node->left);
    avl_free_subtree(node->right);

    free(node->data);
    free(node);
}


static void avl_free(AVL_Tree *self) {
    LOCK(self->mutex);

    avl_free_subtree(self->root);
    
    UNLOCK(self->mutex);
    pthread_mutex_destroy(&self->mutex);
    
    free(self);
}

