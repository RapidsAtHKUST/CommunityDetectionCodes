function [seeds] = spHubSeeds(G,k)

noNodes = size(G,1);
deg = sum(G,2);
mark = zeros(1,noNodes);
seedsInd = zeros(1,noNodes);
candidates = 1:noNodes;
rounds=0;
noSkips=0;

% [~,order]=sort(deg,'descend');
% for i=1:k
%     seed = order(i);
%     if mark(seed)==0            
%         seedsInd(seed)=1;
%         mark(seed)=1;
%         mark(G(:,seed)>0) = 1; % mark neighbors
%     else
%         noSkips=noSkips+1;
%         continue;
%     end
% end

while sum(mark==zeros(1,noNodes))
    rounds=rounds+1;
    ties = find(deg==max(deg));
    selectedDeg=full(max(deg));
    noTies = length(ties);
    for i=1:noTies
        seed = ties(i);
        if mark(seed)==0            
            seedsInd(seed)=1;
            mark(seed)=1;
            mark(G(:,seed)>0) = 1; % mark neighbors
        else
            noSkips=noSkips+1;
            continue;
        end
    end
    deg(ones(1,noNodes)==mark)=0;
    if nnz(seedsInd)>=k
            %fprintf('*** spHubSeeds... finish rounds... deg: %d\n',selectedDeg);
            break;
    end    
end

seeds = candidates(logical(seedsInd));
%fprintf('--- spHubSeeds... %d seeds selected... %6.4f %% (%d out of %d) marked... %d rounds running... no. of skips: %d\n',...
%    nnz(seedsInd),nnz(mark)/noNodes*100,nnz(mark),noNodes,rounds,noSkips);

end