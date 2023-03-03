#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_LINE 1024

int characters = 0;

typedef struct codes {
	char c;
	int code[39];
} codeList;

typedef struct queue {
	char c;
	int count;
	struct queue *next;
	struct queue *left;
	struct queue *right;
} pQueue;

void printTree(pQueue *q, pQueue *parent) { //preOrder
	if(q == NULL) return;
	if(parent == NULL) printf("Tree; Char %c with count %d (Parent: node is root) \n", q -> c, q -> count);
	else printf("Tree; Char %c with count %d (Parent: Char %c with count %d) \n", q -> c, q -> count, parent -> c, parent -> count);
	printTree(q -> left, q);
	printTree(q -> right, q);
}

void printQueue(pQueue *q) {
	pQueue *temp = q;
	while(temp) {
		printf("pQueue; Char %c with count %d\n", temp -> c, temp -> count);
		temp = temp -> next;
	}
}

pQueue *createQNode(char c, int count) {
	pQueue *node = (pQueue *)malloc(sizeof(pQueue));
	node -> c = c;
	node -> count = count;
	node -> next = NULL;
	node -> left = NULL;
	node -> right = NULL;
	//printf("Made node %c with %d\n", node -> c, node -> count);
	return node;
}

pQueue *insertQNode(pQueue *q, pQueue *node) {
	if(q == node) {
		q = q -> next;
		return insertQNode(q, node);
	}
	if(q == NULL) {
		//printf("Inserted as first element of queue\n");
		return node;
	}
	if(q -> next == NULL && q -> count < node -> count) {
		q -> next = node;
		//printf("Inserted as second element of queue\n");
	}
	else {
		pQueue *temp = q;
		while(temp != NULL && temp -> count < node -> count) {
			temp = temp -> next;
		}
		if(temp == q) {
			node -> next = q;
			//printf("Inserted as first element of queue\n");
			return node;
		}
		else {
			pQueue *temp2 = q;
			while(temp2 -> next != temp) temp2 = temp2 -> next;
			node -> next = temp;
			temp2 -> next = node;
			//printf("Inserted after traversal of queue\n");
		}
	}
	return q;
}

pQueue *delete2Root(pQueue *q) {
	if(q == NULL) return q;
	if(q -> next == NULL) {
		pQueue *temp = q -> next;
		return temp;
	}
	pQueue *temp1 = q -> next;
	pQueue *temp2 = temp1 -> next;
	return temp2;
}

pQueue *makeTree(pQueue *q) { //'!' denotes a tree node
	while(q -> next != NULL) { //complete tree
		pQueue *node = createQNode('!', q -> count + q -> next -> count);
		if(q -> c != '!' && q -> next -> c != '!') { //first and second nodes are simple nodes	
			node -> left = createQNode(q -> c, q -> count);
			node -> right = createQNode(q -> next -> c, q -> next -> count);
		}
		else if(q -> c == '!' && q -> next -> c == '!') { //both are trees
			node -> left = q;
			node -> right = q -> next;
		}
		else if(q -> c == '!') { //first node is tree
			node -> left = q;
			node -> right = createQNode(q -> next -> c, q -> next -> count);
		}
		else { //second node is tree
			node -> left = createQNode(q -> c, q -> count);
			node -> right = q -> next;
		}
		q = delete2Root(q);
		q = insertQNode(q, node);
		characters++;
	}
	return q;
}

void printCounts(int *counts) {
	for(int i = 0; i < 26; i++) {
		printf("Counts; Char %c with count %d\n", i + 97, counts[i]);
	}
	for(int i = 26; i < 36; i++) {
		printf("Counts; Char %c with count %d\n", i + 22, counts[i]);
	}
	printf("Counts; Char %c with count %d\n", 32, counts[36]);
	printf("Counts; Char %c with count %d\n", 44, counts[37]);
	printf("Counts; Char %c with count %d\n", 46, counts[38]);
}

void decodeFile(FILE *compressed, FILE *output, FILE *size, pQueue *q) {
	int sized;
	int length = 0;
	int byte = 0;
	fscanf(size, "%d", &sized);
	unsigned char code;
	pQueue *node = q;
	if(q -> c != '!') { //not a tree node, only have 1 char in whole compressed file
		for(; length < sized; length++) fprintf(output, "%c", q -> c);
	}
	while(!feof(compressed) && length != sized) {
		fread(&code, sizeof(code), 1, compressed);
		//printf("code: %u, byte: %d\n", code, ++byte);
		int shifts = 7;
		while(shifts != -1 && length != sized) {
			if((code >> shifts) % 2 == 0) {
				node = node -> left;
			}
			else {
				node = node -> right;
			}
			if(node -> c != '!') {
				fprintf(output, "%c", node -> c);
				node = q;
				length++;
			}
			shifts--;
		}
	}
}

pQueue *readFrequency(FILE *frequency, pQueue *q) {
	char c;
	int freq;
	char line[MAX_LINE];
	fgets(line, sizeof(line), frequency);
	while(!feof(frequency)) {
		sscanf(line, "Character: %c, Frequency: %d", &c, &freq);
		//printf("char: %c, freq: %d\n", c, freq);
		if(c == 'Z') q = insertQNode(q, createQNode(32, freq)); //fix for space char
		else q = insertQNode(q, createQNode(c, freq));
		fgets(line, sizeof(line), frequency);
	}
	return q;
}

int main(int argc, char *argv[]) {
	FILE *frequency = fopen("frequency.txt", "r");
	pQueue *q = NULL;
	q = readFrequency(frequency, q);
	fclose(frequency);
	//printQueue(q);
	q = makeTree(q);
	//printTree(q, NULL);
	//printQueue(q);
	FILE *compressed = fopen("compressed.bin", "rb");
	FILE *output = fopen("decoded.txt", "w");
	FILE *size = fopen("tree.txt", "r");
	decodeFile(compressed, output, size, q);
	fclose(output);
	fclose(size);
	fclose(compressed);
	printf("Created decoded file successfully\n");
	return 0;
}