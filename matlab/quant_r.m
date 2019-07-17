function [ quantized ] = quant_r( matrix, quant_factor )
%Quantize the matrix with a regular net
%   Detailed explanation goes here

    quantized = round(matrix*quant_factor);

end