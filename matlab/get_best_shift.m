function [ d0_opt ] = get_best_shift( dwt_g, qf, N_coef, n_alpha )
%Find the optimal shift for minimal mean square error
%   Detailed explanation goes here

    assert(mod(N_coef,2)==1);
    N_half = (N_coef-1)/2;
    k = -N_half:N_half;
    Fsq = 4*pi./(1:N_half).^2.*(-1).^(1:N_half);
    Fsq = [flip(Fsq), 2/3*pi^3, Fsq];

    dd = 1/qf;
    dwt_g = dwt_g(:);
    dw = 2*pi*qf;
    Fg = sum(exp(-1j*dw*dwt_g(:).*(1:N_half)),1);
    Fg = [conj(flip(Fg)), numel(dwt_g), Fg];

    Fa = Fsq.*Fg;

    alpha_v = linspace(0,dd,n_alpha+1)/dd;
    alpha_v = alpha_v(1:end-1);
    er_v = Fa*exp(1j*2*pi*k.'.*alpha_v);
    [M,I]=min(er_v);
    d0_opt = dd*alpha_v(I);

end

