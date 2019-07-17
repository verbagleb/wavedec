function [ psnr_out ] = get_psnr( dwt_ca, qf, inv_filter, sym_coef, preshift, shift, h_or, w_or )
% get total entropy for a vector of n_qf (for each band)
%   Detailed explanation goes here
    N = size(inv_filter, 1);
    error_res = zeros(h_or,w_or);
    n=1;
    m=1;
%     for n=1:N
%         for m=1:N
            dwt_ca{m,n} = dwt_ca{m,n}+preshift;
            nm = (n-1)*N+m;
            dwt_crop = repmat({zeros(size(dwt_ca{1,1}))},N,N);
            dwt_crop{m,n}=dwt_ca{m,n};
            or_part=idwt(dwt_crop, inv_filter, shift, sym_coef, h_or, w_or);

            dwt_q=round(dwt_ca{m,n}*qf(nm))/qf(nm);
            dwt_crop = repmat({zeros(size(dwt_q))},N,N);
            dwt_crop{m,n}=dwt_q;
            re_part=idwt(dwt_crop, inv_filter, shift, sym_coef, h_or, w_or);
            error_res = error_res + or_part-re_part;
%         end
%     end
    
    psnr_out = 20*log10(255/rms(error_res(:)));

end

