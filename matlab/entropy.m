function [ ent ] = entropy( array )
%Entropy of an arrray
%   Detailed explanation goes here
    arraynan = isnan(array);
    if sum(arraynan(:))
        warning('entropy: nan values are omitted');
    end
    array = array(~arraynan).';
    [C, ia, ic]=unique(array);
    p = (sum(ic==1:numel(C),1)/numel(array));
    ent = -p*log2(p.')*numel(array);
end

