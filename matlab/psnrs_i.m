function [ out ] = psnrs_i( qf_i, ind_i, n_it_qf, n_it_nz, ssq_m, ssq_12, ssq_34, ssq_5 )
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here

    [sub12, sub34, sub5] = ind2sub([n_it_qf*n_it_nz n_it_qf*n_it_nz n_it_qf],ind_i);
    ssq_i = ssq_m(qf_i) + ssq_12(qf_i,sub12) + ssq_34(qf_i,sub34) + ssq_5(qf_i, sub5);
    out = 20*log10(255./sqrt(ssq_i));
end

