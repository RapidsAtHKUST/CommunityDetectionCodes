function A = readSMAT(filename)
% readSMAT reads an indexed sparse matrix representation of a matrix
% and creates a MATLAB sparse matrix.
%
%   A = readSMAT(filename)
%   filename - the name of the SMAT file
%   A - the MATLAB sparse matrix
%

% David Gleich
% Copyright, Stanford University, 2005-2010

if (~exist(filename,'file'))
    error('readSMAT:fileNotFound', 'Unable to read file %s', filename);
end

if (exist(strcat(filename, '.info'), 'file'))
    s = load(filename);
    mdata = load(strcat(filename, '.info'));
    ind_i = s(:,1)+1;
    ind_j = s(:,2)+1;
    val = s(:,3);
    A = sparse(ind_i,ind_j,val, mdata(1), mdata(2));
    return;
end;

[pathstr,name,ext] = fileparts(filename);
if ~isempty(strfind(ext,'.gz'))
    [m n i j v] = readSMATGZ(realpath(filename));

    A = sparse(i,j,v,m,n);
    return;
end;


s = load(filename,'-ascii');
m = s(1,1);
n = s(1,2);
try
    ind_i = s(2:length(s),1)+1;
    ind_j = s(2:length(s),2)+1;
    val = s(2:length(s),3);
    clear s;
    A = sparse(ind_i,ind_j,val, m, n);
catch
    fprintf('... trying block read ...\n');
    blocksize = 1000000;
    curpos = 2;
    blocknum = 1;
    nzleft = s(1,3);
    A = sparse(m,n);
    while (nzleft > 0)
        curblock = min(nzleft, blocksize);
        fprintf('block %i (%i - %i)\n', blocknum, curpos-1, curpos+curblock-2);
        curpart = curpos:(curpos+curblock-1);
        ind_i = s(curpart,1)+1;
        ind_j = s(curpart,2)+1;
        val = s(curpart,3);
        A = A + sparse(ind_i, ind_j, val, m, n);
        
        nzleft = nzleft - curblock;
        curpos = curpos + curblock;
        blocknum = blocknum + 1;
    end
    
end


function S = merge_structs(A, B)
% MERGE_STRUCTS Merge two structures.
%
% S = merge_structs(A, B) makes the structure S have all the fields from A
% and B.  Conflicts are resolved by using the value in A.
%

%
% merge_structs.m
% David Gleich
%
% Revision 1.00
% 19 Octoboer 2005
%

S = A;

fn = fieldnames(B);

for ii = 1:length(fn)
    if ~isfield(A, fn{ii})
        S.(fn{ii}) = B.(fn{ii});
    end
end
