function [G,xy] = chirikov_operator(Nx,Np,T,k,eta,Nc)

% misc variables
bigN = Nx*Np;
tpi = 2*pi;
xstep = 2*pi/Nx;
pstep = 2*pi/Np;
xscale = Nx/tpi;
pscale = Np/tpi;

% random data for phase adjustment
rp = 2*pi*rand(T,1);

% allocate triplet arrays for P
nlinks = 0;
ei=zeros(20*Nx*Np,1); % generally 2x as much space as needed
ej=ei;
ev=ei;

% setup inline sub2ind
sz = [Nx,Np];
s2i_k = [1 cumprod(sz(1:end-1))];
% compute xy coords
xy = zeros(Nx*Np,2);
for i=1:Nx
    for j=1:Np
        ind = 1 + (i-1)*s2i_k(1);
        ind = ind + (j-1)*s2i_k(2);
        xy(ind,1) = (i-1)*xstep;
        xy(ind,2) = (j-1)*pstep;
    end
end

% allocate a few work arrays
counts = zeros(bigN,1);
used = zeros(bigN,1);
for pj=1:Np
    for xi=1:Nx
        % compute the index for the current point
        ind1 = 1 + (xi-1)*s2i_k(1);
        ind1 = ind1 + (pj-1)*s2i_k(2);
        % reset the used counts set
        curused = 0;
        
        % rand is really expensive, so generate all the random data
        % in one go
        xpstart = rand(2,Nc);
        
        % from xi,pi, generate Nc samples and record data about them
        for ci=1:Nc
            % get the coordinates of a point inside the current cell
            x = xpstart(1,ci)*xstep + (xi-1)*xstep;
            p = xpstart(2,ci)*pstep + (pj-1)*pstep;
            % find it's position after T steps
            for step=1:T
                p = mod(eta*p + k*sin(x+rp(step)),tpi);
                x = mod(x + p,tpi);
            end
            % figure out what bin x,p is in
            xi2 = floor(x*xscale)+1;
            pi2 = floor(p*pscale)+1;
            ind2 = 1 + (xi2-1)*s2i_k(1);
            ind2 = ind2 + (pi2-1)*s2i_k(2);
            if counts(ind2) == 0
                curused = curused + 1;
                used(curused) = ind2;
                counts(ind2) = 1;
            else
                counts(ind2) = counts(ind2)+1;
            end
        end
        
        % add the edges
        if nlinks+curused > length(ei) % check for space, double if required
            ei = [ei; ei]; ej = [ej; ej]; ev = [ev; ev]; %#ok<AGROW> 
        end
        ei(nlinks+1:nlinks+curused) = ind1;
        ej(nlinks+1:nlinks+curused) = used(1:curused);
        ev(nlinks+1:nlinks+curused) = counts(used(1:curused));
        nlinks = nlinks + curused;
        % reset the counts
        for i=1:curused
            counts(used(i)) = 0;
        end
    end
end
% resize links
ei = ei(1:nlinks);
ej = ej(1:nlinks);
ev = ev(1:nlinks);
G = sparse(ei,ej,ev,bigN,bigN);
        

