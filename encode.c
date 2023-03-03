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

void writeFrequency(FILE *frequency, int *counts) {
	for(int i = 0; i < 26; i++) {
		if(counts[i] > 0) fprintf(frequency, "Character: %c, Frequency: %d\n", i + 97, counts[i]);
	}
	for(int i = 26; i < 36; i++) {
		if(counts[i] > 0) fprintf(frequency, "Character: %c, Frequency: %d\n", i + 22, counts[i]);
	}
	if(counts[36] > 0) fprintf(frequency, "Character: %c, Frequency: %d\n", 'Z', counts[36]); //need char other than space for space to read in decode properly
	if(counts[37] > 0) fprintf(frequency, "Character: %c, Frequency: %d\n", 44, counts[37]);
	if(counts[38] > 0) fprintf(frequency, "Character: %c, Frequency: %d\n", 46, counts[38]);
}

void writeCodes(FILE *codes, pQueue *q, int *code, int len, codeList *codesList) {
	if(q -> left != NULL) {
		code[len] = 0;
		writeCodes(codes, q -> left, code, len + 1, codesList);
	}
	if(q -> right != NULL) {
		code[len] = 1;
		writeCodes(codes, q -> right, code, len + 1, codesList);
	}
	if(q -> c != '!') {
		if(q -> c == ' ') fprintf(codes, "Character: %c(Z), Code: ", q -> c); //fix to make space work
		else fprintf(codes, "Character: %c, Code: ", q -> c);
		codesList[characters].c = q -> c;
		for(int i = 0; i < len; i++) {
			fprintf(codes, "%d", code[i]);
			codesList[characters].code[i] = code[i];
		}
		codesList[characters].code[len] = 2;
		characters++;
		fprintf(codes, "\n");
	}
}

void compressFile(FILE *compressed, FILE *input, pQueue *q, codeList *codesList) {
	//unsigned char test = 0b00000000; //binary 8 bit (1 byte)
	//fwrite(&test, sizeof(test), 1, compressed); //write from var test, which is size test, 1 time, at file compressed
	char c;
	unsigned char code = 0;
	int length = 1;
	int byte = 0;
	rewind(input);
	while(!feof(input)) {
		c = fgetc(input);
		if(feof(input)) break; //end count chars on file end
		if((c <= 90 && c >= 65) || (c <= 122 && c >= 97) || (c <= 57 && c >= 48) || c == 32 || c == 44 || c == 46 || c == 10) {
			if(c <= 90 && c >= 65) c = c + 32; //convert uppercase to lowercase
			if(c == 10) c = 32; //newline to space
			int i;
			for(i = 0; codesList[i].c != c; i++) {};
			for(int j = 0; codesList[i].code[j] < 2; j++) {
				if(codesList[i].code[j] == 1) code = code + 1;
				//printf("code bin: %u, char: %c, code dec: %d\n", code, codesList[i].c, codesList[i].code[j]);
				if(length == 8) {
					fwrite(&code, sizeof(code), 1, compressed);
					//printf("Wrote: %u, byte: %d\n", code, ++byte);
					code = 0;
					length = 1;
				}
				else {
					code = code * 2;
					length++;
				}
			}
		}
	}
	if(length != 1) {
		code = code << (8 - length);
		fwrite(&code, sizeof(code), 1, compressed);
		//printf("Wrote: %u, byte: %d\n", code, ++byte);
	}
}

void findSize(FILE *treeOutput, int *counts, codeList *codesList) {
	int size = 0;
	for(int i = 0; i < 39; i++) size += counts[i];
	fprintf(treeOutput, "%d", size);
}

int main(int argc, char *argv[]) {
	if(argc < 2) {
		printf("No input file passed. Please do so from the command line. Aborting...\n");
		return -1;
	}
	FILE *input = fopen(argv[1], "r");
	if(!input) {
		printf("Failed to find input file. Aborting...\n");
		return -1;
	}
	int *counts = (int *)malloc(sizeof(int) * 39); //initialize counter for chars
	for(int i = 0; i < 39; i++) {
		counts[i] = 0; //set count of each char to 0
		//0-25 = lowercase letters, 26-35 = digits, 36 = space, 47 = comma, 48 = period
	}char c;
	while(!feof(input)) {
		c = fgetc(input);
		if(feof(input)) break; //end count chars on file end
		//printf("char read: (%d)%c\n", c, c);
		if(c <= 90 && c >= 65) { //uppercase
			counts[c - 65]++;
		}
		else if(c <= 122 && c >= 97) { //lowercase
			counts[c - 97]++;
		}
		else if(c <= 57 && c >= 48) { //digits
			counts[c - 22]++;
		}
		else if(c == 32 || c == 10) { //space, newline
			counts[36]++;
		}
		else if(c == 44) { //comma
			counts[37]++;
		}
		else if(c == 46) { //period
			counts[38]++;
		}
	}
	//printCounts(counts);
	pQueue *q = NULL;
	for(int i = 0; i < 39; i++) {
		if(counts[i] > 0) {
			int offset = 0;
			if(i <= 25 && i >= 0) { //lowercase
				offset = i + 97;
			}
			else if(i <= 35 && i >= 26) { //digits
				offset = i + 22;
			}
			else if(i == 36) { //space
				offset = 32;
			}
			else if(i == 37) { //comma
				offset = 44;
			}
			else if(i == 38) { //period
				offset = 46;
			}
			q = insertQNode(q, createQNode(offset, counts[i]));
		}
	}
	FILE *frequency = fopen("frequency.txt", "w");
	writeFrequency(frequency, counts);
	fclose(frequency);
	//printQueue(q);
	q = makeTree(q);
	FILE *codes = fopen("codes.txt", "w");
	int code[39];
	codeList *codesList = (codeList *)malloc(sizeof(codeList) * (characters + 1));
	characters = 0;
	writeCodes(codes, q, code, 0, codesList);
	fclose(codes);
	//printQueue(q);
	//printTree(q, NULL);
	FILE *compressed = fopen("compressed.bin", "wb");
	compressFile(compressed, input, q, codesList);
	fclose(compressed);
	fclose(input);
	FILE *treeOutput = fopen("tree.txt", "w");
	findSize(treeOutput, counts, codesList);
	fclose(treeOutput);
	printf("Created encoded file successfully\n");
	return 0;
}