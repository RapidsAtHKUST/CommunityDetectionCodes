function [egonetseeds] = localEgoSeeds(A)

fprintf('Computing local egonets...\n');
data.d = full(sum(A,2));
[data.cond data.cut data.vol data.cc data.t] = triangleclusters(A);

mindegree=5;
minverts = neighborhoodmin(A,data.cond);
minverts = minverts(data.d(minverts)>=mindegree);
egonetseeds = minverts;

end

