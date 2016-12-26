function set_figure_size(h,sz)
% SET_FIGURE_SIZE Set the output size for a matlab figure.
%
% A frequent problem with Matlab figures is that they are designed to be
% output at 8-by-6 inches.  However, most figures are included in papers at
% much smaller sizes.  The function adjusts the paper position property of
% a figure based on many different arguments:
%
% set_figure_size('tiny') % set figure size to 1.25 in height
% set_figure_size('small') % set figure size to 2.5 in height
% set_figure_size('medium') % set figure size to 4 in height
% set_figure_size('big'); % Matlab's default (set fig size to 6 in height)
%
% set_figure_size([w,h]); % set figure size to w-by-h inches
%   This call respects the figures PaperUnits, so w-by-h would be
%   centimeters if PaperUnits is 'centimeters'
%
% set_figure_size(h,...); % set the figure size of the figure with handle h
%   this form can work with any of the other 
%

% David F. Gleich

% History
% -------
% :2010-02-12: Initial writing

% set up the single call case
if nargin==1
    sz = h;
    h = gcf;
end

pp = get(h,'PaperPosition');

if ischar(sz)
    switch lower(sz)
        case 'tiny'
            pp(3) = 1.5;
            pp(4) = 1.25;
        
        case 'small'
            pp(3) = 3;
            pp(4) = 2.5;
            
        case 'medium'
            pp(3) = 5;
            pp(4) = 4;
            
        case 'big'
            pp(3) = 8;
            pp(4) = 6;
            
        otherwise
            error('set_figure_size:unknownSize',...
                'the string ''%s'' is not a recognized size', sz);
    end
elseif length(sz)==2
    pp(3) = sz(1);
    pp(4) = sz(2);
else
    error('set_figure_size:invalidType',...
        'the figure size should be a string or a two-element array');
end

set(h,'PaperPosition',pp);