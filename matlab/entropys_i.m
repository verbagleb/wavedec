function [ out ] = entropys_i( qf_i, ind_i, n_it_qf, n_it_nz, entropys_m, entropys_12, entropys_34, entropys_5 )
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here

    [sub12, sub34, sub5] = ind2sub([n_it_qf*n_it_nz n_it_qf*n_it_nz n_it_qf],ind_i);
    out = entropys_m(qf_i) + entropys_12(qf_i,sub12) + entropys_34(qf_i,sub34) + entropys_5(qf_i, sub5);

end

