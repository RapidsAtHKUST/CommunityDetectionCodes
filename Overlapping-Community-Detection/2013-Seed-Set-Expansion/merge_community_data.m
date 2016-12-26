function [H,setdata] = merge_community_data(H1,setdata1,H2,setdata2)

H = [H1 H2];

setdata.cond = [setdata1.cond; setdata2.cond];
setdata.vol = [setdata1.vol; setdata2.vol];
setdata.cut = [setdata1.cut; setdata2.cut];
setdata.size = [setdata1.size; setdata2.size];
setdata.seeds = [setdata1.seeds; setdata2.seeds];
setdata.expand = [setdata1.expand; setdata2.expand];
setdata = add_extra_community_data(H,setdata);