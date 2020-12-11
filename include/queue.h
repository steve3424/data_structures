#ifndef QUEUE_H
#include <stdbool.h>
/*
* Circular queue.
* Elements are inserted into the back and removed from the front.
* Back index points to the place the next element will be put (one after the last element)
* Front index points to the next element to be removed (the first element)
*/

typedef struct QueueInt {
	int capacity;
	int size;
	int front;
	int back;
	int* elements;
} QueueInt;

QueueInt CreateQueueInt(int capacity) {
	if(capacity <= 0) {
		printf("Size is 0 or negative...defaulting to 10\n");
		capacity = 10;
	}

	QueueInt q;
	q.capacity = capacity;
	q.size = 0;
	q.front = 0;
	q.back = 0;
	q.elements = (int*)malloc(sizeof(int) * capacity);

	return q;
}

void EnqueueInt(QueueInt* q, int val) {
	if(q == NULL) {
		printf("\nQ is NULL\n");
		return;
	}

	if(q->elements == NULL) {
		printf("\nElements are NULL\n");
		return;
	}

	if(q->size == q->capacity) {
		printf("\nQueue is full\n");
		return;
	}

	q->elements[q->back] = val;
	++q->back;
	q->back %= q->capacity;
	++q->size;
}

int DequeueInt(QueueInt* q, int* error) {
	if(q == NULL) {
		printf("Q is NULL\n");
		*error = 1;
		return 0;
	}

	if(q->elements == NULL) {
		printf("Elements are NULL\n");
		*error = 1;
		return 0;
	}

	if(q->size == 0) {
		printf("Queue is empty\n");
		*error = 1;
		return 0;
	}
	
	int ret = q->elements[q->front];
	++q->front;
	q->front %= q->capacity;
	--q->size;

	*error = 0;
	return ret;
}

int PeekFrontQueueInt(QueueInt* q, int* error) {
	if(q == NULL) {
		printf("Q is NULL\n");
		*error = 1;
		return 0;
	}

	if(q->elements == NULL) {
		printf("Elements are NULL\n");
		*error = 1;
		return 0;
	}

	if(q->size == 0) {
		printf("Queue is empty\n");
		*error = 1;
		return 0;
	}

	*error = 0;
	return q->elements[q->front];
}

int PeekBackQueueInt(QueueInt* q, int* error) {
	if(q == NULL) {
		printf("Q is NULL\n");
		*error = 1;
		return 0;
	}

	if(q->elements == NULL) {
		printf("Elements are NULL\n");
		*error = 1;
		return 0;
	}

	if(q->size == 0) {
		printf("Queue is empty\n");
		*error = 1;
		return 0;
	}

	int new_back = q->back - 1;
	if(new_back < 0) {
		new_back = q->size - 1;
	}

	*error = 0;
	return q->elements[new_back];
}

void EmptyQueueInt(QueueInt* q) {
	if(q == NULL) {
		printf("Q is NULL\n");
		return;
	}

	q->size = 0;
	q->front = 0;
	q->back = 0;
}

void DeleteQueueInt(QueueInt* q) {
	if(q == NULL) {
		printf("Q is NULL\n");
		return;
	}

	if(q->elements == NULL) {
		printf("Elements are NULL\n");
		return;
	}

	q->size = 0;
	q->front = 0;
	q->back = 0;
	free(q->elements);
	q->elements = NULL;
}

void ResizeQueueInt(QueueInt* q, int capacity) {
	if(q == NULL) {
		printf("Q is NULL\n");
		return;
	}

	if(q->elements == NULL) {
		printf("elements are null...\n");
		return;
	}

	if(capacity <= q->size) {
		printf("There are too many elements already in the queue. Can't resize...\n");
		return;
	}

	int* temp_ptr = (int*)malloc(sizeof(int) * capacity);
	int j = q->front;
	for(int i = 0; i < q->size; ++i) {
		temp_ptr[i] = q->elements[j];
		++j;
		j %= q->capacity;
	}

	q->capacity = capacity;
	q->front = 0;
	q->back = q->size;
	free(q->elements);
	q->elements = temp_ptr;
}

void PrintQueueInt(const QueueInt* q) {
	if(q == NULL) {
		printf("Q is NULL\n");
		return;
	}

	if(q->elements == NULL) {
		printf("Elements are NULL\n");
		return;
	}

	printf("--------------------\n");
	printf("Capacity: %d\n", q->capacity);
	printf("Size: %d\n", q->size);
	printf(">>> ");
	if(q->size == 0) {
		printf("[queue empty]\n");
	}
	else {
		bool is_wrapped = (q->back <= q->front);
		int i;
		printf("[");
		for(i = 0; i < q->capacity - 1; ++i) {
			if(i == q->front) {
				printf("|f|");
			}
			if(i == q->back) {
				printf("|b|");
			}

			if(is_wrapped) {
				if(i < q->back || q->front <= i) {
					printf("%d, ", q->elements[i]);
				}
				else {
					printf("--, ");
				}
			}
			else {
				if(q->front <= i && i < q->back) {
					printf("%d, ", q-> elements[i]);
				}
				else {
					printf("--, ");
				}
			}
		}

		if(i == q->front) {
			printf("|f|");
		}
		if(i == q->back) {
			printf("|b|");
		}
		if(is_wrapped) {
			if(i < q->back || q->front <= i) {
				printf("%d", q->elements[i]);
			}
			else {
				printf("--");
			}
		}
		else {
			if(q->front <= i && i < q->back) {
				printf("%d", q-> elements[i]);
			}
			else {
				printf("--");
			}
		}
		printf("]\n\n");
	}
}

INTERNAL bool IndexIsInQueue(QueueInt* q, int i) {
	bool is_wrapped = (q->back <= q->front);
	if(q->size == 0) {
		return false;
	}
	else if(is_wrapped) {
		return i < q->back || q->front <= i;
	}
	else {
		return q->front <= i && i < q->back;
	}
}

#define QUEUE_H
#endif
