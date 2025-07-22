#pragma once


#include "./internals.h"
#include "./List.h"


typedef struct tree_node {
    int key;
    void *data;
    size_t type_size;

    int height;

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
    List* (*in_order_traversal)(struct AVL_Tree *self);
    void (*merge)(struct AVL_Tree *self, struct AVL_Tree *t2);
    void (*free)(struct AVL_Tree *self);
} AVL_Tree;


static int height(tree_node *node);
static void update_height(tree_node *node);
static tree_node* tree_node_left_rotate(tree_node *x);
static tree_node* tree_node_right_rotate(tree_node *y);
static int tree_node_balance(tree_node *node);
static tree_node* init_avl_node(int key, void *data, size_t type_size);
static inline AVL_Tree* New_AVL_Tree();
static tree_node* avl_insert_node(tree_node *node, int key, void *data, size_t type_size);
static inline void avl_insert(AVL_Tree *self, int key, void *data, size_t type_size);
static tree_node* get_min_node(tree_node* node);
static tree_node* avl_delete_node(tree_node* node, int key);
static inline void avl_delete(AVL_Tree *self, int key);
static tree_node* avl_search_node(tree_node *node, int key);
static inline tree_node* avl_lookup(AVL_Tree *self, int key);
static void tree_in_order_traversal_helper(tree_node *root, List *lst);
static inline List* tree_in_order_traversal(AVL_Tree *self);
static void avl_free_subtree(tree_node *node);
void avl_merge(AVL_Tree *self, AVL_Tree *t2);
static void avl_free(AVL_Tree *self);


static int height(tree_node *node) {
    return node ? node->height : 0;
}


static void update_height(tree_node *node) {
    if (node)
        node->height = 1 + MAX(height(node->left), height(node->right));
}


static tree_node* tree_node_left_rotate(tree_node *x) {
    tree_node *y = x->right;
    tree_node *t2 = y->left;

    y->left = x;
    x->right = t2;

    if (t2) 
    t2->parent = x;

    y->parent = x->parent;
    x->parent = y;

    update_height(x);
    update_height(y);

    return y;
}


static tree_node* tree_node_right_rotate(tree_node *y) {
    tree_node *x = y->left;
    tree_node *t2 = x->right;

    x->right = y;
    y->left = t2;

    if (t2) 
        t2->parent = y;

    x->parent = y->parent;
    y->parent = x;

    update_height(y);
    update_height(x);

    return x;
}


static int tree_node_balance(tree_node *node) {
    return node ? height(node->left) - height(node->right) : 0;
}


static tree_node* init_avl_node(int key, void *data, size_t type_size) {
    tree_node *node = (tree_node*) malloc(sizeof(tree_node));

    if (!node)
        throw_memory_allocation_error();

    node->key = key;
    node->data = copy_from_void_ptr(data, type_size);
    node->type_size = type_size;
    node->height = 1;
    node->parent = node->left = node->right = NULL;

    return node;
}


static inline AVL_Tree* New_AVL_Tree() {
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
    self->in_order_traversal = tree_in_order_traversal;
    self->merge = avl_merge;
    self->free = avl_free;

    return self;
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
        free(node->data);
        node->data = copy_from_void_ptr(data, type_size);
        node->type_size = type_size;
        return node;
    }

    update_height(node);

    int balance = tree_node_balance(node);

    if (balance > 1 && key < node->left->key)
        return tree_node_right_rotate(node);

    if (balance < -1 && key > node->right->key)
        return tree_node_left_rotate(node);

    if (balance > 1 && key > node->left->key) {
        node->left = tree_node_left_rotate(node->left);
        return tree_node_right_rotate(node);
    }

    if (balance < -1 && key < node->right->key) {
        node->right = tree_node_right_rotate(node->right);
        return tree_node_left_rotate(node);
    }

    return node;
}


static inline void avl_insert(AVL_Tree *self, int key, void *data, size_t type_size) {
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
            free(node->data);
            free(node);
            return temp;
        } else {
            tree_node *successor = get_min_node(node->right);
            node->key = successor->key;

            free(node->data);
            node->data = copy_from_void_ptr(successor->data, successor->type_size);
            node->type_size = successor->type_size;

            node->right = avl_delete_node(node->right, successor->key);
            if (node->right)
                node->right->parent = node;
        }
    }

    update_height(node);

    int balance = tree_node_balance(node);

    if (balance > 1) {
        if (tree_node_balance(node->left) >= 0)
            return tree_node_right_rotate(node);
        else {
            node->left = tree_node_left_rotate(node->left);
            return tree_node_right_rotate(node);
        }
    }

    if (balance < -1) {
        if (tree_node_balance(node->right) <= 0)
            return tree_node_left_rotate(node);
        else {
            node->right = tree_node_right_rotate(node->right);
            return tree_node_left_rotate(node);
        }
    }

    return node;
}


static inline void avl_delete(AVL_Tree *self, int key) {
    LOCK(self->mutex);

    self->root = avl_delete_node(self->root, key);
    if (self->root)
        self->root->parent = NULL;
    
    UNLOCK(self->mutex);
}


static tree_node* avl_search_node(tree_node *node, int key) {
    if (!node || node->key == key)
        return node;

    return key > node->key ? avl_search_node(node->right, key) : avl_search_node(node->left, key);
}


static inline tree_node* avl_lookup(AVL_Tree *self, int key) {
    LOCK(self->mutex);

    tree_node *res = avl_search_node(self->root, key);
    
    UNLOCK(self->mutex);
    
    return res;
}


static void tree_in_order_traversal_helper(tree_node *root, List *lst) {
    if (!root) 
        return;
    
    tree_in_order_traversal_helper(root->left, lst);
    
    lst->push(lst, root, sizeof(tree_node));
    
    tree_in_order_traversal_helper(root->right, lst);
}


static inline List* tree_in_order_traversal(AVL_Tree *self) {
    LOCK(self->mutex);

    List *res = New_List();
    tree_in_order_traversal_helper(self->root, res);
    
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


void avl_merge(AVL_Tree *self, AVL_Tree *t2) {
    LOCK(self->mutex);

    List *l1 = self->in_order_traversal(self);
    List *l2 = t2->in_order_traversal(t2);

    l1->merge(l1, l2);

    avl_free_subtree(self->root);
    self->root = NULL;

    for (size_t i=0; i<l1->len; ++i) {
        tree_node *current_node = (tree_node*) l1->get_at(l1, i);
        self->insert(self, current_node->key, current_node->data, current_node->type_size);
    }

    l1->free(l1);
    l2->free(l2);

    UNLOCK(self->mutex);
}


static void avl_free(AVL_Tree *self) {
    LOCK(self->mutex);

    avl_free_subtree(self->root);
    
    UNLOCK(self->mutex);
    pthread_mutex_destroy(&self->mutex);
    
    free(self);
}


