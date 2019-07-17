function [ quantized ] = quant_z( matrix, quant_factor, zero_factor )
%Quantize the matrix with a regular net with an extended zero zone
%   Detailed explanation goes here
    
    if zero_factor==1.0
        quantized = quant_r( matrix, quant_factor);
        return;
    end
    
    ind_p = matrix > +zero_factor/quant_factor/2;
    ind_n = matrix < -zero_factor/quant_factor/2;

    quantized = zeros(size(matrix));
    quantized(ind_p) = round(matrix(ind_p)*quant_factor-(zero_factor-1)/2);
    quantized(ind_n) = round(matrix(ind_n)*quant_factor+(zero_factor-1)/2);

end

