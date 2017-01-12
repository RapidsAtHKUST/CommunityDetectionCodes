function setdata = add_extra_community_data(C,setdata);
% ADD_EXTRA_COMMUNITY_DATA Compute easy-to-derive community data

n = size(C,1);

setdata.max_cond = max(setdata.cond);
setdata.min_cond = min(setdata.cond);
setdata.covered_verts = nnz(sum(C,2));
setdata.covered_ratio = setdata.covered_verts/n;
setdata.overlapped_verts = sum(sum(C,2)>1);
%setdata.overlap = setdata.overlapped_verts/n;
setdata.overlap = full(sum(C,2));
setdata.overlapped_ratio = setdata.overlapped_verts/n;
setdata.sum_cut = sum(setdata.cut);
setdata.density = 0.5*(setdata.vol - setdata.cut)./setdata.size;

