function [ restored ] = dequant_r( quantized, quant_factor )
%Restore the matrix quantized with a regular net
%   Detailed explanation goes here

    restored = quantized/quant_factor;

end

