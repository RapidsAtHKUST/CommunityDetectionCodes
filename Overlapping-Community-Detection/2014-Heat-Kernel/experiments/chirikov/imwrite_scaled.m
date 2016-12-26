function imwrite_scaled(imdata,cmap,filename)


m = min(imdata(:));
M = max(imdata(:));

nl = size(cmap,1);

imwrite(nl*(imdata-m)/(M-m),cmap,filename);
