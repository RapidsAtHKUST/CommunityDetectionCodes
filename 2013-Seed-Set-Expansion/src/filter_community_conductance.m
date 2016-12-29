function [H,setdata] = filter_community_conductance(H,setdata,maxcond)
% FILTER_COMMUNITY_CONDUCTANCE
% 
% [H,setdata] = filter_community_conductance(H,setdata,maxcond)
%   recomputes statistics for a subset of the communities with maximum
%   conductance maxcond
%
%

valid = setdata.cond < maxcond;
H = H(:,valid);
setdata.cond = setdata.cond(valid);
setdata.cut = setdata.cut(valid);
setdata.vol = setdata.vol(valid);
setdata.size = setdata.size(valid);
if isfield(setdata,'seeds')
    setdata.seeds = setdata.seeds(valid);
end
setdata = add_extra_community_data(H,setdata);
