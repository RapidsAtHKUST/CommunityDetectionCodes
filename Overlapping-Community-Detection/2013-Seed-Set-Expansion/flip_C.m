function [flipped_C] = flip_C(C,A)

flipped_C=C;
noNodes = size(A,1);
noClusters = size(C,2);
cluster_size = sum(C,1);
noFlipped=0;
for i=1:noClusters
    if cluster_size(i)>noNodes*0.5
        flipped_C(:,i) = double( C(:,i)==zeros(noNodes,1) );
        noFlipped=noFlipped+1;
    end
end

%fprintf('-No. of flipped clusters: %d out of %d\n',noFlipped,noClusters);

end

