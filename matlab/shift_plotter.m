% filter "long"
delta = 368.332995/128; %the response of LL to shift

for shift=[16 107 95 100 30 230]
    f_ent=@(qf)get_entropy(dwt_ca, 3, qf, -shift*delta);
    f_psnr=@(qf)get_psnr( dwt_ca, qf, inv_filter, sym_coef, -shift*delta, shift, h_or, w_or );

    np=200;
    qf=logspace(log10(0.1), log10(0.01),np);
    x_LL = zeros(1,np);
    y_LL = zeros(1,np);
    for i=1:numel(qf)
        x_LL(i)=f_ent(qf(i)*ones(1,9));
        y_LL(i)=f_psnr(qf(i)*ones(1,9));
    end
    hold on;
    plot(x_LL, y_LL, 'DisplayName', num2str(shift))
    evalc(['x_LL_' num2str(shift) '= x_LL']);
    evalc(['y_LL_' num2str(shift) '= y_LL']);
end