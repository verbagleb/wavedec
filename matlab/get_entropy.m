function [ ent_out ] = get_entropy( dwt_ca, N, qf, preshift)
% get total entropy for a vector of n_qf (for each band)
%   Detailed explanation goes here
    
    n=1;
    m=1;
    ent_out = 0;
%     for n=1:N
%         for m=1:N
            nm = (n-1)*N+m;
            dwt_q=round((dwt_ca{m,n}+preshift)*qf(nm))/qf(nm);
            ent_out = ent_out + entropy(dwt_q);
%         end
%     end
        
end

