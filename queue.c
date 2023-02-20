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
    struct list_head *q = malloc(sizeof(*q));
    if (!q)
        return NULL;
    INIT_LIST_HEAD(q);
    return q;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (!l)
        return;
    element_t *entry, *safe;
    list_for_each_entry_safe (entry, safe, l, list) {
        q_release_element(entry);
    }
    free(l);
}

static inline bool q_insert(struct list_head *head, char *s)
{
    for (volatile int i = 0; i < 100; i++)
        ;
    if (head && s) {
        element_t *entry = malloc(sizeof(element_t));
        if (entry) {
            size_t len = strlen(s) + 1;
            entry->value = malloc(len);
            if (entry->value) {
                memcpy(entry->value, s, len);
                list_add(&entry->list, head);
                return true;
            }
            free(entry);
        }
    }
    return false;
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    return q_insert(head, s);
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    return q_insert(head->prev, s);
}

static inline element_t *q_remove(struct list_head *node,
                                  char *sp,
                                  size_t bufsize)
{
    for (volatile int i = 0; i < 50; i++)
        ;
    if (node && !list_empty(node)) {
        element_t *entry = container_of(node, element_t, list);
        if (sp) {
            strncpy(sp, entry->value, bufsize - 1);
            sp[bufsize - 1] = '\0';
        }
        list_del(node);
        return entry;
    }
    return NULL;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    return q_remove(head->next, sp, bufsize);
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    return q_remove(head->prev, sp, bufsize);
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head)
        return -1;
    int size = 0;
    struct list_head *node;
    list_for_each (node, head)
        size++;
    return size;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/

    if (!head || list_empty(head))
        return false;

    struct list_head *fast = head, *slow = head;
    do {
        fast = fast->next->next;
        slow = slow->next;
    } while (fast->prev != head && fast != head);

    list_del(slow);
    q_release_element(list_entry(slow, element_t, list));

    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/

    if (!head)
        return false;

    element_t *entry, *next, *temp;
    list_for_each_entry_safe (entry, next, head, list) {
        if (&next->list != head && strcmp(entry->value, next->value) == 0) {
            do {
                temp = next;
                next = list_entry(next->list.next, element_t, list);
                q_release_element(temp);
            } while (&next->list != head &&
                     strcmp(entry->value, next->value) == 0);

            entry->list.prev->next = &next->list;
            next->list.prev = entry->list.prev;

            q_release_element(entry);
        }
    }

    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/

    if (!head)
        return;

    struct list_head *node = head->next, *next = node->next, *prev = node->prev,
                     *nextnext = next->next;
    while (node != head && next != head) {
        next->next = node;
        next->prev = prev;
        node->next = nextnext;
        node->prev = next;

        prev->next = next;
        nextnext->prev = node;

        node = nextnext;
        next = node->next;
        nextnext = next->next;
        prev = node->prev;
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;

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

    if (!head || list_empty(head) || k <= 0)
        return;

    struct list_head *node = head->next, *safe;
    for (int n = q_size(head); n >= k; n -= k) {
        LIST_HEAD(stack);

        for (int i = 0; i < k; ++i) {
            safe = node->next;
            list_del(node);
            list_add(node, &stack);
            node = safe;
        }

        list_splice_tail(&stack, node);
    }
}

int q_remove_special(struct list_head *head, int descend)
{
    if (!head || list_empty(head))
        return 0;

    element_t *entry = list_entry(head->prev, element_t, list),
              *safe = list_entry(entry->list.prev, element_t, list);
    char *best = entry->value;
    while (&entry->list != head) {
        if (strcmp(entry->value, best) * (2 * descend - 1) < 0) {
            list_del(&entry->list);
            q_release_element(entry);
        } else {
            best = entry->value;
        }

        entry = safe;
        safe = list_entry(entry->list.prev, element_t, list);
    }

    return q_size(head);
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/

    return q_remove_special(head, 0);
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/

    return q_remove_special(head, 1);
}

static inline void l_merge(struct list_head *head,
                           struct list_head *left,
                           struct list_head *right,
                           int descend)
{
    while (!list_empty(left) && !list_empty(right)) {
        if (strcmp(list_entry(left->next, element_t, list)->value,
                   list_entry(right->next, element_t, list)->value) *
                (1 - 2 * descend) <
            0) {
            list_move_tail(left->next, head);
        } else {
            list_move_tail(right->next, head);
        }
    }

    if (!list_empty(left)) {
        list_splice_tail_init(left, head);
    } else {
        list_splice_tail_init(right, head);
    }
}

/* Sort elements of queue in ascending order */
void q_sort(struct list_head *head, bool descend)
{
    if (!head || q_size(head) < 2)
        return;

    struct list_head *fast = head, *slow = head;
    do {
        fast = fast->next->next;
        slow = slow->next;
    } while (fast != head && fast->next != head);

    LIST_HEAD(left);
    LIST_HEAD(right);
    list_splice_tail_init(head, &right);
    list_cut_position(&left, &right, slow);
    q_sort(&left, descend);
    q_sort(&right, descend);
    l_merge(head, &left, &right, descend);
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    if (!head || list_empty(head))
        return 0;

    int n = q_size(head);
    while (n > 1) {
        struct list_head *fast = head->next, *slow = head->next;
        for (int i = 0; i < n / 2; ++i) {
            LIST_HEAD(temp);
            l_merge(&temp, container_of(fast, queue_contex_t, chain)->q,
                    container_of(fast->next, queue_contex_t, chain)->q,
                    descend);

            list_splice_tail(&temp,
                             container_of(slow, queue_contex_t, chain)->q);

            fast = fast->next->next;
            slow = slow->next;
        }
        if (n & 1)
            list_splice_tail_init(container_of(fast, queue_contex_t, chain)->q,
                                  container_of(slow, queue_contex_t, chain)->q);

        n = (n + 1) / 2;
    }

    return q_size(container_of(head->next, queue_contex_t, chain)->q);
}
