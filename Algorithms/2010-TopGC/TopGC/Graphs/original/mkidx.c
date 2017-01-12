#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#define NDATA 10000

struct list {
	struct list *next;
	int ndata;
	long data[NDATA];
};

struct list *Nodes[2];
struct list *Edges[2];
long Nnodes= 0;
long Nedges = 0;
int Wts = -1;
int Line = 0;
float Scale = 1.0;

void add(struct list **head, long data);
void dowrite();
void outInt(long data);

main (int argc, char *argv[]) {
	char inbuf[BUFSIZ];
	int nf, ch;
	long node, edgeto, last = -1;
	float wt;
	while ((ch = getopt(argc, argv, "s:")) != -1)
		switch (ch) {
			case 's':
				Scale = atof(optarg);
				break;
			default:
				fprintf(stderr, "usage: %s [-s wt-scale-factor]\n", argv[0]);
				exit(1);
		}
	while (fgets(inbuf, sizeof inbuf, stdin)) {
		nf = sscanf(inbuf, "%ld %ld %g", &node, &edgeto, &wt);
		Line++;
		if (Wts == -1)
			Wts = (nf == 3) ? 1 : 0;
		else if ((2 + Wts) != nf)
			fprintf(stderr, "Bad input, line %d\n", Line);
		if (node < last) {
			fprintf(stderr, "Bad node# %ld expected %ld, line %d\n",
					node, last + 1, Line);
			exit(1);
		}
		while (last < node) {
			add(Nodes, Nedges);
			last++;
			if ((++Nnodes % 1000000) == 0)
				fprintf(stderr, "%ld nodes, %ld edge words\n", Nnodes, Nedges);
		}
		if (edgeto != node) {
			add(Edges, edgeto);
			if (Wts) {
				add(Edges, (int) (wt * Scale));
				Nedges++;
			}
			Nedges++;
		}
	}
	add(Nodes, Nedges);
	fprintf(stderr, "%ld nodes, %ld edge words\n", Nnodes, Nedges);
	dowrite();
	exit(0);
}

void
add(struct list **head, long data) {
	struct list *hp = head[1];
	if ((hp == NULL) || (hp->ndata >= NDATA)) {
		hp = malloc(sizeof (struct list));
		if (hp == NULL) {
			fprintf(stderr, "No memory for list\n");
			exit(1);
		}
		hp->next = NULL;
		hp->ndata = 0;
		if (head[1])
			head[1]->next = hp;
		else
			head[0] = hp;
		head[1] = hp;
	}
	hp->data[hp->ndata++] = data;
}

void dowrite() {
	struct list *hp;
	int i;
	outInt(Nnodes);
	outInt(Nedges);
	outInt(Wts);
	for (hp = Nodes[0]; hp; hp = hp->next)
		for (i = 0; i < hp->ndata; i++)
			outInt(hp->data[i]);
	for (hp = Edges[0]; hp; hp = hp->next)
		for (i = 0; i < hp->ndata; i++)
			outInt(hp->data[i]);
}

void outInt(long data) {
	putchar(data >> 24);
	putchar(data >> 16);
	putchar(data >> 8);
	putchar(data);
}
