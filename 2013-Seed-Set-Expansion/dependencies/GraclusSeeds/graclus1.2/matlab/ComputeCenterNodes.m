function [center] = ComputeCenterNodes(A, partition, minSize)

sigma=1.0;
%fprintf('sigma in ComputeCenterNodes... %f\n',sigma);

if min(partition)==0
    partition=partition+1;
end

m=max(partition);
%fprintf('No. of clusters: %d\n',m);

degree = sum(A,2);
links_Vc = zeros(m,1);
w_Vc = zeros(m,1);
for k=1:m
    ind = find(partition==k);
    select = A(ind,ind);
    links_Vc(k) = sum(sum(select));
    w_Vc(k) = sum(degree(ind));
    %fprintf('compute cluster constant %d\n',k);
end

% compute center nodes
%center = zeros(m,1);
center=[];
for k=1:m
    nodes = find(partition==k);
    n=length(nodes);
    if n > minSize % if the cluster contains more than minSize nodes
        distance = zeros(n,1);
        for i=1:n
            friends = find(A(:,nodes(i)));
            links=sum(partition(friends)==k);
            wi = degree(nodes(i));
            distance(i) = -(2*links/(wi*w_Vc(k))) + links_Vc(k)/((w_Vc(k))^2) + sigma/wi - sigma/w_Vc(k);
            %fprintf('cluster: %d, node: %d, distance: %f \n',k,nodes(i),distance(i));
        end
        center = [center; nodes(distance==min(distance))];
%         %distance
%         if nnz(distance==min(distance))>1
%             %fprintf('min distance tie nodes...\n');
%             ties=nodes(distance==min(distance));
%             center(k)=ties(1);
%         else 
%             center(k)=nodes(distance==min(distance)); 
%         end
        %fprintf('cluster: %d, cluster size: %d, center: %d, distance: %f\n',k, n,center(k),min(distance));
    else % too small cluster
        fprintf('too samll cluster\n');
    end
end

%fprintf('=== [ComputeCenterNodes2] returns %d seeds...\n',length(center));
%fprintf('Computing cluster centers done...\n');

end

