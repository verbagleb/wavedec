% or_part = cell(N,N);
% for n=1:N
%     for m=1:N
%         dwt_crop = repmat({zeros(size(dwt_ca{1,1}))},N,N);
%         dwt_crop{m,n}=dwt_ca{m,n};
%         or_part{m,n}=idwt(dwt_crop, inv_filter, shift, sym_coef, h_or, w_or);
%     end
% end
% 
% N_q = 50;
% qf_list = linspace(0.01, 0.1, N_q);
% ent = zeros(N,N,N_q);
% re_part = cell(N,N,N_q);
% for n_q = 1:N_q
%     qf = qf_list(n_q);
%     dwt_q = dwt_ca;
%     for n=1:N
%         for m=1:N
%             dwt_q{m,n}=round(dwt_ca{m,n}*qf)/qf;
%             ent(m,n,n_q) = entropy(dwt_q{m,n});
%             dwt_crop = repmat({zeros(size(dwt_q{1,1}))},N,N);
%             dwt_crop{m,n}=dwt_q{m,n};
%             re_part{m,n,n_q}=idwt(dwt_crop, inv_filter, shift, sym_coef, h_or, w_or);
%             error_part{m,n,n_q}=or_part{m,n}-re_part{m,n,n_q};
%         end
%     end
% end
% 
% 
% xd = ent(m,n,:);
% xd = xd(:);
% yd = zeros(1,N_q);
% for n_q=1:N_q
%     yd(n_q) = psnr(or_part{m,n},re_part{m,n,n_q},255);
% end
% plot(xd,yd);

f_ent=@(qf)get_entropy(dwt_ca, 3, qf, -shift*delta);
f_psnr=@(qf)get_psnr( dwt_ca, qf, inv_filter, sym_coef, -shift*delta, shift, h_or, w_or );
f_mpsnr=@(qf)-f_psnr(qf);
f_opt = @(qf)30+dydx*f_ent(qf)-f_psnr(qf);
qf_0 = 0.08;
qf_l = qf_0/5;
qf_u = qf_0*5;
qf_0_v=qf_0*ones(1,9);%+0.01*randn(1,9);
% ent_val = f_ent(qf_0*ones(1,9));
% mpsnr_val = f_mpsnr(qf_0_v);
% f_c = @(qf)deal([],f_ent(qf)-ent_val);
% f_c = @(qf)deal([],(f_mpsnr(qf)-mpsnr_val)*100);
% f_c = @(qf)deal([],qf*qf.'-qf_0^2*9);
options = optimoptions('fmincon','Display','iter-detailed');
% [qf_opt, f_opt] = fmincon(f_mpsnr, qf_0_v,[],[],[],[],qf_l*ones(1,9),qf_u*ones(1,9),f_c,options);
% qf_opt = fmincon(f_opt, qf_0_v,[],[],[],[],qf_l*ones(1,9),qf_u*ones(1,9),[],options);


state = randperm(n); % задаём начальное состояние, как случайный маршрут
% Функция randperm(n) - генерирует случайныую последовательность из целых чисел от 1 до n

qf = qf_0_v;
currentEnergy = f_opt(qf);
initialTemperature = currentEnergy / 100;
T = initialTemperature;
% endTemperature = 
fprintf("%d: ",0);
fprintf("%f: ", currentEnergy);
fprintf("%f ", qf);
fprintf("\n");

for i = 800:1000  %на всякий случай ограничеваем количество итераций
% может быть полезно при тестировании сложных функций изменения температуры T       

    n_c = 5;
    while n_c==5
        n_c = randi(9);
    end
    qf_t = qf;
    qf_t(n_c)=qf(n_c)+(rand-.5)*qf_0/10;
    qf_t = min(qf_t, qf_u);
    qf_t = max(qf_t, qf_l);
    candidateEnergy = f_opt(qf_t); % вычисляем его энергию

    if(candidateEnergy < currentEnergy) % если кандидат обладает меньшей энергией
        currentEnergy = candidateEnergy; % то оно становится текущим состоянием
        qf = qf_t;
        qf_best = qf;
    else
        p = exp(-(candidateEnergy-currentEnergy)/T); % иначе, считаем вероятность
        if (rand<=p) % и смотрим, осуществится ли переход
            currentEnergy = candidateEnergy;
            qf = qf_t;
        end
    end
    
    if currentEnergy == candidateEnergy
        fprintf("%d: ",i);
        fprintf("%f: ", currentEnergy);
        fprintf("%f ", qf);
        fprintf("\n");
    end
    
    T = initialTemperature / i; 
   
%     if(T <= endTemperature) % условие выхода
%         break;
%     end
end