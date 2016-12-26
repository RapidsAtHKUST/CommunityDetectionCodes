function [C_full] = Assign_bi(FL,FL_bicore3,seeddata,pdata)
% written by Joyce on Apr. 30, 2013
% assignment module for bicore graph: just propagate the clustering results

%fprintf('Assignment starts...\n');
org_nodes = size(FL,1);
org_nnz = nnz(FL);

bi3_nodes = size(FL_bicore3,1);
bi3_nnz = nnz(FL_bicore3);

C = seeddata.C'; % clustering of bicore3 (C: no. of clusters x no. of nodes)
numAssigned = nnz(sum(C,1));

% mapping (bicore-org)
% mapvec = find(pdata.two_core_filter);
% mapvec = mapvec(pdata.bicore_filter);
% bo_vmap = mapvec;
bo_vmap = pdata.vid;

k = size(C,1); % no. of clusters
C_full = sparse(k,org_nodes);
C_full(:,bo_vmap) = C;

mark = zeros(org_nodes,1);
mark(bo_vmap) = 1; % nodes in bicore3

% Breadth First Search (from bicore3, expanding to original graph)
%fprintf('***** BFS...\n');
level=0;
new_selected=mark;
%fprintf(' level: %d, no. of selected nodes: %d\n',level,nnz(new_selected));

numNewAssigned = 0;

while nnz(mark)~=org_nodes
    level=level+1;
    surface = FL(:,logical(new_selected)); % neighbours of surface nodes
    new_selected = (sum(surface,2)>=1) - mark; % newly selected nodes in this round
    new_selected = (new_selected==1);
    %fprintf(' level: %d, no. of selected nodes: %d\n',level,nnz(new_selected));
    
    submatrix = FL(:,new_selected);
    neighbours = sum(submatrix,2);
    neighbours = (neighbours>0); % vertices on the original graph    
    
    selected_nodes = find(new_selected);
    random_visit = randperm(length(selected_nodes));
    row_ind=[];
    col_ind=[];
    numAssignedThisLevel = 0;
    for i=1:length(selected_nodes)
        % ordering does not matter anymore
        node = selected_nodes(random_visit(i)); % visit random order
        mark(node) = 1; % mark when visited 
        selected_column = FL(:,node);
        degree = sum(selected_column);
        negibour_partitions = C_full(:,logical(selected_column));
        denominator = sum(negibour_partitions,2);
        numerator = repmat(degree,k,1);
        confScore = denominator./numerator;
        max_conf = max(confScore);
        if max_conf > 0
            assign_clust = find(confScore>0);
            row_ind = [row_ind; assign_clust];
            col_ind = [col_ind; repmat(node,length(assign_clust),1)];
            numNewAssigned = numNewAssigned+1;
            numAssignedThisLevel = numAssignedThisLevel+1;
        end
        if mod(i,10000)==0
            %fprintf('..%d(%2.1f%%)',i,(i/length(selected_nodes))*100);
        end
    end
    
    % delay assignment... now update C_full
    C_full=C_full+sparse(row_ind,col_ind,ones(length(row_ind),1),k,org_nodes);
        
end
numRemainNodes = org_nodes-bi3_nodes;
% fprintf('\n ===== Finally, among %d nodes which were not included in bicore graph, \n',numRemainNodes);
% fprintf(' ===== %d nodes are assigned... (%2.2f%%)\n',numNewAssigned,numNewAssigned/numRemainNodes*100);
% totalAssigned = numAssigned+numNewAssigned;
% fprintf(' ===== Overall, %d nodes are assigned to some clusters... (%2.2f%%)\n',totalAssigned,totalAssigned/org_nodes*100);
% fprintf('Assignment done...\n');

end

