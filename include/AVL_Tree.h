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


AVL_Tree* New_AVL_Tree();


