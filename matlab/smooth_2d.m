function [ sm_field ] = smooth_2d( field, width )
% Smooth the distrubution
%   Detailed explanation goes here

    assert(mod(width,2)==1);
    field = field([ones(1,(width-1)/2), 1:end, ones(1,(width-1)/2)*end], [ones(1,(width-1)/2), 1:end, ones(1,(width-1)/2)*end]);
    sm_field = sqrt(conv2(field.^2,ones(width)/width/width));
    sm_field = sm_field (1+(width-1):end-(width-1),1+(width-1):end-(width-1));

end

