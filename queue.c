#include "queue.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static typedef struct queueNode {
    Header header;
    char* data; // Allocate a new buffer on the heap for this!
    bool awaiting_ack;
    struct queueNode* next;
} QueueNode;

static QueueNode* queue = NULL;
static int num_msgs = 0;

void push_msg(MessageType type, int session_id, char* data, int num_bytes) {
    // create node
    QueueNode* node = calloc(sizeof(QueueNode), 1);
    node->header.type = type;
    node->header.length = num_bytes;
    node->header.session_id = session_id;
    node->header.msg_num = ++num_msgs;
    node.data = data;
    node.awaiting_ack = false;
    node.next = NULL;

    // Place at end of queue
    if (queue == NULL) {
        queue = node;
    } else {
        QueueNode* curr = queue;
        while (curr.next != NULL) {
            curr = curr.next
        }
        curr.next = node;
    }
}

void pop_front() {
    // pop front of queue
    QueueNode* node = queue;
    queue = node.next;
    free(node.data);
}

void send_front() {
    // if the front message is not awaiting ack, send, mark awaiting_ack
    send_header();
    queue.awaiting_ack = true;
}