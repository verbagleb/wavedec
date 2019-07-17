N_coef =7;
qf = 0.03;

assert(mod(N_coef,2)==1);
N_half = (N_coef-1)/2;
k = -N_half:N_half;
Fsq = 4*pi./(1:N_half).^2.*(-1).^(1:N_half);
Fsq = [flip(Fsq), 2/3*pi^3, Fsq];

dd = 1/qf;
dwt_g = dwt_gathered{1,1}(ind{1,end});
shift_v = dd*(0:200)/201;
sqe=zeros(size(shift_v));
ind_sh = 0;
for shift = shift_v
    ind_sh = ind_sh + 1;
    dwt_q = round((dwt_g-shift)*qf);
    dwt_d = dwt_q/qf+shift;
    dwt_e = dwt_g - dwt_d;
    sqe(ind_sh) = dwt_e(:)'*dwt_e(:);
end
figure(1);
clf
plot(shift_v, sqe)
hold on
% ksdensity(dwt_e(:),'width',.1);
% xlim([-dd/2, dd/2]);

dw = 2*pi*qf;
Fg = sum(exp(-1j*dw*dwt_g(:).*(1:N_half)));
Fg = [conj(flip(Fg)), numel(dwt_g), Fg];

Fa = Fsq.*Fg;

% x = dd*k/N_coef;

sigmasq = Fa*exp(1j*dw*k.'*shift_v);
figure(1);
plot(shift_v,sigmasq/qf/qf/248, '.')

n_alpha = 100;
alpha_v = linspace(0,dd,n_alpha+1)/dd;
alpha_v = alpha_v(1:end-1);
er_v = Fa*exp(1j*2*pi*k.'.*alpha_v);
[M,I]=min(er_v);
d0_opt = dd*alpha_v(I);

dwt_q = round((dwt_g-d0_opt)*qf);
dwt_d = dwt_q/qf+d0_opt;
dwt_e = dwt_g - dwt_d;
plot(d0_opt, dwt_e(:)'*dwt_e(:), 'o')