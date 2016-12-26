/*
 * Copyright 1997, Regents of the University of Minnesota
 *
 * debug.c
 *
 * This file contains code that performs self debuging
 *
 * Started 7/24/97
 * George
 *
 * $Id: debug.c,v 1.1 1998/11/27 17:59:13 karypis Exp $
 *
 */

#include <metis.h>

/*************************************************************************
* This function computes the ratio assoc. given the graph and a where vector
**************************************************************************/
float ComputeRAsso(GraphType *graph, idxtype *where, int npart)
{
  int i, j, cm, nvtxs;
  idxtype *rasso, *clusterSize, *xadj, *adjncy;
  float result;
  idxtype * adjwgt;

  rasso = idxsmalloc(npart, 0, "ComputeNCut: ncut");
  clusterSize = idxsmalloc(npart, 0, "ComputeNCut: degree");
  nvtxs = graph->nvtxs;
  xadj = graph->xadj;
  adjncy = graph->adjncy;
  adjwgt = graph->adjwgt;

  for (i=0; i<nvtxs; i++)
    clusterSize[where[i]] ++;
  
  if (graph->adjwgt == NULL) {
    for (i=0; i<nvtxs; i++) {
      cm = where[i];
      for (j=xadj[i]; j<xadj[i+1]; j++)
	if (cm == where[adjncy[j]])
	  rasso[where[adjncy[j]]] ++;
    }
  }
  else {
    for (i=0; i<nvtxs; i++){
      cm = where[i];
      for (j=xadj[i]; j<xadj[i+1]; j++)
	if (cm == where[adjncy[j]])
	  rasso[where[adjncy[j]]] += adjwgt[j];
    }
  }
    
  result =0;
  for (i=0; i<npart; i++){
    if (clusterSize[i] >0)
      result +=  rasso[i] *1.0/ clusterSize[i];
  }
  free(rasso);
  free(clusterSize);
  return result;
}

/*************************************************************************
* This function computes the normalized cut given the graph and a where vector
**************************************************************************/
float ComputeNCut(GraphType *graph, idxtype *where, int npart)
{
  int i, j, cm, nvtxs;
  idxtype *ncut, *degree, *xadj, *adjncy;
  float result;
  idxtype * adjwgt;

  ncut = idxsmalloc(npart, 0, "ComputeNCut: ncut");
  degree = idxsmalloc(npart, 0, "ComputeNCut: degree");
  nvtxs = graph->nvtxs;
  xadj = graph->xadj;
  adjncy = graph->adjncy;
  adjwgt = graph->adjwgt;

  if (graph->adjwgt == NULL) {
    for (i=0; i<nvtxs; i++) {
      cm = where[i];
      for (j=xadj[i]; j<xadj[i+1]; j++){
       	degree[cm] ++; 
        if (cm != where[adjncy[j]])
          ncut[cm] ++;
      }
    }
  }
  else {
    for (i=0; i<nvtxs; i++) {
      cm = where[i];
      for (j=xadj[i]; j<xadj[i+1]; j++){
	degree[cm] += adjwgt[j];
        if (cm != where[adjncy[j]])
          ncut[cm] += adjwgt[j];
      }
    }
  }
  int empty = 0;
  result =0;
  for (i=0; i<npart; i++){
    if (degree[i] == 0)
      empty++;
    if (degree[i] >0)
      result +=  ncut[i] *1.0/ degree[i];
  }
  //printf("Empty clusters: %d\n", empty);
  free(ncut);
  free(degree);
  return result+empty;
}


/*************************************************************************
* This function computes the cut given the graph and a where vector
**************************************************************************/
int ComputeCut(GraphType *graph, idxtype *where)
{
  int i, j, cut;

  if (graph->adjwgt == NULL) {
    for (cut=0, i=0; i<graph->nvtxs; i++) {
      for (j=graph->xadj[i]; j<graph->xadj[i+1]; j++)
        if (where[i] != where[graph->adjncy[j]])
          cut++;
    }
  }
  else {
    for (cut=0, i=0; i<graph->nvtxs; i++) {
      for (j=graph->xadj[i]; j<graph->xadj[i+1]; j++)
        if (where[i] != where[graph->adjncy[j]])
          cut += graph->adjwgt[j];
    }
  }

  return cut/2;
}


/*************************************************************************
* This function checks whether or not the boundary information is correct
**************************************************************************/
int CheckBnd(GraphType *graph) 
{
  int i, j, nvtxs, nbnd;
  idxtype *xadj, *adjncy, *where, *bndptr, *bndind;

  nvtxs = graph->nvtxs;
  xadj = graph->xadj;
  adjncy = graph->adjncy;
  where = graph->where;
  bndptr = graph->bndptr;
  bndind = graph->bndind;

  for (nbnd=0, i=0; i<nvtxs; i++) {
    if (xadj[i+1]-xadj[i] == 0)
      nbnd++;   /* Islands are considered to be boundary vertices */

    for (j=xadj[i]; j<xadj[i+1]; j++) {
      if (where[i] != where[adjncy[j]]) {
        nbnd++;
        ASSERT(bndptr[i] != -1);
        ASSERT(bndind[bndptr[i]] == i);
        break;
      }
    }
  }

  ASSERTP(nbnd == graph->nbnd, ("%d %d\n", nbnd, graph->nbnd));

  return 1;
}



/*************************************************************************
* This function checks whether or not the boundary information is correct
**************************************************************************/
int CheckBnd2(GraphType *graph) 
{
  int i, j, nvtxs, nbnd, id, ed;
  idxtype *xadj, *adjncy, *where, *bndptr, *bndind;

  nvtxs = graph->nvtxs;
  xadj = graph->xadj;
  adjncy = graph->adjncy;
  where = graph->where;
  bndptr = graph->bndptr;
  bndind = graph->bndind;

  for (nbnd=0, i=0; i<nvtxs; i++) {
    id = ed = 0;
    for (j=xadj[i]; j<xadj[i+1]; j++) {
      if (where[i] != where[adjncy[j]]) 
        ed += graph->adjwgt[j];
      else
        id += graph->adjwgt[j];
    }
    if (ed - id >= 0 && xadj[i] < xadj[i+1]) {
      nbnd++;
      ASSERTP(bndptr[i] != -1, ("%d %d %d\n", i, id, ed));
      ASSERT(bndind[bndptr[i]] == i);
    }
  }

  ASSERTP(nbnd == graph->nbnd, ("%d %d\n", nbnd, graph->nbnd));

  return 1;
}

/*************************************************************************
* This function checks whether or not the boundary information is correct
**************************************************************************/
int CheckNodeBnd(GraphType *graph, int onbnd) 
{
  int i, j, nvtxs, nbnd;
  idxtype *xadj, *adjncy, *where, *bndptr, *bndind;

  nvtxs = graph->nvtxs;
  xadj = graph->xadj;
  adjncy = graph->adjncy;
  where = graph->where;
  bndptr = graph->bndptr;
  bndind = graph->bndind;

  for (nbnd=0, i=0; i<nvtxs; i++) {
    if (where[i] == 2) 
      nbnd++;   
  }

  ASSERTP(nbnd == onbnd, ("%d %d\n", nbnd, onbnd));

  for (i=0; i<nvtxs; i++) {
    if (where[i] != 2) {
      ASSERTP(bndptr[i] == -1, ("%d %d\n", i, bndptr[i]));
    }
    else {
      ASSERTP(bndptr[i] != -1, ("%d %d\n", i, bndptr[i]));
    }
  }

  return 1;
}



/*************************************************************************
* This function checks whether or not the rinfo of a vertex is consistent
**************************************************************************/
int CheckRInfo(RInfoType *rinfo)
{
  int i, j;

  for (i=0; i<rinfo->ndegrees; i++) {
    for (j=i+1; j<rinfo->ndegrees; j++)
      ASSERTP(rinfo->edegrees[i].pid != rinfo->edegrees[j].pid, ("%d %d %d %d\n", i, j, rinfo->edegrees[i].pid, rinfo->edegrees[j].pid));
  }

  return 1;
}



/*************************************************************************
* This function checks the correctness of the NodeFM data structures
**************************************************************************/
int CheckNodePartitionParams(GraphType *graph)
{
  int i, j, k, l, nvtxs, me, other;
  idxtype *xadj, *adjncy, *adjwgt, *vwgt, *where;
  idxtype edegrees[2], pwgts[3];

  nvtxs = graph->nvtxs;
  xadj = graph->xadj;
  vwgt = graph->vwgt;
  adjncy = graph->adjncy;
  adjwgt = graph->adjwgt;

  where = graph->where;

  /*------------------------------------------------------------
  / Compute now the separator external degrees
  /------------------------------------------------------------*/
  pwgts[0] = pwgts[1] = pwgts[2] = 0;
  for (i=0; i<nvtxs; i++) {
    me = where[i];
    pwgts[me] += vwgt[i];

    if (me == 2) { /* If it is on the separator do some computations */
      edegrees[0] = edegrees[1] = 0;

      for (j=xadj[i]; j<xadj[i+1]; j++) {
        other = where[adjncy[j]];
        if (other != 2)
          edegrees[other] += vwgt[adjncy[j]];
      }
      if (edegrees[0] != graph->nrinfo[i].edegrees[0] || edegrees[1] != graph->nrinfo[i].edegrees[1]) {
        printf("Something wrong with edegrees: %d %d %d %d %d\n", i, edegrees[0], edegrees[1], graph->nrinfo[i].edegrees[0], graph->nrinfo[i].edegrees[1]);
        return 0;
      }
    }
  }

  if (pwgts[0] != graph->pwgts[0] || pwgts[1] != graph->pwgts[1] || pwgts[2] != graph->pwgts[2])
    printf("Something wrong with part-weights: %d %d %d %d %d %d\n", pwgts[0], pwgts[1], pwgts[2], graph->pwgts[0], graph->pwgts[1], graph->pwgts[2]);

  return 1;
}


/*************************************************************************
* This function checks if the separator is indeed a separator
**************************************************************************/
int IsSeparable(GraphType *graph)
{
  int i, j, nvtxs, other;
  idxtype *xadj, *adjncy, *where;

  nvtxs = graph->nvtxs;
  xadj = graph->xadj;
  adjncy = graph->adjncy;
  where = graph->where;

  for (i=0; i<nvtxs; i++) {
    if (where[i] == 2)
      continue;
    other = (where[i]+1)%2;
    for (j=xadj[i]; j<xadj[i+1]; j++) {
      ASSERTP(where[adjncy[j]] != other, ("%d %d %d %d %d %d\n", i, where[i], adjncy[j], where[adjncy[j]], xadj[i+1]-xadj[i], xadj[adjncy[j]+1]-xadj[adjncy[j]]));
    }
  }

  return 1;
}







void ComputeCenterNode(GraphType *graph, int nparts, idxtype * where, char *fname){
  // w is the weights

  int nvtxs, nbnd, nedges, me, i, j, k, s;
  idxtype *squared_sum, *sum, *xadj, *adjncy, *adjwgt;
  idxtype *w, *center;
  float *inv_sum, *squared_inv_sum;
  idxtype *linearTerm;
  float sigma = 1.0;
  float min_dist=10000.0;
  float dist;
  int min_node = 0;
  int clusterSize;
  float *distance2center;

  nedges = graph->nedges;
  nvtxs = graph->nvtxs;
  xadj = graph->xadj;
  adjncy = graph->adjncy;
  adjwgt = graph->adjwgt;

  w = idxsmalloc(nvtxs, 0, "pingpong: weight");
  if (adjwgt == NULL){
      for (i=0; i<nvtxs; i++)
	for (j=xadj[i]; j<xadj[i+1]; j++) 
	  w[i] ++;
	//printf("adjwgt == NULL\n");
  }else{
      for (i=0; i<nvtxs; i++)
	for (j=xadj[i]; j<xadj[i+1]; j++) 
	  w[i] += adjwgt[j];
  }
  
  sum = idxsmalloc(nparts,0, "Weighted_kernel_k_means: weight sum");
  inv_sum = fmalloc(nparts, "Weighted_kernel_k_means: sum inverse"); 
  squared_inv_sum = fmalloc(nparts, "Weighted_kernel_k_means: squared sum inverse"); 
  squared_sum = idxsmalloc(nparts,0, "Weighted_kernel_k_means: weight squared sum");

  for (i=0; i<nvtxs; i++){
    sum[where[i]] += w[i]; // w(V_c)
  }

  for (i=0; i<nparts; i++)
    if(sum[i] >0){
      inv_sum[i] = 1.0/sum[i];
      squared_inv_sum[i] = inv_sum[i]*inv_sum[i]; // 1/(w(V_c))^2
    }
    else
      inv_sum[i] = squared_inv_sum[i] = 0;

  for (i=0; i<nvtxs; i++){
    me = where[i];
    for (j=xadj[i]; j<xadj[i+1]; j++) 
      if (where[adjncy[j]] == me){
	//squared_sum[me] += adjwgt[j]; // links(V_c, V_c)
	squared_sum[me] += 1; // links(V_c, V_c)
	}
  }

  linearTerm = idxmalloc(nparts, "Weighted_kernel_k_means: linearTerm");

  printf("\n\n##### Print Cluster Constants ##### \n");
  for(k=0; k<nparts; k++){
	printf("cluster: %d, w(V_c): %d, 1/(w(V_c))^2: %1.20f, links(V_c, V_c): %d\n", k+1, sum[k], squared_inv_sum[k], squared_sum[k]);
  }

  printf("\n\n##### ComputeCenterNode ##### \n");  
  center = idxsmalloc(nparts, 0, "center");
  distance2center = fmalloc(nvtxs, "Weighted_kernel_k_means: distance to center");
  for (k=0; k<nparts; k++){ // for each cluster

	min_dist = 10000.0;
	min_node = 0;
	clusterSize=0;
	for (i=0; i<nvtxs; i++){ // scan nodes
		if(where[i]==k){
			me = k;
			clusterSize++;
			if(w[i] > 0){
				float inv_wi=1.0/w[i];
				for (s=0; s<nparts; s++)
 	  				linearTerm[s] = 0;
        			for (j=xadj[i]; j<xadj[i+1]; j++)
	  				//linearTerm[where[adjncy[j]]] += adjwgt[j]; // links(v_i, V_c)
					linearTerm[where[adjncy[j]]] += 1; // links(v_i, V_c)

				dist = squared_sum[me]*squared_inv_sum[me] -2*inv_wi*linearTerm[me]*inv_sum[me] + sigma/w[i] - sigma/sum[me];
				distance2center[i] = dist;
				//printf("node: %d, distance: %f, cluster: %d\n",i+1,dist,me+1);
				if(dist<min_dist){
					min_dist = dist;
					min_node = i;
				}

			}else{
				printf("\n WARNING!!! w[i] does not greater than 0 \n");
			}
			
		} // if the node i is in the cluster k
	} // finish scanning all the nodes

	printf("*** cluster: %d, cluster size: %d, center node: %d\n", k+1, clusterSize, min_node+1);
	center[k]=min_node+1;
  }

  ComputeAllDistance(graph, nparts, where, center);
  WriteCenterNodes(fname, center, nparts);
  WriteDistances(fname, distance2center, nparts, nvtxs);

  free(w); free(sum); free(squared_sum); free(linearTerm); free(inv_sum); free(squared_inv_sum); free(center); free(distance2center);
}



void WriteCenterNodes(char *fname, idxtype *center, int nparts)
{
  FILE *fpout;
  int i;
  char filename[256];
  char extracted[256];

  extractfilename(fname, extracted);
  sprintf(filename,"%s.part.%d.seeds", extracted, nparts);

  if ((fpout = fopen(filename, "w")) == NULL) 
    errexit("Problems in opening the partition file: %s", filename);

  for (i=0; i<nparts; i++)
    fprintf(fpout,"%d\n",center[i]);

  fclose(fpout);

}


void WriteDistances(char *fname, float *distance2center, int nparts, int nvtxs)
{
  FILE *fpout;
  int i;
  char filename[256];
  char extracted[256];

  extractfilename(fname, extracted);
  sprintf(filename,"%s.part.%d.distances", extracted, nparts);

  if ((fpout = fopen(filename, "w")) == NULL) 
    errexit("Problems in opening the partition file: %s", filename);

  for (i=0; i<nvtxs; i++)
    fprintf(fpout,"%f\n",distance2center[i]);

  fclose(fpout);

}



void ComputeAllDistance(GraphType *graph, int nparts, idxtype * where, idxtype *center){
  // w is the weights

  int nvtxs, nbnd, nedges, me, i, j, k, s;
  idxtype *squared_sum, *sum, *xadj, *adjncy, *adjwgt;
  idxtype *w;
  float *inv_sum, *squared_inv_sum;
  idxtype *linearTerm;
  float sigma = 1.0;
  float min_dist=10000.0;
  float dist;
  int min_node = 0;
  int clusterSize;

  nedges = graph->nedges;
  nvtxs = graph->nvtxs;
  xadj = graph->xadj;
  adjncy = graph->adjncy;
  adjwgt = graph->adjwgt;

  w = idxsmalloc(nvtxs, 0, "pingpong: weight");
  if (adjwgt == NULL){
      for (i=0; i<nvtxs; i++)
	for (j=xadj[i]; j<xadj[i+1]; j++) 
	  w[i] ++;
	//printf("adjwgt == NULL\n");
  }else{
      for (i=0; i<nvtxs; i++)
	for (j=xadj[i]; j<xadj[i+1]; j++) 
	  w[i] += adjwgt[j];
  }
  
  sum = idxsmalloc(nparts,0, "Weighted_kernel_k_means: weight sum");
  inv_sum = fmalloc(nparts, "Weighted_kernel_k_means: sum inverse"); 
  squared_inv_sum = fmalloc(nparts, "Weighted_kernel_k_means: squared sum inverse"); 
  squared_sum = idxsmalloc(nparts,0, "Weighted_kernel_k_means: weight squared sum");

  for (i=0; i<nvtxs; i++){
    sum[where[i]] += w[i]; // w(V_c)
  }

  for (i=0; i<nparts; i++)
    if(sum[i] >0){
      inv_sum[i] = 1.0/sum[i];
      squared_inv_sum[i] = inv_sum[i]*inv_sum[i]; // 1/(w(V_c))^2
    }
    else
      inv_sum[i] = squared_inv_sum[i] = 0;

  for (i=0; i<nvtxs; i++){
    me = where[i];
    for (j=xadj[i]; j<xadj[i+1]; j++) 
      if (where[adjncy[j]] == me){
	//squared_sum[me] += adjwgt[j]; // links(V_c, V_c)
	squared_sum[me] += 1; // links(V_c, V_c)
	}
  }

  linearTerm = idxmalloc(nparts, "Weighted_kernel_k_means: linearTerm");

  printf("\n\n##### Print Cluster Constants ##### \n");
  for(k=0; k<nparts; k++){
	printf("cluster: %d, w(V_c): %d, 1/(w(V_c))^2: %f, links(V_c, V_c): %d\n", k+1, sum[k], squared_inv_sum[k], squared_sum[k]);
  }

  //printf("%d %d\n",center[0],center[1]);
  for(k=0; k<2; k++){
	center[k] = center[k]-1;
  }
  //printf("%d %d\n",center[0],center[1]);
  printf("\n\n");
  for (i=0; i<nvtxs; i++){ // scan nodes
  	float dist1;
	float dist2;
	float s_dist1;
	float s_dist2;
	float midTerm1=0.0;
	float midTerm2=0.0;
	
	float inv_wi=1.0/w[i];
	for (s=0; s<nparts; s++)
		linearTerm[s] = 0;
	for (j=xadj[i]; j<xadj[i+1]; j++){
		linearTerm[where[adjncy[j]]] += 1; // links(v_i, V_c)
		for(k=0; k<2; k++){
			if(adjncy[j]==center[k]){ // has link to seed
				if(k==0)
					midTerm1 = 2.0/(w[i]*w[center[k]]);
				else
					midTerm2 = 2.0/(w[i]*w[center[k]]);
			}
		}
	}

	for (k=0; k<2; k++){
		if(where[i]==k){
			dist = squared_sum[k]*squared_inv_sum[k] -2.0*inv_wi*linearTerm[k]*inv_sum[k] + sigma/w[i] - sigma/sum[k];			
		}else{
			dist = squared_sum[k]*squared_inv_sum[k] -2.0*inv_wi*linearTerm[k]*inv_sum[k] + sigma/w[i] + sigma/sum[k];
		}
		if(k==0){
			dist1=dist; // dist to center
			s_dist1 = sigma/w[i] - midTerm1 + sigma/w[center[k]];
			if(i==center[k]){
				//printf("%d\n",i);
				s_dist1=0;
			}
		}else{
			dist2=dist; // dist to center
			s_dist2 = sigma/w[i] - midTerm2 + sigma/w[center[k]];
			if(i==center[k]){
				//printf("%d\n",i);
				s_dist2=0;
			}
		}
	}
	//printf("%d & %d & %f & %f & %f & %f \\\\ \\hline \n", i+1, where[i]+1, dist1, dist2, s_dist1, s_dist2);
	//if(where[i]==1)
		//printf("%d; ",i+1);
		//printf("%f; ",dist2);
		//printf("%f; ",s_dist2);
  }
  printf("\n");

  free(w); free(sum); free(squared_sum); free(linearTerm); free(inv_sum); free(squared_inv_sum);
}


// added by Joyce
float ComputeWithinEdges(GraphType *graph, int numParts, idxtype* parts){

	int nvtxs = graph->nvtxs;
  	idxtype *xadj = graph->xadj;
  	idxtype *adjncy = graph->adjncy;
  	idxtype *adjwgt = graph->adjwgt;
	int totalEdge=0;
	int withinEdge=0;
	int i, j, cm;

   	for (i=0; i<nvtxs; i++) {
     		cm = parts[i];
     		for (j=xadj[i]; j<xadj[i+1]; j++){
			totalEdge++;
       			if (cm == parts[adjncy[j]])
         			withinEdge++;
     		}
   	}

 	return (float) withinEdge/totalEdge;		
}


// Joyce added this part 
void PrintClusterSizes(GraphType *graph, int numParts, idxtype* parts)
{ 
	int nvtxs = graph->nvtxs;
	int i, k, cm, sum, minSize, maxSize;
	idxtype *clusterSize = idxsmalloc(numParts, 0, "Cluster size");

	for (i=0; i<nvtxs; i++) {
     		cm = parts[i];
     		clusterSize[cm] = clusterSize[cm]+1; 
   	}
		
	sum=0;
	minSize=clusterSize[0];
	maxSize=clusterSize[0];
	for(k=0; k<numParts; k++){
		//printf("cluster: %d, size: %d\n",k,clusterSize[k]);
		sum=sum+clusterSize[k];
		if(clusterSize[k]<minSize){
			minSize = clusterSize[k];
		}
		if(clusterSize[k]>maxSize){
			maxSize = clusterSize[k];
		}
	}
	printf("*** min cluster size = %d, (%3.2f %)\n", minSize, (float) 100.0*minSize/nvtxs);
	printf("*** max cluster size = %d, (%3.2f %)\n", maxSize, (float) 100.0*maxSize/nvtxs);
	printf("sum: %d", sum);
}

