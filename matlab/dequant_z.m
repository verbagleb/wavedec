function [ restored ] = dequant_z( quantized, quant_factor, zero_factor )
%Restore the matrix quantized with a regular net with an extended zero zone
%   Detailed explanation goes here

    if zero_factor==1.0
        restored = dequant_r( quantized, quant_factor);
        return;
    end
    
    ind_p = quantized > 0;
    ind_n = quantized < 0;
    
    restored = zeros(size(quantized));
    restored(ind_p) = (quantized(ind_p)+(zero_factor-1)/2)/quant_factor;
    restored(ind_n) = (quantized(ind_n)-(zero_factor-1)/2)/quant_factor;

end

