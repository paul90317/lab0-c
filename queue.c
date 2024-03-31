#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */


/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *q = malloc(sizeof(struct list_head));
    if (!q)
        return NULL;
    INIT_LIST_HEAD(q);
    return q;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (!head)
        return;
    element_t *entry, *safe;
    list_for_each_entry_safe (entry, safe, head, list) {
        q_release_element(entry);
    }
    free(head);
}

static element_t *new_element(char *s)
{
    char *t = strdup(s);
    if (!t)
        return NULL;
    element_t *element = malloc(sizeof(element_t));
    if (!element) {
        free(t);
        return NULL;
    }
    element->value = t;
    return element;
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    element_t *element = new_element(s);
    if (!element)
        return false;
    list_add(&element->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    element_t *element = new_element(s);
    if (!element)
        return false;
    list_add_tail(&element->list, head);
    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *element = container_of(head->next, element_t, list);
    list_del(&element->list);
    if (element->value && sp) {
        strncpy(sp, element->value, bufsize - 1);
        sp[bufsize - 1] = 0;
    }
    return element;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *element = container_of(head->prev, element_t, list);
    list_del(&element->list);
    if (element->value && sp) {
        strncpy(sp, element->value, bufsize - 1);
        sp[bufsize - 1] = 0;
    }
    return element;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    int cnt = 0;
    struct list_head *node;
    list_for_each (node, head) {
        ++cnt;
    }
    return cnt;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (!head || list_empty(head)) {
        return false;
    }
    struct list_head *slow = head, *fast = head;
    do {
        slow = slow->next;
        fast = fast->next->next;
    } while (fast != head && fast->next != head);
    if (fast->next == head) {
        slow = slow->next;
    }
    list_del(slow);
    q_release_element(container_of(slow, element_t, list));
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (!head) {
        return false;
    }
    element_t *element, *safe;
    bool flag = false;
    list_for_each_entry_safe (element, safe, head, list) {
        if (&safe->list != head && !strcmp(element->value, safe->value)) {
            flag = true;
            list_del(&element->list);
            q_release_element(element);
        } else if (flag) {
            flag = false;
            list_del(&element->list);
            q_release_element(element);
        }
    }
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    struct list_head *node1, *node2, *safe;
    for (node1 = head->next, node2 = node1->next, safe = node2->next;
         node1 != head && node2 != head;
         node1 = safe, node2 = node1->next, safe = node2->next) {
        list_del(node1);
        list_add(node1, node2);
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    struct list_head *node, *safe, *temp;
    list_for_each_safe (node, safe, head) {
        temp = node->next;
        node->next = node->prev;
        node->prev = temp;
    }
    temp = head->next;
    head->next = head->prev;
    head->prev = temp;
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
    struct list_head *node;
    for (node = head->next; node != head;) {
        LIST_HEAD(stack);
        struct list_head *safe;
        int n;
        for (n = 0, safe = node->next; n < k && node != head;
             n++, node = safe, safe = node->next) {
            list_del(node);
            list_add(node, &stack);
        }
        while (!list_empty(&stack)) {
            struct list_head *temp = n == k ? stack.next : stack.prev;
            list_del(temp);
            list_add_tail(temp, node);
        }
    }
}

typedef struct {
    int size;
    struct list_head list;
} list_t;

static void merge_tail_init(list_t *list, list_t *head)
{
    list_t c;
    INIT_LIST_HEAD(&c.list);
    while (!list_empty(&head->list) && !list_empty(&list->list)) {
        int cmp = strcmp(container_of(head->list.next, element_t, list)->value,
                         container_of(list->list.next, element_t, list)->value);
        struct list_head *curr = cmp <= 0 ? head->list.next : list->list.next;
        list_del(curr);
        list_add_tail(curr, &c.list);
    }
    head->size += list->size;
    list->size = 0;
    if (list_empty(&list->list)) {
        list_splice(&c.list, &head->list);
    } else {
        list_splice_tail(&c.list, &head->list);
        list_splice_tail_init(&list->list, &head->list);
    }
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    int stack_cap = 33 - __builtin_clz(q_size(head) - 1);
    list_t stack[33];
    for (int i = 0; i < stack_cap; ++i) {
        INIT_LIST_HEAD(&stack[i].list);
        stack[i].size = 0;
    }
    int stack_size = 0;
    while (!list_empty(head)) {
        struct list_head *curr = head->next;
        list_del(curr);
        list_add(curr, &stack[stack_size].list);
        stack[stack_size++].size = 1;
        while (stack_size >= 2 &&
               stack[stack_size - 2].size <= stack[stack_size - 1].size) {
            merge_tail_init(stack + stack_size - 1, stack + stack_size - 2);
            --stack_size;
        }
    }

    while (stack_size >= 2) {
        merge_tail_init(stack + stack_size - 1, stack + stack_size - 2);
        --stack_size;
    }
    list_splice(&stack->list, head);
    if (descend)
        q_reverse(head);
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    int cnt = 0;
    struct list_head *node, *safe;
    char *min_str = NULL;
    for (node = head->prev, safe = node->prev; node != head;
         node = safe, safe = node->prev) {
        element_t *element = container_of(node, element_t, list);
        int cmp = min_str ? strcmp(element->value, min_str) : -1;
        if (cmp < 0) {
            min_str = element->value;
            ++cnt;
        } else if (cmp > 0) {
            list_del(&element->list);
            q_release_element(element);
        } else {
            ++cnt;
        }
    }
    return cnt;
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    int cnt = 0;
    struct list_head *node, *safe;
    char *max_str = NULL;
    for (node = head->prev, safe = node->prev; node != head;
         node = safe, safe = node->prev) {
        element_t *element = container_of(node, element_t, list);
        int cmp = max_str ? strcmp(element->value, max_str) : 1;
        if (cmp > 0) {
            max_str = element->value;
            ++cnt;
        } else if (cmp < 0) {
            list_del(&element->list);
            q_release_element(element);
        } else {
            ++cnt;
        }
    }
    return cnt;
}

static void swap(list_t *a, list_t *b)
{
    list_t c;
    INIT_LIST_HEAD(&c.list);
    list_splice_init(&a->list, &c.list);
    c.size = a->size;
    list_splice_init(&b->list, &a->list);
    a->size = b->size;
    list_splice_init(&c.list, &b->list);
    b->size = c.size;
}

static void heapify(list_t *heap, int heap_size, int curr)
{
    while (curr < heap_size) {
        int left = 2 * curr + 1, right = 2 * curr + 2;
        left = left < heap_size ? left : curr;
        right = right < heap_size ? right : curr;
        if (heap[curr].size <= heap[left].size &&
            heap[curr].size <= heap[right].size) {
            break;
        }
        int down = heap[left].size <= heap[right].size ? left : right;
        swap(heap + down, heap + curr);
        curr = down;
    }
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    int heap_size = q_size(head);
    list_t heap[100];

    struct list_head *next = head->next;
    for (int i = 0; i < heap_size; ++i) {
        INIT_LIST_HEAD(&heap[i].list);
        list_splice_init(container_of(next, queue_contex_t, chain)->q,
                         &heap[i].list);
        heap[i].size = q_size(&heap[i].list);
        next = next->next;
    }

    for (int i = heap_size - 1; i >= 0; i--) {
        heapify(heap, heap_size, i);
    }

    while (heap_size > 1) {
        swap(heap + --heap_size, heap);
        heapify(heap, heap_size, 0);
        merge_tail_init(heap + heap_size, heap);
        heapify(heap, heap_size, 0);
    }

    struct list_head *ret = container_of(head->next, queue_contex_t, chain)->q;
    list_splice(&heap->list, ret);
    if (descend)
        q_reverse(ret);
    return q_size(ret);
}