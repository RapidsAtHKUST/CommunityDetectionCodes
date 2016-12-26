function [distances,distRatio,squared_dist] = ComputeDistances(A, partition)

tic
sigma=1;

if min(partition)==0
    partition=partition+1;
end

m=max(partition);

degree = sum(A,2);
links_Vc = zeros(m,1);
w_Vc = zeros(m,1);
for k=1:m
    ind = find(partition==k);
    select = A(ind,ind);
    links_Vc(k) = sum(sum(select));
    w_Vc(k) = sum(degree(ind));
    fprintf('cluster %d\n',k);
    p=links_Vc(k)/((w_Vc(k))^2);
    p
end

% compute all distances
distances = zeros(k,size(A,1)); % k * nodes
distances = distances - 1;
for i=1:size(A,1) % for each node
    iCluster = partition(i);
    friends = find(A(:,i));
    for k=1:m % for each cluster
        links=sum(partition(friends)==k);
        wi = degree(i);
        if k==iCluster
            distances(k,i) = -(2*links/(wi*w_Vc(k))) + links_Vc(k)/((w_Vc(k))^2) + sigma/wi - sigma/w_Vc(k);
            if i==217584
                first=-(2*links/(wi*w_Vc(k)))
                second=links_Vc(k)/((w_Vc(k))^2)
                third=sigma/wi
                forth=sigma/w_Vc(k)
            end
        else
            distances(k,i) = -(2*links/(wi*w_Vc(k))) + links_Vc(k)/((w_Vc(k))^2) + sigma/wi + sigma/w_Vc(k);
        end
    end
    if mod(i,10000)==0
        fprintf('finish node %d\n',i);
    end
end

% the computed distance is squared distance...
squared_dist=distances;
distances = sqrt(distances);
% for each node, compute d_i/sum(d)
distRatio = zeros(k,size(A,1)); % k * nodes
for i=1:size(A,1)
    dist = distances(:,i);
    sum_dist = sum(dist);
    distRatio(:,i)=dist./sum_dist;
end

% % compute center nodes
% center = zeros(m,1);
% for k=1:m
%     nodes = find(partition==k);
%     n=length(nodes);
%     distance = zeros(n,1);
%     for i=1:n
%         friends = find(A(:,nodes(i)));
%         links=sum(partition(friends)==k);
%         wi = degree(nodes(i));
%         distance(i) = -(2*links/(wi*w_Vc(k))) + links_Vc(k)/((w_Vc(k))^2) + sigma/wi - sigma/w_Vc(k);
%         %fprintf('cluster: %d, node: %d, distance: %f \n',k,nodes(i),distance(i));
%     end
%     %distance
%     if nnz(distance==min(distance))>1
%         fprintf('min distance tie nodes...\n');
%         ties=nodes(distance==min(distance))
%         center(k)=ties(1);
%     else 
%        center(k)=nodes(distance==min(distance)); 
%     end
%     fprintf('cluster: %d, cluster size: %d, center: %d, distance: %f\n',k, n,center(k),min(distance));
% end
toc
end

